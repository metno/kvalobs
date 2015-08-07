AM_CPPFLAGS = -I$(top_srcdir)/src/lib \
               -I$(top_srcdir)/src/lib/decoder \
               -I$(top_builddir)/src/lib \
               -I$(top_builddir)/src/lib/kvskel  \
               $(omniORB4_CFLAGS)  $(putools_CFLAGS)\
               -D_REENTRANT

noinst_PROGRAMS = createTestData

bin_PROGRAMS = kvDataInputd kvdataclt
dist_bin_SCRIPTS = kvDataInputd_ExecDecoder_log_cleanup


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



kvDataInputd_CPPFLAGS = $(AM_CPPFLAGS) $(libxmlpp_CFLAGS)
kvDataInputd_LDFLAGS = -pthread -export-dynamic
kvDataInputd_LDADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
		     		 $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
		     		 $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                     $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                     $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                     $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                     $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                     $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                     $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                     $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                     $(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
                     $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
		     $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
                     $(putools_LIBS) $(omniORB4_LIBS) $(libxmlpp_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) -lboost_system -ldl


kvdataclt_SOURCES =  kvdataclt.cc


kvdataclt_CPPFLAGS = $(AM_CPPFLAGS) \
                     -DPKGLIBDIR="\""$(pkglibdir)"\"" \
                     -DSYSCONFDIR="\""$(sysconfdir)"\"" \
                     -DLIBDIR="\""$(libdir)"\"" \
                     -DBINDIR="\""$(bindir)"\"" \
                     -DDATADIR="\""$(datadir)"\"" \
                     -DLOCALSTATEDIR="\""$(localstatedir)"\"" \
                     -DPREFIX="\""$(prefix)"\"" 

kvdataclt_LDFLAGS = -pthread  
kvdataclt_LDADD = $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                  $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
                  $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                  $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                  $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                  $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                  $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
		  		  $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                  $(putools_LIBS) \
                  $(BOOST_THREAD_LIB)  $(BOOST_SYSTEM_LIB) \
                  $(omniORB4_LIBS) 
