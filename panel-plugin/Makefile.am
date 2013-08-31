
INCLUDES = \
	-I$(top_srcdir) \
	-DG_LOG_DOMAIN=\"libindicator-plugin\" \
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\" \
	$(PLATFORM_CPPFLAGS)

#
# Indicator plugin
#
plugindir = \
	$(libdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = \
	libindicator-plugin.la

libindicator_built_sources = \
	indicator-dialog_ui.h

libindicator_plugin_la_SOURCES = \
	$(libindicator_built_sources) \
	indicator-config.c \
	indicator-config.h \
	indicator-button-box.c \
	indicator-button-box.h \
	indicator-button.c \
	indicator-button.h \
	indicator-box.c \
	indicator-box.h \
	indicator-dialog.c \
	indicator-dialog.h \
	indicator.c \
	indicator.h

libindicator_plugin_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS) \
	$(LIBXFCE4UI_CFLAGS) \
	$(LIBXFCE4PANEL_CFLAGS) \
	$(XFCONF_CFLAGS) \
	$(INDICATOR_CFLAGS) \
	$(IDO_CFLAGS) \
	$(PLATFORM_CFLAGS) \
	-DDATADIR=\""$(datadir)"\" \
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\"

libindicator_plugin_la_LDFLAGS = \
	-avoid-version \
	-module \
	-no-undefined \
	-export-symbols-regex '^xfce_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

libindicator_plugin_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBXFCE4UTIL_LIBS) \
	$(LIBXFCE4UI_LIBS) \
	$(LIBXFCE4PANEL_LIBS) \
	$(XFCONF_LIBS) \
	$(INDICATOR_LIBS) \
	$(IDO_LIBS)

#
# Desktop file
#
desktopdir = \
	$(datadir)/xfce4/panel/plugins

desktop_in_files = \
	indicator.desktop.in.in

desktop_DATA = $(desktop_in_files:.desktop.in.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = \
	indicator-dialog.glade \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA)

if MAINTAINER_MODE
BUILT_SOURCES = \
	$(libindicator_built_sources)

DISTCLEANFILES += \
	$(libindicator_built_sources)

indicator-dialog_ui.h: indicator-dialog.glade
	$(AM_V_GEN) xdt-csource --static --strip-comments --strip-content --name=indicator_dialog_ui $< >$@
endif

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
