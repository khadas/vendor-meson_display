/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include "compositor-drm-help.h"
#include "ipc/ipc.h"

#define BEGING_EVENT \
    pthread_mutex_lock(&mutex)

#define END_EVENT \
    pthread_mutex_unlock(&mutex)


//Must leave a NULL to list end.
#define for_each_list(pos, list_p)  for (pos = list_p; pos->next != NULL; pos = pos->next)

/**
 * DRM properties are allocated dynamically, and maintained as DRM objects
 * within the normal object ID space; they thus do not have a stable ID
 * to refer to.
 */
typedef struct _drm_property_info {
    const char *name; /**< name as string (static, not freed) */
    uint32_t prop_id; /**< KMS property object ID */
    int need_change; /**< the flag if true that means need update new_value through help_atomic_req_add_prop */
    uint64_t new_value; /**< the need update value */
} drm_property_info;

/**
 * List of properties attached to a DRM connector
 */
enum drm_connector_property {
    DRM_CONNECTOR_PROPERTY_CP,
    DRM_CONNECTOR_PROPERTY__COUNT
};


static const drm_property_info connector_props[] = {
    [DRM_CONNECTOR_PROPERTY_CP] = {
        .name = "Content Protection",
        .need_change = 0,
    },
};

/*
 * TODO:Use a common list struct
 */
typedef struct _connector_list {
    struct _connector_list* prev;
    struct _connector_list* next;
    drmModeConnector* data;
    drm_property_info props[DRM_CONNECTOR_PROPERTY__COUNT];
} connector_list;


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
int g_drm_fd = -1;
int g_atomic_modeset_enable = 0;
connector_list global_connector_list = {.prev = NULL,.next = NULL,};



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


#define get_range_properties(TYPE, id, p_name, p_value) do {    \
    int i;\
    drmModeObjectPropertiesPtr  props = drmModeObjectGetProperties(g_drm_fd,\
            id, DRM_MODE_OBJECT_##TYPE);\
    if (!props) {\
        break;\
    }\
    for (i = 0; i < props->count_props; i++) {\
        drmModePropertyPtr p = drmModeGetProperty(g_drm_fd,\
                props->props[i]);\
        if (p) {\
            if (NULL != strstr(p->name, p_name)) {\
                *p_value = props->prop_values[i];\
            }\
            drmModeFreeProperty(p);\
        }\
    }\
    drmModeFreeObjectProperties(props);\
} while (0)


