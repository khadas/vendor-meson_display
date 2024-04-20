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
#define DEFAULT_CARD "/dev/dri/card0"
#include "libdrm_meson/meson_drm_log.h"

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

int setDisplayHDCPEnable(int enable, DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
    switch (enable)
    {
        case 0:
            str = "HDCP UNENABLED";
            break;
        case 1:
            str = "HDCP ENABLED";
            break;
        default :
            break;
    }
    DEBUG("%s %d set %s",__FUNCTION__,__LINE__,str);
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayAVMute(int mute, DISPLAY_CONNECTOR_TYPE connType) {
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                                    DISPLAY_CONNECTOR_TYPE connType) {
    int res1 = -1;
    int res2 = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
    ENUM_MESON_COLOR_SPACE mesonColorSpace = MESON_COLOR_SPACE_RESERVED;
    switch (colorSpace)
    {
        case DISPLAY_COLOR_SPACE_RGB:
            str = "DISPLAY_COLOR_SPACE_RGB";
            mesonColorSpace = MESON_COLOR_SPACE_RGB;
            break;
        case DISPLAY_COLOR_SPACE_YCBCR422:
            str = "DISPLAY_COLOR_SPACE_YCBCR422";
            mesonColorSpace = MESON_COLOR_SPACE_YCBCR422;
            break;
        case DISPLAY_COLOR_SPACE_YCBCR444:
            str = "DISPLAY_COLOR_SPACE_YCBCR444";
            mesonColorSpace = MESON_COLOR_SPACE_YCBCR444;
            break;
        case DISPLAY_COLOR_SPACE_YCBCR420:
            str = "DISPLAY_COLOR_SPACE_YCBCR420";
            mesonColorSpace = MESON_COLOR_SPACE_YCBCR420;
            break;
        default:
            str = "DISPLAY_COLOR_SPACE_RESERVED";
            mesonColorSpace = MESON_COLOR_SPACE_RESERVED;
            break;
    }
    DEBUG(" %s %d set colorDepth: %d colorSpace: %s",__FUNCTION__,__LINE__,colorDepth,str);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        ERROR(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res1 = meson_drm_setColorDepth(fd, req, colorDepth, connType);
    res2 = meson_drm_setColorSpace(fd, req, mesonColorSpace, connType);
    if (res1 == -1 || res2 == -1) {
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayHDRPolicy(ENUM_DISPLAY_HDR_POLICY hdrPolicy, DISPLAY_CONNECTOR_TYPE connType) {
    int respolicy = -1;
    int resforce = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    ENUM_MESON_DRM_FORCE_MODE forcemode = MESON_DRM_UNKNOWN_FMT;
    ENUM_MESON_HDR_POLICY mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SOURCE;
    DEBUG("%s %d set hdr policy %d",__FUNCTION__,__LINE__,hdrPolicy);
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    switch (hdrPolicy) {
        case DISPLAY_HDR_POLICY_FOLLOW_SINK:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SINK;
            break;
        case DISPLAY_HDR_POLICY_FOLLOW_SOURCE:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SOURCE;
            break;
         case DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_FORCE_MODE;
            break;
        default:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SOURCE;
            break;
    }
    fd = display_meson_set_open();
    respolicy = meson_drm_setHDRPolicy(fd, req, mesonHdrPolicy, connType);
    if (respolicy == -1) {
        ERROR("%s %d set hdr policy fail",__FUNCTION__,__LINE__);
        goto out;
    }
    if (hdrPolicy == DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE) {
        forcemode = MESON_DRM_BT709;
    } else {
        forcemode = MESON_DRM_UNKNOWN_FMT;
    }
    DEBUG("%s %d set force mode %d",__FUNCTION__,__LINE__,forcemode);
    resforce = meson_drm_setHdrForceMode(fd, req, forcemode, connType);
    if (resforce == -1) {
        ERROR("%s %d set hdr force mode fail",__FUNCTION__,__LINE__);
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayHDCPContentType(ENUM_DISPLAY_HDCP_Content_Type HDCPType, DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set hdcp content type %s",__FUNCTION__,__LINE__,HDCPType == 0? "DISPLAY_HDCP_Type0":"DISPLAY_HDCP_Type1");
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayDvEnable(int dvEnable, DISPLAY_CONNECTOR_TYPE connType) {
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayActive(int active, DISPLAY_CONNECTOR_TYPE connType) {
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
    meson_close_drm(fd);
    return  ret;
}


int setDisplayVrrEnabled(int VrrEnable, DISPLAY_CONNECTOR_TYPE connType) {
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
    meson_close_drm(fd);
    return  ret;
}

#define MAX_RETRY 3
#define WAIT_TIME_MS 30
int setDisplayMode(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int resNum = -1;
    int fd = 0;
    int retries = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set modeInfo %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h,
                               (modeInfo->interlace == 0? "p":"i") , modeInfo->vrefresh);
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
        if (connType == DISPLAY_CONNECTOR_DUMMY) {
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
    DEBUG("%s %d master status %d \n",__FUNCTION__,__LINE__,drmIsMaster(fd));
    while (retries < MAX_RETRY) {
        if (drmIsMaster(fd)) {
            DEBUG("%s %d already the master.\n",__FUNCTION__,__LINE__);
            break;
        } else {
            DEBUG("%s %d setting master attempt number %d\n",__FUNCTION__,__LINE__,retries + 1);
            if (drmSetMaster(fd)) {
                DEBUG("%s %d became the master\n",__FUNCTION__,__LINE__);
                break;
            } else {
                usleep(WAIT_TIME_MS * 1000);
            }
            retries++;
        }
    }
    if (retries == MAX_RETRY) {
        ERROR("%s %d failed to set the master after %d attempts. Exiting.\n",__FUNCTION__,__LINE__, MAX_RETRY);
        goto out;
    }
    DEBUG("%s %d set the master status %d\n",__FUNCTION__,__LINE__,drmIsMaster(fd));
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayAutoMode(DISPLAY_CONNECTOR_TYPE connType) {
    int count = 0;
    int i = 0;
    int miBest= -1;
    int area, largestArea = 0;
    int refresh = 0;
    int ret = -1;
    int maxArea = 0;
    DisplayModeInfo* modes = NULL;
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
    DEBUG("%s %d get largest modeInfo %dx%d%s%dhz", __FUNCTION__,__LINE__, modes[miBest].w, modes[miBest].h,
                                (modes[miBest].interlace == 0? "p":"i"), modes[miBest].vrefresh);
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
    meson_close_drm(fd);
    return ret;
}

int setDisplayAspectRatioValue(ENUM_DISPLAY_ASPECT_RATIO aspectRatio, DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
    ENUM_MESON_ASPECT_RATIO mesonAspectRatio = MESON_ASPECT_RATIO_RESERVED;
    switch (aspectRatio) {
        case DISPLAY_ASPECT_RATIO_AUTOMATIC:
            str = "DISPLAY_ASPECT_RATIO_AUTOMATIC";
            mesonAspectRatio = MESON_ASPECT_RATIO_AUTOMATIC;
            break;
        case DISPLAY_ASPECT_RATIO_4_3:
            str = "DISPLAY_ASPECT_RATIO_4_3";
            mesonAspectRatio = MESON_ASPECT_RATIO_4_3;
            break;
        case DISPLAY_ASPECT_RATIO_16_9:
            str = "DISPLAY_ASPECT_RATIO_16_9";
            mesonAspectRatio = MESON_ASPECT_RATIO_16_9;
            break;
        default:
            str = "DISPLAY_ASPECT_RATIO_RESERVED";
            mesonAspectRatio = MESON_ASPECT_RATIO_RESERVED;
            break;
    }
    DEBUG("%s %d set aspect ratio %s",__FUNCTION__,__LINE__, str);
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    fd = display_meson_set_open();
    res = meson_drm_setAspectRatioValue(fd, req, mesonAspectRatio, connType);
    if (res == -1) {
        ERROR(" %s %d set aspect ratio value fail",__FUNCTION__,__LINE__);
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayModeAttr(DisplayModeInfo* modeInfo,uint32_t colorDepth,
                     ENUM_DISPLAY_COLOR_SPACE colorSpace,DISPLAY_CONNECTOR_TYPE connType) {
    int SupportedCheck = -1;
    int changeModeNum = -1;
    int ColorDepthNum = -1;
    int ColorSpaceNum = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    if (modeInfo == NULL || modeInfo->name == NULL) {
        ERROR("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        return ret;
    }
    DEBUG("%s %d modeName: %s colorSpace: %d,colorDepth: %d,connType: %d",__FUNCTION__,__LINE__,
                   modeInfo->name, colorSpace,colorDepth,connType);
    SupportedCheck = modeAttrSupportedCheck(modeInfo->name, colorSpace,colorDepth, connType );
    if (SupportedCheck == 0) {
        ERROR(" %s %d Mode Attr SupportedCheck Fail",__FUNCTION__,__LINE__);
        return ret;
    } else {
        DEBUG("%s %d Mode Attr SupportedCheck Success",__FUNCTION__,__LINE__);
    }
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    DEBUG("%s %d modeInfo %dx%d%s%dhz,colorSpace: %d,colorDepth: %d,connType: %d",__FUNCTION__,__LINE__,
            modeInfo->w,modeInfo->h,(modeInfo->interlace == 0? "p":"i"),modeInfo->vrefresh,colorSpace,
                   colorDepth,connType);
    changeModeNum = meson_drm_changeMode(fd, req, modeInfo, connType);
    ColorDepthNum = meson_drm_setColorDepth(fd, req, colorDepth, connType);
    ColorSpaceNum = meson_drm_setColorSpace(fd, req, colorSpace, connType);
    if (changeModeNum == -1 || ColorDepthNum == -1 || ColorSpaceNum == -1) {
        ERROR("%s %d fail parameter return",__FUNCTION__,__LINE__);
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayFunctionAttribute( DisplayModeInfo* modeInfo,ENUM_DISPLAY_HDR_POLICY hdrPolicy,uint32_t colorDepth,
                 ENUM_DISPLAY_COLOR_SPACE colorSpace, int FracRate, DISPLAY_CONNECTOR_TYPE connType) {
    int resmode = -1;
    int ret = -1;
    int respolicy = -1;
    int resforce = -1;
    int resNum = -1;
    int rescolordepth = -1;
    int rescolorspace = -1;
    int resFracRate = -1;
    int fd = 0;
    char* str = NULL;
    ENUM_MESON_DRM_FORCE_MODE forcemode = MESON_DRM_UNKNOWN_FMT;
    ENUM_MESON_HDR_POLICY mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SOURCE;
    ENUM_MESON_COLOR_SPACE mesonColorSpace = MESON_COLOR_SPACE_RESERVED;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set modeInfo: %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h,
                               (modeInfo->interlace == 0? "p":"i"), modeInfo->vrefresh);
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
    resmode = meson_drm_changeMode(fd, req, modeInfo, connType);
    if (resmode == -1) {
        ERROR("%s %d change mode fail",__FUNCTION__,__LINE__);
        goto out;
    }
    switch (hdrPolicy) {
        case DISPLAY_HDR_POLICY_FOLLOW_SINK:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SINK;
            break;
        case DISPLAY_HDR_POLICY_FOLLOW_SOURCE:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SOURCE;
            break;
         case DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_FORCE_MODE;
            break;
        default:
            mesonHdrPolicy = MESON_HDR_POLICY_FOLLOW_SOURCE;
            break;
    }
    DEBUG("%s %d set hdr policy：%d colordepth: %d colorspace： %d",__FUNCTION__,__LINE__,mesonHdrPolicy,colorDepth,colorSpace,FracRate);
    respolicy = meson_drm_setHDRPolicy(fd, req, mesonHdrPolicy, connType);
    if (respolicy == -1) {
        ERROR("%s %d set hdr policy fail",__FUNCTION__,__LINE__);
        goto out;
    }
    if (hdrPolicy == DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE) {
        forcemode = MESON_DRM_BT709;
    } else {
        forcemode = MESON_DRM_UNKNOWN_FMT;
    }
    DEBUG("%s %d set force mode： %d",__FUNCTION__,__LINE__,forcemode);
    resforce = meson_drm_setHdrForceMode(fd, req, forcemode, connType);
    if (resforce == -1) {
        ERROR("%s %d set hdr force mode fail",__FUNCTION__,__LINE__);
        goto out;
    }
    rescolordepth = meson_drm_setColorDepth(fd, req, colorDepth, connType);
    if (rescolordepth == -1) {
        ERROR("%s %d set color depth fail",__FUNCTION__,__LINE__);
        goto out;
    }
    switch (colorSpace)
    {
        case DISPLAY_COLOR_SPACE_RGB:
            str = "DISPLAY_COLOR_SPACE_RGB";
            mesonColorSpace = MESON_COLOR_SPACE_RGB;
            break;
        case DISPLAY_COLOR_SPACE_YCBCR422:
            str = "DISPLAY_COLOR_SPACE_YCBCR422";
            mesonColorSpace = MESON_COLOR_SPACE_YCBCR422;
            break;
        case DISPLAY_COLOR_SPACE_YCBCR444:
            str = "DISPLAY_COLOR_SPACE_YCBCR444";
            mesonColorSpace = MESON_COLOR_SPACE_YCBCR444;
            break;
        case DISPLAY_COLOR_SPACE_YCBCR420:
            str = "DISPLAY_COLOR_SPACE_YCBCR420";
            mesonColorSpace = MESON_COLOR_SPACE_YCBCR420;
            break;
        default:
            str = "DISPLAY_COLOR_SPACE_RESERVED";
            mesonColorSpace = MESON_COLOR_SPACE_RESERVED;
            break;
    }
    rescolorspace = meson_drm_setColorSpace(fd, req, mesonColorSpace, connType);
    if (rescolorspace == -1) {
        ERROR("%s %d set color space fail",__FUNCTION__,__LINE__);
        goto out;
    }
    resFracRate = meson_drm_setFracRatePolicy(fd, req, FracRate, connType);
    if (resFracRate == -1) {
        ERROR("%s %d set frac_rate fail",__FUNCTION__,__LINE__);
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
    meson_close_drm(fd);
    return  ret;
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

int setDisplayBackGroundColor(unsigned char red, unsigned char green, unsigned char blue,
                              DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    uint64_t backgroundColor = 0;
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    backgroundColor |= ((uint64_t)(0xff<<8 | 0xff) << 48);
    backgroundColor |= ((uint64_t)(red<<8 | 0xff) << 32);
    backgroundColor |= ((uint64_t)(green<<8 | 0xff) << 16);
    backgroundColor |= ((uint64_t)(blue<<8 | 0xff) << 0);
    DEBUG("%s %d set background current color %llx", __FUNCTION__, __LINE__, backgroundColor);
    res = meson_drm_setBackGroundColor(fd, req, backgroundColor, connType);
    if (res == -1) {
        ERROR("%s %d set background color fail",__FUNCTION__,__LINE__);
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayDvMode(int dvmode,DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set dvmode value %d",__FUNCTION__,__LINE__,dvmode);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG(" %s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setDvMode(fd, req, dvmode, connType);
    if (res == -1) {
        ERROR("%s %d set dv mode fail",__FUNCTION__,__LINE__);
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayPlaneMute(unsigned int plane_type, unsigned int plane_mute) {
    int res = -1;
    int fd = -1;
    DEBUG("%s %d set plane_type: %d plane_mute:%d",__FUNCTION__,__LINE__,plane_type,plane_mute);
    fd = display_meson_set_open();
    res = meson_drm_setPlaneMute(fd, plane_type, plane_mute);
    if (res) {
        ERROR("%s %d set plane mute fail",__FUNCTION__,__LINE__);
    }
    meson_close_drm(fd);
    return res;
}

