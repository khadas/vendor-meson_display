 /*
 * Copyright (c) 2023 Amlogic, Inc. All rights reserved.
 * Description:
 */

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
#include "../display_settings.h"

void display_event( ENUM_MESON_DISPLAY_EVENT enEvent, void *eventData/*Optional*/)
{
    printf("\n MESON_DISPLAY_EVENT_CONNECTED   = 0,//!< Display connected event.\n"
               "MESON_DISPLAY_EVENT_DISCONNECTED, //!< Display disconnected event.\n"
               "MESON_DISPLAY_HDCP_AUTHENTICATED,//!< HDCP authenticate success.\n"
               "MESON_DISPLAY_HDCP_AUTHENTICATIONFAILURE \n"
               "Eevent:%d\n",enEvent);
}

int main()
{
    printf("\n 0->set  1->get\n");
    int select_s = 0;
    int select_len = 0;
    int get = 0;
    int set = 0;
    int len = 0;
    DisplayModeInfo* modeInfo;
    modeInfo= (DisplayModeInfo*)malloc(sizeof(DisplayModeInfo));
    select_len = scanf("%d",&select_s);
    if (select_s == 0 && select_len == 1) {
        printf("The current weston set API is not developed\n");
    }
    else if(select_s == 1 && select_len == 1) {
        printf("get:0->hdrPolicy 1->modeinfo 2->HDCP version 3->HDMI connected 4->color depth 5->color space"
         " 6->EDID 7->hdcp auth status 8->supportedModesList 9->prefer mode 10->HDCP Content Type 11->Content Type"
         " 12->Dv Enable 13->active 14->vrr Enable 15->av mute 16->hdr mode 17->CvbsModesList 18-> mode support check"
         "19->current aspect ratio 20->event test 21->frac rate policy 22->scaling 23->Supported dvmode"
         " 24->hdr supportedlist 25->DvCap 26->display enabled 27->dpms status 28->mode support attrlist 29->framrate"
         " 30->primar plane fb size 31->physical size 32->Timing information 33->dv mode 34->rx supported hdcp version\n");
        len = scanf("%d",&get);
        if (get == 0 && len == 1) {
            ENUM_DISPLAY_HDR_POLICY value = getDisplayHDRPolicy( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_HDR_POLICY_FOLLOW_SINK = 0 \n"
            "DISPLAY_HDR_POLICY_FOLLOW_SOURCE = 1 \n  value:%d\n", value);
       } else if(get == 1 && len == 1) {
            if (getDisplayMode( modeInfo, DISPLAY_CONNECTOR_HDMIA) == 0) {
                printf("\n mode (%d %d %d %d)\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            } else {
                printf("\n getDisplayModeFail\n");
            }
            if (modeInfo)
                free(modeInfo);
        } else if(get == 2 && len == 1) {
            ENUM_DISPLAY_HDCP_VERSION value = getDisplayHdcpVersion( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_HDCP_14      = 0\n"
                     " DISPLAY_HDCP_22      = 1\n value:%d \n", value);
        } else if(get == 3 && len == 1) {
            ENUM_DISPLAY_CONNECTION value = getDisplayConnectionStatus( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_DISCONNECTED      = 0\n"
                     " DISPLAY_CONNECTED         = 1\n value:%d \n",value);
        } else if (get == 4 && len == 1) {
            int value = getDisplayColorDepth( DISPLAY_CONNECTOR_HDMIA);
            printf("\n color depth:%d\n",value);
        } else if (get == 5 && len == 1) {
            ENUM_DISPLAY_COLOR_SPACE value = getDisplayColorSpace( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_COLOR_SPACE_RGB      = 0 \n"
                      "DISPLAY_COLOR_SPACE_YCBCR422 = 1 \n"
                      "DISPLAY_COLOR_SPACE_YCBCR444 = 2 \n"
                      "DISPLAY_COLOR_SPACE_YCBCR420 = 3 \n value:%d\n"
                      , value);
        } else if (get == 6 && len == 1) {
            int len = 0;
            char *edid = NULL;
            int i;
            getDisplayEDIDData(DISPLAY_CONNECTOR_HDMIA, &len, &edid );
            printf("\n EDID data len:%d\n", len);
            for (i = 0; i < len; i++) {
                if (i % 16 == 0)
                    printf("\n\t\t\t");
                if (edid)
                    printf("%.2hhx", edid[i]);
            }
            printf("\n");
            if (edid)
                free(edid);
        } else if (get == 7&& len == 1) {
            ENUM_DISPLAY_HDCPAUTH_STATUS value = getDisplayHdcpAuthStatus( DISPLAY_CONNECTOR_HDMIA);
             printf("\n DISPLAY_AUTH_STATUS_FAIL      = 0 \n"
                      " DISPLAY_AUTH_STATUS_SUCCESS = 1 \n value:%d\n", value);
        } else if (get == 8 && len == 1) {
            int count = 0;
            DisplayModeInfo* modeInfo = NULL;
            if (0 == getDisplayModesList( &modeInfo, &count,DISPLAY_CONNECTOR_HDMIA )) {
                printf("\n mode count:%d\n",count);
                int i = 0;
                for (int i=0; i<count; i++) {
                    printf("mode (%s %d %d %d %d)\n", modeInfo[i].name, modeInfo[i].w, modeInfo[i].h, modeInfo[i].interlace,modeInfo[i].vrefresh);
                }
                if (modeInfo)
                    free(modeInfo);
            } else {
                 printf("\n %s fail\n",__FUNCTION__);
            }
        } else if (get == 9 && len == 1) {
            DisplayModeInfo mode;
            if (0 == getDisplayPreferMode(&mode,DISPLAY_CONNECTOR_HDMIA)) {
                printf(" (%s %d %d %d)\n", mode.name, mode.w, mode.h, mode.interlace);
            } else {
                 printf("\n %s fail\n",__FUNCTION__);
            }
        } else if (get == 10 && len == 1) {
            ENUM_DISPLAY_HDCP_Content_Type value = getDisplayHDCPContentType( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_HDCP_Type0      = 0 \n"
                      " DISPLAY_HDCP_Type1 = 1 \n value:%d\n", value);
        } else if (get == 11 && len == 1) {
            ENUM_DISPLAY_Content_Type value = getDisplayContentType( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_Content_Type_Data      = 0 \n"
                      "DISPLAY_Content_Type_Graphics = 1 \n"
                      "DISPLAY_Content_Type_Photo = 2 \n"
                      "DISPLAY_Content_Type_Cinema = 3 \n"
                      "DISPLAY_Content_Type_Game = 4 \n value:%d\n"
                      , value);
        } else if (get == 12 && len == 1) {
            int value = getDisplayDvEnable( DISPLAY_CONNECTOR_HDMIA );
            printf("\n DvEnable:%d\n",value);
            if (value == 1) {
                printf("Support Dolbyvision\n");
            } else {
                printf("Dolbyvision not supported\n");
            }
        } else if (get == 13 && len == 1) {
            uint32_t value = getDisplayActive( DISPLAY_CONNECTOR_HDMIA );
            printf("\n Active:%d\n",value);
        } else if (get == 14 && len == 1) {
            int value = getDisplayVrrEnabled( DISPLAY_CONNECTOR_HDMIA );
            printf("\n VrrEnabled:%d\n",value);
        } else if (get == 15 && len == 1) {
            int value = getDisplayAVMute( DISPLAY_CONNECTOR_HDMIA );
            printf("\n AVMute:%d\n",value);
        } else if (get == 16 && len == 1) {
            ENUM_DISPLAY_HDR_MODE value = getDisplayHdrStatus( DISPLAY_CONNECTOR_HDMIA );
            printf("\n MESON_DISPLAY_HDR10PLUS      = 0 \n"
                     " MESON_DISPLAY_DolbyVision_STD    \n"
                     " MESON_DISPLAY_DolbyVision_Lowlatency    \n"
                     " MESON_DISPLAY_HDR10_ST2084    \n"
                     " MESON_DISPLAY_HDR10_TRADITIONAL    \n"
                     " MESON_DISPLAY_HDR_HLG    \n"
                     " MESON_DISPLAY_SDR    \n value:%d\n"
                     , value);
        } else if (get == 17 && len == 1) {
            int count = 0;
            DisplayModeInfo* modeInfo = NULL;
            if (getDisplayModesList( &modeInfo, &count,DISPLAY_CONNECTOR_CVBS ) == 0) {
                printf("\n mode count:%d\n",count);
                int i = 0;
                for (int i=0; i< count; i++) {
                    printf(" (%s %d %d %d %d)\n", modeInfo[i].name, modeInfo[i].w, modeInfo[i].h, modeInfo[i].interlace,modeInfo[i].vrefresh);
                }
                if (modeInfo)
                    free(modeInfo);
            } else {
                 printf("\n %s get Display cvbs ModesList fail\n",__FUNCTION__);
            }
        }
        else if (get == 18 && len == 1) {
            char mode[32] = {'\0'};
            int color_space = 255;
            int color_depth = 255;
            bool ret = false;
            printf("\n please input mode name:\n");
            scanf("%s", mode);
            printf("\n please input color space:\n");
            scanf("%d", &color_space);
            printf("\n please input color depth:\n");
            scanf("%d", &color_depth);
            ret = modeAttrSupportedCheck(mode, (ENUM_DISPLAY_COLOR_SPACE)color_space,
            color_depth, DISPLAY_CONNECTOR_HDMIA);
            printf("\n mode:%s color attr %d, depth %dbit, SupportedCheck:%d\n",
                mode, color_space, color_depth, ret);
        } else if (get == 19 && len == 1) {
            int value = getDisplayAspectRatioValue(DISPLAY_CONNECTOR_HDMIA);
            if (value == 0) {
                printf("\n current mode do not support aspect ratio change\n");
            } else if (value == 1) {
                printf("\n current aspect ratio is 4:3 and you can switch to 16:9\n");
            } else if (value == 2) {
                printf("\n  current aspect ratio is 16:9 and you can switch to 4:3\n");
            } else {
                printf("\n invalid value\n");
            }
        } else if(get == 20 && len == 1) {
            registerMesonDisplayEventCallback(display_event);
            startMesonDisplayUeventMonitor();
            while (1)
            {
                usleep(20000);
            }
        } else if (get == 21 && len == 1) {
            int value = getDisplayFracRatePolicy( DISPLAY_CONNECTOR_HDMIA );
            if (value == -1) {
                printf("\n invalid value\n");
            } else {
                printf("\n FracRate: %d\n",value);
            }
        } else if (get == 22 && len == 1) {
            printf("The current weston set API is not developed\n");
        } else if (get == 23 && len == 1) {
            int value = getDisplaySupportedDvMode(DISPLAY_CONNECTOR_HDMIA);
            printf("getDisplaySupportedDvMode %d\n",value);
        } else if (get == 24 && len == 1) {
            uint32_t value  = getDisplayHDRSupportList(DISPLAY_CONNECTOR_HDMIA);
            printf("\n value %d\n",value);
            if (value & 0x1)
                printf("\n MESON_DRM_HDR10PLUS\n");
            if (value & 0x2)
                printf("\n MESON_DRM_DOLBYVISION_STD\n");
            if (value & 0x4)
                printf("\n MESON_DRM_DOLBYVISION_LL\n");
            if (value & 0x8)
                printf("\n MESON_DRM_HDR10_ST2084\n");
            if (value & 0x10)
                printf("\n MESON_DRM_HDR10_TRADITIONAL\n");
            if (value & 0x20)
                printf("\n MESON_DRM_HDR_HLG\n");
            if (value & 0x40)
                printf("\n MESON_DRM_SDR\n");
       } else if (get == 25 && len == 1) {
            int value = getDisplayDvCap( DISPLAY_CONNECTOR_HDMIA );
            if (value == 0) {
                printf("The Rx don't support DolbyVision\n");
            } else {
                printf("\n DvCap:%d\n",value);
            }
        } else if (get == 26 && len == 1) {
           printf("The current weston set API is not developed\n");
        } else if (get == 27 && len == 1) {
            int value = getDisplayDpmsStatus( DISPLAY_CONNECTOR_HDMIA );
            printf("\n get dpms status: %d\n",value);
        } else if(get == 28 && len == 1) {
            int num = getDisplaySupportAttrList( modeInfo, DISPLAY_CONNECTOR_HDMIA);
            if (num == 0) {
                printf("\n getDisplaySupportAttrList Success");
            } else {
                printf("\n getDisplaySupportAttrList Fail");
            }
            if (modeInfo)
                free(modeInfo);
        } else if(get == 29 && len == 1) {
            float value = getDisplayFrameRate( DISPLAY_CONNECTOR_HDMIA);
            printf("\n get framrate %.2f",value);
        } else if(get == 30 && len == 1) {
            int width = 0;
            int height = 0;
            int value = getDisplayPlaneSize( &width,&height );
            if (value == 0 ) {
               printf("\n get graphic plane Size width = %d, height = %d\n",width,height);
           } else {
               printf("\n getDisplayPlaneSize fail\n");
           }
        } else if(get == 31 && len == 1) {
            int width = 0;
            int height = 0;
            int value  = getDisplayPhysicalSize( &width, &height, DISPLAY_CONNECTOR_HDMIA );
            if (value == 0 ) {
               printf("\n get physical Size width = %d, height = %d\n",width,height);
           } else {
               printf("\n getDisplayPhysicalSize fail\n");
           }
        } else if(get ==32 && len == 1) {
           uint16_t htotal = 0;
           uint16_t vtotal = 0;
           uint16_t hstart = 0;
           uint16_t vstart = 0;
            if (getDisplaySignalTimingInfo(&htotal,&vtotal, &hstart,&vstart, DISPLAY_CONNECTOR_HDMIA) == 0) {
                printf("htotal: %d vtotal: %d hstart: %d vstart: %d\n", htotal, vtotal,
                                  hstart,vstart);
            } else {
                printf("\n getDisplaySignalTimingInfo fail\n");
            }
        } else if (get == 33 && len == 1) {
            int value = getDisplayDvMode( DISPLAY_CONNECTOR_HDMIA );
            if (value == -1) {
                printf("\n get dv mode fail\n");
            } else  {
                printf("\n get dv mode value: %d\n",value);
            }
        } else if (get == 34 && len == 1) {
            int value = getDisplayRxSupportedHdcpVersion(DISPLAY_CONNECTOR_HDMIA );
            if (value & 0x1) {
               printf("\nRX HDCP 1.4 supported \n");
               if (value & 0x2) {
                   printf("RX HDCP 2.2 supported as well\n");
               } else {
                   printf("RX HDCP 2.2 not supported\n");
               }
           } else {
               if (!(value & 0x2)) {
                   printf("\n get_prop fail\n");
               }
           }
        }
    }
    else {
        printf("\n Incorrect input method\n");
    }
    return 0;
}


