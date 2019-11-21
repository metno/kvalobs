noinst_LTLIBRARIES = libkafkakvapp.la

libkafkakvappincludedir =  $(pkgincludedir)/kvcpp/kafka
libkafkakvappinclude_HEADERS = \
	KafkaKvApp.h \
	KafkaSubscribe.h

libkafkakvapp_la_SOURCES = \
	KafkaKvApp.cpp \
	KafkaSubscribe.cpp

libkafkakvapp_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	-I$(top_srcdir)/src/service-libs \
	-I$(top_srcdir)/src/service-libs/kvcpp \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	$(rdkafka_CFLAGS) \
	$(omniORB4_CFLAGS) \
	$(putools_CFLAGS) 
	

libkafkakvapp_la_LIBADD = \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(rdkafka_LIBS)

 
	