noinst_LTLIBRARIES = libsqlkvapp.la

libsqlkvappincludedir =  $(pkgincludedir)/kvcpp/sql
libsqlkvappinclude_HEADERS = \
	SqlKvApp.h

libsqlkvapp_la_SOURCES = \
	SqlKvApp.cpp

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/service-libs/kvcpp \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS) 
	

libsqlkvapp_la_LIBADD = \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la
