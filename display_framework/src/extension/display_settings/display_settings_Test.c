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

int main()
{
    printf("\n 0->set  1->get\n");
    int select_s = 0;
    int select_len = 0;
    int get = 0;
    int set = 0;
    int len = 0;
    DisplayMode* modeInfo;
    modeInfo= (DisplayMode*)malloc(sizeof(DisplayMode));
    select_len = scanf("%d",&select_s);
    if (select_s == 0 && select_len == 1) {
        printf("set:0->hdmi mode 1->cvbs mode 2->event test 3->hdr policy 4->av mute 5->HDMI HDCP enable 6-><colorDepth, colorSpace>"
        "7->HDCP Content Type  8->DvEnable 9->active 10->vrr Enable 11->auto mode 12->dummy mode 13->video zorder\n");
        len = scanf("%d",&set);
        if (set == 0 && len == 1) {
            printf("please input modeInfo:interlace, w, h, vrefresh\n");
            scanf("%d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
            if (setDisplayMode(modeInfo, MESON_CONNECTOR_HDMIA) == 0) {
                printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            }else{
                printf("setDisplayModeFail\n");
            }
        } else if(set == 1 && len == 1){
            printf("please input modeInfo:interlace, w, h, vrefresh\n");
            scanf("%d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
            if (setDisplayMode(modeInfo, MESON_CONNECTOR_CVBS) == 0) {
                printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            }else{
                printf("setDisplayModeFail\n");
            }
        } else if(set == 2 && len == 1) {
            registerMesonDisplayEventCallback(display_event);
            startMesonDisplayUeventMonitor();
        } else if (set == 3 && len == 1) {
            printf("0->set Always Hdr 1->set Adaptive Hdr\n");
            int Policy = 0;
            scanf("%d",&Policy);
            if (Policy == 0) {
            if (setDisplayHDRPolicy(MESON_HDR_POLICY_FOLLOW_SINK, MESON_CONNECTOR_HDMIA) == 0) {
                printf("set always hdr success\n");
                }else{
                    printf("set always hdr fail\n");
                }
            } else if (Policy == 1){
                if (setDisplayHDRPolicy(MESON_HDR_POLICY_FOLLOW_SOURCE, MESON_CONNECTOR_HDMIA) == 0) {
                    printf("set adaptive hdr success\n");
                }else{
                    printf("set adaptive hdr fail\n");
                }
            }
        } else if(set == 4 && len == 1){
            printf("\n AVMUTE:\n");
            int avmute = 0;
            len = scanf("%d", &avmute);
            if (len == 1) {
                if (setDisplayAVMute(avmute, MESON_CONNECTOR_HDMIA))
                    printf("\n setDisplayAVMute fail:\n");
            } else {
                printf("\n scanf fail\n");
            }
        } else if (set == 5 && len == 1) {
            printf("\n HDCP enable:\n");
            int hdcpEnable = 0;
            len = scanf("%d", &hdcpEnable);
            if (len == 1) {
                if (setDisplayHDCPEnable(hdcpEnable, MESON_CONNECTOR_HDMIA))
                    printf("\n setDisplayHDCPEnable fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 6 && len == 1) {
                uint32_t colorSpace = 0;
                uint32_t colorDepth = 0;
                printf("\n Please set <colorDepth, colorSpace> property value:\n");
                scanf("%d %d", &colorDepth,&colorSpace);
                int ret = setDisplayColorSpacedDepth(colorDepth, colorSpace, MESON_CONNECTOR_HDMIA);
                if (ret == 0) {
                    printf("\n set <colorDepth, colorSpace> Success!\n");
                } else {
                    printf("\n set value Fail!\n");
                }
        } else if (set == 7 && len == 1) {
            printf("\n HDCP Content Type:\n");
            int HDCPContentType = 0;
            len = scanf("%d", &HDCPContentType);
            if (len == 1) {
                if (setDisplayHDCPContentType(HDCPContentType, MESON_CONNECTOR_HDMIA))
                    printf("\n setDisplayHDCPContentType fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 8 && len == 1) {
            printf("\n DvEnable:\n");
            int dvEnable = 0;
            len = scanf("%d", &dvEnable);
            if (len == 1) {
                if (setDisplayDvEnable(dvEnable, MESON_CONNECTOR_HDMIA))
                    printf("\n setDisplayDvEnable fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 9 && len == 1) {
            printf("\n Active:\n");
            int active = 0;
            len = scanf("%d", &active);
            if (len == 1) {
                if (setDisplayActive( active, MESON_CONNECTOR_HDMIA))
                    printf("\n setDisplayActive fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 10 && len == 1) {
            printf("\n vrr Enable:\n");
            int vrrEnable = 0;
            len = scanf("%d", &vrrEnable);
            if (len == 1) {
                if (setDisplayVrrEnabled( vrrEnable, MESON_CONNECTOR_HDMIA))
                    printf("\n setDisplayVrrEnabled fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
    } else if (set == 11 && len == 1) {
             if (0 == setDisplayAutoMode(MESON_CONNECTOR_HDMIA)) {
                printf("Successfully set the optimal resolution!\n");
        } else {
            printf("scanf fail\n");
        }
    } else if (set == 12 && len == 1) {
            printf("please input dummy modeInfo:interlace, w, h, vrefresh\n");
            scanf("%d %d %d %d", &modeInfo->interlace, &modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
            if (setDisplayMode(modeInfo, MESON_CONNECTOR_DUMMY) == 0) {
                printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            }else{
                printf("setModeFail\n");
            }
    } else if (set == 13 && len == 1) {
            printf("\n please enter the parameters in order(index zorder flag): \n");
            //<--index：Representing video index  Index 0 corresponds to modifying video 0;Index 1 corresponds to modifying video 1 -->//
            //<--zpos：Represents the zorder value set-->//
            //<--flag： Make the settings effective  Set flag equal to 1 to indicate effectiveness-->//
            int zorder = 0;
            int index = 0;
            int flag = 0;
            len = scanf("%d %d %d", &index,&zorder,&flag);
            if (len == 3) {
                if (setDisplayVideoZorder(index, zorder, flag))
                    printf("\n setDisplayVideoZorder fail:\n");
                } else {
                    printf("\n \ scanf fail\n");
                }
        }
    }
    else if(select_s == 1 && select_len == 1) {
        printf("get:0->hdrPolicy 1->modeinfo 2->HDCP version 3->HDMI connected 4->color depth 5->color space"
         " 6->EDID 7->hdcp auth status 8->supportedModesList 9->prefer mode 10->HDCP Content Type 11->Content Type"
         " 12->Dv Enable 13->active 14->vrr Enable 15->av mute 16->hdr mode 17->CvbsModesList 18-> mode support check \n");
        len = scanf("%d",&get);
        if (get == 0 && len == 1) {
            ENUM_MESON_HDR_POLICY value = getDisplayHDRPolicy( MESON_CONNECTOR_HDMIA);
            printf("\n MESON_HDR_POLICY_FOLLOW_SINK = 0 \n"
            "MESON_HDR_POLICY_FOLLOW_SOURCE = 1 \n  value:%d\n", value);
       } else if(get == 1 && len == 1) {
            if (getDisplayMode( modeInfo, MESON_CONNECTOR_HDMIA) == 0) {
                printf("\n mode (%d %d %d %d)\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            } else {
                printf("\n getDisplayModeFail\n");
            }
        } else if(get == 2 && len == 1) {
            ENUM_MESON_HDCP_VERSION value = getDisplayHdcpVersion( MESON_CONNECTOR_HDMIA);
            printf("\n MESON_HDCP_14      = 0\n"
                     " MESON_HDCP_22      = 1\n value:%d \n", value);
        } else if(get == 3 && len == 1) {
            ENUM_MESON_CONN_CONNECTION value = getDisplayConnectionStatus( MESON_CONNECTOR_HDMIA);
            printf("\n MESON_DISCONNECTED      = 0\n"
                     " MESON_CONNECTED         = 1\n value:%d \n",value);
        } else if (get == 4 && len == 1) {
            int value = getDisplayColorDepth( MESON_CONNECTOR_HDMIA);
            printf("\n color depth:%d\n",value);
        } else if (get == 5 && len == 1) {
            ENUM_MESON_COLOR_SPACE value = getDisplayColorSpace( MESON_CONNECTOR_HDMIA);
            printf("\n MESON_COLOR_SPACE_RGB      = 0 \n"
                      "MESON_COLOR_SPACE_YCBCR422 = 1 \n"
                      "MESON_COLOR_SPACE_YCBCR444 = 2 \n"
                      "MESON_COLOR_SPACE_YCBCR420 = 3 \n value:%d\n"
                      , value);
        } else if (get == 6 && len == 1) {
            int len = 0;
            char *edid = NULL;
            int i;
            getDisplayEDIDData(MESON_CONNECTOR_HDMIA, &len, &edid );
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
            ENUM_MESON_HDCPAUTH_STATUS value = getDisplayHdcpAuthStatus( MESON_CONNECTOR_HDMIA);
             printf("\n MESON_AUTH_STATUS_FAIL      = 0 \n"
                      " MESON_AUTH_STATUS_SUCCESS = 1 \n value:%d\n", value);
        } else if (get == 8 && len == 1) {
            DisplayMode* modes = NULL;
            int count = 0;
            if (0 == getDisplayModesList( &modes, &count,MESON_CONNECTOR_HDMIA )) {
                printf("\n mode count:%d\n",count);
                int i = 0;
                for (int i=0; i<count; i++) {
                    printf(" (%s %d %d %d %d)\n", modes[i].name, modes[i].w, modes[i].h, modes[i].interlace,modes[i].vrefresh);
                }
                if (modes)
                    free(modes);
            } else {
                 printf("\n %s fail\n",__FUNCTION__);
            }
        } else if (get == 9 && len == 1) {
            DisplayMode mode;
            if (0 == getDisplayPreferMode(&mode,MESON_CONNECTOR_HDMIA)) {
                printf(" (%s %d %d %d)\n", mode.name, mode.w, mode.h, mode.interlace);
            } else {
                 printf("\n %s fail\n",__FUNCTION__);
            }
        } else if (get == 10 && len == 1) {
            ENUM_MESON_HDCP_Content_Type value = getDisplayHDCPContentType( MESON_CONNECTOR_HDMIA);
            printf("\n MESON_HDCP_Type0      = 0 \n"
                      " MESON_HDCP_Type1 = 1 \n value:%d\n", value);
        } else if (get == 11 && len == 1) {
            ENUM_MESON_Content_Type value = getDisplayContentType( MESON_CONNECTOR_HDMIA);
            printf("\n MESON_Content_Type_Data      = 0 \n"
                      "MESON_Content_Type_Graphics = 1 \n"
                      "MESON_Content_Type_Photo = 2 \n"
                      "MESON_Content_Type_Cinema = 3 \n"
                      "MESON_Content_Type_Game = 4 \n value:%d\n"
                      , value);
        } else if (get == 12 && len == 1) {
            int value = getDisplayDvEnable( MESON_CONNECTOR_HDMIA );
            printf("\n DvEnable:%d\n",value);
            if (value == 1) {
                printf("Support Dolbyvision\n");
            } else {
                printf("Dolbyvision not supported\n");
            }
        } else if (get == 13 && len == 1) {
            uint32_t value = getDisplayActive( MESON_CONNECTOR_HDMIA );
            printf("\n Active:%d\n",value);
        } else if (get == 14 && len == 1) {
            int value = getDisplayVrrEnabled( MESON_CONNECTOR_HDMIA );
            printf("\n VrrEnabled:%d\n",value);
        } else if (get == 15 && len == 1) {
            int value = getDisplayAVMute( MESON_CONNECTOR_HDMIA );
            printf("\n AVMute:%d\n",value);
        } else if (get == 16 && len == 1) {
            ENUM_MESON_HDR_MODE value = getDisplayHdrStatus( MESON_CONNECTOR_HDMIA );
            printf("\n MESON_HDR10PLUS      = 0 \n"
                     " MESON_DOLBYVISION_STD    \n"
                     " MESON_DOLBYVISION_LL    \n"
                     " MESON_HDR10_ST2084    \n"
                     " MESON_HDR10_TRADITIONAL    \n"
                     " MESON_HDR_HLG    \n"
                     " MESON_SDR    \n value:%d\n"
                     , value);
        } else if (get == 17 && len == 1) {
            DisplayMode* modes = NULL;
            int count = 0;
            if (getDisplayModesList( &modes, &count,MESON_CONNECTOR_CVBS ) == 0) {
                printf("\n mode count:%d\n",count);
                int i = 0;
                for (int i=0; i< count; i++) {
                    printf(" (%s %d %d %d %d)\n", modes[i].name, modes[i].w, modes[i].h, modes[i].interlace,modes[i].vrefresh);
                }
                if (modes)
                    free(modes);
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
            ret = modeAttrSupportedCheck(mode, (ENUM_MESON_COLOR_SPACE)color_space,
            color_depth, MESON_CONNECTOR_HDMIA );
            printf("\n mode:%s color attr %d, depth %dbit, SupportedCheck:%d\n",
                mode, color_space, color_depth, ret);
        }
    }
    else {
        printf("\n Incorrect input method\n");
    }
    return 0;
}


