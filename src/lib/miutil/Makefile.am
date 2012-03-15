AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel

lib_LTLIBRARIES = libkvalobs_miutil.la

libkvalobs_miutil_la_SOURCES = \
	commastring.h commastring.cc \
	trimstr.h trimstr.cc \
	cmprspace.h cmprspace.cc \
	miTimeParse.h miTimeParse.cc \
	replace.h replace.cc \
	base64.h base64.cc \
	splitstr.h splitstr.cc

libkvalobs_miutil_la_CPPFLAGS = $(putools_CFLAGS)
libkvalobs_miutil_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
#libkvalobs_miutil_la_LIBADD = 

miutilincludedir = $(pkgincludedir)/miutil
miutilinclude_HEADERS = cmprspace.h commastring.h miTimeParse.h trimstr.h


TESTS = libmiutiltest

check_PROGRAMS = libmiutiltest

libmiutiltest_SOURCES = \
	test/miTimeParseTest.cc \
	test/test.cc

libmiutiltest_CPPFLAGS = $(AM_CPPFLAGS) $(libkvalobs_miutil_la_CPPFLAGS) $(gtest_CFLAGS)

libmiutiltest_LDADD = \
	libkvalobs_miutil.la \
	$(libkvalobs_miutil_la_LIBADD) \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(putools_LIBS)\
	$(gtest_LIBS)
