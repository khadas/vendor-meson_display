/** \file hwalFBSurface.h
*/
#ifndef __HWALFBSURFACE_H__
#define __HWALFBSURFACE_H__
#include <stdio.h>
#include <linux/fb.h>
#include <stdint.h>

typedef enum _eHwalSurfaceType
{
   HwalSurfaceTypeClut8  =0,
   HwalSurfaceTypeXRgb16,
   HwalSurfaceTypeARgb16,
   HwalSurfaceTypeXRgb32,
   HwalSurfaceTypeARgb32
} eHwalSurfaceType;

typedef enum _HwalError
{
   HwalError_EOPNOTSUPP  =0,
   HwalError_EINVAL,
   HwalError_ENODEV,
   HwalError_EIO,
   HwalError_EFAULT,
   HwalError_EPERM,
   HwalError_SUCCESS
} HwalError;

class HwalFBSurface
{
private:
  HwalError set_screen_info();
  HwalError set_pixel_format(eHwalSurfaceType type);

public:
  int mfd;
  void *mptr;
  struct fb_var_screeninfo mVarInfo;
  struct fb_fix_screeninfo mFixInfo;
  uint16_t mBitspp;
  uint16_t mWidth;
  uint16_t mHeight;
  uint16_t mBytepl;
  bool mVisible;
  uint32_t mNumSurfaces;
  uint32_t mCurSurface;

  HwalFBSurface(HwalError &error, eHwalSurfaceType type, uint16_t width, uint16_t height, uint32_t numSurfaces, uint32_t refreshRate);
   ~HwalFBSurface();
   HwalError setDimensions(uint16_t width, uint16_t height);
   HwalError getDimensions(uint16_t & width, uint16_t & height);
   HwalError clean();
   HwalError clean(uint16_t x, uint16_t y, uint16_t width, uint16_t height);
   HwalError getMemAddress(void* &mem);
   HwalError getMemPalette(void* &palette);
   void *getMemAddress();
   void *getMemPalette();
   HwalError getBitsPerPixel(uint16_t &bpp);
   uint16_t getBitsPerPixel(void);
   HwalError getBytesPerLine(uint16_t &Bpl);
   uint16_t getBytesPerLine();
   HwalError Update();
   HwalError blitImage(void *imageAreaPtr, uint16_t x, uint16_t y, uint16_t width, uint16_t height);
   HwalError setZorder(uint16_t zorder);
   HwalError setVisible(bool on);
   bool isVisible(void);
  void print_screen_info(struct fb_var_screeninfo* varInfo, struct fb_fix_screeninfo* fixInfo );
};

#endif