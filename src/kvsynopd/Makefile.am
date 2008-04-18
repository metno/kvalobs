AM_CPPFLAGS = -D_REENTRANT \
              -I$(top_builddir)/src/lib \
              -I$(top_builddir)/src/lib/kvskel \
              -I$(top_srcdir)/src/lib  \
              $(omniORB4_CFLAGS)  \
              $(putools_CFLAGS)  

bin_PROGRAMS = kvsynopd kvsynopclt kvsynopdbclt kvsynopdparse

kvsynopd_SOURCES = kvsynopd.cc \
                   App.cc App.h \
                   DataReceiver.cc DataReceiver.h \
                   Data.cc Data.h \
                   SynopWorker.cc SynopWorker.h \
                   SynopData.cc SynopData.h \
                   DataList.cc DataList.h \
                   StationInfo.cc StationInfo.h \
                   StationInfoParse.cc StationInfoParse.h \
                   tblSynop.cc tblSynop.h \
                   kvsynopdImpl.cc kvsynopdImpl.h \
                   Replay.cc Replay.h \
                   synop.cc synop.h \
                   ValidData.cc ValidData.h \
                   tblWaiting.cc tblWaiting.h \
                   Waiting.cc Waiting.h \
                   delaycontrol.cc delaycontrol.h \
                   tblKeyVal.cc tblKeyVal.h \
                   InitLogger.cc InitLogger.h \
                   getDataReceiver.cc getDataReceiver.h \
                   GetDataThread.cc GetDataThread.h \
                   kvsynopdImpl.h obsevent.h

kvsynopd_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/service-libs \
                    $(omniORB4_CFLAGS) 
kvsynopd_LDFLAGS = -pthread 
kvsynopd_LIBS=   $(omniORB4_LIBS) $(putools_LIBS) $(BOOST_THREAD_LIB) \
                 -lm

kvsynopd_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                 $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                 $(top_builddir)/src/lib/miutil/libmiutil.la \
                 $(top_builddir)/src/lib/kvdb/libkvdb.la \
                 $(top_builddir)/src/lib/fileutil/libfileutil.la \
                 $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
                 $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                 $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                 $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                 $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                 $(top_builddir)/src/lib/kvskel/libkvskel.la \
                 $(top_builddir)/src/lib/milog/libmilog.la 
            
EXTRA_DIST = kvsynopd_schema.sql
 
kvsynopclt_SOURCES = kvsynopclt.cc \
                     kvsynopCltSynopcbImp.cc kvsynopCltSynopcbImp.h \
                     kvsynopCltApp.cc  kvsynopCltApp.h \
                     kvsynopCorbaThread.cc kvsynopCorbaThread.h
kvsynopclt_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/service-libs \
                      $(omniORB4_CFLAGS)
kvsynopclt_LDFLAGS = -pthread 
kvsynopclt_LIBS = $(omniORB4_LIBS) $(putools_LIBS) $(BOOST_THREAD_LIB) \
                  -lm
kvsynopclt_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                   $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                   $(top_builddir)/src/lib/miutil/libmiutil.la \
                   $(top_builddir)/src/lib/kvdb/libkvdb.la \
                   $(top_builddir)/src/lib/fileutil/libfileutil.la \
                   $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
                   $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                   $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                   $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                   $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                   $(top_builddir)/src/lib/kvskel/libkvskel.la \
                   $(top_builddir)/src/lib/milog/libmilog.la 
                   
kvsynopdbclt_SOURCES = kvsynopdbclt.cc \
                       tblSynop.cc tblSynop.h
kvsynopdbclt_LDFLAGS = -pthread 
kvsynopdbclt_LIBS = $(omniORB4_LIBS) $(putools_LIBS) $(BOOST_THREAD_LIB) \
                  -lm
kvsynopdbclt_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                     $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                     $(top_builddir)/src/lib/miutil/libmiutil.la \
                     $(top_builddir)/src/lib/kvdb/libkvdb.la \
                     $(top_builddir)/src/lib/fileutil/libfileutil.la \
                     $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
                     $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                     $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                     $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                     $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                     $(top_builddir)/src/lib/kvskel/libkvskel.la \
                     $(top_builddir)/src/lib/milog/libmilog.la 

kvsynopdparse_SOURCES = kvsynopdparse.cc \
                        StationInfo.cc StationInfo.h \
                        StationInfoParse.cc StationInfoParse.h
kvsynopdparse_LDFLAGS = -pthread 
kvsynopdparse_LIBS = $(omniORB4_LIBS) $(putools_LIBS) $(BOOST_THREAD_LIB) \
                  -lm
kvsynopdparse_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                     $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                     $(top_builddir)/src/lib/miutil/libmiutil.la \
                     $(top_builddir)/src/lib/kvdb/libkvdb.la \
                     $(top_builddir)/src/lib/fileutil/libfileutil.la \
                     $(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
                     $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                     $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                     $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                     $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                     $(top_builddir)/src/lib/kvskel/libkvskel.la \
                     $(top_builddir)/src/lib/milog/libmilog.la 
                        
sqldir=$(datadir)/kvalobs/kvsynop
sql_DATA=kvsynopd_schema.sql
bin_SCRIPTS = kvsynopd_initdb
