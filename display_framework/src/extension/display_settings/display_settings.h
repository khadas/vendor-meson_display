#ifndef MESON_DRM_SETTINGS_H_
#define MESON_DRM_SETTINGS_H_
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <xf86drmMode.h>
#if defined(__cplusplus)
extern "C" {
#endif
#define DRM_DISPLAY_MODE_LEN 32

typedef enum _ENUM_MESON_HDR_MODE {
    MESON_HDR10PLUS      = 0,
    MESON_DOLBYVISION_STD,
    MESON_DOLBYVISION_LL,
    MESON_HDR10_ST2084,
    MESON_HDR10_TRADITIONAL,
    MESON_HDR_HLG,
    MESON_SDR
} ENUM_MESON_HDR_MODE;

typedef enum _ENUM_MESON_Content_Type {
    MESON_Content_Type_Data      = 0,
    MESON_Content_Type_Graphics,
    MESON_Content_Type_Photo,
    MESON_Content_Type_Cinema,
    MESON_Content_Type_Game,
    MESON_Content_Type_RESERVED
} ENUM_MESON_Content_Type;

/*HDCP transmission time divided into Type0&Type1 content*/
typedef enum _ENUM_MESON_HDCP_Content_Type{
    MESON_HDCP_Type0 = 0,  //Type0 represents support for both 1.4 and 2.2
    MESON_HDCP_Type1,   //Type1 represents only support for 2.2
    MESON_HDCP_Type_RESERVED
} ENUM_MESON_HDCP_Content_Type;

typedef enum _ENUM_MESON_HDCP_AUTH_STATUS {
    MESON_AUTH_STATUS_FAIL      = 0,
    MESON_AUTH_STATUS_SUCCESS
} ENUM_MESON_HDCPAUTH_STATUS;

typedef enum _ENUM_MESON_COLOR_SPACE {
    MESON_COLOR_SPACE_RGB      = 0,
    MESON_COLOR_SPACE_YCBCR422,
    MESON_COLOR_SPACE_YCBCR444,
    MESON_COLOR_SPACE_YCBCR420,
    MESON_COLOR_SPACE_RESERVED
} ENUM_MESON_COLOR_SPACE;

typedef enum {
    MESON_DISCONNECTED      = 0,
    MESON_CONNECTED         = 1,
    MESON_UNKNOWNCONNECTION = 2
} ENUM_MESON_CONN_CONNECTION;

typedef enum _ENUM_MESON_HDCP_VERSION {
    MESON_HDCP_14      = 0,
    MESON_HDCP_22,
    MESON_HDCP_RESERVED
} ENUM_MESON_HDCP_VERSION;

typedef enum _ENUM_MESON_CONNECTOR_TYPE {
    MESON_CONNECTOR_HDMIA  = 0,
    MESON_CONNECTOR_HDMIB,
    MESON_CONNECTOR_LVDS,
    MESON_CONNECTOR_CVBS,
    MESON_CONNECTOR_DUMMY,
    MESON_CONNECTOR_RESERVED
} MESON_CONNECTOR_TYPE;

typedef struct _DisplayMode {
    uint16_t w;  //<--Number of horizontal pixels in the effective display area-->//
    uint16_t h;   //<--Number of vertical pixels in the effective display area-->//
    uint32_t vrefresh;  //<--Display refresh rate--->//
    bool interlace;  //<--Indicates which scanning form to choose, P represents progressive scanning, and i represents interlaced scanning; The default interlace value is 0 for P 1 for i-->//
    char name[DRM_DISPLAY_MODE_LEN];
} DisplayMode;

typedef enum _ENUM_MESONDISPLAY_EVENT {
    MESONDISPLAY_EVENT_CONNECTED    = 0,//!< Display connected event.\n"
    MESONDISPLAY_EVENT_DISCONNECTED , //!< Display disconnected event.\n"
}ENUM_MESONDISPLAY_EVENT;

typedef enum _ENUM_MESON_HDR_POLICY {
    MESON_HDR_POLICY_FOLLOW_SINK      = 0,  //<--Always  HDR-->//
    MESON_HDR_POLICY_FOLLOW_SOURCE     //<--Adaptive  HDR-->//
} ENUM_MESON_HDR_POLICY;

int setDisplayHDCPEnable(bool enable, MESON_CONNECTOR_TYPE connType);
void getDisplayEDIDData(MESON_CONNECTOR_TYPE connType, int * data_Len, char **data );
int getDisplayAVMute(MESON_CONNECTOR_TYPE connType );
int setDisplayAVMute(int mute, MESON_CONNECTOR_TYPE connType);
ENUM_MESON_HDCPAUTH_STATUS getDisplayHdcpAuthStatus(MESON_CONNECTOR_TYPE connType );
ENUM_MESON_COLOR_SPACE getDisplayColorSpace(MESON_CONNECTOR_TYPE connType);
ENUM_MESON_CONN_CONNECTION getDisplayConnectionStatus(MESON_CONNECTOR_TYPE connType);
ENUM_MESON_HDCP_VERSION getDisplayHdcpVersion(MESON_CONNECTOR_TYPE connType );
ENUM_MESON_HDR_POLICY getDisplayHDRPolicy(MESON_CONNECTOR_TYPE connType);
int setDisplayHDRPolicy(ENUM_MESON_HDR_POLICY hdrPolicy, MESON_CONNECTOR_TYPE connType);
uint32_t getDisplayColorDepth(MESON_CONNECTOR_TYPE connType);
int getDisplayMode(DisplayMode* modeInfo, MESON_CONNECTOR_TYPE connType);
int setDisplayMode(DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType);
typedef void (*mesonDisplayEventCallback)(ENUM_MESONDISPLAY_EVENT enEvent, void *eventData/*Optional*/);
bool registerMesonDisplayEventCallback(mesonDisplayEventCallback cb);
void startMesonDisplayUeventMonitor();
void stopMesonDisplayUeventMonitor();
int getDisplayModesList(DisplayMode** modeInfo, int* modeCount ,MESON_CONNECTOR_TYPE connType);
int getDisplayPreferMode( DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType);
int setDisplayColorSpacedDepth(uint32_t colorDepth, ENUM_MESON_COLOR_SPACE colorSpace, MESON_CONNECTOR_TYPE connType);
int setDisplayHDCPContentType(ENUM_MESON_HDCP_Content_Type HDCPType, MESON_CONNECTOR_TYPE connType);
ENUM_MESON_Content_Type getDisplayContentType(MESON_CONNECTOR_TYPE connType);
int setDisplayDvEnable(int dvEnable, MESON_CONNECTOR_TYPE connType);
int getDisplayDvEnable(MESON_CONNECTOR_TYPE connType );
int setDisplayActive(int active, MESON_CONNECTOR_TYPE connType);
int getDisplayActive(MESON_CONNECTOR_TYPE connType );
int setDisplayVrrEnabled(int VrrEnable, MESON_CONNECTOR_TYPE connType);
int getDisplayVrrEnabled(MESON_CONNECTOR_TYPE connType );
ENUM_MESON_HDR_MODE getDisplayHdrStatus(MESON_CONNECTOR_TYPE connType );
int setDisplayAutoMode(MESON_CONNECTOR_TYPE connType);

int display_meson_get_open();
int display_meson_set_open();

void display_meson_close(int fd);

#if defined(__cplusplus)
}
#endif

#endif
