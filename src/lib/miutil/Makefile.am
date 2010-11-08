lib_LTLIBRARIES = libkvalobs_miutil.la

libkvalobs_miutil_la_SOURCES = \
	commastring.h commastring.cc \
	trimstr.h trimstr.cc \
	cmprspace.h cmprspace.cc \
	miTimeParse.h miTimeParse.cc \
	replace.h replace.cc \
	base64.h base64.cc

libkvalobs_miutil_la_CPPFLAGS = $(putools_CFLAGS)
libkvalobs_miutil_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_miutil_la_LIBADD = $(putools_LIBS)

miutilincludedir = $(pkgincludedir)/miutil
miutilinclude_HEADERS = cmprspace.h commastring.h miTimeParse.h trimstr.h
