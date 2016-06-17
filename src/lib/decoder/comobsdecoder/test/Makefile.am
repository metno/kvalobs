AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/lib/decoder \
	-I$(top_builddir)/src/lib \
	$(putools_CFLAGS) \
	-D_REENTRANT

if HAVE_GMOCK 
EXTRA_DIST = param.csv \
             testdata/sms02-29-201012280654.xml
             
clean-local:
	rm -rf testresult
			 
check_PROGRAMS = gtestsms2 
TESTS = $(check_PROGRAMS)
bin_PROGRAMS = testsms2  

testsms2_SOURCES=testsms2.cc ReadParamsFromFile.cc ReadParamsFromFile.h\
				 FakeComobsDecoder.h FakeComobsDecoder.cc \
				 ../comobsdecode.cc ../decodeddata.cc \
				  ../smsmeldingparser.cc ../smsmelding.cc\
				../sms2.cc ../sms13.cc ../smsbase.cc  ../smsdata.cc

testsms2_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS) \
					-DTESTDIR="\""$(top_srcdir)/src/lib/decoder/comobsdecoder/test"\"" \
					-DDBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\"" 
testsms2_LDFLAGS = -pthread -export-dynamic
testsms2_LDADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
               $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                $(putools_LIBS) $(libxml2_LIBS) \
	 	        $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) \
		        -lm -ldl

gtestsms2_SOURCES=gtestsms2.cc ReadParamsFromFile.cc \
				 FakeComobsDecoder.h FakeComobsDecoder.cc \
				 ../comobsdecode.cc ../decodeddata.cc \
				  ../smsmeldingparser.cc ../smsmelding.cc\
				../sms2.cc ../sms13.cc ../smsbase.cc  ../smsdata.cc

gtestsms2_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS) $(synop_la_CPPFLAGS) $(gtest_CFLAGS) \
					-DDECODERBASE_TESTDIR="\""$(top_srcdir)/src/lib/decoder/decoderbase/test"\"" \
					-DTESTDIR="\""$(top_srcdir)/src/lib/decoder/comobsdecoder/test"\"" \
					-DDBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\""
gtestsms2_LDFLAGS =  -pthread -export-dynamic
gtestsms2_LDADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
               $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                $(putools_LIBS) $(libxml2_LIBS) \
	 	$(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) \
		$(gtest_LIBS) \
		-lm -ldl

endif




