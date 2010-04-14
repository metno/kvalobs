SUBDIRS = corba mock

lib_LTLIBRARIES = libkvalobs_kvcpp.la

libkvalobs_kvcpp_la_SOURCES = \
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
	
libkvalobs_kvcpp_la_LDFLAGS= -version-info @KVALOBSLIBS_SO_VERSION@

libkvalobs_kvcpp_la_LIBADD = \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la  
	
	
	
libkvalobs_kvcpp_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS)


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvcpp.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
