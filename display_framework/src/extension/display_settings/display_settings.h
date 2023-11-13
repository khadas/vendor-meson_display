 /*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 * Description:
 */

#ifndef DISPLAY_SETTINGS_H_
#define DISPLAY_SETTINGS_H_
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <xf86drmMode.h>
#if defined(__cplusplus)
extern "C" {
#endif
#define MESON_DISPLAY_MODE_LEN 32

typedef enum _ENUM_DISPLAY_ASPECT_RATIO {
    DISPLAY_ASPECT_RATIO_AUTOMATIC  = 0, //current mode do not support aspect ratio change
    DISPLAY_ASPECT_RATIO_4_3,
    DISPLAY_ASPECT_RATIO_16_9,
    DISPLAY_ASPECT_RATIO_RESERVED
} ENUM_DISPLAY_ASPECT_RATIO;

typedef enum _ENUM_DISPLAY_HDR_MODE {
    MESON_DISPLAY_HDR10PLUS      = 0,
    MESON_DISPLAY_DolbyVision_STD,
    MESON_DISPLAY_DolbyVision_Lowlatency,
    MESON_DISPLAY_HDR10_ST2084,
    MESON_DISPLAY_HDR10_TRADITIONAL,
    MESON_DISPLAY_HDR_HLG,
    MESON_DISPLAY_SDR
} ENUM_DISPLAY_HDR_MODE;

typedef enum _ENUM_DISPLAY_Content_Type {
    DISPLAY_Content_Type_Data      = 0,
    DISPLAY_Content_Type_Graphics,
    DISPLAY_Content_Type_Photo,
    DISPLAY_Content_Type_Cinema,
    DISPLAY_Content_Type_Game,
    DISPLAY_Content_Type_RESERVED
} ENUM_DISPLAY_Content_Type;

/*HDCP transmission time divided into Type0&Type1 content*/
typedef enum _ENUM_DISPLAY_HDCP_Content_Type{
    DISPLAY_HDCP_Type0 = 0,  //Type0 represents support for both 1.4 and 2.2
    DISPLAY_HDCP_Type1,   //Type1 represents only support for 2.2
    DISPLAY_HDCP_Type_RESERVED
} ENUM_DISPLAY_HDCP_Content_Type;

typedef enum _ENUM_DISPLAY_HDCP_AUTH_STATUS {
    DISPLAY_AUTH_STATUS_FAIL      = 0,
    DISPLAY_AUTH_STATUS_SUCCESS
} ENUM_DISPLAY_HDCPAUTH_STATUS;

typedef enum _ENUM_DISPLAY_COLOR_SPACE {
    DISPLAY_COLOR_SPACE_RGB      = 0,
    DISPLAY_COLOR_SPACE_YCBCR422,
    DISPLAY_COLOR_SPACE_YCBCR444,
    DISPLAY_COLOR_SPACE_YCBCR420,
    DISPLAY_COLOR_SPACE_RESERVED
} ENUM_DISPLAY_COLOR_SPACE;

typedef enum _ENUM_DISPLAY_CONNECTION{
    DISPLAY_DISCONNECTED      = 0,
    DISPLAY_CONNECTED         = 1,
    DISPLAY_UNKNOWNCONNECTION = 2
} ENUM_DISPLAY_CONNECTION;

typedef enum _ENUM_DISPLAY_HDCP_VERSION {
    DISPLAY_HDCP_14      = 0,
    DISPLAY_HDCP_22,
    DISPLAY_HDCP_RESERVED
} ENUM_DISPLAY_HDCP_VERSION;

typedef enum _ENUM_DISPLAY_CONNECTOR_TYPE {
    DISPLAY_CONNECTOR_HDMIA  = 0,
    DISPLAY_CONNECTOR_HDMIB,
    DISPLAY_CONNECTOR_LVDS,
    DISPLAY_CONNECTOR_CVBS,
    DISPLAY_CONNECTOR_DUMMY,
    DISPLAY_CONNECTOR_RESERVED
} DISPLAY_CONNECTOR_TYPE;

typedef struct _DisplayModeInfo {
    uint16_t w;  //<--Number of horizontal pixels in the effective display area-->//
    uint16_t h;   //<--Number of vertical pixels in the effective display area-->//
    uint32_t vrefresh;  //<--Display refresh rate--->//
    bool interlace;  //<--Indicates which scanning form to choose, P represents progressive scanning,-->//
                     //<--and i represents interlaced scanning;The default interlace value is 0 for P 1 for i-->//
    char name[MESON_DISPLAY_MODE_LEN];
} DisplayModeInfo;

typedef enum _ENUM_MESON_DISPLAY_EVENT {
    MESON_DISPLAY_EVENT_CONNECTED    = 0,//!< Display connected event.\n"
    MESON_DISPLAY_EVENT_DISCONNECTED , //!< Display disconnected event.\n"
}ENUM_MESON_DISPLAY_EVENT;

typedef enum _ENUM_DISPLAY_HDR_POLICY {
    DISPLAY_HDR_POLICY_FOLLOW_SINK      = 0,  //<--Always  HDR-->//
    DISPLAY_HDR_POLICY_FOLLOW_SOURCE     //<--Adaptive  HDR-->//
} ENUM_DISPLAY_HDR_POLICY;

int setDisplayHDCPEnable(bool enable, DISPLAY_CONNECTOR_TYPE connType);
void getDisplayEDIDData(DISPLAY_CONNECTOR_TYPE connType, int * data_Len, char **data );
int getDisplayAVMute(DISPLAY_CONNECTOR_TYPE connType );
int setDisplayAVMute(int mute, DISPLAY_CONNECTOR_TYPE connType);
ENUM_DISPLAY_HDCPAUTH_STATUS getDisplayHdcpAuthStatus(DISPLAY_CONNECTOR_TYPE connType );
ENUM_DISPLAY_COLOR_SPACE getDisplayColorSpace(DISPLAY_CONNECTOR_TYPE connType);
ENUM_DISPLAY_CONNECTION getDisplayConnectionStatus(DISPLAY_CONNECTOR_TYPE connType);
ENUM_DISPLAY_HDCP_VERSION getDisplayHdcpVersion(DISPLAY_CONNECTOR_TYPE connType );
ENUM_DISPLAY_HDR_POLICY getDisplayHDRPolicy(DISPLAY_CONNECTOR_TYPE connType);
int setDisplayHDRPolicy(ENUM_DISPLAY_HDR_POLICY hdrPolicy, DISPLAY_CONNECTOR_TYPE connType);
uint32_t getDisplayColorDepth(DISPLAY_CONNECTOR_TYPE connType);
int getDisplayMode(DisplayModeInfo* modeInfo, DISPLAY_CONNECTOR_TYPE connType);
int setDisplayMode(DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType);
typedef void (*mesonDisplayEventCallback)(ENUM_MESON_DISPLAY_EVENT enEvent, void *eventData/*Optional*/);
bool registerMesonDisplayEventCallback(mesonDisplayEventCallback cb);
void startMesonDisplayUeventMonitor();
void stopMesonDisplayUeventMonitor();
int getDisplayModesList(DisplayModeInfo** modeInfo, int* modeCount ,DISPLAY_CONNECTOR_TYPE connType);
int getDisplayPreferMode( DisplayModeInfo* modeInfo,DISPLAY_CONNECTOR_TYPE connType);
int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                                                    DISPLAY_CONNECTOR_TYPE connType);
int setDisplayHDCPContentType(ENUM_DISPLAY_HDCP_Content_Type HDCPType, DISPLAY_CONNECTOR_TYPE connType);
ENUM_DISPLAY_Content_Type getDisplayContentType(DISPLAY_CONNECTOR_TYPE connType);
int setDisplayDvEnable(int dvEnable, DISPLAY_CONNECTOR_TYPE connType);
int getDisplayDvEnable(DISPLAY_CONNECTOR_TYPE connType );
int setDisplayActive(int active, DISPLAY_CONNECTOR_TYPE connType);
int getDisplayActive(DISPLAY_CONNECTOR_TYPE connType );
int setDisplayVrrEnabled(int VrrEnable, DISPLAY_CONNECTOR_TYPE connType);
int getDisplayVrrEnabled(DISPLAY_CONNECTOR_TYPE connType );
ENUM_DISPLAY_HDR_MODE getDisplayHdrStatus(DISPLAY_CONNECTOR_TYPE connType );
int setDisplayAutoMode(DISPLAY_CONNECTOR_TYPE connType);
int setDisplayVideoZorder(  unsigned int index, unsigned int zorder, unsigned int flag);
ENUM_DISPLAY_ASPECT_RATIO getDisplayAspectRatioValue(DISPLAY_CONNECTOR_TYPE connType );
int setDisplayAspectRatioValue(ENUM_DISPLAY_ASPECT_RATIO ASPECTRATIO, DISPLAY_CONNECTOR_TYPE connType);
int setDisplayModeAttr(DisplayModeInfo* modeInfo,uint32_t colorDepth,
                     ENUM_DISPLAY_COLOR_SPACE colorSpace,DISPLAY_CONNECTOR_TYPE connType);

int display_meson_get_open();
int display_meson_set_open();

void display_meson_close(int fd);
bool modeAttrSupportedCheck(char* modeName, ENUM_DISPLAY_COLOR_SPACE colorSpace,
                          uint32_t colorDepth, DISPLAY_CONNECTOR_TYPE connType );

#if defined(__cplusplus)
}
#endif

#endif
