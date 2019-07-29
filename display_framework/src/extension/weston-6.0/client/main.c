/*
 * Copyright (c) 2019 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description: 
 */
#include <pthread.h>
#include <sys/time.h>
#include <stdio.h>
#include <sys/wait.h>
#include <json.h>
#include "drm-help-client.h"
#include <stdio.h>
#include <getopt.h>
#include <string.h>


static const char* short_option = "";
static const struct option long_option[] = {
        {"list-modes", no_argument, 0, 'l'},
        {"chang-mode", required_argument, 0, 'c'},
        {"get-property", required_argument, 0, 'g'},
        {"set-property", required_argument, 0, 's'},
        {"raw-cmd", required_argument, 0, 'r'},
        {0, 0, 0, 0}
};

static void print_usage(const char* name) {
        printf("Usage: %s [-lcrgs]\n"
               "Get or change the mode setting of the weston drm output.\n"
               "Options:\n"
               "       -l,--list-modes        \tlist connector support modes\n"
               "       -c,--change-mode MODE  \tchange connector current mode, MODE format like:%%dx%%d@%%d width,height,refresh\n"
               "       -g,--get-connector-property \"PROPERTY\"\tget connector property\n"
               "       -s,--set-connector-property \"PROPERTY\"=value\tset connector property\n"
               "                               \t eg: \"Content Protection\"=1\n"
               "       -r,--raw-cmd           \tsend raw cmd\n", name);
}

static void list_modes(drm_client_ctx* client) {
    drm_connection_list* conn;
    conn = drm_help_client_get_connection(client);
    if (conn == NULL) {
        printf("Without connector\n");
        return;
    }
    drm_connection_list* curr_conn;
    for (curr_conn = conn; curr_conn != NULL; curr_conn = curr_conn->next) {
        printf("Connnection %d has %d modes:\n", curr_conn->id, curr_conn->count_modes);
        drm_output_mode_list* curr_mode;
        if (curr_conn->modes == NULL) {
            printf("This connector not connected\n");
        }
        for (curr_mode = curr_conn->modes; curr_mode != NULL; curr_mode = curr_mode->next) {
            printf("[%s] refresh %d\n", curr_mode->mode.name, curr_mode->mode.refresh);
        }
    }

    free_connection_list(conn);
}

int main(int argc, char* argv[]) {
    drm_client_ctx* client;
    if (argc == 1) {
        print_usage(argv[0]);
        return 0;
    }
    DEBUG_INFO("Start client");
    do {
        client = drm_help_client_create();
    } while (client == NULL);

    int opt;
    json_object* ret;
    while ((opt = getopt_long_only(argc, argv, short_option, long_option, NULL)) != -1) {
        switch (opt) {
            case 'l':
                list_modes(client);
                break;
            case 'c':
                drm_help_client_switch_mode_s(client, optarg);
                break;
            case 'g':
                if (optarg == NULL)
                    break;
                ret = send_cmd_sync(client, "get connector range properties", optarg);
                if (ret != NULL) {
                    printf("%s", json_object_to_json_string(ret));
                    json_object_put(ret);
                }
                break;
            case 's':
                if (optarg == NULL)
                    break;
                {
                    char name[32] = {0};
                    uint64_t value = 0;
                    int count = 0;
                    count = sscanf(optarg, "%32[^=]=%"SCNu64, name, &value);
                    if (count == 2) {
                        drm_help_client_set_connector_properties(client, name, value);
                        DEBUG_INFO("Set %s = %"PRIu64, name ,value);
                    } else {
                        DEBUG_INFO("Set properties format error");
                    }
                }
                break;
            case 'r':
                if (optarg == NULL)
                    break;
                if (strcmp("get ", optarg) == 0) {
                    ret = send_cmd_sync(client, optarg, NULL);
                    if (ret != NULL) {
                        printf("%s", json_object_to_json_string(ret));
                        json_object_put(ret);
                    }
                } else {
                    send_cmd(client, optarg,"");
                }
                break;
            default:
                print_usage(argv[0]);
        }
    };

    drm_help_client_destory(client);
    DEBUG_INFO("Exit client");
    return 0;
}
