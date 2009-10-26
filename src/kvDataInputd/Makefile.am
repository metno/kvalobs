AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(OpenSSL_CFLAGS) $(putools_CFLAGS)  -D_REENTRANT -DSMHI_LOG -DUSE_OPENSSL

noinst_PROGRAMS = createTestData

bin_PROGRAMS = kvDataInputd kvDataInputd_clt


createTestData_SOURCES = createTestData.cc

createTestData_CPPFLAGS = $(AM_CPPFLAGS)
createTestData_LDFLAGS = -pthread 
createTestData_LDADD = $(putools_LIBS) 



 

kvDataInputd_SOURCES=InitLogger.cc InitLogger.h \
                     DataSrcImpl.cc DataSrcImpl.h \
                     DecodeCommand.cc DecodeCommand.h \
	             DataSrcApp.cc DataSrcApp.h \
                     ConnectionCache.cc ConnectionCache.h \
                     force.cc \
                     kvDataInputd.cc 

kvDataInputd_clt_SOURCES = kvinputd_clt.cc

kvDataInputd_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS)
kvDataInputd_LDFLAGS = -pthread -export-dynamic
kvDataInputd_LDADD = $(top_builddir)/src/lib/decoder/decoderbase/libdecoderbase.la \
		     $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
		     $(top_builddir)/src/lib/miutil/libmiutil.la \
                     $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                     $(top_builddir)/src/lib/kvdb/libkvdb.la \
                     $(top_builddir)/src/lib/fileutil/libfileutil.la \
                     $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                     $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
                     $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                     $(top_builddir)/src/lib/milog/libmilog.la \
                     $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                     $(top_builddir)/src/lib/kvskel/libkvskel.la \
                     $(putools_LIBS) $(omniORB4_LIBS) -lomnisslTP4 $(OpenSSL_LIBS) $(libxmlpp_LIBS) $(BOOST_THREAD_LIB)  -ldl

kvDataInputd_clt_CPPFLAGS = $(AM_CPPFLAGS)
kvDataInputd_clt_LDFLAGS = -pthread -export-dynamic 
kvDataInputd_clt_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                         $(top_builddir)/src/lib/kvdb/libkvdb.la \
                         $(top_builddir)/src/lib/fileutil/libfileutil.la \
                         $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                         $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
                         $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                         $(top_builddir)/src/lib/milog/libmilog.la \
                         $(top_builddir)/src/lib/miutil/libmiutil.la \
                         $(top_builddir)/src/lib/kvskel/libkvskel.la \
                         $(putools_LIBS) $(omniORB4_LIBS) -lomnisslTP4 $(OpenSSL_LIBS) $(libxmlpp_LIBS) $(BOOST_THREAD_LIB) -ldl
