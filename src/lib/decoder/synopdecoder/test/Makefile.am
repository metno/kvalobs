AM_CPPFLAGS = \
	-I$(top_srcdir)/src/lib/decoder/synopdecoder \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/lib/decoder \
	-I$(top_builddir)/src/lib \
	$(putools_CFLAGS) \
	-D_REENTRANT

if HAVE_GTEST

TESTS=kvSynopDecodeTest
check_PROGRAMS=kvSynopDecodeTest
AM_LFLAGS = -PyySynop -olex.yy.c

testdb=$(top_builddir)/src/lib/decoder/synopdecoder/test/testdb.sqlite

CLEANFILES = $(testdb) synopLexer.cc


kvSynopDecodeTest_SOURCES = \
	kvSynopDecodeTest.cc \
	$(top_srcdir)/src/lib/decoder/decoderbase/test/ReadTypesFromFile.cc \
   $(top_srcdir)/src/lib/decoder/decoderbase/test/ReadDataFromFile.cc \
	$(top_srcdir)/src/lib/decoder/synopdecoder/kvSynopDecoder.cc  \
	$(top_srcdir)/src/lib/decoder/synopdecoder/synop.cc \
	$(top_srcdir)/src/lib/decoder/synopdecoder/synopLexer.ll

kvSynopDecodeTest_CPPFLAGS = $(AM_CPPFLAGS) $(synop_la_CPPFLAGS) $(gtest_CFLAGS) \
					-DDECODERBASE_TESTDIR="\""$(top_srcdir)/src/lib/decoder/decoderbase/test"\"" \
					-DTESTDIR="\""$(top_srcdir)/src/lib/decoder/synopdecoder/test"\"" \
					-DTESTDB="\""$(testdb)"\"" \
					-DDECODERDIR="\""$(top_builddir)/src/lib/decoder/synopdecoder/.libs"\"" \
					-DDBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\""
kvSynopDecodeTest_LDFLAGS = \
				$(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
				$(top_builddir)/src/lib/decoder/synopdecoder/synop.la \
            $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
            $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
            $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
            $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
            $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
            $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
            $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
            $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
				$(putools_LIBS) \
				$(gtest_LIBS) \
    			$(synop_la_LIBADD) \
    			$(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB)

endif

