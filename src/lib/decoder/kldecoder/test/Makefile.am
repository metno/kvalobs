AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/lib/decoder \
	-I$(top_builddir)/src/lib \
	$(putools_CFLAGS) \
	-D_REENTRANT

if HAVE_GMOCK


##check_LTLIBRARIES = testdecoder.la
check_PROGRAMS = testKlDecoder

#testdecoder_la_SOURCES = testdecoder.cc  testdecoder.h  testentry.cc  testentry.h
#testdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder \
                          $(putools_CFLAGS)
#testdecoder_la_LIBADD = \
#                 $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
#		         $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
#                 $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la
                 
#The -rpath /nowhere is a hack to force check_LTLIBRARIES to create a shared library
#testdecoder_la_LDFLAGS = -module -rpath /nowhere -version-info @KVALOBSLIBS_SO_VERSION@

testdb=$(top_builddir)/src/lib/decoder/kldecoder/test/testdb.sqlite
#EXTRA_DIST = kvparam.csv \
#             kvtypes.csv 
             
EXTRA_DIST = n59680-t311.dat
CLEANFILES = $(testdb)
TESTS = $(check_PROGRAMS)
 
testKlDecoder_SOURCES=testKlDecoder.cc \
	../DataDecode.cc 

testKlDecoder_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS) $(synop_la_CPPFLAGS) $(gtest_CFLAGS) \
					-DDECODERBASE_TESTDIR=\"$(top_srcdir)/src/lib/decoder/decoderbase/test\" \
					-DTESTDIR=\"$(top_srcdir)/src/lib/decoder/kldecoder/test\" \
					-DTESTDB=\"$(testdb)\" \
					-DDECODERDIR=\"$(top_builddir)/src/lib/decoder/kldecoder/.libs\" \
					-DDBDIR=\"$(top_builddir)/src/lib/dbdrivers/.libs\"
testKlDecoder_LDFLAGS =  -pthread -export-dynamic
testKlDecoder_LDADD = \
				$(top_builddir)/src/lib/decoder/decoderbase/test/libtestdecoder.a \
				$(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
				$(top_builddir)/src/lib/decoder/kldecoder/kldecoder.la \
        $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
        $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
        $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
        $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
        $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
        $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
        $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
        $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
        $(putools_LIBS) $(libxml2_LIBS) \
	 	    $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(gtest_LIBS) \
	 	    $(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) \
		    -lm -ldl

endif
