/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: 
 */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "compositor-drm-help.h"
#include "ipc/ipc.h"

#define BEGING_EVENT \
    pthread_mutex_lock(&mutex)

#define END_EVENT \
    pthread_mutex_unlock(&mutex)


#define for_each_list(pos, list_p)  for (pos = list_p; pos->next != NULL; pos = pos->next)


/* The server only one instance in one process so use global value
 * g_output : hold the weston output
 * g_switch_mode_fun : hold the drm output's switch mode function
 * mutex : for the globle value.
 * g_server_ctx : use to save a only one instance ipc server context
 * global_connector_list : the connector list current.
*/
output_ctx g_output = NULL;
switch_mode g_switch_mode_fun = NULL;
pthread_mutex_t mutex;
server_ctx* g_server_ctx = NULL;
connector_list global_connector_list = {0};


/* TODO: reduce the codesize
 * NOTE :The format need same as the client's json resovle format.
*/
static json_object* dumpConnectorInfo(drmModeConnector* connector)
{
    int i = 0;
    char id[8];
    json_object* data = json_object_new_object();
    json_object_object_add(data, "connection", json_object_new_int(connector->connection));
    json_object_object_add(data, "connection_type", json_object_new_int(connector->connector_type));
    json_object_object_add(data, "encoder_id", json_object_new_int(connector->encoder_id));
    json_object_object_add(data, "mmWidth", json_object_new_int(connector->mmWidth));
    json_object_object_add(data, "mmHeight", json_object_new_int(connector->mmHeight));

    json_object_object_add(data, "count_props", json_object_new_int(connector->count_props));
    json_object* props = json_object_new_object();
    for (i = 0; i < connector->count_props; i++) {
        snprintf(id, 8, "%d", i);
        json_object_object_add(props, id, json_object_new_int(connector->props[i]));
    }
    json_object_object_add(data, "props", props);

    json_object_object_add(data, "count_encoders", json_object_new_int(connector->count_encoders));
    json_object* encoders = json_object_new_object();
    for (i = 0; i < connector->count_encoders; i++) {
        snprintf(id, 8, "%d", i);
        json_object_object_add(encoders, id, json_object_new_int(connector->encoders[i]));
    }
    json_object_object_add(data, "encoders", encoders);

    json_object_object_add(data, "count_modes", json_object_new_int(connector->count_modes));
    json_object* modes = json_object_new_object();
    for (i = 0; i < connector->count_modes; i++) {
        json_object* mode = json_object_new_object();
        json_object_object_add(mode, "name", json_object_new_string(connector->modes[i].name));
        json_object_object_add(mode, "vrefresh", json_object_new_int(connector->modes[i].vrefresh));
        json_object_object_add(mode, "hdisplay", json_object_new_int(connector->modes[i].hdisplay));
        json_object_object_add(mode, "hsync_start", json_object_new_int(connector->modes[i].hsync_start));
        json_object_object_add(mode, "hsync_end", json_object_new_int(connector->modes[i].hsync_end));
        json_object_object_add(mode, "htotal", json_object_new_int(connector->modes[i].htotal));
        json_object_object_add(mode, "vdisplay", json_object_new_int(connector->modes[i].vdisplay));
        json_object_object_add(mode, "vsync_start", json_object_new_int(connector->modes[i].vsync_start));
        json_object_object_add(mode, "vsync_end", json_object_new_int(connector->modes[i].vsync_end));
        json_object_object_add(mode, "vtotal", json_object_new_int(connector->modes[i].vtotal));
        json_object_object_add(mode, "clock", json_object_new_int(connector->modes[i].clock));
        json_object_object_add(mode, "flags", json_object_new_int(connector->modes[i].flags));
        json_object_object_add(mode, "type", json_object_new_int(connector->modes[i].type));
        snprintf(id, 8, "%d", i);
        json_object_object_add(modes, id, mode);
    }
    json_object_object_add(data, "modes", modes);
    return data;
}

/*
   json formate:
   {
   "cmd":"set mode",
   "value":"value",
   }
   Note.need use json_object_put to decrease the ref of data_in
 */
