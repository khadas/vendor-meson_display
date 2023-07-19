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
        ERROR("meson_open_drm drm card:%s open fail",card);
    else
        drmDropMaster(fd);
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret < 0)
        INFO("Unable to set DRM atomic capability");
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_UNIVERSAL_PLANES, 1);
    if (ret < 0)
        INFO("Unable to set UNIVERSAL_PLANES\n");
    return fd;
}

int display_meson_set_open() {
    int fd = -1;
    int ret = -1;
    fd = open(DEFAULT_CARD, O_RDWR|O_CLOEXEC);
    if (fd < 0) {
        ERROR("failed to open device %s", strerror(errno));
    }
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret < 0) {
        INFO("no atomic modesetting support");
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
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setHDCPEnable(fd, req, enable, connType);
    if (res == -1) {
        ERROR("setHDCPEnableFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        DEBUG("failed to set setHDCPEnable: %d-%s", ret, strerror(errno));
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
    display_meson_close(fd);
}

int setDisplayAVMute(int mute, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setAVMute(fd, req, mute, connType);
    if (res == -1) {
        ERROR("setAVMuteFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set AVMute: %d-%s", ret, strerror(errno));
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
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setColorDepth(fd, req, colorDepth, connType);
    res = meson_drm_setColorSpace(fd, req, colorSpace, connType);
    if (res == -1) {
        ERROR("set Fail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set: %d-%s", ret, strerror(errno));
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
    fd = display_meson_get_open();
    ENUM_MESON_COLOR_SPACE colorSpace = MESON_COLOR_SPACE_RESERVED;
    colorSpace = meson_drm_getColorSpace(fd, connType);
    display_meson_close(fd);
    return colorSpace;
}

uint32_t getDisplayColorDepth(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    int value = 0;
    value = meson_drm_getColorDepth(fd, connType);
    display_meson_close(fd);
    return value;
}

ENUM_MESON_CONN_CONNECTION getDisplayConnectionStatus(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_MESON_CONN_CONNECTION ConnStatus = MESON_UNKNOWNCONNECTION;
    ConnStatus = meson_drm_getConnectionStatus(fd, connType);
    display_meson_close(fd);
    return ConnStatus;
}

ENUM_MESON_HDCP_VERSION getDisplayHdcpVersion(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_MESON_HDCP_VERSION hdcpVersion = MESON_HDCP_RESERVED;
    hdcpVersion = meson_drm_getHdcpVersion(fd, connType);
    display_meson_close(fd);
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
        ERROR("%s %d modeInfo get fail ",__FUNCTION__,__LINE__);
    }
    display_meson_close(fd);
    return ret;
}

ENUM_MESON_HDR_POLICY getDisplayHDRPolicy(MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_MESON_HDR_POLICY hdrPolicy = MESON_HDR_POLICY_FOLLOW_SINK;
    hdrPolicy = meson_drm_getHDRPolicy(fd, connType);
    display_meson_close(fd);
    return hdrPolicy;
}

int setDisplayHDRPolicy(ENUM_MESON_HDR_POLICY hdrPolicy, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setHDRPolicy(fd, req, hdrPolicy, connType);
    if (res == -1) {
        ERROR("setHDRPolicyFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set HDR Policy: %d-%s", ret, strerror(errno));
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
    display_meson_close(fd);
    return ret;
}

int getDisplayPreferMode( DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType) {
    int ret = -1;
    ret = meson_drm_getPreferredMode(modeInfo,connType);
    return ret;
}

int setDisplayHDCPContentType(ENUM_MESON_HDCP_Content_Type HDCPType, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setHDCPContentType(fd, req, HDCPType, connType);
    if (res == -1) {
        ERROR("setDisplayHDCPContentTypeFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set HDCP Content Type: %d-%s", ret, strerror(errno));
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
    fd = display_meson_get_open();
    ENUM_MESON_HDCP_Content_Type ContentType = MESON_HDCP_Type_RESERVED;
    ContentType = meson_drm_getHDCPContentType(fd, connType);
    display_meson_close(fd);
    return ContentType;
}

ENUM_MESON_Content_Type getDisplayContentType( MESON_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_MESON_Content_Type ContentType = MESON_Content_Type_RESERVED;
    ContentType = meson_drm_getContentType(fd, connType);
    display_meson_close(fd);
    return ContentType;
}

int setDisplayDvEnable(int dvEnable, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setDvEnable(fd, req, dvEnable, connType);
    if (res == -1) {
        ERROR("setDisplayDvEnableFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set  Display Dv Enable : %d-%s", ret, strerror(errno));
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
    display_meson_close(fd);
    return ret;
}

int setDisplayActive(int active, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setActive(fd, req, active, connType);
    if (res == -1) {
        ERROR("setDisplayActiveFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set Display Active : %d-%s", ret, strerror(errno));
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
    display_meson_close(fd);
    return ret;
}

int setDisplayVrrEnabled(int VrrEnable, MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setVrrEnabled(fd, req, VrrEnable, connType);
    if (res == -1) {
        ERROR("setDisplayVrrEnabledFail");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set Display VrrEnabled: %d-%s", ret, strerror(errno));
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
    return ret;
}

int getDisplayAVMute(MESON_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    int ret = 0;
    ret = meson_drm_getAVMute(fd, connType );
    display_meson_close(fd);
    return ret;
}

int setDisplayMode(DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    if (modeInfo == NULL) {
        ERROR(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
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
        ERROR("changeModeFail\n");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("failed to set mode: %d-%s", ret, strerror(errno));
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
    fd = display_meson_get_open();
    ENUM_MESON_HDR_MODE hdrMode = MESON_SDR;
    hdrMode = meson_drm_getHdrStatus(fd, connType);
    display_meson_close(fd);
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
    int fd  = 0;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    ret = meson_drm_getsupportedModesList(fd, &modes, &count ,connType);
    if (ret) {
        ERROR("getsupportedModesList failed : %d-%s", ret, strerror(errno));
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
    DEBUG("\n %d %d %d %d \n", modes[miBest].interlace, modes[miBest].w, modes[miBest].h,modes[miBest].vrefresh);
    ret = meson_drm_changeMode(fd, req, &modes[miBest], connType);
    if (ret) {
        ERROR("changeModeFail\n");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        ERROR("auto mode failed : %d-%s", ret, strerror(errno));
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


