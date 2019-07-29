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
 * drm_fd: the drm fd
 */
void start_help_worker(int drm_fd, int atomic_modeset_enable);

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

/*Call it when need update you prop befor atomic commit*/
int help_atomic_req_add_prop(drmModeAtomicReq *req);

/*Call it when repaint cycle completed*/
void help_do_repaint_cycle_completed(void);

#ifdef __cplusplus
}
#endif

#endif //__COMPOSITOR_DRM_HELP_H

