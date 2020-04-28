AC_INIT([meson_display_framework], [0.1], [lingjie.li@amlogic.com])
AM_INIT_AUTOMAKE([-Wall foreign])
AC_PROG_CC
AC_PROG_CXX
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([
  Makefile
  src/ipc/Makefile
  src/extension/weston-6.0/Makefile
  src/extension/drm/Makefile
])
AM_PROG_AR
AC_PROG_LIBTOOL
LT_INIT()
AC_CONFIG_MACRO_DIR([m4])
AC_CHECK_LIB(json-c, json_tokener_parse,
			 have_json_c=yes, have_json_c=no)
if test "x$have_json_c" = "xyes";then
	PKG_CHECK_MODULES(JSONC, [json-c])
fi

AC_CHECK_LIB(drm, drmOpen,
			 have_libdrm=yes, have_libdrm=no)
if test "x$have_libdrm" = "xyes";then
	PKG_CHECK_MODULES(LIBDRM, [libdrm])
fi

AC_ARG_ENABLE(debug,
			  [  --enable-debug		Turn on debugging], [
if test "x$enableval" = "xyes"; then
	debug=true;
fi
], [debug=false])
AM_CONDITIONAL(DEBUG, test "x$debug" = "xtrue")

AM_CONDITIONAL(ENABLE_WESTON_EXTENTION, test "x$have_json_c" = "xyes" -a "x$have_libdrm" = "xyes")

AM_CONDITIONAL(ENABLE_DRM_FUNCTION, test "x$have_libdrm" = "xyes")

AC_OUTPUT

