AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

noinst_PROGRAMS = createTestData

bin_PROGRAMS = kvDataInputd kvdataclt


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


#To prevent that we make dependency on a large number of
#of package that we do not need we only compile the files that is needed.
kvdataclt_SOURCES =  $(top_srcdir)/src/lib/kvalobs/kvapp.h \
                     $(top_srcdir)/src/lib/kvalobs/kvapp.cc \
                     $(top_srcdir)/src/lib/kvalobs/kvPath.h \
                     $(top_srcdir)/src/lib/kvalobs/kvPath.cc \
                     $(top_srcdir)/src/lib/fileutil/pidfileutil.h \
                     $(top_srcdir)/src/lib/fileutil/pidfileutil.cc \
                     $(top_srcdir)/src/lib/miutil/trimstr.h \
                     $(top_srcdir)/src/lib/miutil/trimstr.cc \
                     kvdataclt.cc

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
                     $(putools_LIBS) $(omniORB4_LIBS) $(libxmlpp_LIBS) $(BOOST_THREAD_LIB)  -ldl


kvdataclt_CPPFLAGS = $(AM_CPPFLAGS) -static \
                     -DPKGLIBDIR="\""$(pkglibdir)"\"" \
                     -DSYSCONFDIR="\""$(sysconfdir)"\"" \
                     -DLIBDIR="\""$(libdir)"\"" \
                     -DBINDIR="\""$(bindir)"\"" \
                     -DDATADIR="\""$(datadir)"\"" \
                     -DLOCALSTATEDIR="\""$(localstatedir)"\"" \
                     -DPREFIX="\""$(prefix)"\"" 

kvdataclt_LDFLAGS = -pthread -static  
kvdataclt_LDADD = $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                  $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
                  $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                  $(top_builddir)/src/lib/milog/libmilog.la \
                  $(top_builddir)/src/lib/kvskel/libkvskel.la \
                  $(omniORB4_LIBS) 

#kvdataclt_LDADD = $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
#                         $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
#                         $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
#                         $(top_builddir)/src/lib/milog/libmilog.la \
#                         $(top_builddir)/src/lib/miutil/libmiutil.la \
#                         $(top_builddir)/src/lib/kvskel/libkvskel.la \
#                         $(omniORB4_LIBS) $(BOOST_THREAD_LIB) -ldl
