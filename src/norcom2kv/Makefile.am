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
norcom2kv_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                   $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                  $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                  $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                  $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                  $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                  $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                  $(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
                  $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                  $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                  $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_REGEX_LIB) $(BOOST_THREAD_LIB) \
                  -ldl -lm
