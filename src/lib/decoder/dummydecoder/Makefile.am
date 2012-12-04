decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = dummydecoder.la

dummydecoder_la_SOURCES = dummydecoder.cc  dummydecoder.h  dummyentry.cc  dummyentry.h
dummydecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS)
dummydecoder_la_LIBADD = \
                 $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
		         $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                 $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 
dummydecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
