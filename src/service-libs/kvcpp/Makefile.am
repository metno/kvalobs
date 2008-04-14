SUBDIRS = corba

lib_LTLIBRARIES = libkvcpp.la

libkvcpp_la_SOURCES = \
	KvApp.cc  \
	kvDataSubscribeInfoHelper.cc \
	kveventinterface.cc \
	kvevents.cc \
	KvObsData.cc \
	kvWhat.cc \
	RejectdecodeIterator.cc \
	WhichDataHelper.cc

kvcppincludedir = $(pkgincludedir)/kvcpp
	
kvcppinclude_HEADERS = \
	KvApp.h \
	kvDataSubscribeInfoHelper.h \
	kveventinterface.h \
	kvevents.h \
	KvObsData.h \
	KvGetDataReceiver.h \
	kvservicetypes.h \
	kvWhat.h \
	RejectdecodeIterator.h \
	WhichDataHelper.h
	

libkvcpp_la_LIBADD = \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(top_builddir)/src/lib/decodeutility/libdecodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs.la \
	$(top_builddir)/src/lib/miutil/libmiutil.la \
	$(top_builddir)/src/lib/kvdb/libkvdb.la \
	$(top_builddir)/src/lib/fileutil/libfileutil.la \
	$(top_builddir)/src/lib/milog/libmilog.la \
	$(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
	$(top_builddir)/src/lib/kvskel/libkvskel.la \
	$(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
	$(top_builddir)/src/lib/dnmithread/libdnmithread.la \
	$(top_builddir)/src/lib/kvskel/libkvadmininterface.la 
	
	
	
libkvcpp_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS)


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = kvcpp.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
