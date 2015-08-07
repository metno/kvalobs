AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/lib/decoder \
	-I$(top_builddir)/src/lib \
	$(putools_CFLAGS) \
	-D_REENTRANT

#testdecoderdir=$(top_builddir)/src/lib/decoder/decoderbase/test
noinst_LIBRARIES = libtestdecoder.a

libtestdecoder_a_SOURCES = testdecoder.cc  testdecoder.h  testentry.cc  testentry.h  \
	ReadTypesFromFile.h ReadTypesFromFile.cc \
	ReadDataFromFile.h ReadDataFromFile.cc

libtestdecoder_a_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder \
                          $(putools_CFLAGS)

#libtestdecoder_a_LIBADD = \
#                 $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
#		         $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
#		         $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
#                 $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
#                 $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la 
                 
#The -rpath /nowhere is a hack to force check_LTLIBRARIES to create a shared library
#libtestdecoder_a_LDFLAGS = -module -rpath /nowhere -version-info @KVALOBSLIBS_SO_VERSION@

if HAVE_GMOCK

testdb=$(top_builddir)/src/lib/decoder/decoderbase/test/testdb.sqlite
EXTRA_DIST = kvparams.csv \
             kvtypes.csv 
             
CLEANFILES = $(testdb)
#TESTS = $(check_PROGRAMS)

check_PROGRAMS = testDecoderBase
 
testDecoderBase_SOURCES = testDecoderBase.cc
	
#ReadParamsFromFile.h ReadParamsFromFile.cc 

testDecoderBase_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS) $(synop_la_CPPFLAGS) $(gtest_CFLAGS) \
					-DTESTDIR="\""$(top_srcdir)/src/lib/decoder/decoderbase/test"\"" \
					-DTESTDB="\""$(testdb)"\"" \
					-DDECODERDIR="\""$(top_builddir)/src/lib/decoder/decoderbase/test/.libs"\"" \
					-DDBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\""
testDecoderBase_LDFLAGS =  -pthread -export-dynamic
testDecoderBase_LDADD = libtestdecoder.a $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
               $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                $(putools_LIBS) $(libxml2_LIBS) \
	 	        $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(gtest_LIBS) \
	 	        $(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) \
		        -lm -ldl

endif
