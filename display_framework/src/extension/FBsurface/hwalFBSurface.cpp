#include<stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <hwalFBSurface.h>
#include <string.h>
#include <stdint.h>
#include <iostream>
#define FBPATH "/dev/fb1"
#include "libdrm_meson/meson_drm_log.h"

HwalFBSurface::HwalFBSurface(HwalError &error, eHwalSurfaceType type, uint16_t width, uint16_t height, uint32_t numSurfaces, uint32_t refreshRate)
{
    DEBUG("HwalFBSurface constructor");
    DEBUG("type(%d),w/h(%d, %d),surNUM(%d),refreshRate(%d)\n", type, width, height, numSurfaces, refreshRate);
    error = HwalError_SUCCESS;
    if ((type == HwalSurfaceTypeClut8) || (type > HwalSurfaceTypeARgb32) || (width > 1920) || (height > 1080))
    {
        error = HwalError_EOPNOTSUPP;
        goto out;
    }
    if ( (width == 0) || (height == 0) || (numSurfaces == 0) )
    {
        error = HwalError_EINVAL;
    }
    mfd = open(FBPATH, O_RDWR);
    if (mfd < 0)
    {
        error = HwalError_ENODEV;
        goto out;
    }
    if (ioctl(mfd, FBIOGET_VSCREENINFO, &mVarInfo))
    {
        error = HwalError_EIO;
        goto out;
    }
    if ((error = set_pixel_format(type)) != HwalError_SUCCESS )
    {
        goto out;
    }
    mVarInfo.xres = width;
    mVarInfo.yres = height;
    mVarInfo.bits_per_pixel = mBitspp;
    mVarInfo.xres_virtual = width;
    mVarInfo.yres_virtual = height * numSurfaces;
    mNumSurfaces = numSurfaces;
    mCurSurface = 255;
    if (ioctl(mfd, FBIOPUT_VSCREENINFO, &mVarInfo))
    {
        error = HwalError_EIO;
        goto out;
    }
    if (ioctl(mfd, FBIOGET_FSCREENINFO, &mFixInfo))
    {
        error = HwalError_EIO;
        goto out;
    }
    mBytepl = mFixInfo.line_length;
    mptr = mmap(0,mVarInfo.yres_virtual * mBytepl,
            PROT_WRITE | PROT_READ, MAP_SHARED, mfd, 0);

    if (mptr == MAP_FAILED)
        error = HwalError_EFAULT;
out:

    INFO(" ret = %d\n",error);
}

HwalFBSurface::~HwalFBSurface()
{
    DEBUG("Destructor");
    if (mfd >= 0 && mptr)
    {
        munmap(mptr, mVarInfo.yres_virtual * mBytepl);
        close(mfd);
    }
}

void  HwalFBSurface::print_screen_info(struct fb_var_screeninfo* varInfo, struct fb_fix_screeninfo* fixInfo )
{
    DEBUG("\n varinfo:res(%d, %d), virtual res(%d,%d), bpp(%d)\n"
         "grayscale(%d)\n (offset,len,msb_right) trans(%d,%d,%d) red(%d,%d,%d), green(%d,%d,%d), blue(%d,%d,%d)\n",
             varInfo->xres, varInfo->yres,varInfo->xres_virtual, varInfo->yres_virtual, varInfo->bits_per_pixel,
                 varInfo->grayscale, varInfo->transp.offset, varInfo->transp.length, varInfo->transp.msb_right,
                 varInfo->red.offset, varInfo->red.length, varInfo->red.msb_right,
                 varInfo->green.offset, varInfo->green.length, varInfo->green.msb_right,
                 varInfo->blue.offset, varInfo->blue.length,varInfo->blue.msb_right
                 );
    DEBUG("\n fixInfo buffer:%lu buffer_len:%d,line_len:%d\n", fixInfo->smem_start, fixInfo->smem_len, fixInfo->line_length);
}

