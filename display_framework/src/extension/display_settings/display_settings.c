#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <drm_fourcc.h>
#include <sys/mman.h>
#include <pthread.h>
#include "xf86drm.h"
#include "xf86drmMode.h"
#include <signal.h>
#include <drm.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/string.h>
#include "display_settings.h"
#include "libdrm_meson/meson_drm_settings.h"
#include "libdrm_meson/meson_drm_event.h"
#define DEFAULT_CARD "/dev/dri/card0"
#include "libdrm_meson/meson_drm_log.h"
#include "linux/amlogic/drm/meson_drm.h"

int display_meson_get_open() {
    int fd = -1;
    const char *card;
    int ret = -1;
    card= getenv("WESTEROS_DRM_CARD");
    if ( !card) {
        card = DEFAULT_CARD;
    }
    fd = open(card, O_RDONLY|O_CLOEXEC);
    if (fd < 0)
        ERROR("%s %d meson_open_drm drm card:%s open fail", __FUNCTION__,__LINE__,card);
    else
        drmDropMaster(fd);
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret < 0)
        ERROR("%s %d Unable to set DRM atomic capability", __FUNCTION__,__LINE__);
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    if (ret < 0)
        ERROR("%s %d Unable to set UNIVERSAL_PLANES\n", __FUNCTION__,__LINE__);
    return fd;
}

int display_meson_set_open() {
    int fd = -1;
    int ret = -1;
    fd = open(DEFAULT_CARD, O_RDWR|O_CLOEXEC);
    if (fd < 0) {
        ERROR("%s %d failed to open device %s",  __FUNCTION__,__LINE__,strerror(errno));
    }
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret < 0) {
        ERROR("%s %d no atomic modesetting support", __FUNCTION__,__LINE__);
    }
    return fd;
}

void display_meson_close(int fd) {
    if (fd >= 0)
        close(fd);
}

bool registerMesonDisplayEventCallback(mesonDisplayEventCallback cb) { //register callback
    return RegisterDisplayEventCallback(cb);
}

void startMesonDisplayUeventMonitor() {
    startDisplayUeventMonitor();
}

void stopMesonDisplayUeventMonitor() {
    stopDisplayUeventMonitor();
}

