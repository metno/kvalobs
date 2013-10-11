SUBDIRS=test
decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kldecoder.la
kldecoder_la_SOURCES = \
	DataDecode.h DataDecode.cc \
	KvDataContainer.h KvDataContainer.cc \
	kldata.h \
	kldecoder.h kldecoder.cc\
	klentry.h klentry.cc\
	paramdef.h paramdef.cc
kldecoder_la_CPPFLAGS = -fPIC -D_REENTRANT -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) $(libxmlpp_CFLAGS)
kldecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
                      $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                      $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                      $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                      $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 

kldecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
