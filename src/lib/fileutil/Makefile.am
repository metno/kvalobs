AM_CPPFLAGS = -I $(top_srcdir)/src/lib
pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@

lib_LTLIBRARIES = libkvalobs_fileutil.la

libkvalobs_fileutil_la_SOURCES = \
	dir.h dir.cc\
	dso.h dso.cc\
	pidfileutil.h pidfileutil.cc\
	readfile.h readfile.cc\
	file.h file.cc\
	copyfile.h copyfile.cc\
	mkdir.h mkdir.cc\
	fileutil.h fileutil.cc
libkvalobs_fileutil_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_fileutil_la_LIBADD = $(putools_LIBS) -ldl

fileutilincludedir = $(pkgincludedir)/fileutil
fileutilinclude_HEADERS = \
	copyfile.h dir.h dso.h file.h mkdir.h pidfileutil.h\
	readfile.h fileutil.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_fileutil.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
