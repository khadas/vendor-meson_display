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

/*                         Interface to be Developed                          */

int setDisplayHDCPEnable(int enable, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayAVMute(int mute, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                                    DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayHDRPolicy(ENUM_DISPLAY_HDR_POLICY hdrPolicy, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayHDCPContentType(ENUM_DISPLAY_HDCP_Content_Type HDCPType, DISPLAY_CONNECTOR_TYPE connType) {
    return 0;
}

int setDisplayDvEnable(int dvEnable, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayActive(int active, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayVrrEnabled(int VrrEnable, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayMode(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayAutoMode(DISPLAY_CONNECTOR_TYPE connType) {
    return 0;
}

int setDisplayAspectRatioValue(ENUM_DISPLAY_ASPECT_RATIO ASPECTRATIO, DISPLAY_CONNECTOR_TYPE connType) {

    return  0;
}

int setDisplayModeAttr(DisplayModeInfo* modeInfo,uint32_t colorDepth,
                     ENUM_DISPLAY_COLOR_SPACE colorSpace,DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayFunctionAttribute( DisplayModeInfo* modeInfo,ENUM_DISPLAY_HDR_POLICY hdrPolicy,uint32_t colorDepth,
                 ENUM_DISPLAY_COLOR_SPACE colorSpace, int FracRate, DISPLAY_CONNECTOR_TYPE connType) {
    return  0;
}

int setDisplayVideoZorder(unsigned int index, unsigned int zorder, unsigned int flag) {
    return 0;
}

int setDisplayDvMode(int dvmode,DISPLAY_CONNECTOR_TYPE connType) {
    return 0;
}

