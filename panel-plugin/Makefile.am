
INCLUDES =										\
	-I$(top_srcdir)								\
	-DG_LOG_DOMAIN=\"libindicator-plugin\"	\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"		\
	$(PLATFORM_CPPFLAGS)

#
# Indicator plugin
#
plugindir = \
	$(libdir)/xfce4/panel/plugins

plugin_LTLIBRARIES = \
	libindicator-plugin.la

libindicator_plugin_la_SOURCES = \
	indicator-button.c									\
	indicator-button.h									\
	indicator-box.c									\
	indicator-box.h									\
	indicator.c									\
	indicator.h									

libindicator_plugin_la_CFLAGS = \
	$(GTK_CFLAGS) \
	$(EXO_CFLAGS) \
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4PANEL_CFLAGS)						\
	$(XFCONF_CFLAGS)					\
	$(INDICATOR_CFLAGS)						\
	$(PLATFORM_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\" 				\
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
	$(EXO_LIBS) \
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4PANEL_LIBS)						\
	$(XFCONF_LIBS)						\
	$(INDICATOR_LIBS)

#
# Desktop file
#
desktopdir =									\
	$(datadir)/xfce4/panel/plugins

desktop_in_files = \
	indicator.desktop.in

desktop_DATA =									\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =									\
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
