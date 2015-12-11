lib_LTLIBRARIES = libkvalobs_kvsubscribe.la

libkvalobs_kvsubscribe_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src/lib \
	$(libxmlpp_CFLAGS) 

libkvalobs_kvsubscribe_la_SOURCES = \
	queue.cpp \
	KafkaConsumer.cpp \
	DataSubscriber.cpp \
	KafkaProducer.cpp \
	DataProducer.cpp
	
libkvalobs_kvsubscribe_la_LIBADD = ../decodeutility/libkvalobs_decodeutility.la $(jsoncpp_LIBS) $(rdkafka_LIBS)

kvsubscribeincludedir = $(pkgincludedir)/kvsubscribe
	
kvsubscribeinclude_HEADERS = \
	queue.h \
	KafkaConsumer.h \
	DataSubscriber.h \
	KafkaProducer.h \
	DataProducer.h


pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvsubscribe.pc
$(pkgconfig_DATA): $(top_builddir)/config.status


include test/kvsubscribe.mk
