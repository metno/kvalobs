decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = synop.la
AM_LFLAGS = -PyySynop -olex.yy.c
synop_la_SOURCES = decode.cc  \
                   kvSynopDecoder.cc  kvSynopDecoder.h  \
                   synop.cc  synop.h \
                   synopdecoder.cc  synopdecoder.h  \
                   synopElements.h \
                   synopentry.cc  synopentry.h \
                   synopLexer.h synopLexer.ll
synop_la_CPPFLAGS = -D_REENTRANT \
                    -I$(top_srcdir)/src/lib \
                    -I$(top_srcdir)/src/lib/decoder \
                    $(putools_CFLAGS)
synop_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs2_decoderbase.la \
                  $(top_builddir)/src/lib/decodeutility/libkvalobs2_decodeutility.la \
                  $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                  $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                  $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
                  $(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
                  $(BOOST_THREAD_LIB)

synop_la_LDFLAGS = -module

TESTS=kvSynopDecodeTest
check_PROGRAMS=kvSynopDecodeTest

kvSynopDecodeTest_SOURCES = test/kvSynopDecodeTest.cc \
							kvSynopDecoder.cc  kvSynopDecoder.h \
							synop.cc  synop.h \
							synopElements.h \
							synopLexer.h synopLexer.ll

kvSynopDecodeTest_CPPFLAGS = $(synop_la_CPPFLAGS) $(gtest_CFLAGS)
kvSynopDecodeTest_LDFLAGS = \
	$(putools_LIBS) \
	$(gtest_LIBS) \
    $(synop_la_LIBADD)
