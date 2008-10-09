noinst_LTLIBRARIES = libmiutil.la

libmiutil_la_SOURCES = \
	commastring.h commastring.cc \
	trimstr.h trimstr.cc \
	cmprspace.h cmprspace.cc \
	miTimeParse.h miTimeParse.cc 

libmiutil_la_CPPFLAGS = $(putools_CFLAGS)
libmiutil_la_LIBADD = $(putools_LIBS)

miutilincludedir = $(pkgincludedir)/miutil
miutilinclude_HEADERS = cmprspace.h commastring.h miTimeParse.h trimstr.h
