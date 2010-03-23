AM_CPPFLAGS = -I$(top_srcdir)/src/lib

lib_LTLIBRARIES = libkvalobs_decodeutility.la


decodeutilityincludedir = $(pkgincludedir)/decodeutility
nobase_decodeutilityinclude_HEADERS = 	decodeutility.h \
	kvalobsdata.h \
	kvalobsdataparser.h \
	kvalobsdataserializer.h \
	kvDataFormatter.h \
	DataReinserter.h bits/DataReinserter.tcc \
	sorteddata.h


libkvalobs_decodeutility_la_SOURCES = \
	decodeutility.h decodeutility.cc \
	kvalobsdata.h kvalobsdata.cc \
	kvalobsdataparser.h kvalobsdataparser.cc \
	kvalobsdataserializer.h kvalobsdataserializer.cc \
	kvDataFormatter.h kvDataFormatter.cc \
	DataReinserter.h bits/DataReinserter.tcc \
	sorteddata.h

libkvalobs_decodeutility_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) $(libxmlpp_CFLAGS) $(omniORB4_CFLAGS) $(BOOST_CPPFLAGS)
libkvalobs_decodeutility_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_decodeutility_la_LIBADD = $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                             $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                             $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
			                 $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                             $(putools_LIBS)  \
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

decodeutilitytest_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(top_srcdir)/src/service-libs \
	-I$(top_builddir)/src/lib/kvskel \
	-I.. $(libdecodeutility_la_CPPFLAGS) \
	$(gtest_CFLAGS)
	

decodeutilitytest_LDADD = \
	libdecodeutility.la \
	../kvalobs/libkvalobs_kvalobs.la \
	../milog/libkvalobs_milog.la \
	../miutil/libkvalobs_miutil.la \
	../kvdb/libkvalobs_kvdb.la \
	../fileutil/libkvalobs_fileutil.la \
	../corbahelper/libkvalobs_corbahelper.la \
	../miconfparser/libkvalobs_miconfparser.a \
	../dnmithread/libkvalobs_dnmithread.la \
	$(gtest_LIBS)

decodeutilitytest_LDFLAGS = $(libdecodeutility_la_LDFLAGS) $(cppunit_LIBS) 
