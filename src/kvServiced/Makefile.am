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
kvServiced_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                   $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                   $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                   $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                   $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                   $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                   $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                   $(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
                   $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                   $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                   $(putools_LIBS) $(omniORB4_LIBS) $(BOOST_THREAD_LIB) \
                   -ldl -lm
