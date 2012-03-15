#SUBDIRS = test
AM_CPPFLAGS = -I$(top_srcdir)/src/lib  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

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
kvServiced_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                   $(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
                   $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
                   $(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
                   $(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
                   $(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
                   $(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la\
                   $(top_builddir)/src/lib/kvskel/libkvalobs2_kvadmininterface.la \
                   $(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
                   $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                   $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) \
                   -ldl -lm
