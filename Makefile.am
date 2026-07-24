
EXTRA_DIST = README.html
EXTRA_DIST += \
	benchmarks/README.md \
	benchmarks/benchmark.c \
	benchmarks/run.sh \
	benchmarks/runner.py \
	benchmarks/selftest.py

FUZZ_CC ?= clang
FUZZ_CFLAGS = -g -O1 -fno-omit-frame-pointer \
	-fsanitize=fuzzer,address,undefined
FUZZ_CXX_LIBS ?= -lstdc++
FUZZ_SECONDS ?= 30
FUZZ_RSS_LIMIT_MB ?= 2048

.PHONY: fuzz fuzz-smoke

fuzz: all
	$(FUZZ_CC) $(FUZZ_CFLAGS) -nostdlib++ -I$(srcdir) \
		-o tests/fuzz_parser tests/test_fuzz_parser.c \
		$(top_builddir)/.libs/libfastjson.a \
		$(top_builddir)/.libs/libfastjson-internal.a -lm $(FUZZ_CXX_LIBS)
	$(FUZZ_CC) $(FUZZ_CFLAGS) -nostdlib++ -I$(srcdir) \
		-o tests/fuzz_serializer tests/test_fuzz_serializer.c \
		$(top_builddir)/.libs/libfastjson.a \
		$(top_builddir)/.libs/libfastjson-internal.a -lm $(FUZZ_CXX_LIBS)

fuzz-smoke: fuzz
	@set -e; fuzz_tmp=$$(mktemp -d); \
		trap 'rm -rf "$$fuzz_tmp"' EXIT; \
		mkdir "$$fuzz_tmp/parser" "$$fuzz_tmp/serializer"; \
		tests/fuzz_parser -dict=tests/fuzz/json.dict -max_len=65536 \
			-max_total_time=$(FUZZ_SECONDS) -rss_limit_mb=$(FUZZ_RSS_LIMIT_MB) \
			"$$fuzz_tmp/parser" tests/fuzz/corpus/parser; \
		tests/fuzz_serializer -max_len=65536 -max_total_time=$(FUZZ_SECONDS) \
			-rss_limit_mb=$(FUZZ_RSS_LIMIT_MB) "$$fuzz_tmp/serializer" tests/fuzz/corpus/serializer

SUBDIRS = . tests

lib_LTLIBRARIES = libfastjson.la
noinst_LTLIBRARIES = libfastjson-internal.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libfastjson.pc

libfastjsonincludedir = $(includedir)/libfastjson
libfastjsoninclude_HEADERS = \
	atomic.h \
	json.h \
	json_object.h \
	json_object_iterator.h \
	json_object_private.h \
	json_tokener.h \
	json_util.h

libfastjson_la_CFLAGS = $(WARN_CFLAGS)
# info on version-info:
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
libfastjson_la_LDFLAGS = \
	-version-info 7:0:3 \
	-export-symbols-regex '^fjson_.*' \
	-no-undefined \
	@JSON_BSYMBOLIC_LDFLAGS@
libfastjson_la_LIBADD = libfastjson-internal.la

libfastjson_la_SOURCES = \
	json_version.c \
	json_object.c \
	json_print.c \
	json_object_iterator.c \
	json_tokener.c \
	json_util.c

libfastjson_internal_la_CFLAGS = $(WARN_CFLAGS)
libfastjson_internal_la_SOURCES = \
	arraylist.h \
	arraylist.c \
	debug.h \
	debug.c \
	printbuf.h \
	printbuf.c

ACLOCAL_AMFLAGS = -I m4
