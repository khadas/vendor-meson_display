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
#include "drm-help-client.h"
#include <stdio.h>
#include <getopt.h>


static const char* short_option = "F:R:";
static const struct option long_option[] = {
        {"list-modes", no_argument, 0, 'l'},
        {"chang-mode", required_argument, 0, 'c'},
        {"raw-cmd", no_argument, 0, 'r'},
        {0, 0, 0, 0}
};

static void print_usage(const char* name) {
        printf("Usage: %s [-lcr]\n"
               "Get or change the mode setting of the weston drm output.\n"
               "Options:\n"
               "       -l,--list-modes        \tlist connector support modes\n"
               "       -c,--change-mode MODE  \tchange connector current mode, MODE format like:%%dx%%d@%%d width,height,refresh\n"
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


static void switch_mode(drm_client_ctx* client, char* mode) {
    if (mode != NULL)
        send_cmd(client, "set mode", mode);
}

/*
static void interactive_mode(drm_client_ctx* client, const char* tip) {
    printf("into interactive mode\n");
    const char* cmd;
    for (;;) {
        cmd = readline(tip);
        if (strcmp("exit", cmd) == 0 ||
            strcmp("quit", cmd)) {
            return;
        }
        send_cmd(client, "cmd", "");

    }
}
*/

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
    while ((opt = getopt_long_only(argc, argv, short_option, long_option, NULL)) != -1) {
        switch (opt) {
            case 'l':
                list_modes(client);
                break;
            case 'c':
                switch_mode(client, optarg);
                break;
            case 'r':
                if (optarg)
                    send_cmd(client, optarg,"");
                break;
            default:
                print_usage(argv[0]);
                return -1;
        }
    };

    drm_help_client_destory(client);
    DEBUG_INFO("Exit client");
    return 0;
}
