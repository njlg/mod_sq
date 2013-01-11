#
#
#
ACLOCAL_AMFLAGS=-I m4

all-local:    module

module: mod_sq.c io.c
		if test "$(srcdir)" != "."; then $(CP) $(srcdir)/mod_sq.c .; fi
		$(APXS) -c -D_REENTRANT -lsquirrel -lsqstdlib mod_sq.c io.c

install-exec-local: module
		$(APXS) -S LIBEXECDIR="$(DESTDIR)`$(APXS) -q LIBEXECDIR`" -i mod_sq.la

install-exec-hook:
		test -z "$(STRIP)" || $(STRIP) $(DESTDIR)`$(APXS) -q LIBEXECDIR`/mod_sq.so

noinst_HEADERS=     mod_sq.h

CLEANFILES=         *.la *.lo *.o *.so *.slo .libs/*

EXTRA_DIST=         CHANGES LICENSE README
