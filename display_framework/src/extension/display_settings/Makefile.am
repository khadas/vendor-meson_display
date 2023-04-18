bin_PROGRAMS = display_settings_Test
lib_LTLIBRARIES = libdisplay_settings.la

AM_CFLAGS = $(LIBDRM_CFLAGS)
AM_CPPFLAGS = $(LIBDRM_CFLAGS)

if DEBUG
AM_CFLAGS += -DDEBUG=1
AM_CPPFLAGS += -DDEBUG=1
AM_LDFLAGS = -lpthread
endif

libdisplay_settings_la_LDFLAGS = -avoid-version -ldrm -ldrm_meson
libdisplay_settings_la_SOURCES = display_settings.h \
                                 display_settings.c
libdisplay_settings_ladir = $(includedir)/display_settings
libdisplay_settings_la_HEADERS = display_settings.h

libdisplay_settings_la_LIBADD = $(LIBDRM_LIBS) $(JSONC_LIBS)
libdisplay_settings_la_CFLAGS = $(LIBDRM_CFLAGS)

display_settings_Test_SOURCES = display_settings_Test.c
display_settings_Test_LDADD =  $(LIBDRM_LIBS) libdisplay_settings.la
