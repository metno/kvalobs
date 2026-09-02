pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
lib_LTLIBRARIES = libkvalobs_kvsubscribe.la

libkvalobs_kvsubscribe_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	$(postgresql_CFLAGS) \
	$(rdkafka_CFLAGS) \
	$(libxmlpp_CFLAGS) 

libkvalobs_kvsubscribe_la_SOURCES = \
  Producer.cpp \
	queue.cpp \
	Consumer.cpp \
	KafkaConfig.cpp \
	KafkaConsumer.cpp \
	DataSubscriber.cpp \
	KafkaProducer.cpp \
	PgProducer.cpp \
	DataProducer.cpp \
	HttpSendData.cc \
	SendDataJsonResult.h SendDataJsonResult.cc \
	ProducerCommand.h ProducerCommand.cc \
	ProducerThread.h ProducerThread.cc \
	KvDataSerializeCommand.h KvDataSerializeCommand.cc \
	creategroupid.h creategroupid.cpp

#	KafkaProducerThread.h KafkaProducerThread.cc 

libkvalobs_kvsubscribe_la_LIBADD = \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/json/libjson.la \
	$(top_builddir)/src/lib/pgqueue/libkvalobs_pgqueue.la \
	$(rdkafka_LIBS)

libkvalobs_kvsubscribe_la_LDFLAGS = \
	-version-info @KVALOBSLIBS_SO_VERSION@ \
	-R "$(pkglibdir)/lib"

kvsubscribeincludedir = $(pkgincludedir)/kvsubscribe
	
kvsubscribeinclude_HEADERS = \
	Producer.h \
	queue.h \
	Consumer.h \
	KafkaConsumer.h \
	DataSubscriber.h \
	KafkaProducer.h \
	PgProducer.h \
	DataProducer.h \
	HttpSendData.h \
	SendData.h \
	ProducerCommand.h \
	ProducerThread.h \
	KafkaProducerThread.h \
	KvDataSerializeCommand.h \
	KafkaConfig.h \
	creategroupid.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvsubscribe.pc
$(pkgconfig_DATA): $(top_builddir)/config.status


include test/kvsubscribe.mk
