AM_CPPFLAGS = \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	-D_REENTRANT

lib_LTLIBRARIES = libkvalobs_miutil.la

libkvalobs_miutil_la_SOURCES = \
   blockingqueue.h runable.h \
   threadpool.h threadpool.cc \
   httpclient.h httpclient.cc \
   commastring.h commastring.cc \
   trimstr.h trimstr.cc \
   cmprspace.h cmprspace.cc \
   miTimeParse.h miTimeParse.cc \
   replace.h replace.cc \
   base64.h base64.cc \
   splitstr.h splitstr.cc \
   gettimeofday.h gettimeofday.cc \
   msleep.h msleep.cc \
   timeconvert.h \
   sockethelper.h sockethelper.cc \
   simplesocket.h simplesocket.cc \
   aexecclient.h aexecclient.cc \
   ArgvClass.h ArgvClass.cc \
   signalhelpers.h signalhelpers.cc \
   strerror.h strerror.cc \
   SemiUniqueName.h SemiUniqueName.cc

libkvalobs_miutil_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS)  -fPIC
libkvalobs_miutil_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@ -fPIC
libkvalobs_miutil_la_LIBADD = \
  $(CURL_LIBS) \
  $(BOOST_DATE_TIME_LIB)

miutilincludedir = $(pkgincludedir)/miutil
miutilinclude_HEADERS = \
  blockingqueue.h threadpool.h runable.h \
  msleep.h replace.h cmprspace.h commastring.h \
  miTimeParse.h trimstr.h base64.h gettimeofday.h \
  splitstr.h timeconvert.h strerror.h signalhelpers.h \
  sockethelper.h simplesocket.h aexecclient.h SemiUniqueName.h


if HAVE_GMOCK 

TESTS = libmiutiltest

check_PROGRAMS = libmiutiltest

libmiutiltest_SOURCES = \
	test/miTimeParseTest.cc \
	test/test.cc

libmiutiltest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(libkvalobs_miutil_la_CPPFLAGS) \
	$(gtest_CFLAGS)

libmiutiltest_LDADD = \
	libkvalobs_miutil.la \
	$(libkvalobs_miutil_la_LIBADD) \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) \
	$(gtest_LIBS)
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_miutil.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
