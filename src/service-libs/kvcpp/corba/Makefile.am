noinst_LTLIBRARIES = libcorbakvapp.la

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
	