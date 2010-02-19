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
synop_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libdecoderbase.la \
                  $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
                  $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                  $(top_builddir)/src/lib/milog/libmilog.la \
                  $(top_builddir)/src/lib/kvdb/libkvdb.la \
                  $(top_builddir)/src/lib/fileutil/libfileutil.la \
                  $(putools_LIBS) $(BOOST_THREAD_LIB)

synop_la_LDFLAGS = -module

TESTS=kvSynopDecodeTest

check_PROGRAMS=kvSynopDecodeTest

kvSynopDecodeTest_SOURCES = test/kvSynopDecodeTest.cc \
							$(synop_SOURCES:synopentry.cc=)

kvSynopDecodeTest_CPPFLAGS = $(synop_la_CPPFLAGS)
kvSynopDecodeTest_LDFLAGS = -lgtest \
                            $(top_builddir)/src/lib/decoder/synopdecoder \
                            $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                            $(top_builddir)/src/lib/kvalobs/libkvalobs.la
