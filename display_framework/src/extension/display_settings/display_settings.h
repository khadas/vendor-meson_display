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

typedef enum _ENUM_MESON_CONNECTOR_TYPE {
    MESON_CONNECTOR_HDMIA  = 0,
    MESON_CONNECTOR_HDMIB,
    MESON_CONNECTOR_LVDS,
    MESON_CONNECTOR_CVBS,
    MESON_CONNECTOR_RESERVED
} MESON_CONNECTOR_TYPE;

typedef struct _DisplayMode {
    uint16_t w;
    uint16_t h;
    uint32_t vrefresh;
    bool interlace;
} DisplayMode;

typedef enum _ENUM_MESONDISPLAY_EVENT {
    MESONDISPLAY_EVENT_CONNECTED    = 0,//!< Display connected event.\n"
    MESONDISPLAY_EVENT_DISCONNECTED , //!< Display disconnected event.\n"
}ENUM_MESONDISPLAY_EVENT;

int setDisplayMode(DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType);
typedef void (*mesonDisplayEventCallback)(ENUM_MESONDISPLAY_EVENT enEvent, void *eventData/*Optional*/);
bool registerMesonDisplayEventCallback(mesonDisplayEventCallback cb);
void startMesonDisplayUeventMonitor();
void stopMesonDisplayUeventMonitor();
#if defined(__cplusplus)
}
#endif

#endif