SUBDIRS = corba mock

lib_LTLIBRARIES = libkvalobs2_kvcpp.la

libkvalobs2_kvcpp_la_SOURCES = \
	KvApp.cc  \
	kvDataSubscribeInfoHelper.cc \
	kveventinterface.cc \
	kvevents.cc \
	KvObsData.cc \
	kvWhat.cc \
	RejectdecodeIterator.cc \
	WhichDataHelper.cc\
	WorkstatistikIterator.cc

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
	WhichDataHelper.h \
	WorkstatistikIterator.h
	
	
libkvalobs2_kvcpp_la_LDFLAGS= -version-info @KVALOBSLIBS_SO_VERSION@

libkvalobs2_kvcpp_la_LIBADD = \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs2_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
	$(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
	$(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvadmininterface.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la   
	
	
	
libkvalobs2_kvcpp_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS)


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvcpp2.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
