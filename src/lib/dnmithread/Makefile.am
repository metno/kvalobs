AM_CPPFLAGS = -I $(top_srcdir)/src/lib

lib_LTLIBRARIES = libkvalobs_dnmithread.la

libkvalobs_dnmithread_la_SOURCES = CommandQue.cc ThrPoolQue.cc thread_private.cc
libkvalobs_dnmithread_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_dnmithread_la_LIBADD = $(BOOST_THREAD_LIB)


dnmithreadincludedir = $(pkgincludedir)/dnmithread
dnmithreadinclude_HEADERS = CommandQue.h ThrPoolQue.h mtcout.h Thread.h
