SUBDIRS=test
pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
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
synop_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
                  $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                  $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                  $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                  $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                  $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                  $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB)

synop_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@


CLEANFILES = synopLexer.cc