decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = rejectdecoder.la
rejectdecoder_la_SOURCES = rejectdecoder.h rejectdecoder.cc rejectentry.h rejectentry.cc
rejectdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS)
rejectdecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
rejectdecoder_la_LIBADD =$(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
			                $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                         $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                         $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 
