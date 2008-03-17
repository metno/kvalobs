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
	../../lib/decodeutility/libdecodeutility.la \
	../../lib/kvalobs/libkvalobs.la \
	../../lib/miutil/libmiutil.la \
	../../lib/kvdb/libkvdb.la \
	../../lib/fileutil/libfileutil.la \
	../../lib/milog/libmilog.la \
	../../lib/miconfparser/libmiconfparser.a \
	../../lib/kvskel/libkvskel.la \
	../../lib/corbahelper/libcorbahelper.la \
	../../lib/dnmithread/libdnmithread.la \
	../../lib/kvskel/libkvadmininterface.la 
	
	
	
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
