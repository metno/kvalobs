pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kv2kvdecoder.la
kv2kvdecoder_la_SOURCES = kv2kvDecoder.h kv2kvDecoder.cc kv2kvEntry.h kv2kvEntry.cc
kv2kvdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kv2kvdecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
			 $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
		     $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
             $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
             $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 

#$(libxml2_LIBS)
kv2kvdecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
