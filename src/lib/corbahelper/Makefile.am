AM_CPPFLAGS = -I $(top_srcdir)/src/lib $(omniORB4_CFLAGS)

pkglib_LTLIBRARIES = libcorbahelper.la

libcorbahelper_la_SOURCES = corbaApp.cc
libcorbahelper_la_LIBADD = $(top_builddir)/src/lib/dnmithread/libdnmithread.la
corbahelperincludedir = $(pkgincludedir)/corbahelper
corbahelperinclude_HEADERS = corbaApp.h corbamacros.h
