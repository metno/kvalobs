AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -D_REENTRANT

lib_LTLIBRARIES = libkvalobs2_decoderbase.la

decoderbaseincludedir = $(pkgincludedir)/decoder/decoderbase
decoderbaseinclude_HEADERS = decoder.h GenCacheElem.h metadata.h

libkvalobs2_decoderbase_la_SOURCES = decodermgr.cc decodermgr.h \
                            decoder.cc decoder.h\
                            metadata.cc metadata.h \
                            DataUpdateTransaction.h DataUpdateTransaction.cc \
                            GenCacheElem.h ConfParser.h

libkvalobs2_decoderbase_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
	-DPKGLIB_DECODE_DIR="\""$(pkglibdir)/decode"\"" \
	$(putools_CFLAGS)\
	$(libxmlpp_CFLAGS)

libkvalobs2_decoderbase_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_decoderbase_la_LIBADD =   $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                             $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                             $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
                             $(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
                             $(BOOST_DATE_TIME_LIB) \
                             $(libxml2_LIBS)
