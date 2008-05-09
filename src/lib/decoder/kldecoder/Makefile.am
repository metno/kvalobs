decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kldecoder.la
kldecoder_la_SOURCES = kldata.h  kldecoder.cc  kldecoder.h  klentry.cc  klentry.h  paramdef.cc  paramdef.h
kldecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kldecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libdecoderbase.la \
                      $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
                      $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                      $(top_builddir)/src/lib/milog/libmilog.la \
                      $(top_builddir)/src/lib/kvdb/libkvdb.la \
                      $(putools_LIBS) 

kldecoder_la_LDFLAGS = -module
