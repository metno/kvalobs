AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

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
data2kv_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
					 $(top_builddir)/src/lib/miutil/libmiutil.la \
                $(top_builddir)/src/lib/fileutil/libfileutil.la \
                $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                $(top_builddir)/src/lib/milog/libmilog.la \
                $(top_builddir)/src/lib/kvskel/libkvskel.la \
                $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB)
