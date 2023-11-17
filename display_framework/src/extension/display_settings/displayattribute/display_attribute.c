 /*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 * Description:
 */

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
#include "../display_settings.h"
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

bool registerMesonDisplayEventCallback(mesonDisplayEventCallback cb) { //register callback
    return RegisterDisplayEventCallback(cb);
}

void startMesonDisplayUeventMonitor() {
    startDisplayUeventMonitor();
}

void stopMesonDisplayUeventMonitor() {
    stopDisplayUeventMonitor();
}

ENUM_DISPLAY_HDCPAUTH_STATUS getDisplayHdcpAuthStatus(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_DISPLAY_HDCPAUTH_STATUS hdcpAuthStatus = DISPLAY_AUTH_STATUS_FAIL;
    int ret = meson_drm_getHdcpAuthStatus(fd, connType );
    if (ret == 0) {
        hdcpAuthStatus = DISPLAY_AUTH_STATUS_FAIL;
    } else if(ret == 1) {
        hdcpAuthStatus = DISPLAY_AUTH_STATUS_SUCCESS;
    }
    meson_close_drm(fd);
    DEBUG(" %s %d get hdcp auth status: %d",__FUNCTION__,__LINE__,hdcpAuthStatus);
    return hdcpAuthStatus;
}

void getDisplayEDIDData(DISPLAY_CONNECTOR_TYPE connType, int * data_Len, char **data ) {
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
    meson_close_drm(fd);
}

ENUM_DISPLAY_COLOR_SPACE getDisplayColorSpace(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_COLOR_SPACE colorSpace = DISPLAY_COLOR_SPACE_RESERVED;
    colorSpace = meson_drm_getColorSpace(fd, connType);
    meson_close_drm(fd);
    switch (colorSpace)
    {
        case 0:
            str = "DISPLAY_COLOR_SPACE_RGB";
            break;
        case 1:
            str = "DISPLAY_COLOR_SPACE_YCBCR422";
            break;
        case 2:
            str = "DISPLAY_COLOR_SPACE_YCBCR444";
            break;
        case 3:
            str = "DISPLAY_COLOR_SPACE_YCBCR420";
            break;
        default:
            str = "DISPLAY_COLOR_SPACE_RESERVED";
            break;
    }
    DEBUG("%s %d get colorSpace: %s",__FUNCTION__,__LINE__,str);
    return colorSpace;
}

uint32_t getDisplayColorDepth(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    int value = 0;
    value = meson_drm_getColorDepth(fd, connType);
    meson_close_drm(fd);
    DEBUG("%s %d get ColorDepth: %d",__FUNCTION__,__LINE__,value);
    return value;
}

ENUM_DISPLAY_CONNECTION getDisplayConnectionStatus(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_CONNECTION ConnStatus = DISPLAY_UNKNOWNCONNECTION;
    ConnStatus = meson_drm_getConnectionStatus(fd, connType);
    meson_close_drm(fd);
    switch (ConnStatus)
    {
        case 0:
            str = "DISPLAY_DISCONNECTED";
            break;
        case 1:
            str = "DISPLAY_CONNECTED";
            break;
        default:
            str = "DISPLAY_UNKNOWNCONNECTION";
            break;
    }
    DEBUG("%s %d get connection status: %s",__FUNCTION__,__LINE__,str);
    return ConnStatus;
}

ENUM_DISPLAY_HDCP_VERSION getDisplayHdcpVersion(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_HDCP_VERSION hdcpVersion = DISPLAY_HDCP_RESERVED;
    hdcpVersion = meson_drm_getHdcpVersion(fd, connType);
    meson_close_drm(fd);
    switch (hdcpVersion)
    {
        case 0:
            str = "DISPLAY_HDCP_14";
            break;
        case 1:
            str = "DISPLAY_HDCP_22";
            break;
        default:
            str = "DISPLAY_HDCP_RESERVED";
            break;
    }
    DEBUG("%s %d get hdcp version: %s",__FUNCTION__,__LINE__,str);
    return hdcpVersion;
}

int getDisplayMode(DisplayModeInfo* modeInfo, DISPLAY_CONNECTOR_TYPE connType) {
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
    meson_close_drm(fd);
    DEBUG("%s %d modeInfo: %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h,
                          (modeInfo->interlace == 0 ?"p":"i"), modeInfo->vrefresh);
    return ret;
}

