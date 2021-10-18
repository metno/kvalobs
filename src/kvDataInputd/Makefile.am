pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/lib/json \
   -I$(top_srcdir)/src/lib/decoder \
   -I$(top_builddir)/src/lib \
   -I$(top_builddir)/src/lib/kvskel  \
   $(libhttpserver_CFLAGS) \
   $(omniORB4_CFLAGS)  $(putools_CFLAGS)\
   -D_REENTRANT

noinst_PROGRAMS = createTestData 

bin_PROGRAMS = kvDataInputd
dist_bin_SCRIPTS = kvDataInputd_ExecDecoder_log_cleanup


createTestData_SOURCES = createTestData.cc

createTestData_CPPFLAGS = $(AM_CPPFLAGS)
createTestData_LDFLAGS = -pthread 
createTestData_LDADD = $(putools_LIBS) 

kvDataInputd_SOURCES=\
   InitLogger.cc InitLogger.h \
   DecoderExecutor.cc DecoderExecutor.h \
   DecodeCommand.cc DecodeCommand.h \
   DataSrcApp.cc DataSrcApp.h \
   ConnectionCache.cc ConnectionCache.h \
   ObservationHandler.cc ObservationHandler.h \
   RawDataCommand.cc RawDataCommand.h \
   PublishDataCommand.cc PublishDataCommand.h \
   kvDataInputd.cc 

kvDataInputd_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS)
kvDataInputd_LDFLAGS = \
   -pthread -export-dynamic \
   -R "$(pkglibdir)/lib" \
   -R "$(rdkafka_LIBDIR)" \
   -R "$(libhttpserver_LIBDIR)" \
   -R "/usr/local/lib"

kvDataInputd_LDADD = \
   $(top_builddir)/src/lib/json/libjson.la \
   $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
   $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
   $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
   $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
   $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
   $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
   $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
   $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
   $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
   $(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la\
   $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la\
   $(putools_LIBS) $(libxmlpp_LIBS) \
   $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_REGEX_LIB) \
   $(BOOST_SYSTEM_LIB) \
   $(omniORB4_LIBS) \
   -lhttpserver \
   $(libhttpserver_LIBS) \
   $(rdkafka_LIBS) \
   -ldl

#if HAVE_GMOCK
#TESTS = kvDatatInputdTest
#
#check_PROGRAMS = kvDatatInputdTest
#kvDatatInputdTest_LDFLAGS=$(kvDataInputd_LDFLAGS)
#kvDatatInputdTest_SOURCES = \
#	DecoderResult2Json.cc DecoderResult2Json.h \
#	test/getMessageIdTest.cc \
#	test/test.cc
#kvDatatInputdTest_CPPFLAGS = $(kvDataInputd_CPPFLAGS) $(gtest_CFLAGS)
#
#kvDatatInputdTest_LDADD = \
#	$(kvDataInputd_LDADD)\
#	$(gtest_LIBS)
#endif




