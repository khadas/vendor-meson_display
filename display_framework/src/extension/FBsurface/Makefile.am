bin_PROGRAMS = surface_test
lib_LTLIBRARIES = libFBsurface.la

AM_CFLAGS = $(LIBDRM_CFLAGS)
AM_CPPFLAGS = $(LIBDRM_CFLAGS)

if DEBUG
AM_CFLAGS += -DDEBUG=1
AM_CPPFLAGS += -DDEBUG=1
AM_LDFLAGS = -lpthread
endif

libFBsurface_la_LDFLAGS = -avoid-version -ldrm -ldrm_meson
libFBsurface_la_SOURCES = hwalFBSurface.h \
                          hwalFBSurface.cpp

libFBsurface_ladir = $(includedir)/FBsurface
libFBsurface_la_HEADERS = hwalFBSurface.h

libFBsurface_la_LIBADD = $(LIBDRM_LIBS) $(JSONC_LIBS)
libFBsurface_la_CFLAGS = $(LIBDRM_CFLAGS)

surface_test_SOURCES = surface_test.cpp
surface_test_LDADD =  $(LIBDRM_LIBS) libFBsurface.la
