#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.61)
AC_INIT([mod_sq], [0.1], [nathan@njlg.info])
AC_CONFIG_SRCDIR([mod_sq.c])
AC_CONFIG_HEADER([config.h])
AC_REVISION($Id$)
AC_PREFIX_DEFAULT(/usr)
AC_CONFIG_MACRO_DIR([m4])
#AC_PROG_MAKE_SET

AM_INIT_AUTOMAKE([-Wall -Werror foreign])

# Checks for required programs
AC_PROG_CC
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_PATH_PROG(APXS, apxs, "no", [/bin /sbin /usr/bin /usr/sbin /usr/local/bin /usr/local/sbin /usr/local/apache/bin])
AC_PATH_PROG(APXS2, apxs2, "no", [/bin /sbin /usr/bin /usr/sbin /usr/local/bin /usr/local/sbin /usr/local/apache/bin])
[test x"$APXS2" = "xno" -a x"$APXS" = "xno" && ] AC_MSG_ERROR([cannot find apxs or apxs2])
[test x"$APXS2" != "xno" && APXS="$APXS2"]

# Check for required libraries
AC_CHECK_LIB(squirrel, sq_open,,
	[AC_MSG_ERROR([required library libsquirrel missing])])

AC_CHECK_LIB(sqstdlib, sqstd_dofile,,
	[AC_MSG_ERROR([required library libsqstdlib missing])])

# Check for required header files
AC_HEADER_STDC
AC_CHECK_HEADERS(squirrel.h stdarg.h stdio.h stdlib.h string.h, [],
	[AC_MSG_ERROR([required header file '$ac_header' not found])])
AC_CHECK_HEADERS(err.h, [], [])


AC_CONFIG_FILES([Makefile])
AC_OUTPUT
