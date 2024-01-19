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
    printf("\n MESON_DISPLAY_EVENT_CONNECTED  = 0,//!< Display connected event.\n"
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
        printf("set:0->hdmi mode 1->cvbs mode 2->hdr policy 3->av mute 4->HDMI HDCP enable 5-><colorDepth, colorSpace>"
        "6->HDCP Content Type  7->DvEnable 8->active 9->vrr Enable 10->auto mode 11->dummy mode 12->aspect ratio"
         "13->mode attr 14->FunctionAttribute 15->video zorder\n");
        len = scanf("%d",&set);
        if (set == 0 && len == 1) {
            printf("please input modeInfo:interlace, w, h, vrefresh\n");
            scanf("%d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
            if (setDisplayMode(modeInfo, DISPLAY_CONNECTOR_HDMIA) == 0) {
                printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            }else{
                printf("setDisplayModeFail\n");
            }
        } else if(set == 1 && len == 1){
            printf("please input modeInfo:interlace, w, h, vrefresh\n");
            scanf("%d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
            if (setDisplayMode(modeInfo, DISPLAY_CONNECTOR_CVBS) == 0) {
                printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->h, modeInfo->h, modeInfo->vrefresh);
            }else{
                printf("setDisplayModeFail\n");
            }
        } else if (set == 2 && len == 1) {
            /*
            * setDisplayHDRPolicy API hdrPolicy Parameter Description
            * hdrPolicy 2  <-- HDR OFF-->
            * hdrPolicy 0,  <--Always  HDR-->
            * hdrPolicy 1,  <--Adaptive  HDR-->
            */
            printf("0->set Always Hdr 1->set Adaptive Hdr 2->set force mode\n");
            int Policy = 0;
            scanf("%d",&Policy);
            if (Policy == 0) {
            if (setDisplayHDRPolicy(DISPLAY_HDR_POLICY_FOLLOW_SINK, DISPLAY_CONNECTOR_HDMIA) == 0) {
                printf("set always hdr success\n");
                } else {
                    printf("set always hdr fail\n");
                }
            } else if (Policy == 1){
                if (setDisplayHDRPolicy(DISPLAY_HDR_POLICY_FOLLOW_SOURCE, DISPLAY_CONNECTOR_HDMIA) == 0) {
                    printf("set adaptive hdr success\n");
                } else {
                    printf("set adaptive hdr fail\n");
                }
            } else if (Policy == 2){
                if (setDisplayHDRPolicy(DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE, DISPLAY_CONNECTOR_HDMIA) == 0) {
                    printf("set force mode success\n");
                } else {
                    printf("set force mode fail\n");
                }
            }
        } else if(set == 3 && len == 1){
            printf("\n AVMUTE:\n");
            int avmute = 0;
            len = scanf("%d", &avmute);
            if (len == 1) {
                if (setDisplayAVMute(avmute, DISPLAY_CONNECTOR_HDMIA))
                    printf("\n setDisplayAVMute fail:\n");
            } else {
                printf("\n scanf fail\n");
            }
        } else if (set == 4 && len == 1) {
            printf("\n HDCP enable:\n");
            int hdcpEnable = 0;
            len = scanf("%d", &hdcpEnable);
            if (len == 1) {
                if (setDisplayHDCPEnable(hdcpEnable, DISPLAY_CONNECTOR_HDMIA))
                    printf("\n setDisplayHDCPEnable fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 5 && len == 1) {
                uint32_t colorSpace = 0;
                uint32_t colorDepth = 0;
                printf("\n Please set <colorDepth, colorSpace> property value:\n");
                scanf("%d %d", &colorDepth,&colorSpace);
                int ret = setDisplayColorSpacedDepth(colorDepth, colorSpace, DISPLAY_CONNECTOR_HDMIA);
                if (ret == 0) {
                    printf("\n set <colorDepth, colorSpace> Success!\n");
                } else {
                    printf("\n set value Fail!\n");
                }
        } else if (set == 6 && len == 1) {
            printf("\n HDCP Content Type:\n");
            int HDCPContentType = 0;
            len = scanf("%d", &HDCPContentType);
            if (len == 1) {
                if (setDisplayHDCPContentType(HDCPContentType, DISPLAY_CONNECTOR_HDMIA))
                    printf("\n setDisplayHDCPContentType fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 7 && len == 1) {
            printf("\n DvEnable:\n");
            int dvEnable = 0;
            len = scanf("%d", &dvEnable);
            if (len == 1) {
                if (setDisplayDvEnable(dvEnable, DISPLAY_CONNECTOR_HDMIA))
                    printf("\n setDisplayDvEnable fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 8 && len == 1) {
            printf("\n Active:\n");
            int active = 0;
            len = scanf("%d", &active);
            if (len == 1) {
                if (setDisplayActive( active, DISPLAY_CONNECTOR_HDMIA))
                    printf("\n setDisplayActive fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
        } else if (set == 9 && len == 1) {
            printf("\n vrr Enable:\n");
            int vrrEnable = 0;
            len = scanf("%d", &vrrEnable);
            if (len == 1) {
                if (setDisplayVrrEnabled( vrrEnable, DISPLAY_CONNECTOR_HDMIA))
                    printf("\n setDisplayVrrEnabled fail:\n");
                } else {
                    printf("\n scanf fail\n");
                }
    } else if (set == 10 && len == 1) {
             if (0 == setDisplayAutoMode(DISPLAY_CONNECTOR_HDMIA)) {
                printf("Successfully set the optimal resolution!\n");
        } else {
            printf("scanf fail\n");
        }
    } else if (set == 11 && len == 1) {
            printf("please input dummy modeInfo:interlace, w, h, vrefresh\n");
            scanf("%d %d %d %d", &modeInfo->interlace, &modeInfo->w, &modeInfo->h,&modeInfo->vrefresh);
            if (setDisplayMode(modeInfo, DISPLAY_CONNECTOR_DUMMY) == 0) {
                printf("\n mode：%d %d %d %d\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            }else{
                printf("setModeFail\n");
            }
    } else if (set == 12 && len == 1 ) {
            printf("\n aspect ratio:\n");
            int ASPECTRATIO =-1;
            scanf("%d",&ASPECTRATIO);
            int value = getDisplayAspectRatioValue( DISPLAY_CONNECTOR_HDMIA );
            if (value == 0) {
                printf("\n current mode do not support aspect ratio change\n"); //automatic
            } else {
                if (ASPECTRATIO == 1 && value == 2) {
                    if (0 == setDisplayAspectRatioValue(ASPECTRATIO, DISPLAY_CONNECTOR_HDMIA))
                        printf("\n aspect ratio 4:3 set success\n");
                } else if (ASPECTRATIO == 2 && value == 1) {
                    if (0 == setDisplayAspectRatioValue(ASPECTRATIO, DISPLAY_CONNECTOR_HDMIA))
                        printf("\n aspect ratio 16:9 set success\n");
                } else {
                    printf("\n aspect ratio invalid\n");
                }
            }
        } else if (set == 13 && len == 1) {
                uint32_t colorSpace = 0;
                uint32_t colorDepth = 0;
                printf("\n modeInfos: interlace, w, h, vrefresh colorDepth, colorSpace, modename: ");
                scanf("%d %d %d %d %d %d %s",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh,
                                     &colorDepth,&colorSpace,modeInfo->name);
                printf("modename:%s modeinfo: %dx%d%s%dhz %d %d",modeInfo->name,modeInfo->w, modeInfo->h,
                        (modeInfo->interlace == 0? "p":"i") ,modeInfo->vrefresh, colorDepth, colorSpace);
                int ret = setDisplayModeAttr(modeInfo, colorDepth, colorSpace, DISPLAY_CONNECTOR_HDMIA);
                if (ret == 0) {
                    printf("\n setDisplayModeAttr Success!\n");
                } else {
                    printf("\n setDisplayModeAttr Fail!\n");
                }
        } else if (set == 14 && len == 1) {
            int  policy = 0;
            int colorDepth = 0;
            int  colorSpace = 0;
            int FracRate = -1;
            printf("please input modeInfo:interlace, w, h, vrefresh, policy,colorDepth,colorSpace,FracRate value \n");
            scanf("%d %d %d %d %d %d %d %d",&modeInfo->interlace,&modeInfo->w, &modeInfo->h,&modeInfo->vrefresh,&policy,&colorDepth,&colorSpace,&FracRate);
            if (setDisplayFunctionAttribute(modeInfo,policy,colorDepth, colorSpace,FracRate, DISPLAY_CONNECTOR_HDMIA) == 0) {
                printf("\n setDisplayFunctionAttribute Success \n");
            }else{
                printf("\n setDisplayFunctionAttribute Fail \n");
            }
        } else if (set == 15 && len == 1) {
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
         " 12->Dv Enable 13->active 14->vrr Enable 15->av mute 16->hdr mode 17->CvbsModesList 18-> mode support check"
         " 19->current aspect ratio 20->event test 21->frac rate policy 22->Supported dvmode 23->hdr supportedlist "
         " 24->DvCap 25->dpms status 26->mode support attrlist 27->framrate 28->primar plane fb size"
         " 29->physical size 30->Timing information\n");
        len = scanf("%d",&get);
        if (get == 0 && len == 1) {
            ENUM_DISPLAY_HDR_POLICY value = getDisplayHDRPolicy( DISPLAY_CONNECTOR_HDMIA);
            printf("\n DISPLAY_HDR_POLICY_FOLLOW_SINK = 0 \n"
                   "DISPLAY_HDR_POLICY_FOLLOW_SOURCE = 1 \n"
                   "DISPLAY_HDR_POLICY_FOLLOW_FORCE_MODE = 2 \n value:%d\n", value);
       } else if(get == 1 && len == 1) {
            if (getDisplayMode( modeInfo, DISPLAY_CONNECTOR_HDMIA) == 0) {
                printf("\n mode (%d %d %d %d)\n",modeInfo->interlace,modeInfo->w, modeInfo->h, modeInfo->vrefresh);
            } else {
                printf("\n getDisplayModeFail\n");
            }
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
            if (0 == getDisplayModesList( &modeInfo, &count,DISPLAY_CONNECTOR_HDMIA )) {
                printf("\n mode count:%d\n",count);
                int i = 0;
                for (int i=0; i<count; i++) {
                    printf(" (%s %d %d %d %d)\n", modeInfo[i].name, modeInfo[i].w, modeInfo[i].h, modeInfo[i].interlace,modeInfo[i].vrefresh);
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
            int value = getDisplaySupportedDVMode(DISPLAY_CONNECTOR_HDMIA);
            printf("getDisplaySupportedDVMode %d\n",value);
        } else if (get == 23 && len == 1) {
            uint32_t value = getDisplayHDRSupportList(DISPLAY_CONNECTOR_HDMIA);
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
       } else if (get == 24 && len == 1) {
            uint32_t value = getDisplayDvCap( DISPLAY_CONNECTOR_HDMIA );
            if (value == 0) {
                printf("The Rx don't support DolbyVision\n");
            } else {
                printf("\n DvCap:%d\n",value);
            }
        } else if (get == 25 && len == 1) {
            int value = getDisplayDpmsStatus( DISPLAY_CONNECTOR_HDMIA );
            printf("\n get dpms status: %d\n",value);
        } else if(get == 26 && len == 1) {
            int num = getDisplaySupportAttrList( modeInfo, DISPLAY_CONNECTOR_HDMIA);
            if (num == 0) {
                printf("\n getDisplaySupportAttrList Success");
            } else {
                printf("\n getDisplaySupportAttrList Fail");
            }
            free(modeInfo);
        } else if(get == 27 && len == 1) {
            //name：FRAC_RATE_POLICY value：0 整数mode， value：1 小数mode , 大部分情况下默认开机是小数
            float value = getDisplayFrameRate(DISPLAY_CONNECTOR_HDMIA);
            printf("\n get framrate %.2f",value);
            free(modeInfo);
        } else if(get == 28 && len == 1) {
            int width = 0;
            int height = 0;
            int value  = getDisplayPlaneSize( &width,&height );
            if (value == 0 ) {
               printf("\n get graphic plane Size width = %d, height = %d\n",width,height);
            } else {
               printf("\n getDisplayPlaneSize fail\n");
            }
        } else if(get == 29 && len == 1) {
            int width = 0;
            int height = 0;
            int value  = getDisplayPhysicalSize( &width,&height, DISPLAY_CONNECTOR_HDMIA );
            if (value == 0 ) {
               printf("\n get physical Size width = %d, height = %d\n",width,height);
           } else {
               printf("\n getDisplayPhysicalSize fail\n");
           }
        } else if(get ==30 && len == 1) {
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
        }
    }
    else {
        printf("\n Incorrect input method\n");
    }
    return 0;
}