void update_connector_props(connector_list* connector) {
    int j,i;
    memcpy(connector->props, connector_props, sizeof(connector->props));
    drmModeObjectPropertiesPtr  props = drmModeObjectGetProperties(g_drm_fd,
            connector->data->connector_id, DRM_MODE_OBJECT_CONNECTOR);
    if (!props) {
        return;
    }
    for (i = 0; i < props->count_props; i++) {
        int j;
        drmModePropertyPtr p = drmModeGetProperty(g_drm_fd, props->props[i]);
        if (p == NULL) {
            continue;
        }
        for (j = 0; j < DRM_CONNECTOR_PROPERTY__COUNT; j++) {
            DEBUG_INFO("has props [%s]:(props->id=%u,props->props[i]=%u", p->name, p->prop_id, props->props[i]);
            if (NULL != strstr(p->name, connector->props[j].name)) {
                //save the prop id.
                DEBUG_INFO("Update the props id [%s]:(props->id=%u,props->props[i]=%u", p->name, p->prop_id, props->props[i]);
                connector->props[j].prop_id = props->props[i];
                break;
            }
        }
        drmModeFreeProperty(p);

    }

    for (j = 0; j < DRM_CONNECTOR_PROPERTY__COUNT; j++) {
        if (connector->props[j].prop_id == 0) {
            DEBUG_INFO("Warning: DRM props [%s]:are missing", connector->props[j].name);
        }
    }

    drmModeFreeObjectProperties(props);
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
    *data_out = NULL;
    if (0 == json_object_object_get_ex(data_in, "cmd", &tmp)) {
        DEBUG_INFO("No cmd sended! (%s)", json_object_to_json_string(data_in));
        json_object_put(data_in);
        return;
    }
    //cmd's buffer under the json object "data_in"
    const char* cmd = json_object_get_string(tmp);
    DEBUG_INFO("Handle CMD:%s", cmd);
    if (0 == strcmp("set mode", cmd) &&
        0 != json_object_object_get_ex(data_in, "value", &tmp)) {
        const char* value = json_object_get_string(tmp);
        DEBUG_INFO("CMD set mode :%s", value);
        drm_helper_mode m;
        if (3 != sscanf(value, "%dx%d@%d", &m.width, &m.height, &m.refresh/*, &aspect_width, &aspect_height*/)) {
            DEBUG_INFO("The Value format error");
        } else {
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
        }
    } else if (0 == strcmp("get modes", cmd)) {
        connector_list* current;
        char buf[5] = {0};
        json_object* connector;
        *data_out = json_object_new_object();
        pthread_mutex_lock(&mutex);
        for_each_list(current, &global_connector_list) {
            if (current->data) {
                snprintf(buf,sizeof(buf), "%d", current->data->connector_id);
                connector = dumpConnectorInfo(current->data);
                json_object_object_add(*data_out, buf, connector);
            }
        }
        pthread_mutex_unlock(&mutex);
        DEBUG_INFO("Get modes :[%d]\n%s", strlen(json_object_to_json_string(*data_out)), json_object_to_json_string(*data_out));
    } else if (0 == strcmp("set connector range properties", cmd) &&
            0 != json_object_object_get_ex(data_in, "value", &tmp)) {
        uint64_t value = 0;
        json_object_object_foreach(tmp, name, json_value) {
            if (name != NULL && strlen(name) != 0) {
                int i;
                for (i = 0; i < DRM_CONNECTOR_PROPERTY__COUNT; i++) {
                    if (0 == strcmp(connector_props[i].name, name)) {
                        break;
                    }
                }
                if (i < DRM_CONNECTOR_PROPERTY__COUNT) {
                    //have this properties.
                    connector_list* current;
                    //get value from json value
                    errno = 0;
                    value = (uint64_t)json_object_get_int64(json_value);
                    if (errno != 0) {
                        DEBUG_INFO("Set properties[%s] with issue value", name);
                        continue;
                    }
                    pthread_mutex_lock(&mutex);
                    for_each_list(current, &global_connector_list) {
                        if (current->data) {
                            current->props[i].need_change = 1;
                            current->props[i].new_value = value;
                        }
                    }
                    pthread_mutex_unlock(&mutex);
                    DEBUG_INFO("Set properties[%s]=[%"PRIu64"] ", name, value);
                }
            }
        }
    } else if (0 == strcmp("get connector range properties", cmd) &&
            0 != json_object_object_get_ex(data_in, "value", &tmp)) {
        connector_list* current;
        char buf[5] = {0};
        const char* name = json_object_get_string(tmp);
        if (name != NULL && strlen(name) != 0) {
            *data_out = json_object_new_object();
            pthread_mutex_lock(&mutex);
            for_each_list(current, &global_connector_list) {
                if (current->data) {
                    int status = -1;
                    get_range_properties(CONNECTOR, current->data->connector_id, name, &status);
                    snprintf(buf,sizeof(buf), "%d", current->data->connector_id);
                    json_object_object_add(*data_out, buf, json_object_new_int(status));
                }
            }
            pthread_mutex_unlock(&mutex);
        }
        DEBUG_INFO("Get properties[%s]:[%d]\n%s", name, strlen(json_object_to_json_string(*data_out)), json_object_to_json_string(*data_out));
    } else if (0 == strcmp("get ", cmd)) {
        DEBUG_INFO("CMD[%s] not support or incorrect parameter!", cmd);
        //reply a empty json object to avoid client block.
        *data_out = json_object_new_object();
    }
    json_object_put(data_in);
}

