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
               $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) \
              -lm 
             
#EXTRA_DIST =                
