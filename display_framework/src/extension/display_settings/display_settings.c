#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <drm_fourcc.h>
#include <sys/mman.h>
#include <pthread.h>
#include "xf86drm.h"
#include "xf86drmMode.h"
#include <signal.h>
#include <drm.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/string.h>
#include "display_settings.h"
#include "libdrm_meson/meson_drm_settings.h"
#define DEFAULT_CARD "/dev/dri/card0"
#include "libdrm_meson/meson_drm_event.h"

bool registerMesonDisplayEventCallback(mesonDisplayEventCallback cb) { //register callback
    return RegisterDisplayEventCallback(cb);
}

void startMesonDisplayUeventMonitor() {
    startDisplayUeventMonitor();
}

void stopMesonDisplayUeventMonitor() {
    stopDisplayUeventMonitor();
}

int setDisplayMode(DisplayMode* modeInfo,MESON_CONNECTOR_TYPE connType) {
    int res = -1;
    int ret = -1;
    int fd = 0;
    drmModeAtomicReq *req = NULL;

    if (modeInfo == NULL) {
        printf("\n %s %d invalid parameter return\n",__FUNCTION__,__LINE__);
        return ret;
    }
    fd = open(DEFAULT_CARD, O_RDWR|O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr, "failed to open device %s\n", strerror(errno));
        return ret;
    }
    ret = drmSetClientCap(fd, DRM_CLIENT_CAP_ATOMIC, 1);
    if (ret) {
        printf("no atomic modesetting support: %d\n",ret);
        goto out;
    }
    req = drmModeAtomicAlloc();
    if (req == NULL) {
        printf("\n %s %d invalid parameter return\n",__FUNCTION__,__LINE__);
        goto out;
    }
    res = meson_drm_changeMode(fd, req, modeInfo, connType);
    if (res == -1) {
        fprintf(stderr,"changeModeFail\n");
        goto out;
    }
    ret = drmModeAtomicCommit(fd, req, DRM_MODE_ATOMIC_ALLOW_MODESET, NULL);
    if (ret) {
        fprintf(stderr, "failed to set mode: %d-%s\n", ret, strerror(errno));
        goto out;
    }
out:
    if (req) {
        drmModeAtomicFree(req);
        req = NULL;
    }
    close(fd);
    return  ret;
}