ENUM_DISPLAY_HDR_POLICY getDisplayHDRPolicy(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    ENUM_DISPLAY_HDR_POLICY hdrPolicy = DISPLAY_HDR_POLICY_FOLLOW_SINK;
    hdrPolicy = meson_drm_getHDRPolicy(fd, connType);
    meson_close_drm(fd);
    DEBUG("%s %d get hdrPolicy type %s",__FUNCTION__,__LINE__,hdrPolicy == 0? "DISPLAY_HDR_POLICY_FOLLOW_SINK":"DISPLAY_HDR_POLICY_FOLLOW_SOURCE");
    return hdrPolicy;
}

int getDisplayModesList(DisplayModeInfo** modeInfo, int* modeCount,DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    int ret = -1;
    fd = display_meson_get_open();
    ret = meson_drm_getsupportedModesList(fd, modeInfo, modeCount,connType);
    if (ret == -1) {
        ERROR("%s %d get supported modeslist failed: ret %d errno %d",__FUNCTION__,__LINE__, ret, errno );
    }
    meson_close_drm(fd);
    DEBUG("%s %d mode count: %d",__FUNCTION__,__LINE__,(*modeCount));
    for (int i=0; i < (*modeCount); i++) {
        DEBUG_EDID(" %s %dx%d%s%dhz\n", (*modeInfo)[i].name, (*modeInfo)[i].w, (*modeInfo)[i].h,
                            ((*modeInfo)[i].interlace == 0? "p":"i"), (*modeInfo)[i].vrefresh);
    }
    return ret;
}

int getDisplayPreferMode( DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    ret = meson_drm_getPreferredMode(modeInfo,connType);
    if (ret == -1) {
        ERROR("%s %d get preferred modes failed: ret %d errno %d",__FUNCTION__,__LINE__, ret, errno );
    }
    DEBUG("%s %d get preferred mode %s %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->name, modeInfo->w,
                        modeInfo->h, (modeInfo->interlace == 0? "p":"i"),modeInfo->vrefresh);
    return ret;
}

ENUM_DISPLAY_HDCP_Content_Type getDisplayHDCPContentType(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_HDCP_Content_Type ContentType = DISPLAY_HDCP_Type_RESERVED;
    ContentType = meson_drm_getHDCPContentType(fd, connType);
    meson_close_drm(fd);
    switch (ContentType)
    {
        case 0:
            str = "DISPLAY_HDCP_Type0";
            break;
        case 1:
            str = "DISPLAY_HDCP_Type1";
            break;
        default:
            str = "DISPLAY_HDCP_Type_RESERVED";
            break;
    }
    DEBUG("%s %d get hdcp content type: %s",__FUNCTION__,__LINE__,str);
    return ContentType;
}

ENUM_DISPLAY_Content_Type getDisplayContentType( DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_Content_Type ContentType = DISPLAY_Content_Type_RESERVED;
    ContentType = meson_drm_getContentType(fd, connType);
    meson_close_drm(fd);
    switch (ContentType)
    {
        case 0:
            str = "DISPLAY_Content_Type_Data";
            break;
        case 1:
            str = "DISPLAY_Content_Type_Graphics";
            break;
         case 2:
            str = "DISPLAY_Content_Type_Photo";
            break;
        case 3:
            str = "DISPLAY_Content_Type_Cinema";
            break;
         case 4:
            str = "DISPLAY_Content_Type_Game";
            break;
        default:
            str = "DISPLAY_Content_Type_RESERVED";
            break;
    }
    DEBUG("%s %d get content type: %s",__FUNCTION__,__LINE__,str);
    return ContentType;
}


