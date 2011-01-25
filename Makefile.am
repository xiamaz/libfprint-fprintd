AUTOMAKE_OPTIONS = dist-bzip2
SUBDIRS = src data tests pam doc po
EXTRA_DIST = TODO intltool-extract.in intltool-merge.in intltool-update.in

check: check-translations

check-translations:
	@for i in $(top_srcdir)/po/*.po ; do								\
		if ! grep -q `basename $$i | sed 's,.po,,'` $(top_srcdir)/po/LINGUAS ; then		\
			echo '**********************************';					\
			echo '***' `basename $$i | sed 's,.po,,'` missing from po/LINGUAS '***' ;	\
			echo '**********************************';					\
			exit 1;										\
		fi;											\
	done;
