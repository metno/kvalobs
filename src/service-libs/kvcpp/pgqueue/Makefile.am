pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
noinst_LTLIBRARIES = libpgkvapp.la

libpgkvappincludedir =  $(pkgincludedir)/kvcpp/pgqueue
libpgkvappinclude_HEADERS = \
	PgKvApp.h \
	PgSubscribe.h \
	creategroupid.h

libpgkvapp_la_SOURCES = \
	PgKvApp.cpp \
	PgSubscribe.cpp \
	creategroupid.cpp

libpgkvapp_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/service-libs \
	-I$(top_srcdir)/src/service-libs/kvcpp \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(pgqueue_CFLAGS) \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS) 
	

libpgkvapp_la_LIBADD = \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(pgqueue_LIBS)

 
	