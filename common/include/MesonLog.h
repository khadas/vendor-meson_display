/*
 * Copyright (c) 2017 Amlogic, Inc. All rights reserved.
 *
 * This source code is subject to the terms and conditions defined in the
 * file 'LICENSE' which is part of this source code package.
 *
 * Description:
 */

#ifndef MESON_LOG_H
#define MESON_LOG_H

#ifndef LOG_TAG
#define LOG_TAG "UVM"
#endif

#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#ifdef UVM_RELEASE
#define MESON_DEBUG_LEVEL 0
#else
#define MESON_DEBUG_LEVEL 1
#endif

#define ALOGV(fmt, ...)  fprintf(stderr, "[V] %s: " fmt "\n", LOG_TAG, ##__VA_ARGS__)
#define ALOGD(fmt, ...)  fprintf(stderr, "[D] %s: " fmt "\n", LOG_TAG, ##__VA_ARGS__)
#define ALOGI(fmt, ...)  fprintf(stderr, "[I] %s: " fmt "\n", LOG_TAG, ##__VA_ARGS__)
#define ALOGW(fmt, ...)  fprintf(stderr, "[W] %s: " fmt "\n", LOG_TAG, ##__VA_ARGS__)
#define ALOGE(fmt, ...)  fprintf(stderr, "[E] %s: " fmt "\n", LOG_TAG, ##__VA_ARGS__)

#if MESON_DEBUG_LEVEL > 0
#define MESON_LOGV(fmt,...)		ALOGV(fmt, ##__VA_ARGS__)
#define MESON_LOGD(fmt,...)		ALOGD(fmt, ##__VA_ARGS__)
#define MESON_LOGI(fmt,...)		ALOGI(fmt, ##__VA_ARGS__)
#define MESON_LOGW(fmt,...)		ALOGW(fmt, ##__VA_ARGS__)
#else
#define MESON_LOGV(fmt,...)		((void)0)
#define MESON_LOGD(fmt,...)		((void)0)
#define MESON_LOGI(fmt,...)		((void)0)
#define MESON_LOGW(fmt,...)		((void)0)
#endif
#define MESON_LOGE(fmt,...)		ALOGE(fmt, ##__VA_ARGS__)

#if MESON_DEBUG_LEVEL > 0
#define MESON_ASSERT(condition,fmt,...) \
        if (!(condition)) { \
            ALOGE(fmt, ##__VA_ARGS__); \
            abort(); \
        }
#else
#define MESON_ASSERT(condition,fmt,...) \
        if (!(condition)) { \
            ALOGE(fmt, ##__VA_ARGS__); \
        }
#endif

#if MESON_DEBUG_LEVEL > 2
#define MESON_LOG_FUN_ENTER()		ALOGV("Enter %s", __func__)
#define MESON_LOG_FUN_LEAVE()		ALOGV("Leave %s", __func__)
#else
#define MESON_LOG_FUN_ENTER()		((void)0)
#define MESON_LOG_FUN_LEAVE()		((void)0)
#endif

#define MESON_LOG_EMPTY_FUN() \
    ALOGD("ERR: PLEASE FIX NON-IMPLEMENT FUN(%s).", __func__);

#endif/*MESON_LOG_H*/
