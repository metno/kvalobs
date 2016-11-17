SUBDIRS=test
AM_CPPFLAGS = \
  -I$(top_srcdir)/src \
  -I$(top_srcdir)/src/lib \
  -I$(top_srcdir)/src/lib/decoder \
  -D_REENTRANT 

lib_LTLIBRARIES = libkvalobs_decoderbase.la

decoderbaseincludedir = $(pkgincludedir)/decoder/decoderbase
decoderbaseinclude_HEADERS = \
	decoder.h GenCacheElem.h metadata.h StationFilter.h

libkvalobs_decoderbase_la_SOURCES = \
  decodermgr.cc decodermgr.h \
  decoder.cc decoder.h\
  metadata.cc metadata.h \
  DataUpdateTransaction.h DataUpdateTransaction.cc \
  GenCacheElem.h ConfParser.h \
  StationFilter.cc StationFilter.h \
  RedirectInfo.cc RedirectInfo.h

libkvalobs_decoderbase_la_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
	-DPKGLIB_DECODE_DIR="\""$(pkglibdir)/decode"\"" \
	$(putools_CFLAGS)\
	$(libxmlpp_CFLAGS)

libkvalobs_decoderbase_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_decoderbase_la_LIBADD =   $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                             $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                             $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                             $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                             $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                             $(BOOST_DATE_TIME_LIB) \
                             $(libxml2_LIBS)

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_decoderbase.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
