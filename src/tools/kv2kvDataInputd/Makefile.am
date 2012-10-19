bin_PROGRAMS = kv2kvDataInputd

kv2kvDataInputd_SOURCES = \
                 App.cc App.h \
                 DataReceiver.cc DataReceiver.h \
                 DataHelper.cc DataHelper.h \
                 CorbaServerConf.cc CorbaServerConf.h\
                 kv2kvDataInputd.cc

kv2kvDataInputd_CPPFLAGS = -D_REENTRANT \
                  -I$(top_builddir)/src/lib \
                  -I$(top_builddir)/src/lib/kvskel \
                  -I$(top_srcdir)/src/lib  \
                  -I$(top_srcdir)/src/service-libs \
                  $(omniORB4_CFLAGS)  \
                  $(putools_CFLAGS)  
kv2kvDataInputd_LDFLAGS = -pthread 
kv2kvDataInputd_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
                $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
		$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
		$(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
		$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
		$(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
		$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
		$(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
               $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) \
              -lm 

#EXTRA_DIST =                
