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
#ifndef XDG_RUNTIME_DIR
#define XDG_RUNTIME_DIR     "/run"
#endif

#define CMDBUF_SIZE 512

static int wstDisplaySendMessage(char* property);
static int wstDisplaySendMessage(char* property) {
    int ret = -1;
    DEBUG("%s %d send message parameters %s ", __FUNCTION__,__LINE__,property);
    char* xdgRunDir = getenv("XDG_RUNTIME_DIR");
    if (!xdgRunDir)
        xdgRunDir = XDG_RUNTIME_DIR;
    if (property) {
        do {
            char cmdBuf[CMDBUF_SIZE] = {'\0'};
            snprintf(cmdBuf, sizeof(cmdBuf)-1, "export XDG_RUNTIME_DIR=%s;westeros-gl-console %s | grep \"Response\"",
                    xdgRunDir, property);
            DEBUG("%s %d Executing '%s'\n", __FUNCTION__,__LINE__,cmdBuf);
            FILE* fp = popen(cmdBuf, "r");
            if (NULL != fp) {
                char output[64] = {'\0'};
                while (fgets(output, sizeof(output)-1, fp)) {
                    if (strlen(output) && strstr(output, "[0:")) {
                        ret = 0;
                        DEBUG("%s %d output:%s",__FUNCTION__,__LINE__,output);
                    }
                }
                pclose(fp);
            } else {
            ERROR("%s %d open failed",__FUNCTION__,__LINE__);
            }
            if (ret != 0 ) {
                if (strcmp(xdgRunDir, XDG_RUNTIME_DIR) == 0) {
                    break;
                }
                xdgRunDir = XDG_RUNTIME_DIR;
            }
        } while (ret != 0);
    }
    return ret;
}

int setDisplayHDCPEnable(int enable, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    int connId = -1;
    int fd = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG(" %s %d westeros set hdcp enable %d connId %d",__FUNCTION__,__LINE__,enable,connId);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_CONTENT_PROTECTION);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", connId, prop_name, enable);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayAVMute(int mute, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    int fd = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG("%s %d westeros set mute value %d connId %d connType %d",__FUNCTION__,__LINE__,
                                      mute,connId,connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_HDMI_ENABLE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", connId, prop_name, mute);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                                    DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    int fd = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* space_prop_name = NULL;
    char* depth_prop_name = NULL;
    struct mesonConnector* conn = NULL;
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
    DEBUG(" %s %d westeros set colorDepth: %d colorSpace: %s",__FUNCTION__,__LINE__,colorDepth,str);
    connId = meson_drm_GetConnectorId(connType);
    if (connId > 0) {
        space_prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_COLOR_SPACE);
        depth_prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_COLOR_DEPTH);
        if (space_prop_name == NULL || depth_prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d space_prop_name: %s depth_prop_name",__FUNCTION__,__LINE__,space_prop_name,depth_prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d -s %d:%s:%d", connId, space_prop_name, colorSpace,
                                       connId, depth_prop_name, colorDepth);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (space_prop_name) {
        free(space_prop_name);
    }
    if (depth_prop_name) {
        free(depth_prop_name);
    }
    return ret;
}

int setDisplayHDRPolicy(ENUM_DISPLAY_HDR_POLICY hdrPolicy, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int crtcId = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* hdrpolicy_name = NULL;
    char* force_output_name = NULL;
    ENUM_DISPLAY_FORCE_MODE forcemode = DISPLAY_UNKNOWN_FMT;
    DEBUG("%s %d set hdr policy %d",__FUNCTION__,__LINE__,hdrPolicy);
    crtcId = meson_drm_GetCrtcId(connType);
    if (crtcId > 0) {
        hdrpolicy_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_HDR_POLICY);
        force_output_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_TX_HDR_OFF);
        if (hdrpolicy_name == NULL || force_output_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, hdrpolicy_name);
        if (hdrPolicy == DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE) {
            forcemode = DISPLAY_BT709;
            snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d -s %d:%s:%d", crtcId, hdrpolicy_name,
                              hdrPolicy,crtcId, force_output_name, forcemode);
            DEBUG("%s %d hdrPolicy property: %d:%s:%d forcemode property: %d:%s:%d",__FUNCTION__,__LINE__,
                   crtcId, hdrpolicy_name, hdrPolicy, crtcId, force_output_name, forcemode);
            rc = wstDisplaySendMessage(cmdBuf);
            if (rc >= 0) {
                ret = 0;
            } else {
                ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
            }
        } else {
            forcemode = DISPLAY_UNKNOWN_FMT;
            snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d -s %d:%s:%d", crtcId, hdrpolicy_name,
                             hdrPolicy,crtcId, force_output_name,forcemode);
            DEBUG("%s %d hdrPolicy property: %d:%s:%d forcemode property: %d:%s:%d",__FUNCTION__,__LINE__,
                   crtcId, hdrpolicy_name, hdrPolicy, crtcId, force_output_name, forcemode);
            rc = wstDisplaySendMessage(cmdBuf);
            if (rc >= 0) {
                ret = 0;
            } else {
                ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
            }
        }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (force_output_name) {
        free(force_output_name);
    }
    if (hdrpolicy_name) {
        free(hdrpolicy_name);
    }
    return ret;
}

