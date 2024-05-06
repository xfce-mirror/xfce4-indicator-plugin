ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

SUBDIRS =								\
	icons								\
	panel-plugin 							\
	po

distclean-local:
	rm -rf *.cache *~ po/.intltool-merge-cache.lock

distuninstallcheck_listfiles = 						\
	find . -type f -print | grep -v ./share/icons/hicolor/icon-theme.cache
