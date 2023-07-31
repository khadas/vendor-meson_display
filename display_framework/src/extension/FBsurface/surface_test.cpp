#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <hwalFBSurface.h>
#include <string.h>
#include <iostream>
#include "libdrm_meson/meson_drm_log.h"

using namespace std;

int main()
{
    HwalError error;
    uint16_t w ,h;
    cout<< "please input width, height\n";
    cin>>w>>h;
    HwalFBSurface* fbSurface = new HwalFBSurface(error,HwalSurfaceTypeARgb32, w, h, 2, 0);

    DEBUG("\n error(%d)\n",error);
    fbSurface->print_screen_info(&fbSurface->mVarInfo, &fbSurface->mFixInfo );
    uint32_t buffer_len;
    char *test_area = NULL;
    char gray;
    uint16_t x,y;
    gray = 0;
    buffer_len = w * h * fbSurface->mBitspp / 8;
    test_area = (char*)calloc(buffer_len, 1);
    memset(test_area, 0, buffer_len);
    while (1)
    {
        for (uint32_t i = 0; i<buffer_len; i++)
        {
            if (i%4 == 3)
                *(test_area +i) = 255;
            if (i%4 == 2)
                *(test_area +i) = gray;
        }
        gray = gray + 1;
        error = fbSurface->blitImage(test_area, 0, 0, w, h);
        if (gray >= 255)
            break;
        usleep(50000);
    }
    fbSurface->setVisible((bool)0);
    sleep(1);
    fbSurface->setVisible((bool)1);
    sleep(1);
    fbSurface->clean(200,200,200,200);
    sleep(1);
    DEBUG("\n error(%d)\n",error);
    free(test_area);
    return 1;
}