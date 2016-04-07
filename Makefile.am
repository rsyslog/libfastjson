
EXTRA_DIST = README.html

SUBDIRS = . tests

lib_LTLIBRARIES = libfastjson.la
noinst_LTLIBRARIES = libfastjson-internal.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfastjson.pc

libfastjsonincludedir = $(includedir)/libfastjson
libfastjsoninclude_HEADERS = \
	atomic.h \
	arraylist.h \
	debug.h \
	json.h \
	json_inttypes.h \
	json_object.h \
	json_object_iterator.h \
	json_object_private.h \
	json_tokener.h \
	json_util.h \
	linkhash.h \
	printbuf.h \
	random_seed.h

libfastjson_la_LDFLAGS = \
	-version-info 3:0:0 \
	-export-symbols-regex '^fjson_.*' \
	-no-undefined \
	@JSON_BSYMBOLIC_LDFLAGS@
libfastjson_la_LIBADD = libfastjson-internal.la

libfastjson_la_SOURCES = \
	json_version.c \
	json_object.c \
	json_object_iterator.c \
	json_tokener.c \
	json_util.c

libfastjson_internal_la_SOURCES = \
	arraylist.c \
	debug.c \
	linkhash.c \
	printbuf.c \
	random_seed.c

ACLOCAL_AMFLAGS = -I m4
