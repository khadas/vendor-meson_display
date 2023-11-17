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
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
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
    meson_close_drm(fd);
    return  ret;
}

int setDisplayHDRPolicy(ENUM_DISPLAY_HDR_POLICY hdrPolicy, DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    DEBUG("%s %d set hdrPolicy type %s",__FUNCTION__,__LINE__,hdrPolicy == 0? "DISPLAY_HDR_POLICY_FOLLOW_SINK":"DISPLAY_HDR_POLICY_FOLLOW_SOURCE");
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

int setDisplayMode(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int resNum = -1;
    int fd = 0;
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


int setDisplayAspectRatioValue(ENUM_DISPLAY_ASPECT_RATIO ASPECTRATIO, DISPLAY_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;
    char* str = NULL;
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
    DEBUG("%s %d set aspect ratio %s",__FUNCTION__,__LINE__, str);
    fd = display_meson_set_open();
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        DEBUG("%s %d invalid parameter return",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_setAspectRatioValue(fd, req, ASPECTRATIO, connType);
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
