pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
lib_LTLIBRARIES = libkvalobs_kvsubscribe.la

libkvalobs_kvsubscribe_la_CPPFLAGS = \
	-D_REENTRANT \
	-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
	$(postgresql_CFLAGS) \
	$(libxmlpp_CFLAGS) 

libkvalobs_kvsubscribe_la_SOURCES = \
  Producer.cpp \
	pgconfig.cpp \
	queue.cpp \
	Consumer.cpp \
	DataSubscriber.cpp \
	PgProducer.cpp \
	PgConsumer.cpp \
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
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la

libkvalobs_kvsubscribe_la_LDFLAGS = \
	-version-info @KVALOBSLIBS_SO_VERSION@ \
	-R "$(pkglibdir)/lib"

kvsubscribeincludedir = $(pkgincludedir)/kvsubscribe
	
kvsubscribeinclude_HEADERS = \
	Producer.h \
	queue.h \
	Consumer.h \
	DataSubscriber.h \
	PgProducer.h \
	PgConsumer.h \
	pgconfig.h \
	DataProducer.h \
	HttpSendData.h \
	SendData.h \
	ProducerCommand.h \
	ProducerThread.h \
	KvDataSerializeCommand.h \
	creategroupid.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvsubscribe.pc
$(pkgconfig_DATA): $(top_builddir)/config.status


include test/kvsubscribe.mk
