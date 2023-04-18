ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS =								\
	icons								\
	panel-plugin 							\
	po

distclean-local:
	rm -rf *.cache *~ po/.intltool-merge-cache.lock

distuninstallcheck_listfiles = 						\
	find . -type f -print | grep -v ./share/icons/hicolor/icon-theme.cache

EXTRA_DIST =								\
	README.md							\
	intltool-extract.in						\
	intltool-merge.in						\
	intltool-update.in

DISTCLEANFILES =							\
	intltool-extract						\
	intltool-merge							\
	intltool-update

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
