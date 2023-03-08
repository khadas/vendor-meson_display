#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <drm.h>
#include <drm_fourcc.h>
#include <sys/mman.h>
#include <pthread.h>
#include "xf86drm.h"
#include "xf86drmMode.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/string.h>
#include "display_settings.h"

void display_event( ENUM_MESONDISPLAY_EVENT enEvent, void *eventData/*Optional*/)
{
    printf("\n MESONDISPLAY_EVENT_CONNECTED    = 0,//!< Display connected event.\n"
               "MESONDISPLAY_EVENT_DISCONNECTED //!< Display disconnected event.\n"
               "Eevent:%d\n",enEvent);
}

int main(void)
{
    printf("\n 0->set hdmi mode 1->set cvbs mode 2-> event test\n");
    int select_s_g = 0;
    DisplayMode* modeInfo = NULL;
    modeInfo = (DisplayMode*)malloc(sizeof(DisplayMode));

    scanf("%d",&select_s_g);
    if (select_s_g == 0) {
        printf("please input modeInfo:interlace, w, h, vrefresh\n");
        scanf("%d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
        if (setDisplayMode(modeInfo,MESON_CONNECTOR_HDMIA) == 0) {
            printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
        }else{
            printf("setDisplayModeFail\n");
        }
    } else if(select_s_g == 1){
        printf("please input modeInfo:interlace, w, h, vrefresh\n");
        scanf("%d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
        if (setDisplayMode(modeInfo,MESON_CONNECTOR_CVBS) == 0) {
            printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
        }else{
            printf("setDisplayModeFail\n");
        }
    } else {
        registerMesonDisplayEventCallback(display_event);
        startMesonDisplayUeventMonitor();
    }
    return 0;
}



