AM_CPPFLAGS = -I$(top_srcdir)/src/lib

pkglib_LTLIBRARIES = libdecodeutility.la


decodeutilityincludedir = $(pkgincludedir)/decodeutility
nobase_decodeutilityinclude_HEADERS = 	decodeutility.h \
	kvalobsdata.h \
	kvalobsdataparser.h \
	kvalobsdataserializer.h \
	kvDataFormatter.h \
	DataReinserter.h bits/DataReinserter.tcc \
	sorteddata.h


libdecodeutility_la_SOURCES = \
	decodeutility.h decodeutility.cc \
	kvalobsdata.h kvalobsdata.cc \
	kvalobsdataparser.h kvalobsdataparser.cc \
	kvalobsdataserializer.h kvalobsdataserializer.cc \
	kvDataFormatter.h kvDataFormatter.cc \
	DataReinserter.h bits/DataReinserter.tcc \
	sorteddata.h

libdecodeutility_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) $(libxmlpp_CFLAGS) $(omniORB4_CFLAGS) $(BOOST_CPPFLAGS)

libdecodeutility_la_LIBADD = $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                             $(top_builddir)/src/lib/kvskel/libkvskel.la \
                             $(top_builddir)/src/lib/miutil/libmiutil.la \
			     $(top_builddir)/src/lib/milog/libmilog.la \
                             $(putools_LIBS)  \
                             $(libxmlpp_LIBS) \
                             $(omniORB4_LIBS) \
                             $(BOOST_LDFLAGS) \
                             $(BOOST_THREAD_LIB)

#libdecodeutility_la_LDFLAGS = $(putools_LIBS) $(libxmlpp_LIBS) $(omniORB4_LIBS) $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB)

TESTS = decodeutilitytest

check_PROGRAMS = decodeutilitytest

decodeutilitytest_SOURCES = \
	test/datareinsertertest.h test/datareinsertertest.cc \
	test/kvalobsdataserializertest.h test/kvalobsdataserializertest.cc \
	test/kvalobsdatatest.h test/kvalobsdatatest.cc \
	test/test.cc

decodeutilitytest_CPPFLAGS = $(AM_CPPFLAGS) \
	-I$(top_builddir)/src/lib/kvskel \
	-I.. $(libdecodeutility_la_CPPFLAGS) \
	$(cppunit_CFLAGS)

decodeutilitytest_LDADD = \
	libdecodeutility.la \
	../kvalobs/libkvalobs.la \
	../milog/libmilog.la \
	../miutil/libmiutil.la \
	../kvdb/libkvdb.la \
	../fileutil/libfileutil.la \
	../corbahelper/libcorbahelper.la \
	../miconfparser/libmiconfparser.a \
	../dnmithread/libdnmithread.la

decodeutilitytest_LDFLAGS = $(libdecodeutility_la_LDFLAGS) $(cppunit_LIBS) 
