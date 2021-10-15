pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
SUBDIRS = corba sql kafka mock

lib_LTLIBRARIES = libkvalobs_kvcpp.la

libkvalobs_kvcpp_la_SOURCES= test/configuration.h \
	KvApp.cc  \
	sql/SqlGet.cpp \
	sql/SqlGet.h \
	sql/KvDataHandler.cpp \
	sql/KvDataHandler.h \
	CurrentKvApp.cpp \
	kvDataSubscribeInfoHelper.cc \
	kveventinterface.cc \
	kvevents.cc \
	KvObsData.cc \
	kvWhat.cc \
	RejectdecodeIterator.cc \
	WhichDataHelper.cc\
	WorkstatistikIterator.cc \
	kvservicetypes.h kvservicetypes.cc 
	

kvcppincludedir = $(pkgincludedir)/kvcpp
	
kvcppinclude_HEADERS = \
	KvApp.h \
	CurrentKvApp.h \
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
	
	
libkvalobs_kvcpp_la_LDFLAGS= \
	-version-info @KVALOBSLIBS_SO_VERSION@ \
	-R "$(pkglibdir)/lib"

libkvalobs_kvcpp_la_LIBADD = \
	$(top_builddir)/src/service-libs/kvcpp/kafka/libkafkakvapp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(rdkafka_LIBS) \
	@BOOST_FILESYSTEM_LIB@

AM_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/service-libs \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(rdkafka_CFLAGS) \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS)


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvcpp.pc
$(pkgconfig_DATA): $(top_builddir)/config.status


# noinst_PROGRAMS = runit
# runit_SOURCES = runit.cpp
# runit_LDFLAGS = \
# 	sql/libsqlkvapp.la \
# 	corba/libcorbakvapp.la \
# 	libkvalobs_kvcpp.la \
# 	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
# 	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
# 	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
# 	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
# 	-lboost_system -lboost_timer -lomniDynamic4 -lomniORB4 -lomnithread \
# 	-R "$(rdkafka_LIBDIR)"


if HAVE_GTEST

TESTS = gtest

check_PROGRAMS = gtest

gtest_SOURCES = \
	test/testKafkaSubcriber.h \
	test/gtest.cc

gtest_CPPFLAGS = \
	-DTESTDIR="\""$(top_srcdir)/src/service-libs/kvcpp/test"\"" \
	$(AM_CPPFLAGS) \
	$(libkvalobs_kvcpp_la_CPPFLAGS) \
	$(gtest_CFLAGS)

gtest_LDADD = \
	libkvalobs_kvcpp.la \
	$(libkvalobs_kvcpp_la_LIBADD) \
	$(gtest_LIBS)
	
endif

EXTRA_DIST = test/etc/kvalobs/gtest.conf
