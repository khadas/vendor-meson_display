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

#define MESON_DISPLAY_DV_MODE_FLAG 0xf8

static uint32_t getDisplayHDRSupportedList(uint64_t hdrlist, uint64_t dvlist);

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
    int ret = -1;
    if (modeInfo == NULL) {
        ERROR("%s %d modeInfo == NULL return",__FUNCTION__,__LINE__);
        return ret;
    }
    fd = display_meson_get_open();
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
    char* str = NULL;
    ENUM_DISPLAY_HDR_POLICY hdrPolicy = DISPLAY_HDR_POLICY_FOLLOW_SINK;
    ENUM_DISPLAY_FORCE_MODE forcemode = DISPLAY_UNKNOWN_FMT;
    hdrPolicy = meson_drm_getHDRPolicy(fd, connType);
    forcemode = meson_drm_getHdrForceMode(fd, connType);
    meson_close_drm(fd);
    switch (hdrPolicy)
    {
        case 0:
            str = "DISPLAY_HDR_POLICY_FOLLOW_SINK";
            break;
        case 1:
            str = "DISPLAY_HDR_POLICY_FOLLOW_SOURCE";
            break;
        case 2:
            str = "DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE";
            break;
        default:
            break;
    }
    DEBUG("%s %d get hdr policy: %s,force mode: %d",__FUNCTION__,__LINE__,str,forcemode);
    return hdrPolicy;
}