int setDisplayHDCPContentType(ENUM_DISPLAY_HDCP_Content_Type HDCPType, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    int connId = -1;
    int fd = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    DEBUG(" %s %d westeros set hdcp content type %d",__FUNCTION__,__LINE__,HDCPType);
    connId = meson_drm_GetConnectorId(connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_HDCP_VERSION);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", connId, prop_name, HDCPType);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayDvEnable(int dvEnable, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    uint32_t crtcId = -1;
    char* prop_name = NULL;
    DEBUG(" %s %d westeros set DvEnable %d",__FUNCTION__,__LINE__,dvEnable);
    crtcId = meson_drm_GetCrtcId(connType);
    if (crtcId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_DOLBY_VISION_ENABLE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", crtcId, prop_name, dvEnable);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
       } else {
           ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayActive(int active, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    uint32_t crtcId = -1;
    char* prop_name = NULL;
    DEBUG(" %s %d westeros set active %d",__FUNCTION__,__LINE__,active);
    crtcId = meson_drm_GetCrtcId(connType);
    if (crtcId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_ACTIVE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", crtcId, prop_name, active);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
       } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
       }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayVrrEnabled(int VrrEnable, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    uint32_t crtcId = -1;
    char* prop_name = NULL;
    DEBUG(" %s %d westeros set VrrEnable %d",__FUNCTION__,__LINE__,VrrEnable);
    crtcId = meson_drm_GetCrtcId(connType);
    if ( crtcId > 0 ) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_VRR_ENABLED);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", crtcId, prop_name, VrrEnable);
        rc = wstDisplaySendMessage(cmdBuf);
        if ( rc >= 0 ) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayMode(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    char modeSet[CMDBUF_SIZE] = {'\0'};
    int rc = -1;
    DEBUG("%s %d westeros set modeInfo %dx%d%s%dhz",__FUNCTION__,__LINE__, modeInfo->w, modeInfo->h, (modeInfo->interlace == 0? "p":"i") , modeInfo->vrefresh);
    snprintf(modeSet, sizeof(modeSet)-1, "set mode %dx%d%s%d", modeInfo->w, modeInfo->h, (modeInfo->interlace == 0? "p":"i"), modeInfo->vrefresh);
    rc = wstDisplaySendMessage(modeSet);
    if ( rc >= 0 ) {
        ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        return ret;
    }
    return ret;
}

int setDisplayAutoMode(DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int rc = -1;
    char modeSet[CMDBUF_SIZE] = {'\0'};
    DEBUG(" %s %d westeros set auto mode",__FUNCTION__,__LINE__);
    snprintf(modeSet, sizeof(modeSet)-1, "set mode %s", "auto");
    rc = wstDisplaySendMessage(modeSet);
    if ( rc >= 0 ) {
        ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        return ret;
    }
    return ret;
}

int setDisplayAspectRatioValue(ENUM_DISPLAY_ASPECT_RATIO ASPECTRATIO, DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int connId = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    connId = meson_drm_GetConnectorId(connType);
    DEBUG(" %s %d westeros set aspect ratio Value %d connId %d connType %d",__FUNCTION__,__LINE__,
                                               ASPECTRATIO, connId, connType);
    if (connId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_ASPECT_RATIO);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", connId, prop_name, ASPECTRATIO);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetConnectorId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

int setDisplayScaling(int value) {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    DEBUG("%s %d westeros set scaling value %d",__FUNCTION__,__LINE__, value);
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "set scaling %d", value);
    rc = wstDisplaySendMessage(cmdBuf);
    if (rc >= 0) {
          ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}

int getDisplayScaling() {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "get scaling");
    rc = wstDisplaySendMessage(cmdBuf);
    if (rc >= 0) {
          ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}

int getDisplayEnabled() {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "get display enable");
    rc = wstDisplaySendMessage(cmdBuf);
    if (rc >= 0) {
          ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}

int setDisplayEnabled(int enabled) {
    int ret = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    DEBUG("%s %d westeros set enabled value %d",__FUNCTION__,__LINE__, enabled);
    snprintf(cmdBuf, sizeof(cmdBuf)-1, "set display enable %d", enabled);
    rc = wstDisplaySendMessage(cmdBuf);
    if (rc >= 0) {
          ret = 0;
    } else {
        ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
    }
    return ret;
}

int setDisplayDVMode(int dvmode,DISPLAY_CONNECTOR_TYPE connType) {
    int ret = -1;
    int crtcId = -1;
    int rc = -1;
    char cmdBuf[CMDBUF_SIZE] = {'\0'};
    char* prop_name = NULL;
    DEBUG("%s %d westeros set dv mode %d",__FUNCTION__,__LINE__,dvmode);
    crtcId = meson_drm_GetCrtcId(connType);
    if (crtcId > 0) {
        prop_name = meson_drm_GetPropName(ENUM_MESON_DRM_PROP_DV_MODE);
        if (prop_name == NULL) {
            ERROR("%s %d meson_drm_GetPropName return NULL",__FUNCTION__,__LINE__);
            goto out;
        }
        DEBUG("%s %d get prop name %s",__FUNCTION__,__LINE__, prop_name);
        snprintf(cmdBuf, sizeof(cmdBuf)-1, "set property -s %d:%s:%d", crtcId, prop_name, dvmode);
        rc = wstDisplaySendMessage(cmdBuf);
        if (rc >= 0) {
            ret = 0;
        } else {
            ERROR("%s %d send message fail",__FUNCTION__,__LINE__);
        }
    } else {
        ERROR("%s %d meson_drm_GetCrtcId return fail",__FUNCTION__,__LINE__);
    }
out:
    if (prop_name) {
        free(prop_name);
    }
    return ret;
}

