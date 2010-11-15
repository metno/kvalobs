#ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-I$(srcdir)/src/ \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(putools_CFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(perl_CFLAGS)

AM_LDFLAGS = \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_REGEX_LIB) \
	$(perl_LIBS)

qabase_src_SOURCES =
EXTRA_DIST = $(qabase_src_SOURCES) 

bin_PROGRAMS = kvQabased
kvQabased_SOURCES = $(qabase_src_SOURCES) 

include src/qabase.mk


if HAVE_GMOCK 

check_PROGRAMS = qabasetest
TESTS = $(check_PROGRAMS)
qabasetest_SOURCES = $(qabase_src_SOURCES)
qabasetest_CPPFLAGS = $(AM_CPPFLAGS) $(gmock_CFLAGS)
qabasetest_LDADD = $(gmock_LIBS)

include test/qabase.mk

endif