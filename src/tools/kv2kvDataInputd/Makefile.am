bin_PROGRAMS = kv2kvDataInputd

kv2kvDataInputd_SOURCES = \
                 App.cc App.h \
                 DataReceiver.cc DataReceiver.h \
                 DataHelper.cc DataHelper.h \
                 CorbaServerConf.cc CorbaServerConf.h\
                 kv2kvDataInputd.cc

kv2kvDataInputd_CPPFLAGS = -D_REENTRANT \
                  -I$(top_srcdir)/src \
                  -I$(top_builddir)/src/lib \
                  -I$(top_builddir)/src/lib/kvskel \
                  -I$(top_srcdir)/src/lib  \
                  -I$(top_srcdir)/src/service-libs \
                  $(omniORB4_CFLAGS)  \
                  $(putools_CFLAGS)  
kv2kvDataInputd_LDFLAGS = -pthread 
kv2kvDataInputd_LDADD = \
		$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
		$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
		$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
		$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
		$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
		$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
		$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
		$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
		$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
		$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
		$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
		$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la\
		$(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB) \
		-lm 

#EXTRA_DIST =                