int getDisplayModesList(DisplayModeInfo** modeInfo, int* modeCount,DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    int ret = -1;
    if (modeInfo == NULL || *modeInfo == NULL || modeCount == NULL) {
        ERROR(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        return ret;
    }
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
    if (modeInfo == NULL) {
        ERROR("%s %d display mode info is NULL.\n",__FUNCTION__,__LINE__);
        return ret;
    }
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

int getDisplayFracRatePolicy(DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    int ret = -1;
    ret = meson_drm_getFracRatePolicy(fd, connType );
    meson_close_drm(fd);
    DEBUG("%s %d get frac_rate value: %d",__FUNCTION__,__LINE__,ret);
    return ret;
}

int getDisplaySupportAttrList(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    char attr[32] = {'\0'};
    char array[215][255] = {{'\0'}};
    int  index = 0;
    char color[5] = {'\0'};
    int supportedcheck = -1;
    int ret = -1;
    int fd = 0;
    int ColorDepth = 0;
    drmModeAtomicReq *req = NULL;
    if (modeInfo == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        return ret;
    }
    fd = display_meson_get_open();
    ret = meson_drm_getModeInfo(fd, connType, modeInfo);
    if (ret == -1) {
        ERROR("%s %d get modeInfo fail",__FUNCTION__,__LINE__);
        return ret;
    }
    DEBUG("%s %d modeInfoName %s",__FUNCTION__,__LINE__, modeInfo->name);
    for (int ColorSpace = 0; ColorSpace < DISPLAY_COLOR_SPACE_RESERVED; ColorSpace++) {
        for (int colordepth = 0; colordepth < 3; colordepth++) {
            switch ( colordepth ) {
                case 0:
                    ColorDepth = 8;
                    break;
                case 1:
                    ColorDepth = 10;
                    break;
                case 2:
                    ColorDepth = 12;
                    break;
                default:
                    break;
             }
             supportedcheck = modeAttrSupportedCheck(modeInfo->name,ColorSpace,ColorDepth,connType );
             if (supportedcheck == 1) {
                 switch ( ColorSpace ) {
                     case 0:
                         sprintf(color, "rgb");
                         break;
                     case 1:
                         sprintf(color, "422");
                         break;
                     case 2:
                         sprintf(color, "444");
                         break;
                     case 3:
                         sprintf(color, "420");
                         break;
                     default:
                         break;
             }
             snprintf(attr, sizeof(attr)-1, "%s,%dbit", color, ColorDepth);
             strcpy(array[index++], attr);
             ret = 0;
            }
        }
    }
    DEBUG("%s %d attr list count: %d",__FUNCTION__,__LINE__,index);
    for (int i = 0; i < index; i++) {
        DEBUG_EDID("%s\n",array[i]);
    }
    meson_close_drm(fd);
    return ret;
}

int getDisplaySupportedDVMode( DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    fd = display_meson_get_open();
    int value = meson_drm_getDvCap(fd, connType );
    meson_close_drm(fd);
    DEBUG("%s %d supported dvmode, dv_cap: %d, supported_dv_mode: %d\n",__FUNCTION__,__LINE__,
                                 value, value & MESON_DISPLAY_DV_MODE_FLAG);
    return value & MESON_DISPLAY_DV_MODE_FLAG;
}

static uint32_t getDisplayHDRSupportedList(uint64_t hdrlist, uint64_t dvlist) {
    uint32_t ret = 0;
    DEBUG("%s %d hdrlist:%llu, dvlist:%llu",__FUNCTION__,__LINE__, hdrlist, dvlist);
    if (!!(hdrlist & 0x1))
        ret = ret | (0x1 << (int)MESON_DISPLAY_HDR10PLUS);

    if (!!(dvlist & 0x1A))
        ret = ret | (0x1 << (int)MESON_DISPLAY_DolbyVision_STD);

    if (!!(dvlist & 0xE0))
        ret = ret | (0x1 << (int)MESON_DISPLAY_DolbyVision_Lowlatency);

    if (!!(hdrlist & 0x8))
        ret = ret | (0x1 << (int)MESON_DISPLAY_HDR10_ST2084);

    if (!!(hdrlist & 0x4))
        ret = ret | (0x1 << (int)MESON_DISPLAY_HDR10_TRADITIONAL);

    if (!!(hdrlist & 0x10))
        ret = ret | (0x1 << (int)MESON_DISPLAY_HDR_HLG);

    if (!!(hdrlist & 0x2))
        ret = ret | (0x1 << (int)MESON_DISPLAY_SDR);

    return ret;
}

uint32_t getDisplayHDRSupportList(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    uint32_t hdrcap = 0;
    uint32_t dvcap = 0;
    uint32_t value_2 = 0;
    fd = display_meson_get_open();
    hdrcap = meson_drm_getHdrCap( fd, connType );
    dvcap = meson_drm_getDvCap( fd, connType );
    value_2 = getDisplayHDRSupportedList(hdrcap, dvcap);
    DEBUG("%s %d meson_drm_getHDRSupportedList return %d",__FUNCTION__,__LINE__,value_2);
    meson_close_drm(fd);
    return value_2;
}

uint32_t getDisplayDvCap(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    uint32_t value = 0;
    fd = display_meson_get_open();
    value = meson_drm_getDvCap(fd, connType );
    meson_close_drm(fd);
    return value;
}

int getDisplayDpmsStatus(DISPLAY_CONNECTOR_TYPE connType) {
    int fd = 0;
    fd = display_meson_get_open();
    int value = 0;
    value = meson_drm_getDpmsStatus(fd, connType);
    meson_close_drm(fd);
    DEBUG("%s %d get dpms status %d",__FUNCTION__,__LINE__,value);
    return value;
}

float getDisplayFrameRate(DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int fd = 0;
    float refresh = 0.00;
    drmModeAtomicReq *req = NULL;
    DisplayModeInfo* modeInfo = NULL;
    modeInfo= (DisplayModeInfo*)malloc(sizeof(DisplayModeInfo));
    fd = display_meson_get_open();
    ret = meson_drm_getModeInfo(fd, connType, modeInfo);
    if (ret == -1) {
        ERROR("%s %d get modeInfo fail",__FUNCTION__,__LINE__);
    }
    DEBUG("%s %d modeInfoName %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h,
                          (modeInfo->interlace == 0 ?"p":"i"), modeInfo->vrefresh);
    if ( ( modeInfo->vrefresh == 60 || modeInfo->vrefresh == 30 || modeInfo->vrefresh == 24
               || modeInfo->vrefresh == 120 || modeInfo->vrefresh == 240 )
               && meson_drm_getFracRatePolicy(fd , DISPLAY_CONNECTOR_HDMIA) == 1 ) {
             refresh = ((float)(modeInfo->vrefresh) * 1000) / 1001;
    } else {
        DEBUG("%s %d get framrate %.2f",__FUNCTION__,__LINE__,(float)modeInfo->vrefresh);
        refresh = (float)modeInfo->vrefresh;
    }
    free(modeInfo);
    meson_close_drm(fd);
    return refresh;
}

int getDisplayPlaneSize( int* width, int* height ) {
    int fd = 0;
    int ret = -1;
    int rc = -1;
    if (width == NULL || height == NULL) {
        ERROR("%s %d Error: One or both pointers are NULL.\n",__FUNCTION__,__LINE__);
        return ret;
    }
    fd = display_meson_get_open();
    ret = meson_drm_getGraphicPlaneSize(fd, width, height);
    if ( ret == 0) {
        DEBUG("%s %d get graphic plane Size %d x %d",__FUNCTION__,__LINE__,*width, *height);
    }
    meson_close_drm(fd);
    return ret;
}

int getDisplayPhysicalSize( int* width, int* height, DISPLAY_CONNECTOR_TYPE connType ) {
    int fd = 0;
    int ret = -1;
    int rc = -1;
    if (width == NULL || height == NULL) {
        ERROR("%s %d Error: One or both pointers are NULL.\n",__FUNCTION__,__LINE__);
        return ret;
    }
    fd = display_meson_get_open();
    ret = meson_drm_getPhysicalSize(fd, width, height, connType);
    if ( ret == 0) {
        DEBUG("%s %d get physical Size %d x %d",__FUNCTION__,__LINE__,*width, *height);
    }
    meson_close_drm(fd);
    return ret;
}