HwalError HwalFBSurface::set_pixel_format(eHwalSurfaceType type)
{
    HwalError ret  = HwalError_SUCCESS;
    if ((type == HwalSurfaceTypeClut8) || (type > HwalSurfaceTypeARgb32))
    {
        ret = HwalError_EOPNOTSUPP;
        goto out;
    }
    if (type == HwalSurfaceTypeXRgb16)
    {
         mVarInfo.grayscale = 0;
        mVarInfo.transp.offset = 0;
        mVarInfo.transp.length = 0;
        mVarInfo.transp.msb_right = 0;
        mVarInfo.red.offset = 11;
        mVarInfo.red.length = 5;
        mVarInfo.red.msb_right = 0;
        mVarInfo.green.offset = 5;
        mVarInfo.green.length = 6;
        mVarInfo.green.msb_right = 0;
        mVarInfo.blue.offset = 0;
        mVarInfo.blue.length = 5;
        mVarInfo.blue.msb_right = 0;
        mBitspp = 16;
    }
    else if (type == HwalSurfaceTypeARgb16)
    {
         mVarInfo.grayscale = 0;
        mVarInfo.transp.offset = 15;
        mVarInfo.transp.length = 1;
        mVarInfo.transp.msb_right = 0;
        mVarInfo.red.offset = 10;
        mVarInfo.red.length = 5;
        mVarInfo.red.msb_right = 0;
        mVarInfo.green.offset = 5;
        mVarInfo.green.length = 5;
        mVarInfo.green.msb_right = 0;
        mVarInfo.blue.offset = 0;
        mVarInfo.blue.length = 5;
        mVarInfo.blue.msb_right = 0;
        mBitspp = 16;
    }
    else if (type == HwalSurfaceTypeARgb32)
    {
         mVarInfo.grayscale = 0;
        mVarInfo.transp.offset = 24;
        mVarInfo.transp.length = 8;
        mVarInfo.transp.msb_right = 0;
        mVarInfo.red.offset = 16;
        mVarInfo.red.length = 8;
        mVarInfo.red.msb_right = 0;
        mVarInfo.green.offset = 8;
        mVarInfo.green.length = 8;
        mVarInfo.green.msb_right = 0;
        mVarInfo.blue.offset = 0;
        mVarInfo.blue.length = 8;
        mVarInfo.blue.msb_right = 0;
        mBitspp = 32;
    }
    else if (type == HwalSurfaceTypeXRgb32)
    {
        mVarInfo.grayscale = 0;
        mVarInfo.transp.offset = 0;
        mVarInfo.transp.length = 0;
        mVarInfo.transp.msb_right = 0;
        mVarInfo.red.offset = 16;
        mVarInfo.red.length = 8;
        mVarInfo.red.msb_right = 0;
        mVarInfo.green.offset = 8;
        mVarInfo.green.length = 8;
        mVarInfo.green.msb_right = 0;
        mVarInfo.blue.offset = 0;
        mVarInfo.blue.length = 8;
        mVarInfo.blue.msb_right = 0;
        mBitspp = 32;
    }
out:
    return ret;
}

HwalError HwalFBSurface::setDimensions(uint16_t width, uint16_t height)
{
    INFO("setDimensions(%dx%d)\n", width, height);
    DEBUG("\n %s %d w:%d h:%d\n", __func__, __LINE__, width, height );
    HwalError ret = HwalError_SUCCESS;
    mVarInfo.xres = width;
    mVarInfo.yres = height;
    mVarInfo.xres_virtual = width;
    mVarInfo.yres_virtual = height * mNumSurfaces;
    if (ioctl(mfd, FBIOPUT_VSCREENINFO, &mVarInfo))
    {
        ret = HwalError_EIO;
        goto out;
    }
    if (ioctl(mfd, FBIOGET_FSCREENINFO, &mFixInfo))
    {
        ret = HwalError_EIO;
        goto out;
    }
    mBytepl = mFixInfo.line_length;
    mptr = mmap(0,mVarInfo.yres_virtual * mBytepl,
            PROT_WRITE | PROT_READ, MAP_SHARED, mfd, 0);

    if (mptr == MAP_FAILED)
         ret = HwalError_EFAULT;
out:
    return ret;
}

HwalError HwalFBSurface::getDimensions(uint16_t & width, uint16_t & height)
{
    width  = mVarInfo.xres;
    height = mVarInfo.yres;
    DEBUG("\n %s %d w:%d h:%d\n", __func__, __LINE__, width, height );
    DEBUG( "getDimensions() : %dx%d", width, height);
    return HwalError_SUCCESS;
}

HwalError HwalFBSurface::clean()
{
    DEBUG("clean()");
    HwalError ret = HwalError_SUCCESS;
    memset( mptr, 0, mVarInfo.yres_virtual * mBytepl);

    return ret;
}

HwalError HwalFBSurface::clean(uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    DEBUG("clean(%d,%d - %dx%d)", x,y,width,height);
    HwalError ret = HwalError_SUCCESS;
    uint32_t buffer_len;
    char *clear_area = NULL;
    buffer_len = width * height * mBitspp / 8;
    clear_area = (char*)calloc(buffer_len, 1);
    char* display_area = (char*)mptr;
    uint32_t i;
    if (mCurSurface == 0)
    {
         for (i=0; i<buffer_len; i++)
            display_area[i+buffer_len] = display_area[i];
    }
    if (mCurSurface == 1)
    {
         for (i=0; i<buffer_len; i++)
            display_area[i] = display_area[i+buffer_len];
    }
    ret = blitImage(clear_area, x, y, width, height);
    return ret;
}

