lib_LTLIBRARIES = libkvalobs_kvsubscribe.la

libkvalobs_kvsubscribe_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	$(libxmlpp_CFLAGS) 

libkvalobs_kvsubscribe_la_SOURCES = \
	Notification.cpp \
	KafkaConsumer.cpp \
	DataSubscriber.cpp \
	NotificationSubscriber.cpp \
	KafkaProducer.cpp
	
libkvalobs_kvsubscribe_la_LIBADD = ../decodeutility/libkvalobs_decodeutility.la $(jsoncpp_LIBS) $(rdkafka_LIBS)

kvsubscribeincludedir = $(pkgincludedir)/kvsubscribe
	
kvsubscribeinclude_HEADERS = \
	Notification.h \
	KafkaConsumer.h \
	NotificationSubscriber.h \
	DataSubscriber.h \
	KafkaProducer.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvsubscribe.pc
$(pkgconfig_DATA): $(top_builddir)/config.status

#include test/kvsubscribe.mk
