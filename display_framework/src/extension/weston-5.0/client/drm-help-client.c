/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: 
 */

#include "drm-help-client.h"
#include "ipc.h"

void send_cmd(drm_client_ctx* client, const char* cmd, const char* opt) {
    json_object* data = json_object_new_object();
    json_object_object_add(data, "cmd", json_object_new_string(cmd));
    if (opt != NULL) {
        json_object_object_add(data, "value", json_object_new_string(opt));
    }
    if (client_send_request((client_ctx*)client, data) <= 0) {
        DEBUG_INFO("Server disconnected");
    }
    return;
}

static json_object* send_cmd_sync(drm_client_ctx* client, const char* cmd) {
    json_object* data = json_object_new_object();
    json_object_object_add(data, "cmd", json_object_new_string(cmd));
    if (client_send_request_wait_reply((client_ctx*)client, &data) < 0) {
        DEBUG_INFO("Server disconnected");
    }
    return data;
}

drm_client_ctx* drm_help_client_create(void) {
    return (drm_client_ctx*)client_create("weston_drm_helper");
}
void drm_help_client_destory(drm_client_ctx* client) {
    client_destory((client_ctx*)client);
}

drm_connection_list* dump_modes(json_object* data) {
    drm_connection_list* conn = NULL;
    json_object* obj;
    obj = json_object_object_get(data, "ret");
    if (obj == NULL) {
        DEBUG_INFO("The replay wrong");
        json_object_put(data);
        return NULL;
    }
    drm_connection_list** curr = &conn;
    json_object_object_foreach(obj, connector, val) {
        *curr = (drm_connection_list*)calloc(sizeof(drm_connection_list), 1);
        if (*curr == NULL) {
            DEBUG_INFO("Memory low");
            break;
        }
        {//don't remove
            DEBUG_INFO("Connection %s:%s", connector, json_object_to_json_string(val));
            (*curr)->id = json_object_get_int(json_object_object_get(val, "connection"));
            (*curr)->type = json_object_get_int(json_object_object_get(val, "connection_type"));
            (*curr)->encoder_id = json_object_get_int(json_object_object_get(val, "encoder_id"));
            (*curr)->mmWidth = json_object_get_int(json_object_object_get(val, "mmWidth"));
            (*curr)->mmHeight = json_object_get_int(json_object_object_get(val, "mmHeight"));
            (*curr)->count_encoders = json_object_get_int(json_object_object_get(val, "count_encoders"));
            (*curr)->count_modes = json_object_get_int(json_object_object_get(val, "count_modes"));
            json_object* modes;
            drm_output_mode_list** curr_mode = &((*curr)->modes);
            modes = json_object_object_get(val, "modes");
            if (modes == NULL) {
                DEBUG_INFO("has no modes");
                break;
            }
            json_object_object_foreach(modes, key, value) {
                const char* name = json_object_get_string(json_object_object_get(value, "name"));
                if (name == NULL) {
                    break;
                }
                *curr_mode = (drm_output_mode_list*)calloc(sizeof(drm_output_mode_list), 1);
                if (*curr_mode == NULL) {
                    DEBUG_INFO("Memory low");
                    break;
                }
                (*curr_mode)->mode.name = strdup(name);
                (*curr_mode)->mode.refresh = json_object_get_int(json_object_object_get(value, "vrefresh"));
                (*curr_mode)->mode.width = json_object_get_int(json_object_object_get(value, "hdisplay"));
                (*curr_mode)->mode.height = json_object_get_int(json_object_object_get(value, "vdisplay"));
                DEBUG_INFO("\tMode %s: %s", key, json_object_to_json_string(value));
                curr_mode = &((*curr_mode)->next);
            }
        }
        curr = &((*curr)->next);
    }

    json_object_put(data);
    return conn;
}

drm_connection_list* drm_help_client_get_connection(drm_client_ctx* client) {
    return dump_modes(send_cmd_sync((client_ctx*)client, "get modes"));
}

void drm_help_client_switch_mode(drm_client_ctx* client, drm_output_mode* mode) {
    char buff[32] = {0};
    snprintf(buff, sizeof(buff), "%dx%d@%d", mode->width, mode->height, mode->refresh);
    send_cmd((client_ctx*)client, "set mode", buff);
}


void free_modes(drm_output_mode_list* data) {
    if (data->next != NULL) {
        free_modes(data->next);
    }
    free(data->mode.name);
    free(data);
}

void free_connection_list(drm_connection_list* data) {
    if (data == NULL) {
        return;
    }
    if (data->next != NULL) {
        free_connection_list(data->next);
    }
    if (data->modes) {
        free_modes(data->modes);
    }
    free(data);
}
