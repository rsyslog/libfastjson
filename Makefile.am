
EXTRA_DIST = README.html README-WIN32.html config.h.win32 json-c.vcproj \
	atomic.h
SUBDIRS = . tests

lib_LTLIBRARIES = libfastjson.la 

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfastjson.pc

libfastjsonincludedir = $(includedir)/libfastjson
libfastjsoninclude_HEADERS = \
	arraylist.h \
	math_compat.h \
	bits.h \
	debug.h \
	json.h \
	json_config.h \
	json_c_version.h \
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

libfastjson_la_LDFLAGS = -version-info 3:0:0 -no-undefined @JSON_BSYMBOLIC_LDFLAGS@

libfastjson_la_SOURCES = \
	arraylist.c \
	debug.c \
	json_c_version.c \
	json_object.c \
	json_object_iterator.c \
	json_tokener.c \
	json_util.c \
	linkhash.c \
	printbuf.c \
	random_seed.c


distclean-local:
	-rm -rf $(testsubdir)
	-rm -rf config.h.in~ Makefile.in aclocal.m4 autom4te.cache/ config.guess config.sub depcomp install-sh ltmain.sh missing
	-rm -f INSTALL test-driver tests/Makefile.in compile

maintainer-clean-local:
	-rm -rf configure

uninstall-local:
	rm -rf "$(DESTDIR)@includedir@/libfastjson"
	rm -f "$(DESTDIR)@includedir@/json"

ANDROID_CFLAGS = -I$(top_srcdir) -DHAVE_CONFIG_H

Android.mk: Makefile.am
	androgenizer -:PROJECT libfastjson \
		-:SHARED libfastjson \
		-:TAGS eng debug \
		-:REL_TOP $(top_srcdir) -:ABS_TOP $(abs_top_srcdir) \
		-:SOURCES $(libfastjson_la_SOURCES) $(nodist_libfastjson_la_SOURCES) \
		-:CFLAGS $(DEFS) $(ANDROID_CFLAGS) $(libfastjson_la_CFLAGS) \
		-:LDFLAGS $(libfastjson_la_LDFLAGS) $(libfastjson_la_LIBADD) \
		-:HEADER_TARGET libfastjson \
		-:HEADERS $(libfastjsoninclude_HEADERS) \
		-:PASSTHROUGH LOCAL_ARM_MODE:=arm \
	> $@

ACLOCAL_AMFLAGS = -I m4
