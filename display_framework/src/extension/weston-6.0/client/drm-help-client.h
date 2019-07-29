/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: 
 */
#ifndef __DRM_HELP_CLIENT_H
#define __DRM_HELP_CLIENT_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>
#include <json.h>

#if DEBUG
#include <time.h>
#define COLOR_F (getpid()%6)+1
#define COLOR_B 8
#define DEBUG_INFO(fmt, arg...) do {fprintf(stderr, "[Debug:PID[%5d]:%8ld]\033[3%d;4%dm " fmt "\033[0m [in %s   %s:%d]\n",getpid(), time(NULL), COLOR_F, COLOR_B, ##arg, __FILE__, __func__, __LINE__);}while(0)
#else
#define DEBUG_INFO(fmt, arg...)
#endif //DEBUG

typedef void drm_client_ctx;
typedef struct _drm_output_mode {
    char* name;
    int width;
    int height;
    uint32_t refresh;
} drm_output_mode;

typedef struct _drm_output_mode_list {
    drm_output_mode mode;
    struct _drm_output_mode_list* next;
} drm_output_mode_list;

typedef struct _drm_connection_list {
    int id;
    int type;
    int encoder_id;
    int mmWidth;
    int mmHeight;
    int count_encoders;
    int count_modes;
    drm_output_mode_list* modes;
    struct _drm_connection_list* next;
} drm_connection_list;

drm_client_ctx* drm_help_client_create(void);

void drm_help_client_destory(drm_client_ctx* client);

drm_connection_list* drm_help_client_get_connection(drm_client_ctx* client);

void drm_help_client_switch_mode(drm_client_ctx* client, drm_output_mode* mode);
void drm_help_client_switch_mode_s(drm_client_ctx* client, const char* mode_s);
void drm_help_client_set_connector_properties(drm_client_ctx* client, const char* name, uint64_t value);
void free_modes(drm_output_mode_list* data);
void free_connection_list(drm_connection_list* data);
void send_cmd(drm_client_ctx* client, const char* cmd, const char* opt);
json_object* send_cmd_sync(drm_client_ctx* client, const char* cmd, const char* opt);

#ifdef __cplusplus
}
#endif //__cplusplus
#endif //__DRM_HELP_CLIENT_H
