
EXTRA_DIST = README.html \
	atomic.h
SUBDIRS = . tests

lib_LTLIBRARIES = libfastjson.la 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfastjson.pc

libfastjsonincludedir = $(includedir)/libfastjson
libfastjsoninclude_HEADERS = \
	arraylist.h \
	bits.h \
	debug.h \
	json.h \
	json_config.h \
	json_inttypes.h \
	json_object.h \
	json_object_iterator.h \
	json_object_private.h \
	json_tokener.h \
	json_util.h \
	linkhash.h \
	printbuf.h \
	random_seed.h

#libjsonx_includedir = $(libdir)/libfastjson-@VERSION@
#
#libjsonx_include_HEADERS = \
#	json_config.h

libfastjson_la_LDFLAGS = -version-info 3:0:0 -no-undefined @JSON_BSYMBOLIC_LDFLAGS@ \
	-export-symbols-regex '^fjson_.*'
libfastjson_la_CPPFLAGS = -Werror

libfastjson_la_SOURCES = \
	arraylist.c \
	debug.c \
	json_version.c \
	json_object.c \
	json_object_iterator.c \
	json_tokener.c \
	json_util.c \
	linkhash.c \
	printbuf.c \
	random_seed.c


uninstall-local:
	rm -rf "$(DESTDIR)@includedir@/libfastjson"

ACLOCAL_AMFLAGS = -I m4
