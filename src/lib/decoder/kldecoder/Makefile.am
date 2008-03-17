decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kldecoder.la
kldecoder_la_SOURCES = kldata.h  kldecoder.cc  kldecoder.h  klentry.cc  klentry.h  paramdef.cc  paramdef.h
kldecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kldecoder_la_LIBS =
kldecoder_la_LDFLAGS = -module