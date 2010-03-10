bin_PROGRAMS = kv2kro

kv2kro_SOURCES = GetData.cc GetData.h \
                 App.cc App.h \
                 DataReceiver.cc DataReceiver.h \
                 kv2kro.cc

kv2kro_CPPFLAGS = -D_REENTRANT \
                  -I$(top_builddir)/src/lib \
                  -I$(top_builddir)/src/lib/kvskel \
                  -I$(top_srcdir)/src/lib  \
                  -I$(top_srcdir)/src/service-libs \
                  $(omniORB4_CFLAGS)  \
                  $(putools_CFLAGS)  
kv2kro_LDFLAGS = -pthread 
kv2kro_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
               $(top_builddir)/src/lib/miutil/libmiutil.la \
               $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
               $(top_builddir)/src/lib/fileutil/libfileutil.la \
               $(top_builddir)/src/lib/kvdb/libkvdb.la \
               $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
               $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
               $(top_builddir)/src/lib/kvskel/libkvskel.la \
               $(top_builddir)/src/lib/milog/libmilog.la \
               $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
               $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) \
              -lm 
             
EXTRA_DIST = README delete_oldfiles.pl obsInsert obsInsertTest.pl               