int setDisplayHDCPEnable(bool enable, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
    switch (enable)
    {
        case 2:
            str = "Enabled";
            break;
        case 1:
            str = "Desired";
            break;
        default :
            str = "Undesired";
            break;
    }
    DEBUG("%s %d set hdcp enable %s",__FUNCTION__,__LINE__,str);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setHDCPEnable(fd, req, enable, connType);
    if (res == -1) {
        ERROR("%s %d set hdcp enable fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__,ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

ENUM_MESON_HDCPAUTH_STATUS getDisplayHdcpAuthStatus(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_MESON_HDCPAUTH_STATUS hdcpAuthStatus = MESON_AUTH_STATUS_FAIL;
    int ret = meson_drm_getHdcpAuthStatus(fd, connType );
    if (ret == 0) {
        hdcpAuthStatus = MESON_AUTH_STATUS_FAIL;
    } else if(ret == 1) {
        hdcpAuthStatus = MESON_AUTH_STATUS_SUCCESS;
    }
    display_meson_close(fd);
    DEBUG(" %s %d get hdcp auth status: %d",__FUNCTION__,__LINE__,hdcpAuthStatus);
    return hdcpAuthStatus;
}

void getDisplayEDIDData(MESON_CONNECTOR_TYPE connType, int * data_Len, char **data ) {
    int fd = 0;
    fd = display_meson_get_open();
    if (data_Len == NULL || data == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        return;
    }
    meson_drm_getEDIDData(fd, connType, data_Len, data);
    DEBUG_EDID("\n");
    DEBUG("%s %d get data_Len: %d",__FUNCTION__,__LINE__, (*data_Len));
    for (int i = 0; i < (*data_Len); i++) {
        if (i % 16 == 0)
            DEBUG_EDID("\n\t\t\t");
            if (*data)
            DEBUG_EDID("%.2hhx", (*data)[i]);
    }
    display_meson_close(fd);
}

int setDisplayAVMute(int mute, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG(" %s %d set mute value %d",__FUNCTION__,__LINE__,mute);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setAVMute(fd, req, mute, connType);
    if (res == -1) {
        ERROR("%s %d set avmute fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__,ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_MESON_COLOR_SPACE colorSpace, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
    switch (colorSpace)
    {
        case 0:
            str = "MESON_COLOR_SPACE_RGB";
            break;
        case 1:
            str = "MESON_COLOR_SPACE_YCBCR422";
            break;
        case 2:
            str = "MESON_COLOR_SPACE_YCBCR444";
            break;
        case 3:
            str = "MESON_COLOR_SPACE_YCBCR420";
            break;
        default:
            str = "MESON_COLOR_SPACE_RESERVED";
            break;
    }
    DEBUG(" %s %d set colorDepth: %d colorSpace: %s",__FUNCTION__,__LINE__,colorDepth,str);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        ERROR(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setColorDepth(fd, req, colorDepth, connType);
    res = meson_drm_setColorSpace(fd, req, colorSpace, connType);
    if (res == -1) {
        ERROR("%s %d set <colorDepth,colorSpace> fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__,ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

ENUM_MESON_COLOR_SPACE getDisplayColorSpace(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_MESON_COLOR_SPACE colorSpace = MESON_COLOR_SPACE_RESERVED;
    colorSpace = meson_drm_getColorSpace(fd, connType);
    display_meson_close(fd);
    switch (colorSpace)
    {
        case 0:
            str = "MESON_COLOR_SPACE_RGB";
            break;
        case 1:
            str = "MESON_COLOR_SPACE_YCBCR422";
            break;
        case 2:
            str = "MESON_COLOR_SPACE_YCBCR444";
            break;
        case 3:
            str = "MESON_COLOR_SPACE_YCBCR420";
            break;
        default:
            str = "MESON_COLOR_SPACE_RESERVED";
            break;
    }
    DEBUG("%s %d get colorSpace: %s",__FUNCTION__,__LINE__,str);
    return colorSpace;
}

uint32_t getDisplayColorDepth(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    int value = 0;
    value = meson_drm_getColorDepth(fd, connType);
    display_meson_close(fd);
    DEBUG("%s %d get ColorDepth: %d",__FUNCTION__,__LINE__,value);
    return value;
}

ENUM_MESON_CONN_CONNECTION getDisplayConnectionStatus(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_MESON_CONN_CONNECTION ConnStatus = MESON_UNKNOWNCONNECTION;
    ConnStatus = meson_drm_getConnectionStatus(fd, connType);
    display_meson_close(fd);
    switch (ConnStatus)
    {
        case 0:
            str = "MESON_DISCONNECTED";
            break;
        case 1:
            str = "MESON_CONNECTED";
            break;
        default:
            str = "MESON_UNKNOWNCONNECTION";
            break;
    }
    DEBUG("%s %d get connection status: %s",__FUNCTION__,__LINE__,str);
    return ConnStatus;
}

ENUM_MESON_HDCP_VERSION getDisplayHdcpVersion(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_MESON_HDCP_VERSION hdcpVersion = MESON_HDCP_RESERVED;
    hdcpVersion = meson_drm_getHdcpVersion(fd, connType);
    display_meson_close(fd);
    switch (hdcpVersion)
    {
        case 0:
            str = "MESON_HDCP_14";
            break;
        case 1:
            str = "MESON_HDCP_22";
            break;
        default:
            str = "MESON_HDCP_RESERVED";
            break;
    }
    DEBUG("%s %d get hdcp version: %s",__FUNCTION__,__LINE__,str);
    return hdcpVersion;
}

int getDisplayMode(DisplayMode* modeInfo, MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    int ret = -1;
    if (modeInfo == NULL) {
        ERROR("%s %d modeInfo == NULL return",__FUNCTION__,__LINE__);
        return ret;
    }
    ret = meson_drm_getModeInfo(fd, connType, modeInfo);
    if (ret == -1) {
        ERROR("%s %d get modeInfo fail",__FUNCTION__,__LINE__);
    }
    display_meson_close(fd);
    DEBUG("%s %d modeInfo: %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h, (modeInfo->interlace == 0 ?"p":"i"), modeInfo->vrefresh);
    return ret;
}

ENUM_MESON_HDR_POLICY getDisplayHDRPolicy(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_MESON_HDR_POLICY hdrPolicy = MESON_HDR_POLICY_FOLLOW_SINK;
    hdrPolicy = meson_drm_getHDRPolicy(fd, connType);
    display_meson_close(fd);
    DEBUG("%s %d get hdrPolicy type %s",__FUNCTION__,__LINE__,hdrPolicy == 0? "MESON_HDR_POLICY_FOLLOW_SINK":"MESON_HDR_POLICY_FOLLOW_SOURCE");
    return hdrPolicy;
}

int setDisplayHDRPolicy(ENUM_MESON_HDR_POLICY hdrPolicy, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set hdrPolicy type %s",__FUNCTION__,__LINE__,hdrPolicy == 0? "MESON_HDR_POLICY_FOLLOW_SINK":"MESON_HDR_POLICY_FOLLOW_SOURCE");
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setHDRPolicy(fd, req, hdrPolicy, connType);
    if (res == -1) {
        ERROR("%s %d set hdr policy fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

int getDisplayModesList(DisplayMode** modeInfo, int* modeCount,MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    int ret = -1;
    fd = display_meson_get_open();
    ret = meson_drm_getsupportedModesList(fd, modeInfo, modeCount,connType);
    if (ret == -1) {
        ERROR("%s %d get supported modeslist failed: ret %d errno %d",__FUNCTION__,__LINE__, ret, errno );
    }
    display_meson_close(fd);
    DEBUG("%s %d mode count: %d",__FUNCTION__,__LINE__,(*modeCount));
    for (int i=0; i < (*modeCount); i++) {
        DEBUG_EDID(" %s %dx%d%s%dhz\n", (*modeInfo)[i].name, (*modeInfo)[i].w, (*modeInfo)[i].h, ((*modeInfo)[i].interlace == 0? "p":"i"), (*modeInfo)[i].vrefresh);
    }
    return ret;
}

int getDisplayPreferMode( DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType) {
    int ret = -1;
    ret = meson_drm_getPreferredMode(modeInfo,connType);
    if (ret == -1) {
        ERROR("%s %d get preferred modes failed: ret %d errno %d",__FUNCTION__,__LINE__, ret, errno );
    }
    DEBUG("%s %d get preferred mode %s %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->name, modeInfo->w, modeInfo->h, (modeInfo->interlace == 0? "p":"i"),modeInfo->vrefresh);
    return ret;
}

int setDisplayHDCPContentType(ENUM_MESON_HDCP_Content_Type HDCPType, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set hdcp content type %s",__FUNCTION__,__LINE__,HDCPType == 0? "MESON_HDCP_Type0":"MESON_HDCP_Type1");
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setHDCPContentType(fd, req, HDCPType, connType);
    if (res == -1) {
        ERROR("%s %d set hdcp content type fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

ENUM_MESON_HDCP_Content_Type getDisplayHDCPContentType(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_MESON_HDCP_Content_Type ContentType = MESON_HDCP_Type_RESERVED;
    ContentType = meson_drm_getHDCPContentType(fd, connType);
    display_meson_close(fd);
    switch (ContentType)
    {
        case 0:
            str = "MESON_HDCP_Type0";
            break;
        case 1:
            str = "MESON_HDCP_Type1";
            break;
        default:
            str = "MESON_HDCP_Type_RESERVED";
            break;
    }
    DEBUG("%s %d get hdcp content type: %s",__FUNCTION__,__LINE__,str);
    return ContentType;
}

ENUM_MESON_Content_Type getDisplayContentType( MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_MESON_Content_Type ContentType = MESON_Content_Type_RESERVED;
    ContentType = meson_drm_getContentType(fd, connType);
    display_meson_close(fd);
    switch (ContentType)
    {
        case 0:
            str = "MESON_Content_Type_Data";
            break;
        case 1:
            str = "MESON_Content_Type_Graphics";
            break;
         case 2:
            str = "MESON_Content_Type_Photo";
            break;
        case 3:
            str = "MESON_Content_Type_Cinema";
            break;
         case 4:
            str = "MESON_Content_Type_Game";
            break;
        default:
            str = "MESON_HDCP_Type_RESERVED";
            break;
    }
    DEBUG("%s %d get content type: %s",__FUNCTION__,__LINE__,str);
    return ContentType;
}

int setDisplayDvEnable(int dvEnable, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set dvEnable value %d",__FUNCTION__,__LINE__,dvEnable);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setDvEnable(fd, req, dvEnable, connType);
    if (res == -1) {
        ERROR("%s %d set DvEnable fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

int getDisplayDvEnable(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    int ret = -1;
    fd = display_meson_get_open();
    ret = meson_drm_getDvEnable(fd, connType );
    if (ret == -1) {
        ERROR("%s %d get DvEnable fail",__FUNCTION__,__LINE__);
    }
    display_meson_close(fd);
    DEBUG("%s %d get DvEnable value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int setDisplayActive(int active, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set active value %d",__FUNCTION__,__LINE__,active);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setActive(fd, req, active, connType);
    if (res == -1) {
        ERROR("%s %d set active fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

int getDisplayActive(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    int ret = -1;
    if (fd < 0) {
       ERROR("%s %d fd < 0",__FUNCTION__,__LINE__);
       return ret;
    }
    ret = meson_drm_getActive(fd, connType );
    if (ret == -1) {
        ERROR("%s %d get active fail",__FUNCTION__,__LINE__);
    }
    display_meson_close(fd);
    DEBUG("%s %d get active value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int setDisplayVrrEnabled(int VrrEnable, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set VrrEnable value %d",__FUNCTION__,__LINE__,VrrEnable);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setVrrEnabled(fd, req, VrrEnable, connType);
    if (res == -1) {
        ERROR("%s %d set VrrEnabled fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

int getDisplayVrrEnabled(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    int ret = 0;
    fd = display_meson_get_open();
    ret = meson_drm_getVrrEnabled(fd, connType );
    display_meson_close(fd);
    DEBUG("%s %d get VrrEnabled value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int getDisplayAVMute(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    int ret = 0;
    ret = meson_drm_getAVMute(fd, connType );
    display_meson_close(fd);
    DEBUG("%s %d get AVMute value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int setDisplayMode(DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int resNum = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set modeInfo %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h, (modeInfo->interlace == 0? "p":"i") , modeInfo->vrefresh);
    if (modeInfo == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        return ret;
    }
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_changeMode(fd, req, modeInfo, connType);
    if (res == -1) {
        ERROR("%s %d connector type does not exist ",__FUNCTION__,__LINE__);
        if (connType == MESON_CONNECTOR_DUMMY) {
            ERROR("%s %d no dummy type connector ,set hdmi dummy_l mode",__FUNCTION__,__LINE__);
            modeInfo->interlace = 0;
            modeInfo->w  = 720;
            modeInfo->h = 480;
            modeInfo->vrefresh = 50;
            resNum = meson_drm_changeMode(fd, req, modeInfo, MESON_CONNECTOR_HDMIA);
            if (resNum == -1) {
                ERROR("%s %d set hdmi dummy_l mode fail",__FUNCTION__,__LINE__);
                goto out;
           }
       }
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    display_meson_close(fd);
    return  ret;
}

ENUM_MESON_HDR_MODE getDisplayHdrStatus(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_MESON_HDR_MODE hdrMode = MESON_SDR;
    hdrMode = meson_drm_getHdrStatus(fd, connType);
    display_meson_close(fd);
    switch (hdrMode)
    {
        case 0:
            str = "MESON_HDR10PLUS";
            break;
        case 1:
            str = "MESON_DOLBYVISION_STD";
            break;
        case 2:
            str = "MESON_DOLBYVISION_LL";
            break;
        case 3:
            str = "MESON_HDR10_ST2084";
            break;
        case 4:
            str = "MESON_HDR10_TRADITIONAL";
            break;
        case 5:
            str = "MESON_HDR_HLG";
            break;
        case 6:
            str = "MESON_SDR";
            break;
        default:
            str = "MESON_SDR";
            break;
    }
    DEBUG("%s %d get hdr status: %s",__FUNCTION__,__LINE__,str);
    return hdrMode;
}

int setDisplayAutoMode(MESON_CONNECTOR_TYPE connType) {
    int count = 0;
    int i = 0;
    int miBest= -1;
    int area, largestArea = 0;
    int refresh = 0;
    int ret = -1;
    int maxArea = 0;
    DisplayMode* modes = NULL;
    drmModeAtomicReq *req = NULL;
    int fd = 0;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    ret = meson_drm_getsupportedModesList(fd, &modes, &count, connType);
    if (ret) {
        ERROR("%s %d get supported modeslist fail",ret, strerror(errno));
        goto out;
    }
    const char *env= getenv("MESON_DISPLAY_MAX_MODE");
    if ( env )
    {
       int w= 0, h= 0;
       if ( sscanf( env, "%dx%d", &w, &h ) == 2 )
       {
          DEBUG("max mode: %dx%d", w, h);
          maxArea= w*h;
       }
    }
    for (i = 0; i < count; i++)
    {
        area= modes[i].w * modes[i].h;
        if ( (area > largestArea) && ((maxArea == 0) || (area <= maxArea)))
        {
            largestArea= area;
            miBest= i;
            refresh= modes[i].vrefresh;
        }
        else if ( area == largestArea )
        {
            if ( modes[i].vrefresh > refresh )
            {
                miBest= i;
                refresh= modes[i].vrefresh;
             }
        }
    }
    DEBUG("%s %d get largest modeInfo %dx%d%s%dhz", __FUNCTION__,__LINE__, modes[miBest].w, modes[miBest].h,(modes[miBest].interlace == 0? "p":"i"), modes[miBest].vrefresh);
    ret = meson_drm_changeMode(fd, req, &modes[miBest], connType);
    if (ret) {
        ERROR("%s %d change auto mode fail",__FUNCTION__,__LINE__);
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("%s %d drmModeAtomicCommit failed: ret %d errno %d", __FUNCTION__,__LINE__, ret, errno );
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    if (modes) {
        free(modes);
    }
    display_meson_close(fd);
    return ret;
}
bool modeAttrSupportedCheck(char* modeName, ENUM_MESON_COLOR_SPACE colorSpace,
                            uint32_t colorDepth, MESON_CONNECTOR_TYPE connType )
{
    bool ret = false;
    char attr[32] = {'\0'};
    char color[5] = {'\0'};
    int fd = -1;
    if (modeName == NULL || colorSpace > MESON_COLOR_SPACE_YCBCR420 ||
        ( connType != MESON_CONNECTOR_HDMIA && connType != MESON_CONNECTOR_HDMIB) ) {
        ERROR("%s %d invalid parameters ", __FUNCTION__,__LINE__);
        return ret;
    }
    switch ( colorSpace ) {
        case MESON_COLOR_SPACE_RGB:
            sprintf(color, "rgb");
            break;
        case MESON_COLOR_SPACE_YCBCR420:
            sprintf(color, "420");
            break;
        case MESON_COLOR_SPACE_YCBCR422:
            sprintf(color, "422");
            break;
        case MESON_COLOR_SPACE_YCBCR444:
            sprintf(color, "444");
            break;
        default:
            sprintf(color, "fail");
            break;
    }
    snprintf(attr, sizeof(attr)-1, "%s,%dbit", color, colorDepth);
    DEBUG("%s %d mode:%s attr:%s", __FUNCTION__,__LINE__,modeName, attr);
    struct drm_mode_test_attr args;
    memset(&args, 0, sizeof(struct drm_mode_test_attr));
    strcpy(args.modename, modeName);
    strcpy(args.attr, attr);
    fd = display_meson_get_open();
    DEBUG("%s %d drm fd:%d, args (%s %s)",__FUNCTION__,__LINE__, fd, args.modename, args.attr);
    if (ioctl(fd, DRM_IOCTL_MESON_TESTATTR, &args) == 0) {
        if (args.valid == 1)
            ret = true;
    } else {
        ERROR("%s %d ioctl fail ", __FUNCTION__,__LINE__);
    }
    display_meson_close(fd);
    return ret;
}


