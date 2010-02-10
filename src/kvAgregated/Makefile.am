CXX = ccache g++

bin_PROGRAMS = kvAgregated
bin_SCRIPTS = kvAgregateDbInit.sh

kvAgregated_SOURCES = \
	configuration/AgregatorConfiguration.h \
	configuration/AgregatorConfiguration.cpp \
	proxy/IncomingHandler.h \
	proxy/IncomingHandler.cc \
	proxy/KvalobsProxy.h \
	proxy/KvalobsProxy.cc \
	proxy/KvDataReceiver.h \
	proxy/KvDataReceiver.cc \
	proxy/KvDataSaver.h \
	proxy/KvDataSaver.cc \
	proxy/Callback.h \
	AbstractAgregator.h \
	AbstractAgregator.cc \
	AgregatorHandler.h \
	AgregatorHandler.cc \
	KvDataFunctors.h \
	KvDataFunctors.cc \
	minmax.h \
	minmax.cc \
	ra2rr_12.h \
	ra2rr_12.cc \
	rr.h rr.cc \
	GenerateZero.h \
	GenerateZero.cc \
	BackProduction.h \
	BackProduction.cc \
	paramID.h \
	times.h \
	main.cc

kvAgregated_CPPFLAGS = \
	-D_REENTRANT \
	-DVERSION=\"$(VERSION)\" \
	-I$(top_srcdir)/src/service-libs \
	-I$(top_srcdir)/src/lib/ \
	-I$(top_builddir)/src/lib/ \
	-I$(top_builddir)/src/lib/kvskel \
	$(omniDynamic4_CFLAGS) \
	$(BOOST_CPPFLAGS) \
	$(putools_CFLAGS)

kvAgregated_LDFLAGS = -rpath $(pkglibdir) -export-dynamic

kvAgregated_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs.la \
	$(top_builddir)/src/lib/fileutil/libfileutil.la \
	$(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
	$(BOOST_PROGRAM_OPTIONS_LIB) 

	
pkgstatedir = $(localstatedir)/$(PACKAGE)/agregate
nodist_pkgstate_DATA = database.sqlite

CLEANFILES = database.sqlite

database.sqlite: 
	. $(srcdir)/kvAgregateDbInit.sh $@ 
	
EXTRA_DIST = kvAgregateDbInit.sh



TESTS = kvAgregatedTest

check_PROGRAMS = kvAgregatedTest

kvAgregatedTest_SOURCES = \
	test/test.cc \
	$(kvAgregated_SOURCES:main.cc=) \
	test/AbstractAgregatorTest.cc \
	test/AbstractAgregatorTest.h \
	test/MinMaxTest.cc \
	test/ra2rr_12ForwardTest.cc \
	test/ra2rr_12Test.cc \
	test/AgregatorConfigurationTest.cc
	

kvAgregatedTest_CPPFLAGS = $(kvAgregated_CPPFLAGS) $(GTEST_CFLAGS)
kvAgregatedTest_CXXFLAGS = $(GTEST_CXXFLAGS)
kvAgregatedTest_LDFLAGS = $(GTEST_LDFLAGS) \
	$(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs.la \
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(GTEST_LIBS)
	
