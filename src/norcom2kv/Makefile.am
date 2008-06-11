AM_CPPFLAGS = -I$(top_srcdir)/src/lib  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

bin_PROGRAMS = norcom2kv

norcom2kv_SOURCES = norcom2kv.cc \
                    CollectSynop.cc  CollectSynop.h \
                    App.cc App.h \
                    WMORaport.cc WMORaport.h \
                    crc_ccitt.cc crc_ccitt.h \
                    CorbaThread.cc CorbaThread.h \
                    AdminImpl.cc AdminImpl.h \
                    File.cc File.h \
                    InitLogger.cc InitLogger.h \
                    FInfo.h \
                    kvDataSrcList.h

norcom2kv_CPPFLAGS = $(AM_CPPFLAGS)
norcom2kv_LDFLAGS = -pthread 
norcom2kv_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                   $(top_builddir)/src/lib/kvdb/libkvdb.la \
                  $(top_builddir)/src/lib/fileutil/libfileutil.la \
                  $(top_builddir)/src/lib/miutil/libmiutil.la \
                  $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                  $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                  $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
                  $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                  $(top_builddir)/src/lib/kvskel/libkvskel.la \
                  $(top_builddir)/src/lib/milog/libmilog.la \
                  $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_REGEX_LIB) $(BOOST_THREAD_LIB) \
                  -ldl -lm
