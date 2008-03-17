decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kv2kvdecoder.la
kv2kvdecoder_la_SOURCES = kv2kvDecoder.h kv2kvDecoder.cc kv2kvEntry.h kv2kvEntry.cc
kv2kvdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kv2kvdecoder_la_LIBS =
kv2kvdecoder_la_LDFLAGS = -module