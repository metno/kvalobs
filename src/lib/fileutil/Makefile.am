AM_CPPFLAGS = -I $(top_srcdir)/src/lib

lib_LTLIBRARIES = libkvalobs2_fileutil.la

libkvalobs2_fileutil_la_SOURCES = dir.h dir.cc dso.h dso.cc pidfileutil.h pidfileutil.cc readfile.h readfile.cc file.h file.cc copyfile.h copyfile.cc mkdir.h mkdir.cc
libkvalobs2_fileutil_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_fileutil_la_LIBADD = -ldl

fileutilincludedir = $(pkgincludedir)/fileutil
fileutilinclude_HEADERS = copyfile.h dir.h dso.h file.h mkdir.h pidfileutil.h readfile.h

