decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kv2kvdecoder.la
kv2kvdecoder_la_SOURCES = kv2kvDecoder.h kv2kvDecoder.cc kv2kvEntry.h kv2kvEntry.cc
kv2kvdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kv2kvdecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs2_decoderbase.la \
			 $(top_builddir)/src/lib/decodeutility/libkvalobs2_decodeutility.la \
		     $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
             $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
             $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la 

#$(libxml2_LIBS)
kv2kvdecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
