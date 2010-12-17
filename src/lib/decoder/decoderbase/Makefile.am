AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -D_REENTRANT

lib_LTLIBRARIES = libkvalobs_decoderbase.la

libkvalobs_decoderbase_la_SOURCES = decodermgr.cc decodermgr.h \
                            decoder.cc decoder.h\
                            metadata.cc metadata.h \
                            DataUpdateTransaction.h DataUpdateTransaction.cc \
                            GenCacheElem.h ConfParser.h
                            
libkvalobs_decoderbase_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) $(libxmlpp_CFLAGS)
libkvalobs_decoderbase_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_decoderbase_la_LIBADD =   $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                             $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                             $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                             $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
			                    $(putools_LIBS) $(libxml2_LIBS)
