AUTOMAKE_OPTIONS = dist-bzip2
SUBDIRS = src data tests pam doc po
EXTRA_DIST = TODO intltool-extract.in intltool-merge.in intltool-update.in

DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc --with-systemdsystemunitdir='$${libdir}/systemd/system-distcheck'

all: check create-pot

check: check-translations

create-pot:
	$(MAKE) -C po fprintd.pot

check-translations:
	@for i in $(top_srcdir)/po/*.po ; do								\
		if ! grep -q `basename $$i | sed 's,.po,,'` $(top_srcdir)/po/LINGUAS ; then		\
			echo '**********************************';					\
			echo '***' `basename $$i | sed 's,.po,,'` missing from po/LINGUAS '***' ;	\
			echo '**********************************';					\
			exit 1;										\
		fi;											\
	done;

update-translations: create-pot check-translations
	@tx pull --all --force --skip
	@tx push --source

-include $(top_srcdir)/git.mk
