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
	$(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
	$(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvadmininterface.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
	$(putools_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) -lboost_system \
	$(omniORB4_LIBS) \
	$(perl_LIBS)

qabase_src_SOURCES =
EXTRA_DIST = $(qabase_src_SOURCES) 

bin_PROGRAMS = kvQabased
kvQabased_SOURCES = $(qabase_src_SOURCES) 

logdir = $(localstatedir)/log/kvalobs

kvQabased_CPPFLAGS = \
	$(AM_CPPFLAGS)

include src/qabase.mk
include scripts/qabase.mk


if HAVE_GMOCK 

check_PROGRAMS = qabasetest
TESTS = $(check_PROGRAMS)
qabasetest_SOURCES = $(qabase_src_SOURCES)
qabasetest_CPPFLAGS = $(AM_CPPFLAGS) $(gmock_CFLAGS)
qabasetest_LDADD = $(BOOST_DATE_TIME_LIB) $(gmock_LIBS)

include test/qabase.mk

endif