int getDisplayDvEnable(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    int ret = -1;
    fd = display_meson_get_open();
    ret = meson_drm_getDvEnable(fd, connType );
    if (ret == -1) {
        ERROR("%s %d get DvEnable fail",__FUNCTION__,__LINE__);
    }
    meson_close_drm(fd);
    DEBUG("%s %d get DvEnable value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int getDisplayActive(DISPLAY_CONNECTOR_TYPE connType ) {
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
    meson_close_drm(fd);
    DEBUG("%s %d get active value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int getDisplayVrrEnabled(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    int ret = 0;
    fd = display_meson_get_open();
    ret = meson_drm_getVrrEnabled(fd, connType );
    meson_close_drm(fd);
    DEBUG("%s %d get VrrEnabled value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int getDisplayAVMute(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    int ret = 0;
    ret = meson_drm_getAVMute(fd, connType );
    meson_close_drm(fd);
    DEBUG("%s %d get AVMute value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

ENUM_DISPLAY_HDR_MODE getDisplayHdrStatus(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_HDR_MODE hdrMode = MESON_DISPLAY_SDR;
    hdrMode = meson_drm_getHdrStatus(fd, connType);
    meson_close_drm(fd);
    switch (hdrMode)
    {
        case 0:
            str = "MESON_DISPLAY_HDR10PLUS";
            break;
        case 1:
            str = "MESON_DISPLAY_DolbyVision_STD";
            break;
        case 2:
            str = "MESON_DISPLAY_DolbyVision_Lowlatency";
            break;
        case 3:
            str = "MESON_DISPLAY_HDR10_ST2084";
            break;
        case 4:
            str = "MESON_DISPLAY_HDR10_TRADITIONAL";
            break;
        case 5:
            str = "MESON_DISPLAY_HDR_HLG";
            break;
        case 6:
            str = "MESON_DISPLAY_SDR";
            break;
        default:
            str = "MESON_DISPLAY_SDR";
            break;
    }
    DEBUG("%s %d get hdr status: %s",__FUNCTION__,__LINE__,str);
    return hdrMode;
}

bool modeAttrSupportedCheck(char* modeName, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                            uint32_t colorDepth, DISPLAY_CONNECTOR_TYPE connType )
{
    bool ret = false;
    char attr[32] = {'\0'};
    char color[5] = {'\0'};
    int fd = -1;
    if (modeName == NULL || colorSpace > DISPLAY_COLOR_SPACE_YCBCR420 ||
        ( connType != DISPLAY_CONNECTOR_HDMIA && connType != DISPLAY_CONNECTOR_HDMIB) ) {
        ERROR("%s %d invalid parameters ", __FUNCTION__,__LINE__);
        return ret;
    }
    switch ( colorSpace ) {
        case DISPLAY_COLOR_SPACE_RGB:
            sprintf(color, "rgb");
            break;
        case DISPLAY_COLOR_SPACE_YCBCR420:
            sprintf(color, "420");
            break;
        case DISPLAY_COLOR_SPACE_YCBCR422:
            sprintf(color, "422");
            break;
        case DISPLAY_COLOR_SPACE_YCBCR444:
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
    meson_close_drm(fd);
    DEBUG("%s %d modeAttrSupportedCheck: %d", __FUNCTION__,__LINE__,ret);
    return ret;
}

int setDisplayVideoZorder(unsigned int index, unsigned int zorder, unsigned int flag) {
    int ret = -1;
    int fd = -1;
    DEBUG("%s %d set video zorder index:%d,zorder:%d,flag:%d",__FUNCTION__,__LINE__,index,zorder,flag);
    fd = open(DEFAULT_CARD, O_RDWR|O_CLOEXEC);
    if (fd < 0) {
        ERROR("%s %d failed to open device %s",  __FUNCTION__,__LINE__,strerror(errno));
    }
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret < 0) {
        ERROR("%s %d no atomic modesetting support", __FUNCTION__,__LINE__);
    }
    ret = meson_drm_setVideoZorder( fd, index, zorder, flag);
    if (ret) {
        ERROR("%s %d set video zorder Fail",ret, strerror(errno));
    }
    meson_close_drm(fd);
    return ret;
}

ENUM_DISPLAY_ASPECT_RATIO getDisplayAspectRatioValue(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    char* str = NULL;
    fd = display_meson_get_open();
    ENUM_DISPLAY_ASPECT_RATIO ASPECTRATIO = meson_drm_getAspectRatioValue(fd, connType );
    meson_close_drm(fd);
    switch (ASPECTRATIO)
    {
    case 0:
        str = "DISPLAY_ASPECT_RATIO_AUTOMATIC";
        break;
    case 1:
        str = "DISPLAY_ASPECT_RATIO_4_3";
        break;
    case 2:
        str = "DISPLAY_ASPECT_RATIO_16_9";
        break;
    default:
        str = "DISPLAY_ASPECT_RATIO_RESERVED";
        break;
    }
    DEBUG("%s %d get aspect ratio %s",__FUNCTION__,__LINE__,str);
    return ASPECTRATIO;
}