void m_message_handle(json_object* data_in, json_object** data_out) {
    json_object* tmp;
    if (data_out == &data_in) {
        DEBUG_INFO("The input data can't same with output data!");
        return;
    }
    if (0 == json_object_object_get_ex(data_in, "cmd", &tmp)) {
        DEBUG_INFO("No cmd sended! (%s)", json_object_to_json_string(data_in));
    } else {
        //cmd's buffer under the json object memory
        const char* cmd = json_object_get_string(tmp);
        DEBUG_INFO("Handle CMD:%s", cmd);
        if (0 == strcmp("set mode", cmd)) {
            *data_out = NULL;
            if (0 == json_object_object_get_ex(data_in, "value", &tmp)) {
                DEBUG_INFO("No value sended!");
            }
            const char* value = json_object_get_string(tmp);
            DEBUG_INFO("CMD set mode :%s", value);
            drm_helper_mode m;
            if (3 != sscanf(value, "%dx%d@%d", &m.width, &m.height, &m.refresh/*, &aspect_width, &aspect_height*/)) {
                DEBUG_INFO("The Value format error");
                return;
            }
            pthread_mutex_lock(&mutex);
            if (g_output && g_switch_mode_fun) {
                g_switch_mode_fun(g_output, &m);
            } else {
                if (g_switch_mode_fun == NULL) {
                    DEBUG_INFO("Output not enabled");
                } else {
                    DEBUG_INFO("Output not ready");
                }
            }
            pthread_mutex_unlock(&mutex);


        } else if (0 == strcmp("get modes", cmd)) {
            connector_list* current;
            *data_out = json_object_new_object();
            char buf[5] ={0};
            int count = 0;
            json_object* connector;
            pthread_mutex_lock(&mutex);
            for_each_list(current, &global_connector_list) {
                count++;
                snprintf(buf,sizeof(buf), "%d", count);
                connector = dumpConnectorInfo(current->data);
                json_object_object_add(*data_out, buf, connector);
            }
            DEBUG_INFO("Get modes :[%d]\n%s", strlen(json_object_to_json_string(*data_out)), json_object_to_json_string(*data_out));
            pthread_mutex_unlock(&mutex);
        }
    }
    json_object_put(data_in);
}

void start_help_worker() {
    pthread_mutex_init(&mutex, NULL);
    g_server_ctx = server_create("weston_drm_helper");
    if (g_server_ctx == NULL) {
        DEBUG_INFO("server create failed with error: %s", strerror(errno));
        return;
    }
    server_register_handle(g_server_ctx, m_message_handle);
    if (0 != server_start(g_server_ctx)) {
        server_destory(g_server_ctx);
        DEBUG_INFO("server create failed with error: %s", strerror(errno));
        return;
    }
}

void stop_help_worker() {
    pthread_mutex_lock(&mutex);
    server_destory(g_server_ctx);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
}

void help_append_connector(drmModeConnector* connector) {
    BEGING_EVENT;
    connector_list* current;
    for_each_list(current, &global_connector_list) {};
    if (!current->data) {
        //for the first time to append connector into global_connector_list
        //it have space just data is NULL.
        //or the data was update to NULL by help_update_connector but the element not removed
        current->data = connector;
    } else {
        current->next = malloc(sizeof(connector_list));
        if (current->next) {
            current->next->data = connector;
            current->next->prev = current;
            current->next->next = NULL;
        }
    }
    END_EVENT;
}

void help_update_connector(drmModeConnector* old_connector, drmModeConnector* new_connector) {
    BEGING_EVENT;
    connector_list* current;
    int updated = 0;
    for_each_list(current, &global_connector_list) {
        if (current->data == old_connector) {
            current->data = new_connector;
            updated = 1;
            break;
        }
    };
    END_EVENT;

    if (updated == 0) {
        help_append_connector(new_connector);
    }
}

void help_delete_connector(drmModeConnector* connector) {
    BEGING_EVENT;
    connector_list* current;
    for_each_list(current, &global_connector_list) {
        if (current->data == connector) {
            if (current == &global_connector_list) {
                current->data = NULL;
                break;
            }
            current->prev->next = current->next;
            if (current->next) {
                current->next->prev = current->prev;
            }
            free(current);
            break;
        }

    };

    END_EVENT;
}

void help_set_switch_mode_function(output_ctx ctx, switch_mode fun) {
    BEGING_EVENT;
    g_output = ctx;
    g_switch_mode_fun = fun;
    END_EVENT;
}

#ifdef __cplusplus
}
#endif
