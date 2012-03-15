AM_CPPFLAGS = -I$(top_srcdir)/src/lib \
              -I$(top_builddir)/src/lib \
              -I$(top_builddir)/src/lib/kvskel  \
              $(omniORB4_CFLAGS)  $(putools_CFLAGS) \
              -D_REENTRANT

bin_PROGRAMS = data2kv 

data2kv_SOURCES=data2kv.cc \
                App.cc App.h \
                CollectData.cc CollectData.h \
                newfileImpl.cc newfileImpl.h \
     			kvDataSrcList.cc kvDataSrcList.h \
     			NewData.cc NewData.h \
     			AdminImpl.cc AdminImpl.h
     				 

data2kv_CPPFLAGS = $(AM_CPPFLAGS) 
data2kv_LDFLAGS = -pthread
data2kv_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                $(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
                $(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
                $(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
                $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                $(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
                $(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
                $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB)
