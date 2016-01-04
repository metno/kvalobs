noinst_LTLIBRARIES = libsqlkvapp.la

libsqlkvappincludedir =  $(pkgincludedir)/kvcpp/sql
libsqlkvappinclude_HEADERS = \
	SqlKvApp.h

libsqlkvapp_la_SOURCES = \
	SqlKvApp.cpp \
	KvDataHandler.h \
	KvDataHandler.cpp

AM_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/service-libs \
	-I$(top_srcdir)/src/service-libs/kvcpp \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS) 
	

libsqlkvapp_la_LIBADD = \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la
