dnl xfce4-indicator-plugin - A indicator plugin for the Xfce panel
dnl
dnl 2009 Mark Trompell <mark@foresightlinux.org>
dnl

dnl ***************************
dnl *** Version information ***
dnl ***************************
m4_define([copyright_year], [2025])
XDT_VERSION_INIT([2.5.0], [git])

dnl ***************************
dnl *** Initialize autoconf ***
dnl ***************************
AC_COPYRIGHT([Copyright (c) 2009-copyright_year()
        The Xfce development team. All rights reserved.])
AC_INIT([xfce4-indicator-plugin], [xdt_version], [https://gitlab.xfce.org/panel-plugins/xfce4-indicator-plugin], [xfce4-indicator-plugin])
AC_PREREQ([2.69])
AC_CONFIG_MACRO_DIRS([m4])
AC_CONFIG_HEADERS([config.h])
AC_CANONICAL_TARGET()
AC_REVISION([xdt_version_build])
AC_DEFINE([VERSION_FULL], [PACKAGE_VERSION], [Alias for VERSION and PACKAGE_VERSION for meson compatibility])
COPYRIGHT_YEAR=copyright_year()
AC_DEFINE_UNQUOTED([COPYRIGHT_YEAR], ["$COPYRIGHT_YEAR"], [Copyright year])
AC_SUBST([COPYRIGHT_YEAR])

dnl ***************************
dnl *** Initialize automake ***
dnl ***************************
AM_INIT_AUTOMAKE([1.8 dist-bzip2 tar-ustar no-dist-gzip foreign])
AM_MAINTAINER_MODE()
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl ********************************
dnl *** Check for basic programs ***
dnl ********************************
AC_PROG_CC()
LT_PATH_LD()
AC_PROG_INSTALL()
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl **************************
dnl *** Initialize libtool ***
dnl **************************
LT_PREREQ([2.2.6])
LT_INIT([disable-static])

dnl **********************************
dnl *** Check for standard headers ***
dnl **********************************
AC_CHECK_HEADERS([math.h memory.h stdlib.h string.h sys/stat.h \
                  sys/time.h time.h])

dnl ************************************
dnl *** Check for standard functions ***
dnl ************************************
AC_CHECK_FUNCS([])

dnl ******************************
dnl *** Check for i18n support ***
dnl ******************************
GETTEXT_PACKAGE="$PACKAGE"
AC_DEFINE_UNQUOTED([GETTEXT_PACKAGE], ["$GETTEXT_PACKAGE"], [Name of default gettext domain])
AC_SUBST([GETTEXT_PACKAGE])
AM_GNU_GETTEXT([external])
AM_GNU_GETTEXT_VERSION([0.19.8])

dnl ***********************************
dnl *** Check for required packages ***
dnl ***********************************
m4_define([libxfce4panel_version_api], [2.0])
LIBXFCE4PANEL_VERSION_API=libxfce4panel_version_api()
AC_DEFINE([LIBXFCE4PANEL_VERSION_API], "libxfce4panel_version_api()", [libxfce4panel api version])
AC_SUBST([LIBXFCE4PANEL_VERSION_API])

XDT_CHECK_PACKAGE([GLIB], [glib-2.0], [2.50.0])
XDT_CHECK_PACKAGE([GTK], [gtk+-3.0], [3.22.0])
XDT_CHECK_PACKAGE([LIBXFCE4UTIL], [libxfce4util-1.0], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4UI], [libxfce4ui-2], [4.16.0])
XDT_CHECK_PACKAGE([LIBXFCE4PANEL], [libxfce4panel-${LIBXFCE4PANEL_VERSION_API}], [4.16.0])
XDT_CHECK_PACKAGE([XFCONF], [libxfconf-0], [4.16.0])
INDICATOR_PKGNAME=ayatana-indicator3-0.4
XDT_CHECK_PACKAGE([INDICATOR], [${INDICATOR_PKGNAME}], [0.5.0])
XDT_CHECK_PACKAGE_BINARY([GLIB_COMPILE_RESOURCES], [gio-2.0], [glib_compile_resources], [glib-compile-resources])

dnl ********************************************************************
dnl *** Check if libindicator has indicator-ng.h headery             ***
dnl *** At the moment this cannot be derived from the version number ***
dnl ********************************************************************
AC_LANG_PUSH([C])
CPPFLAGS=`$PKG_CONFIG --cflags ${INDICATOR_PKGNAME}`
AC_CHECK_HEADERS([libayatana-indicator/indicator-ng.h], [HAVE_INDICATOR_NG_H=yes], [HAVE_INDICATOR_NG_H=no])
AC_LANG_POP

dnl ****************************
dnl *** Optional IDO library ***
dnl ****************************
XDT_CHECK_OPTIONAL_PACKAGE([IDO], [libayatana-ido3-0.4], [0.4.0], [ido], [IDO library])

dnl ***********************************
dnl *** Check for debugging support ***
dnl ***********************************
XDT_FEATURE_DEBUG([xdt_debug_default])

dnl ***********************
dnl *** Indicator info ***
dnl ***********************
INDICATORDIR=`$PKG_CONFIG --variable=indicatordir ${INDICATOR_PKGNAME}`
INDICATORICONSDIR=`$PKG_CONFIG --variable=iconsdir ${INDICATOR_PKGNAME}`

AC_SUBST([INDICATORDIR])
AC_SUBST([INDICATORICONSDIR])

dnl **********************************************
dnl *** Optional indicator-application support ***
dnl **********************************************
AC_ARG_ENABLE([application], AS_HELP_STRING([--disable-application],
              [Disable indicator-application support]), [], [enable_application=yes])
if test x"$enable_application" = x"no"; then
        AC_DEFINE([DISABLE_APPLICATION], [], [indicator-application suport disabled])
fi

dnl *********************************
dnl *** Substitute platform flags ***
dnl *********************************
AC_MSG_CHECKING([PLATFORM_CPPFLAGS])
AC_MSG_RESULT([$PLATFORM_CPPFLAGS])
AC_SUBST([PLATFORM_CPPFLAGS])
AC_MSG_CHECKING([PLATFORM_CFLAGS])
AC_MSG_RESULT([$PLATFORM_CFLAGS])
AC_SUBST([PLATFORM_CFLAGS])
AC_MSG_CHECKING([PLATFORM_LDFLAGS])
AC_MSG_RESULT([$PLATFORM_LDFLAGS])
AC_SUBST([PLATFORM_LDFLAGS])

AC_CONFIG_FILES([
Makefile
icons/Makefile
icons/48x48/Makefile
icons/scalable/Makefile
panel-plugin/Makefile
panel-plugin/indicator.desktop.in
po/Makefile.in
])
AC_OUTPUT

dnl ***************************
dnl *** Print configuration ***
dnl ***************************
echo
echo "Build Configuration:"
echo
echo "* Debug Support:        $enable_debug"
echo "* Application Support:  $enable_application"
echo "* Use IndicatorNg API:  $HAVE_INDICATOR_NG_H"
if test x"$IDO_FOUND" = x"yes"; then
echo "* Use IDO library:      yes"
else
echo "* Use IDO library:      no"
fi
echo
