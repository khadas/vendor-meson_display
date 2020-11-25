/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

//#define DEBUG 1
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>
#include "compositor-drm-help.h"
#include "ipc/ipc.h"

#define BEGING_EVENT \
    pthread_mutex_lock(&mutex)

#define END_EVENT \
    pthread_mutex_unlock(&mutex)


//Must leave a NULL to list end.
//The last one element will not be handle.
#define for_each_list(pos, list_p)  for (pos = list_p; pos->next != NULL; pos = pos->next)

/**
 *
 */
static const char *const aspect_ratio_as_string[] = {
    "",
    " 4:3",
    " 16:9",
    " 64:27",
    " 256:135",
};

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
    DRM_COLOR_DEPTH_CP,
    DRM_COLOR_SPACE_CP,
    DRM_CONNECTOR_PROPERTY__COUNT
};


static const drm_property_info connector_props[] = {
    [DRM_CONNECTOR_PROPERTY_CP] = {
        .name = "Content Protection",
        .need_change = 0,
    },
    [DRM_COLOR_DEPTH_CP] = {
        .name = "Color Depth",
        .need_change = 0,
    },
    [DRM_COLOR_SPACE_CP] = {
        .name = "Color Space",
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


typedef struct _compositor_interface {
    switch_mode switch_mode;
    force_refresh force_refresh;
} compositor_interface;

typedef struct _compositor_output_list {
    struct _compositor_output_list* prev;
    struct _compositor_output_list* next;
    struct compositor_output* data;
    connector_list conn;
    bool enable;
} compositor_output_list;

#define MAX_TASK_NUM 10
typedef struct _helper_task_queue {
    json_object* task[MAX_TASK_NUM];
    int put_index;
    int get_index;
} helper_task_queue;

/* The server only one instance in one process so use global value
 * g_output_list: hold the weston output
 * g_interface: hold the api of compositor
 * mutex : for the globle value.
 * g_server_ctx : use to save a only one instance ipc server context
 * global_connector_list : the connector list current.
 * g_ui_viewport : the ui viewport.
 */

/* For compatible with old version */
output_ctx g_output = NULL;

compositor_output_list g_output_list = { 0 };
compositor_interface g_interface = { 0 };


helper_task_queue  g_task_after_repaint_cycle = { 0 };


pthread_mutex_t mutex;
server_ctx* g_server_ctx = NULL;
int g_drm_fd = -1;
int g_atomic_modeset_enable = 0;
connector_list global_connector_list = {.prev = NULL,.next = NULL,};
pthread_rwlock_t info_rwlock;
bool g_ui_viewport_need_modify = false;
drm_helper_rect g_ui_viewport = { 0, 0, 0, 0 };
drm_helper_size g_ui_size = { 0, 0 };
drmModeModeInfo g_display_mode;



void update_connector_props(connector_list* connector);

/* TODO: reduce the codesize
 * NOTE :The format need same as the client's json resovle format.
 */
static json_object* dumpConnectorInfo(connector_list* current)
{
    int i = 0;
    char id[8];
    json_object* data = json_object_new_object();
    drmModeConnector* connector = current->data;
    if (!current->data)
        return data;
    json_object_object_add(data, "connection", json_object_new_int(connector->connection));
    json_object_object_add(data, "connection_type", json_object_new_int(connector->connector_type));
    json_object_object_add(data, "encoder_id", json_object_new_int(connector->encoder_id));
    json_object_object_add(data, "mmWidth", json_object_new_int(connector->mmWidth));
    json_object_object_add(data, "mmHeight", json_object_new_int(connector->mmHeight));

    json_object_object_add(data, "count_props", json_object_new_int(connector->count_props));
    json_object* props = json_object_new_object();
    update_connector_props(current);
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

bool is_hdmi_connector(connector_list* current) {
    if (current->data &&
            (current->data->connector_type == DRM_MODE_CONNECTOR_HDMIA ||
             current->data->connector_type == DRM_MODE_CONNECTOR_HDMIB)) {
        return true;
    }
    return false;
}


bool parse_modestring(const char* modestring, drm_helper_mode* mode) {
    int32_t width = 0;
    int32_t height = 0;
    char* height_str = NULL;
    uint32_t refresh = 0;
    uint32_t flags = 0;
    uint32_t aspect_width = 0;
    uint32_t aspect_height = 0;
    char* others = NULL;
    const char* aspect_ratio = aspect_ratio_as_string[0];
    int n, k;

    n = sscanf(modestring , "%dx%m[^@]@%d %u:%u", &width, &height_str, &refresh, &aspect_width, &aspect_height);
    if (n == 5) {
        if (aspect_width == 4 && aspect_height == 3)
            aspect_ratio = aspect_ratio_as_string[1];
        else if (aspect_width == 16 && aspect_height == 9)
            aspect_ratio = aspect_ratio_as_string[2];
        else if (aspect_width == 64 && aspect_height == 27)
            aspect_ratio = aspect_ratio_as_string[3];
        else if (aspect_width == 256 && aspect_height == 135)
            aspect_ratio = aspect_ratio_as_string[4];
        else
            fprintf(stderr, "Invalid modestring \"%s\"\n", modestring);
    }
    if (height_str == NULL || (n != 2 && n != 3 && n != 5)) {
        if (height_str != NULL) {
            free(height_str);
        }
        return false;
    }
    n = sscanf(height_str, "%d%ms", &height, &others);
    if (n == 0) {
        free(height_str);
        return false;
    }
    if (n == 2) {
        n = strlen(others);
        for (k = 0; k < n; k++) {
            switch (others[k]) {
                case 'i':
                    flags |= DRM_MODE_FLAG_INTERLACE;
                    break;
                case 'd':
                    flags |= DRM_MODE_FLAG_DBLSCAN;
                    break;
                default:
                    break;
            }
        }
        free(others);
    }
    mode->width = width;
    mode->height = height;
    mode->flags = flags;
    mode->refresh = refresh;
    mode->aspect_ratio = aspect_ratio;
    DEBUG_INFO("set mode to: %dx%d%c %dhz %s", mode->width, mode->height,
            (mode->flags & DRM_MODE_FLAG_INTERLACE) ? 'i': (mode->flags & DRM_MODE_FLAG_DBLSCAN) ? 'd' : ' ',
            mode->refresh, mode->aspect_ratio);
    free(height_str);
    return true;
}

bool schedule_task(helper_task_queue* queue, json_object* task) {
    json_object* cmd = NULL;
    int next = (queue->put_index + 1) % MAX_TASK_NUM;

    if (next == queue->get_index) {
        DEBUG_INFO("task queue full");
        return false;
    }
    if (0 != json_object_deep_copy(task, &cmd, NULL)) {
        return false;
    }
    queue->task[queue->put_index] = cmd;
    queue->put_index = next;
    return true;
}


bool have_task(helper_task_queue* queue) {
    return (queue->get_index != queue->put_index);
}

bool get_task(helper_task_queue* queue, json_object** task) {
    int next = (queue->get_index + 1) % MAX_TASK_NUM;

    if (have_task(queue)) {
        *task = queue->task[queue->get_index];
        queue->task[queue->get_index] = NULL;
        queue->get_index = next;
        return true;
    }
    return false;
}

void process_task(json_object* data_in, json_object** data_out) {
    int ret = 0;
    json_object* tmp = NULL;
    json_object* opt = NULL;
    *data_out = NULL;
    assert(0 != json_object_object_get_ex(data_in, "cmd", &tmp));
    //cmd's buffer under the json object "data_in"
    const char* cmd = json_object_get_string(tmp);
    json_object_object_get_ex(data_in, "value", &opt);
    DEBUG_INFO("process task:%s", cmd);
    if (0 == strcmp("set mode", cmd)) {
        drm_helper_mode mode;
        assert(opt);
        const char* value = json_object_get_string(opt);
        if (true == parse_modestring(value, &mode)) {
            pthread_mutex_lock(&mutex);
            if (g_interface.switch_mode) {
                if (g_output_list.data) {
                    compositor_output_list* current;
                    for_each_list(current, &g_output_list) {
                        if (current->enable) {
                            assert(current->data);
                            g_interface.switch_mode(current->data, &mode);
                        }
                    }
                } else if (g_output) {
                    g_interface.switch_mode(g_output, &mode);
                }
            }
            pthread_mutex_unlock(&mutex);
        } else {
            ret = -1;
        }
    }
    json_object_put(data_in);
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
    int ret = 0;
    json_object* tmp = NULL;
    json_object* opt = NULL;
    *data_out = NULL;
    if (0 == json_object_object_get_ex(data_in, "cmd", &tmp)) {
        DEBUG_INFO("No cmd sended! (%s)", json_object_to_json_string(data_in));
        json_object_put(data_in);
        return;
    }
    //cmd's buffer under the json object "data_in"
    const char* cmd = json_object_get_string(tmp);

    json_object_object_get_ex(data_in, "value", &opt);
    DEBUG_INFO("Handle CMD:%s", cmd);
    if (0 == strcmp("set mode", cmd)) {
        //Support mode string like: "%dx%m[^@]@%d %u:%u": "1920x1080i@60 16:9"
        if (opt == NULL) {
            ret = -1;
        } else {
            const char* value = json_object_get_string(opt);
            drm_helper_mode mode;
            DEBUG_INFO("CMD set mode :%s", value);
            if (false == parse_modestring(value, &mode)) {
                ret = -1;
            } else {
                pthread_mutex_lock(&mutex);
                if (false == schedule_task(&g_task_after_repaint_cycle, data_in)) {
                    DEBUG_INFO("schedule task failed");
                    ret = -1;
                }
                //trigger a refresh, for the next repaint
                if (g_interface.force_refresh && g_output_list.data) {
                    //TODO: refresh for need output olny
                    compositor_output_list* current;
                    for_each_list(current, &g_output_list) {
                        if (current->enable) {
                            assert(current->data);
                            g_interface.force_refresh(current->data);
                        }
                    }
                }

                pthread_mutex_unlock(&mutex);
            }
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
                connector = dumpConnectorInfo(current);
                json_object_object_add(*data_out, buf, connector);
            }
        }
        pthread_mutex_unlock(&mutex);
        DEBUG_INFO("Get modes :[%d]\n%s", strlen(json_object_to_json_string(*data_out)), json_object_to_json_string(*data_out));
    } else if (0 == strcmp("set connector range properties", cmd)) {
        if (opt == 0) {
            ret = -1;
        } else {
            uint64_t value = 0;
            json_object_object_foreach(opt , name, json_value) {
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
                            if (!is_hdmi_connector(current)) {
                                if (i == DRM_COLOR_DEPTH_CP || i == DRM_COLOR_SPACE_CP || i == DRM_CONNECTOR_PROPERTY_CP) {
                                    continue;
                                }
                            }
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
        }
    } else if (0 == strcmp("get connector range properties", cmd)) {
        if (opt == 0) {
            ret = -1;
        } else {
            connector_list* current;
            char buf[5] = {0};
            const char* name = json_object_get_string(opt);
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
        }
    } else if (0 == strcmp("get display mode", cmd)) {
        *data_out = json_object_new_object();
        pthread_rwlock_rdlock(&info_rwlock);
        ret |= json_object_object_add(*data_out, "name", json_object_new_string(g_display_mode.name));
        ret |= json_object_object_add(*data_out, "vrefresh", json_object_new_int(g_display_mode.vrefresh));
        ret |= json_object_object_add(*data_out, "hdisplay", json_object_new_int(g_display_mode.hdisplay));
        ret |= json_object_object_add(*data_out, "vdisplay", json_object_new_int(g_display_mode.vdisplay));
        pthread_rwlock_unlock(&info_rwlock);
        if (ret != 0) {
            DEBUG_INFO("Send display mode with error");
        }
    } else if (0 == strcmp("get ui rect", cmd)) {
        *data_out = json_object_new_object();
        pthread_rwlock_rdlock(&info_rwlock);
        ret |= json_object_object_add(*data_out, "x", json_object_new_int(g_ui_viewport.x));
        ret |= json_object_object_add(*data_out, "y", json_object_new_int(g_ui_viewport.y));
        ret |= json_object_object_add(*data_out, "w", json_object_new_int(g_ui_viewport.w));
        ret |= json_object_object_add(*data_out, "h", json_object_new_int(g_ui_viewport.h));
        pthread_rwlock_unlock(&info_rwlock);
        if (ret != 0) {
            DEBUG_INFO("Send ui rect with error");
        }
    } else if (0 == strcmp("set ui rect", cmd)) {
        if (opt != NULL) {
            DEBUG_INFO("Set ui rect");
            drm_helper_rect rect;
            errno = 0;
            rect.x = json_object_get_int(json_object_object_get(opt, "x"));
            ret |= errno;
            rect.y = json_object_get_int(json_object_object_get(opt, "y"));
            ret |= errno;
            rect.w = json_object_get_int(json_object_object_get(opt, "w"));
            ret |= errno;
            rect.h = json_object_get_int(json_object_object_get(opt, "h"));
            ret |= errno;
            if (ret == 0) {
                pthread_rwlock_wrlock(&info_rwlock);
                g_ui_viewport_need_modify = false;
                usleep(2000);
                g_ui_viewport = rect;
                g_ui_viewport_need_modify = true;
                pthread_rwlock_unlock(&info_rwlock);
            } else {
                DEBUG_INFO("Set ui rect with error");
            }
        } else {
            ret = -1;
            DEBUG_INFO("Set ui rect with error opt");
        }
    } else if (0 == strcmp("get ", cmd)) {
        DEBUG_INFO("CMD[%s] not support or incorrect parameter!", cmd);
        //reply a empty json object to avoid client block.
        *data_out = json_object_new_object();
    }
    json_object_put(data_in);
    if (ret != 0) {
        DEBUG_INFO("CMD[%s] handle failed!", cmd);
    }
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

void dump_connector_status(const char* fname) {
#if DEBUG
    connector_list* current;
    fprintf(stderr, "Dump connector info at [%s]:", fname);
    for_each_list(current, &global_connector_list) {
        fprintf(stderr, "==>[connect:%d p=%p type=%d] ", current->data->connector_id, current->data, current->data->connector_type);
    };
    fprintf(stderr, "\n");
#endif
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
        //we already hold a empty at tail, and keep a empty at tail.
        assert(!current->data);
        current->data = connector;
        current->next = malloc(sizeof(connector_list));
        current->next->prev = current;
        current->next->next = NULL;
        current->next->data = NULL;;
        dump_connector_status(__func__);
        memcpy(current->props, connector_props, sizeof(current->props));
    }
    update_connector_props(current);
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
                memcpy(current->props, connector_props, sizeof(current->props));
                update_connector_props(current);
                updated = 1;
                dump_connector_status(__func__);
                break;
            }
        }
    }
    END_EVENT;

    if (updated == 0) {
        help_append_connector(new_connector);
    }
}


