
LDADD = $(top_builddir)/libfastjson.la \
        $(top_builddir)/libfastjson-internal.la

TESTS=
TESTS+= test1.test
TESTS+= test2.test
TESTS+= test4.test
TESTS+= testReplaceExisting.test
TESTS+= test_parse_int64.test
TESTS+= test_cast.test
TESTS+= test_parse.test
TESTS+= test_locale.test
TESTS+= test_charcase.test
TESTS+= test_printbuf.test
TESTS+= test_set_serializer.test
# we officially do NOT support NUL bytes (however, we may
# later add a workaround to at least transparently pass them
# through, thus I keep this as reference).
#TESTS+= test_null.test 

check_PROGRAMS=
check_PROGRAMS += $(TESTS:.test=)

# some programs that do internal checking
TESTS += chk_version

check_PROGRAMS += chk_version
chk_version_SOURCES = chk_version.c

test_printbuf_SOURCES = test_printbuf.c
test_set_serializer_SOURCES = test_set_serializer.c

# Note: handled by test1.test
check_PROGRAMS += test1Formatted 
test1Formatted_SOURCES = test1.c parse_flags.c parse_flags.h
test1Formatted_CPPFLAGS = -DTEST_FORMATTED

# Note: handled by test2.test
check_PROGRAMS += test2Formatted
test2Formatted_SOURCES = test2.c parse_flags.c parse_flags.h
test2Formatted_CPPFLAGS = -DTEST_FORMATTED

EXTRA_DIST=
EXTRA_DIST += $(TESTS)

testsubdir=testSubDir
TESTS_ENVIRONMENT       = top_builddir=$(top_builddir)
