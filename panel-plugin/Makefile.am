INCLUDES =										\
	-I$(top_srcdir)								\
	-DG_LOG_DOMAIN=\"xfce4-indicator-plugin\"	\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"		\
	$(PLATFORM_CPPFLAGS)

#
# Indicator plugin
#
plugin_PROGRAMS =								\
	xfce4-indicator-plugin

plugindir =										\
	$(libexecdir)/xfce4/panel-plugins

xfce4_indicator_plugin_SOURCES =				\
	indicator.c									\
	indicator.h									

xfce4_indicator_plugin_CFLAGS =					\
	$(LIBXFCE4UTIL_CFLAGS)						\
	$(LIBXFCE4PANEL_CFLAGS)						\
	$(INDICATOR_CFLAGS)						\
	$(PLATFORM_CFLAGS)					\
	-DDATADIR=\""$(datadir)"\" 				\
	-DINDICATOR_ICONS_DIR=\""$(INDICATORICONSDIR)"\" \
	-DINDICATOR_DIR=\""$(INDICATORDIR)"\"


xfce4_indicator_plugin_LDADD =					\
	$(LIBXFCE4UTIL_LIBS)						\
	$(LIBXFCE4PANEL_LIBS)						\
	$(INDICATOR_LIBS)

#
# Desktop file
#
desktopdir =									\
	$(datadir)/xfce4/panel-plugins

desktop_in_in_files =							\
	indicator.desktop.in.in

desktop_in_files =								\
	$(desktop_in_in_files:.desktop.in.in=.desktop.in)

%.desktop.in: %.desktop.in.in
	sed -e "s,\@libexecdir\@,$(libexecdir),g" < $< > $@

desktop_DATA =									\
	$(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST =									\
	$(desktop_in_in_files)

CLEANFILES =									\
	$(desktop_in_files)							\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