void help_get_scanout_viewport(int32_t* x, int32_t* y, uint32_t* w, uint32_t* h) {
    if (g_ui_viewport_need_modify) {
        *x = g_ui_viewport.x;
        *y = g_ui_viewport.y;
        *w = g_ui_viewport.w;
        *h = g_ui_viewport.h;
    }
    return;
}

void help_update_ui_logic_size_info(uint32_t w, uint32_t h) {
    pthread_rwlock_wrlock(&info_rwlock);
    g_ui_size.w = w;
    g_ui_size.h = h;
    if (!g_ui_viewport_need_modify) {
        g_ui_viewport.x = 0;
        g_ui_viewport.y = 0;
        g_ui_viewport.w = w;
        g_ui_viewport.h = h;
    }
    pthread_rwlock_unlock(&info_rwlock);
}

void help_update_display_mode_info(drmModeModeInfo* mode) {
    pthread_rwlock_wrlock(&info_rwlock);
    memcpy(&g_display_mode, mode, sizeof(drmModeModeInfo));
    pthread_rwlock_unlock(&info_rwlock);
}


void help_delete_connector(drmModeConnector* connector) {
    if (connector == NULL) {
        return;
    }
    BEGING_EVENT;
    connector_list* current;
    for_each_list(current, &global_connector_list) {
        if (current->data != connector) {
            continue;
        }
        dump_connector_status(__func__);
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

void help_set_switch_mode_function(struct compositor_output* ctx, switch_mode fun) {
    BEGING_EVENT;
    g_interface.switch_mode = fun;
    g_output = ctx;
    END_EVENT;
}

void help_set_force_refresh_function(force_refresh fun) {
    BEGING_EVENT;
    g_interface.force_refresh = fun;
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
    json_object* task = NULL;
    json_object* result = NULL;
    bool got_task = false;
    DEBUG_INFO("cycle_completed");
    do {
        if (!have_task(&g_task_after_repaint_cycle)) {
            break;
        }

        BEGING_EVENT;
        got_task = get_task(&g_task_after_repaint_cycle, &task);
        END_EVENT;

        if (got_task && task) {
            process_task(task, &result);
        }
    } while (got_task);
}

void dump_output_status(const char* fname) {
#if DEBUG
    compositor_output_list* current;
    fprintf(stderr, "Dump output info at [%s]:", fname);
    int id = 0;
    for_each_list(current, &g_output_list) {
        fprintf(stderr, "==>[output:%d(%p):%p enable=%d conn=%p ", id, current, current->data, current->enable, current->conn.data);
        if (current->conn.data)
            fprintf(stderr, "type=%d", current->conn.data->connector_type);
        fprintf(stderr, "]");
        id++;
    };
    fprintf(stderr, "\n");
#endif
}

void help_updata_compositor_output(struct compositor_output* older, struct compositor_output* newer) {
    if (older == newer) {
        return;
    }
    BEGING_EVENT;
    compositor_output_list* current;
    bool updated = false;
    int update_index = 0;

    int index = 0;
    DEBUG_INFO("update old:%p, newer:%p", older, newer);
    for_each_list(current, &g_output_list) {
        //remove old output or replace same one
        if (older == NULL) {
            if (current->data == newer) {
                updated = true;
                update_index = index;
                break;
            }
        } else if (current->data == older) {
            updated = true;
            update_index = index;
            break;
        }
        index++;
    }

    index = 0;
    //remove the output same as newer
    for_each_list(current, &g_output_list) {
        if (current->data == newer) {
            if (updated && update_index != index) {
                //hold a empty output
                current->data = NULL;
                //current->conn = NULL;
                current->enable = false;
            } else if (!updated) {
                updated = true;
                update_index = index;
            }
        }
        index++;
    }
    //Seek to newer position
    index = 0;
    for_each_list(current, &g_output_list) {
        if (updated && index == update_index) {
            //current is the newer position
            break;
        }
        if (!updated && current->data == NULL) {
            //put into empty pos
            updated = true;
            update_index = index;
            break;
        }
        index++;
    }

    if (!updated && newer != NULL) {
        //Need append a empty element at end
        //And Use current element
        assert(current->next == NULL);
        current->next = calloc(sizeof(compositor_output_list), 1);
        if (current->next == NULL) {
            fprintf(stderr, "out of memory, can't append output list [%s]\n", __func__);
            goto error_out;
        }
        current->next->prev = current;
        current->next->next = NULL;
        updated = true;
        update_index = index;
    }
    {
        // do real update newer output
        current->data = newer;
        //current->conn = NULL;
        current->enable = false;
    }

    //shrink empty output
    index = 0;
    bool need_remove_prev = false;
    compositor_output_list* need_remove = NULL;
    compositor_output_list* last_element = &g_output_list;
    for (current = &g_output_list; current != NULL; current = current->next) {
        //remain one empty element at the end
        if (current->data == NULL && current != &g_output_list && current->next != NULL) {
            current->prev->next = current->next;
            current->next->prev = current->prev;
            need_remove = current;
            current = current->prev;
            free(need_remove);
        } else if (current->next != NULL) {
            //last not empty element or first element
            last_element = current;
        }
    }

    if (g_output_list.data == NULL && last_element != &g_output_list) {
        //swap end with first, then remove the last
        last_element->prev->next = last_element->next;
        last_element->next = g_output_list.next;
        last_element->prev = NULL;
        memcpy(&g_output_list, last_element, sizeof(connector_list));
        free(last_element);
    }

error_out:
    dump_output_status(__func__);
    END_EVENT;
}

void help_switch_compositor_output(struct compositor_output* output, bool enable) {
    compositor_output_list* current;
    if (output == NULL)
        return;

    BEGING_EVENT;
    for_each_list(current, &g_output_list) {
        if (output == current->data) {
            current->enable = enable;
            break;
        }
    }

    dump_output_status(__func__);
    END_EVENT;
}
