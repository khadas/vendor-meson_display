/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: 
 */
#ifndef __COMPOSITOR_DRM_HELP_H
#define __COMPOSITOR_DRM_HELP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <libudev.h>
#include <stdint.h>

/*
 * TODO:Use a common list struct
 */
typedef struct _connector_list {
    struct _connector_list* prev;
    struct _connector_list* next;
    drmModeConnector* data;
} connector_list;

/*
 * TODO:Add more mode info
*/
typedef struct _drm_helper_mode {
     int32_t width, height;
     uint32_t refresh;
} drm_helper_mode;

typedef void* output_ctx;

/* the switch_mode function which used by message handle*/
typedef int (*switch_mode)(output_ctx ctx, drm_helper_mode* mode);

/* create a ipc thread to handle(m_message_handle)
 * the message from client
 */
void start_help_thread(void);

/* destory the ipc thread */
void stop_help_worker(void);

/*Call it when new connector added*/
void help_append_connector(drmModeConnector* connector);

/*Call it when the connector updated */
void help_update_connector(drmModeConnector* old_connector, drmModeConnector* new_connector);

/*Call it when connector go to disconnect*/
void help_delete_connector(drmModeConnector* connector);

/*Call it when update weston compositor's switch mode function*/
void help_set_switch_mode_function(output_ctx ctx, switch_mode fun);

#ifdef __cplusplus
}
#endif

#endif //__COMPOSITOR_DRM_HELP_H

