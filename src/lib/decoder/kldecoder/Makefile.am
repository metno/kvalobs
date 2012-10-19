decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kldecoder.la
kldecoder_la_SOURCES = kldata.h  kldecoder.cc  kldecoder.h  klentry.cc  klentry.h  paramdef.cc  paramdef.h
kldecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kldecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs2_decoderbase.la \
                      $(top_builddir)/src/lib/decodeutility/libkvalobs2_decodeutility.la \
                      $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                      $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                      $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la 

kldecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
