#SUBDIRS = test
AM_CPPFLAGS = -I$(top_srcdir)/src/lib  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(OpenSSL_CFLAGS) $(putools_CFLAGS)  -D_REENTRANT -DSMHI_LOG -DUSE_OPENSSL

bin_PROGRAMS = kvServiced 

kvServiced_SOURCES=kvServiced.cc \
                   ServiceApp.cc  ServiceApp.h\
                   DataReadyInputImpl.cc DataReadyInputImpl.h\
                   ObjReaper.cc ObjReaper.h\
                   ReaperBase.cc ReaperBase.h \
                   ServiceSubscriber.cc ServiceSubscriber.h\
                   InitLogger.cc InitLogger.h\
                   kvSubscriberCollection.cc kvSubscriberCollection.h\
                   kvDataSubscriber.cc kvDataSubscriber.h\
                   kvDataSubscriberInfo.cc kvDataSubscriberInfo.h\
                   kvServiceImpl.cc kvServiceImpl.h\
                   kvModelDataIteratorImpl.cc kvModelDataIteratorImpl.h\
                   kvDataIteratorImpl.cc kvDataIteratorImpl.h\
                   kvRejectedIteratorImpl.cc kvRejectedIteratorImpl.h\
                   kvSubscriberBase.cc kvSubscriberBase.h\
                   kvDataNotifySubscriber.cc kvDataNotifySubscriber.h\
                   DataReadyCommand.cc DataReadyCommand.h\
                   SubscriberThread.cc SubscriberThread.h \
                   kvHintSubscriber.h kvDataNotifySubscriberImpl.h
             
kvServiced_CPPFLAGS = $(AM_CPPFLAGS)
kvServiced_LDFLAGS =  -pthread -export-dynamic
kvServiced_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                   $(top_builddir)/src/lib/miutil/libmiutil.la \
                   $(top_builddir)/src/lib/kvdb/libkvdb.la \
                   $(top_builddir)/src/lib/fileutil/libfileutil.la \
                   $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                   $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                   $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
                   $(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
                   $(top_builddir)/src/lib/kvskel/libkvskel.la \
                   $(top_builddir)/src/lib/milog/libmilog.la \
                   $(putools_LIBS) $(omniORB4_LIBS) -lomnisslTP4 $(OpenSSL_LIBS) $(BOOST_THREAD_LIB) \
                   -ldl -lm
