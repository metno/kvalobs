decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = execdecoder.la

execdecoder_la_SOURCES = execdecoder.cc  execdecoder.h  execentry.cc  execentry.h
execdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS)
execdecoder_la_LIBADD = \
                 $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
		         $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                 $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 
execdecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
