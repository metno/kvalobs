AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -D_REENTRANT

pkglib_LTLIBRARIES = libdecoderbase.la

libdecoderbase_la_SOURCES = decodermgr.cc decodermgr.h \
                            decoder.cc decoder.h\
                            metadata.cc metadata.h \
                            GenCacheElem.h ConfParser.h
                            
libdecoderbase_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) $(libxmlpp_CFLAGS)
libdecoderbase_la_LDFLAGS = $(putools_LIBS)