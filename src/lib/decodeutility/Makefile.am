AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib/ -I$(top_builddir)/src/lib/kvskel

lib_LTLIBRARIES = libkvalobs2_decodeutility.la


decodeutilityincludedir = $(pkgincludedir)/decodeutility
nobase_decodeutilityinclude_HEADERS = 	decodeutility.h \
	kvalobsdata.h \
	kvalobsdataparser.h \
	kvalobsdataserializer.h \
	kvDataFormatter.h \
	DataReinserter.h bits/DataReinserter.tcc \
	sorteddata.h


libkvalobs2_decodeutility_la_SOURCES = \
	decodeutility.h decodeutility.cc \
	kvalobsdata.h kvalobsdata.cc \
	kvalobsdataparser.h kvalobsdataparser.cc \
	kvalobsdataserializer.h kvalobsdataserializer.cc \
	kvDataFormatter.h kvDataFormatter.cc \
	DataReinserter.h bits/DataReinserter.tcc \
	sorteddata.h

libkvalobs2_decodeutility_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) $(libxmlpp_CFLAGS) $(omniORB4_CFLAGS) $(BOOST_CPPFLAGS)
libkvalobs2_decodeutility_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_decodeutility_la_LIBADD = $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                             $(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
                             $(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
			                 $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                             $(libxmlpp_LIBS) \
                             $(omniORB4_LIBS) \
                             $(BOOST_LDFLAGS) \
                             $(BOOST_THREAD_LIB)

TESTS = decodeutilitytest

check_PROGRAMS = decodeutilitytest

decodeutilitytest_SOURCES = \
	test/datareinsertertest.cc \
	test/kvalobsdataserializertest.cc \
	test/kvalobsdatatest.cc \
	test/kvDataFormatterTest.cc \
	test/test.cc

decodeutilitytest_CPPFLAGS = $(libkvalobs2_decodeutility_la_CPPFLAGS) $(gtest_CFLAGS)
	

decodeutilitytest_LDADD = \
	libkvalobs2_decodeutility.la \
	$(libkvalobs2_decodeutility_la_LIBADD) \
	$(putools_LIBS)\
	$(gtest_LIBS)
