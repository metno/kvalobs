bin_PROGRAMS = kvQc2

kvQc2_SOURCES = \
	CheckedDataHelper.cc CheckedDataHelper.h \
	InitLogger.cc InitLogger.h \
	ProcessControl.cc ProcessControl.h \
	ProcessImpl.cc ProcessImpl.h \
	Qc2App.cc Qc2App.h \
	Qc2Thread.cc Qc2Thread.h \
	ReadProgramOptions.cc ReadProgramOptions.h \
	StopWatch.cc StopWatch.h \
	CheckedDataCommandBase.h \
	Qc2Connection.h \
	algorithms/BasicStatistics.h \
	algorithms/Distribute.cc algorithms/Distribute.h \
	algorithms/kvQABaseTypes.h \
	algorithms/proj++.cc algorithms/proj++.h \
	algorithms/Qc2D.cc algorithms/Qc2D.h \
	algorithms/scone.h \
	algorithms/StationSelection.cc algorithms/StationSelection.h \
	algorithms/table_delaunay.cc algorithms/table_delaunay.h \
	algorithms/tround.h \
        algorithms/Process4D.cc \
	algorithms/ProcessRedistribution.cc \
	algorithms/ProcessVariability.cc \
        Qc2Main.cc

kvQc2_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	-I$(top_srcdir)/src/service-libs \
	$(BOOST_CPPFLAGS) \
	$(omniDynamic4_CFLAGS) \
	$(putools_CFLAGS) \
	$(perl_CFLAGS) \
        $(NETCDF_CPPFLAGS)

#
# netCDF notes
#
# $(NETCDF_CPPFLAGS) needs to pick up the include files in /usr/include/netcdf-3
# $(NETCDF_LIBS) shall correspond to /usr/lib/netcdf-3
#                libnetcdf.a  libnetcdf_c++.a  
	
kvQc2_LDFLAGS = -pthread -export-dynamic	
kvQc2_LDADD = \
	$(top_builddir)/src/lib/milog/libmilog.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvdb.la \
	$(top_builddir)/src/lib/fileutil/libfileutil.la \
	$(top_builddir)/src/lib/miutil/libmiutil.la \
	$(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
	$(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
	$(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
	$(top_builddir)/src/lib/kvskel/libkvskel.la \
	$(top_builddir)/src/lib/dnmithread/libdnmithread.la \
	$(putools_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(omniDynamic4_LIBS) \
	$(perl_LIBS) \
        $(NETCDF_LIBS) \
	-ldl


clean-local:

TESTS = kvQc2Test

check_PROGRAMS = kvQc2Test

kvQc2Test_SOURCES= \
	test/Test.cc \
	test/TestQc2D.h test/TestQc2D.cc \
	$(kvQc2Test_SOURCES:kvQc2Main.cc=)

kvQc2Test_CPPFLAGS = \
	$(kvQc2_CPPFLAGS) \
	$(cppunit_CFLAGS) \
	-DLOG_CHECK_SCRIPT 
	
kvQc2Test_LDADD = \
	$(kvQc2_LDADD) \
	$(cppunit_LIBS)
