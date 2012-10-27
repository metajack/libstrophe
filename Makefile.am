AUTOMAKE_OPTIONS = subdir-objects

COUPLET_LIBS = libcouplet.la

pkgconfigdir = $(libdir)/pkgconfig

## Main build targets
lib_LTLIBRARIES = libcouplet.la

libcouplet_la_SOURCES = src/auth.c src/conn.c src/ctx.c \
	src/event.c src/handler.c src/hash.c \
	src/jid.c src/md5.c src/sasl.c src/sha1.c \
	src/snprintf.c src/sock.c src/stanza.c src/thread.c \
	src/tls_openssl.c src/util.c src/list.c \
	src/common.h src/hash.h src/md5.h src/ostypes.h src/parser.h \
	src/sasl.h src/sha1.h src/sock.h src/thread.h src/tls.h src/util.h \
	src/list.h

if PARSER_EXPAT
libcouplet_la_SOURCES += src/parser_expat.c
else
libcouplet_la_SOURCES += src/parser_libxml2.c
endif

include_HEADERS = couplet.h
noinst_HEADERS = coupletpp.h

EXTRA_DIST = docs

pkgconfig_DATA = libcouplet.pc

## Tests
TESTS = tests/check_parser
check_PROGRAMS = tests/check_parser
tests_check_parser_SOURCES = tests/check_parser.c tests/test.h
tests_check_parser_CFLAGS = @check_CFLAGS@ $(AM_CFLAGS) \
	-I$(top_srcdir)/src
tests_check_parser_LDADD = @check_LIBS@ $(COUPLET_LIBS)
