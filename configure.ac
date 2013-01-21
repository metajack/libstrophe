AC_INIT([libcouplet], [0.8], [pasis.ua@gmail.com])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile libcouplet.pc])

AC_PROG_CC
AM_PROG_CC_C_O
LT_INIT

AC_ARG_WITH([libxml2],
            [AS_HELP_STRING([--with-libxml2], [use libxml2 for XML parsing])])

PKG_CHECK_MODULES([openssl], [openssl], [],
    [AC_MSG_ERROR([openssl is required for libcouplet])])
PKG_CHECK_MODULES([check], [check >= 0.9.4], [],
    [AC_MSG_WARN([libcheck not found; unit tests will not be compilable])])
AC_CHECK_LIB(pthread, pthread_create, [],
    [AC_MSG_ERROR([pthread support is required])])
AC_CHECK_LIB(resolv, res_query, [],
    [AC_MSG_ERROR([libresolv is not found])])

if test "x$with_libxml2" = xyes; then
    PKG_CHECK_MODULES([libxml2], [libxml-2.0 >= 2.7], [],
        [AC_MSG_ERROR([libxml2 is required for libcouplet])])
else 
    PKG_CHECK_MODULES([expat], [expat], [],
        [AC_MSG_ERROR([expat is required for libcouplet])])
fi

if test "x$with_libxml2" = xyes; then
  PARSER_NAME=libxml2
  LIBS="$LIBS $libxml2_LIBS"
  AM_CFLAGS="$AM_CFLAGS $libxml2_CFLAGS"
else
  PARSER_NAME=expat
  LIBS="$LIBS $expat_LIBS"
  AM_CFLAGS="$AM_CFLAGS $expat_CFLAGS"
fi

LIBS="$LIBS $openssl_LIBS"
AM_CFLAGS="$AM_CFLAGS $openssl_CFLAGS -I. -Wall"

AC_MSG_NOTICE([libcouplet will use the $PARSER_NAME XML parser])
AC_SEARCH_LIBS([socket], [socket])

AC_CHECK_HEADERS([arpa/nameser_compat.h])

AM_CONDITIONAL([PARSER_EXPAT], [test x$PARSER_NAME != xlibxml2])
AC_SUBST(PARSER_NAME)
AC_SUBST(AM_CFLAGS)
AC_OUTPUT
