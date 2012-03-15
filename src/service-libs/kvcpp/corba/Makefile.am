noinst_LTLIBRARIES = libcorbakvapp.la

libcorbakvappincludedir =  $(pkgincludedir)/kvcpp/corba
libcorbakvappinclude_HEADERS = 	CorbaGetFunction.h \
	CorbaKvApp.h \
	kvCorbaThread.h \
	kvDataNotifySubscriberImpl.h \
	kvDataSubscriberImpl.h \
	kvHintSubscriberImpl.h 


libcorbakvapp_la_SOURCES = \
	CorbaGetFunction.cc CorbaGetFunction.h \
	CorbaKvApp.cc CorbaKvApp.h \
	kvCorbaThread.cc kvCorbaThread.h \
	kvDataNotifySubscriberImpl.cc kvDataNotifySubscriberImpl.h \
	kvDataSubscriberImpl.cc kvDataSubscriberImpl.h \
	kvHintSubscriberImpl.cc kvHintSubscriberImpl.h 

libcorbakvapp_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS) 
	
libcorbakvapp_la_LIBADD = $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                          $(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
                          $(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
                          $(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
                          $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la
 
	