void start_help_worker(int drm_fd, int atomic_modeset_enable) {
    g_drm_fd = drm_fd;
    g_atomic_modeset_enable = atomic_modeset_enable;
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
    int exist = 0;
    //Get the last empty connector of the list
    for_each_list(current, &global_connector_list) {
        if (current->data == connector) {
            exist = 1;
            break;
        }
    };
    if (exist == 0) {
        if (current->data == NULL) {
            current->data = connector;
        } else {
            current->next = malloc(sizeof(connector_list));
            current->next->prev = current;
            current = current->next;
            current->next = NULL;
            current->data = connector;
        }
        update_connector_props(current);
    }
    END_EVENT;
}

void help_update_connector(drmModeConnector* old_connector, drmModeConnector* new_connector) {
    BEGING_EVENT;
    connector_list* current;
    int updated = 0;
    if (old_connector != NULL) {
        //remove old_connector
        for_each_list(current, &global_connector_list) {
            if (current->data == old_connector) {
                current->data = new_connector;
                update_connector_props(current);
                updated = 1;
                break;
            }
        }
    }
    END_EVENT;

    if (updated == 0) {
        help_append_connector(new_connector);
    }
}

void help_delete_connector(drmModeConnector* connector) {
    if (connector == NULL) {
        return;
    }
    BEGING_EVENT;
    connector_list* current;
    for_each_list(current, &global_connector_list) {
        if (current->data != connector) {
            break;
        }
        if (current == &global_connector_list) {
            current->data = NULL;
            break;
        }
        current->prev->next = current->next;
        assert(current->next);
        current->next->prev = current->prev;
        free(current);
        break;
    }
    END_EVENT;
}

void help_set_switch_mode_function(output_ctx ctx, switch_mode fun) {
    BEGING_EVENT;
    g_output = ctx;
    g_switch_mode_fun = fun;
    END_EVENT;
}

int help_atomic_req_add_prop(drmModeAtomicReq *req) {
    int ret = 0;
    assert(g_atomic_modeset_enable);
    BEGING_EVENT;
    connector_list* current;
    for_each_list(current, &global_connector_list) {
        if (current->data == NULL) {
            continue;
        }
        int i;
        int err = 0;
        drmModeConnector* conn = current->data;
        drm_property_info* props = current->props;

        for (i = 0; i < DRM_CONNECTOR_PROPERTY__COUNT; i++) {
            if (props[i].need_change) {
                err = drmModeAtomicAddProperty(req, conn->connector_id, props[i].prop_id, props[i].new_value);
                props[i].need_change = 0;
                DEBUG_INFO("Set conn[%d] %s[%d] to %lld ret=%d", conn->connector_id, props[i].name, props[i].prop_id, props[i].new_value, err);
                ret |= (err <= 0) ? -1 : 0;
            }
        }
    }
    END_EVENT;
    return ret;
}

void help_do_repaint_cycle_completed(void) {
    int ret;
    if (g_atomic_modeset_enable == 0) {
        //Update the props
        BEGING_EVENT;
        connector_list* current;
        for_each_list(current, &global_connector_list) {
            int i;
            if (current->data == NULL) {
                continue;
            }
            drmModeConnector* conn = current->data;
            drm_property_info* props = current->props;
            for (i = 0; i < DRM_CONNECTOR_PROPERTY__COUNT; i++) {
                if (props[i].need_change) {
                    if (props[i].prop_id == 0) {
                        DEBUG_INFO("%s prop_id is 0", props[i].name);
                        continue;
                    }
                    if (props[i].prop_id) {
                        DEBUG_INFO("Set %s to %lld", props[i].name, props[i].new_value);
                        //ret = drmModeConnectorSetProperty(g_drm_fd, conn->connector_id, props[i].prop_id, props[i].new_value);
                        if (ret) {
                            DEBUG_INFO("Update property error");
                        }
                    }
                    props[i].need_change = 0;
                }
            }
        }
        END_EVENT;

    }
}
