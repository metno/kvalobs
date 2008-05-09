decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = dummydecoder.la

dummydecoder_la_SOURCES = dummydecoder.cc  dummydecoder.h  dummyentry.cc  dummyentry.h
dummydecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS)
dummydecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libdecoderbase.la \
		         $(top_builddir)/src/lib/milog/libmilog.la \
                         $(top_builddir)/src/lib/kvdb/libkvdb.la \
                         $(putools_LIBS) 
dummydecoder_la_LDFLAGS = -module
