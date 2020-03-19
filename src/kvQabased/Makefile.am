#ACLOCAL_AMFLAGS = -I m4

AM_CPPFLAGS = \
	-I$(srcdir)/src/ \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(libxmlpp_CFLAGS) \
	$(rdkafka_CFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(perl_CFLAGS)

AM_LDFLAGS = \
	-R "$(libdir)/@PACKAGE_NAME@/lib" \
	-R "$(rdkafka_LIBDIR)" \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(putools_LIBS) \
	$(libxmlpp_LIBS) \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) -lboost_system \
	$(omniORB4_LIBS) \
	$(rdkafka_LIBS) \
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
qabasetest_CPPFLAGS = $(AM_CPPFLAGS) $(gmock_CFLAGS) $(gtest_CFLAGS) 
qabasetest_LDADD = $(BOOST_DATE_TIME_LIB) $(gmock_LIBS) $(gtest_LIBS)

include test/qabase.mk

endif