HwalError HwalFBSurface::getMemAddress(void* &mem)
{
    mem = getMemAddress();
    if (!mem)
        return HwalError_EPERM;
    return HwalError_SUCCESS;
}

void *HwalFBSurface::getMemAddress()
{
  return mptr;
}

HwalError HwalFBSurface::getMemPalette(void* &palette)
{
    palette = NULL;
    printf("getMemPalette not support\n");
    return HwalError_EOPNOTSUPP;
}

void *HwalFBSurface::getMemPalette()
{
    printf("getMemPalette not support\n");
    return NULL;
}

HwalError HwalFBSurface::getBitsPerPixel(uint16_t &bpp)
{
    bpp = getBitsPerPixel();
    return HwalError_SUCCESS;
}

uint16_t HwalFBSurface::getBitsPerPixel()
{
    return mBitspp;
}

HwalError HwalFBSurface::getBytesPerLine(uint16_t &Bpl)
{
    Bpl = getBytesPerLine();
    return HwalError_SUCCESS;
}

uint16_t HwalFBSurface::getBytesPerLine()
{
    return mBytepl;
}

HwalError HwalFBSurface::Update()
{
    //to do:
    //Update is called when the palette is changed.
    //For Nagra watermarking, Nagra generates a pattern image witch is blotting.
    //On a CLUT8 surface. Later they only changes the palette.
    //Every time that the Nagra function is called and the palette is updated, we call Update,
    //because the Image pattern is the same but just only the palette is changed.
    return HwalError_EPERM;
}

HwalError HwalFBSurface::blitImage(void *imageAreaPtr, uint16_t x, uint16_t y, uint16_t width, uint16_t height)
{
    DEBUG("blitImage(%p, %d,%d - %dx%d)", imageAreaPtr,x,y,width,height);
    DEBUG("\n blitImage(%p, %d,%d - %dx%d)\n", imageAreaPtr,x,y,width,height);
    HwalError ret = HwalError_SUCCESS;
    uint32_t buffer_len;
    uint32_t w_real;
    uint32_t h_real;
    uint32_t loc;
    uint32_t i;
    uint32_t curSur;
    int ioret;

    buffer_len = mVarInfo.xres * mVarInfo.yres * mBitspp / 8;
    char *fbuffer = (char *)mptr;
    char *src = (char *)imageAreaPtr;
    char *src_temp = NULL;
    if (x >= mVarInfo.xres || y >= mVarInfo.yres || !imageAreaPtr)
    {
         ret = HwalError_EINVAL;
        goto out;
    }
    if ( (x+width) > mVarInfo.xres )
        w_real = mVarInfo.xres - x;
    else
        w_real = width;
    if ( (y+height) > mVarInfo.yres )
        h_real = mVarInfo.yres - y;
    else
        h_real = height;
    if (mNumSurfaces == 1)
    {
         mVarInfo.yoffset = 0;
    }
    else
    {
        if (mCurSurface == 255)
        {
            curSur = 0;
            mVarInfo.yoffset = 0;
        }
        if (mCurSurface == 0)
        {
            curSur = 1;
            mVarInfo.yoffset = mVarInfo.yres;
            fbuffer = fbuffer + buffer_len;
        }
        if (mCurSurface == 1)
        {
            mVarInfo.yoffset = 0;
            curSur = 0;
        }
    }
    for (i = 0; i < h_real; i++)
    {
        loc = x * (mVarInfo.bits_per_pixel / 8) + (y + i ) * mBytepl;
        src_temp = src + i * width * mVarInfo.bits_per_pixel/8;
        memcpy(fbuffer + loc, src_temp, w_real * (mVarInfo.bits_per_pixel)/8);
    }
    if (ioret = ioctl(mfd, FBIOPAN_DISPLAY, &mVarInfo))
    {

        printf("\nblitImage ioret(%d)\n",ioret);
        ret = HwalError_EIO;
        goto out;
    }
    mCurSurface = curSur;

out:
    DEBUG("\n blitImage ret(%d)\n",ret);
    return ret;
}

HwalError HwalFBSurface::setZorder(uint16_t zorder)
{
    DEBUG("setZorder not support\n");
    return HwalError_EOPNOTSUPP;
}

HwalError HwalFBSurface::setVisible(bool on)
{
    HwalError  ret = HwalError_SUCCESS;
    if (ioctl(mfd, FBIOBLANK, !on) == 0)
    {
        mVisible = on;
    }
    else
    {
         ret = HwalError_EIO;
    }
    return ret;
}

bool HwalFBSurface::isVisible(void)
{
  return mVisible;
}