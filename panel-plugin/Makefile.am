
AM_CPPFLAGS = \
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
	indicator-dialog-resources.c

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

%.desktop: %.desktop.in
	$(AM_V_GEN) $(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

EXTRA_DIST = \
	indicator-dialog.glade \
	indicator-dialog.gresource.xml \
	meson.build \
	$(desktop_in_files)

DISTCLEANFILES = \
	$(desktop_DATA) \
	$(libindicator_built_sources) \
	$(NULL)

BUILT_SOURCES = \
	$(libindicator_built_sources)

libindicator_dialog_glade_files = $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies --sourcedir $(srcdir) $(srcdir)/indicator-dialog.gresource.xml)

indicator-dialog-resources.c: $(srcdir)/indicator-dialog.gresource.xml $(libindicator_dialog_glade_files) Makefile
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source $<

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
