decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = rejectdecoder.la
rejectdecoder_la_SOURCES = rejectdecoder.h rejectdecoder.cc rejectentry.h rejectentry.cc
rejectdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS)
rejectdecoder_la_LDFLAGS = -module