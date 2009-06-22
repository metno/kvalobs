bin_PROGRAMS = kvQc2

kvQc2_SOURCES = \
	CheckedDataCommandBase.h \
	CheckedDataHelper.cc \
	CheckedDataHelper.h \
	InitLogger.cc \
	InitLogger.h \
	ProcessControl.cc \
	ProcessControl.h \
	ProcessImpl.cc \
	ProcessImpl.h \
	Qc2App.cc \
	Qc2App.h \
	Qc2Connection.h \
	Qc2Main.cc \
	Qc2Thread.cc \
	Qc2Thread.h \
	ReadProgramOptions.cc \
	ReadProgramOptions.h \
	StopWatch.cc \
	StopWatch.h \
	algorithms/BasicStatistics.h \
	algorithms/Distribute.cc \
	algorithms/Distribute.h \
	algorithms/epsx.cc \
	algorithms/epsx.h \
	algorithms/kvQABaseTypes.h \
	algorithms/Process4D.cc \
	algorithms/ProcessRedistribution.cc \
	algorithms/ProcessUnitT.cc \
	algorithms/Interpolate.cc \
	algorithms/ProcessVariability.cc \
	algorithms/proj++.cc \
	algorithms/proj++.h \
	algorithms/Qc2D.cc \
	algorithms/Qc2D.h \
	algorithms/scone.h \
	algorithms/StationSelection.cc \
	algorithms/StationSelection.h \
	algorithms/table_delaunay.cc \
	algorithms/table_delaunay.h \
	algorithms/tround.h 

kvQc2_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	-I$(top_srcdir)/src/service-libs \
	$(BOOST_CPPFLAGS) \
	$(omniDynamic4_CFLAGS) \
	$(putools_CFLAGS) \
	$(perl_CFLAGS)
	
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
	$(BOOST_PROGRAM_OPTIONS_LIB) \
	$(omniDynamic4_LIBS) \
	$(perl_LIBS) \
        -lproj \
        -lnetcdf \
        -lnetcdf_c++ \
	-ldl 


clean-local:
	rm -rf test/var

TESTS = kvQc2Test

check_PROGRAMS = kvQc2Test

kvQc2Test_SOURCES= \
	test/test.cc \
	test/kvalobscachetest.h test/kvalobscachetest.cc \
	test/checkrunnertest.h test/checkrunnertest.cc \
	test/database/kvalobsdatabase.h test/database/kvalobsdatabase.cc \
	$(kvQc2_SOURCES:kvQc2.cc=)

#	-DSETUPDB_SQL=\"$(srcdir)/test/database/setupdb.sql\"

SETUP_DB = \"$(srcdir)/test/database/setupdb.sql\"
#SETUP_DB = \"$(top_srcdir)/src/kvalobs_database/kvalobs_schema.sql\"
	
kvQc2Test_CPPFLAGS = \
	-DDBDRIVER=\"$(top_builddir)/src/lib/dbdrivers/.libs/sqlite3driver.so\" \
	-DSETUPDB_SQL=$(SETUP_DB) \
	-DCHECKRUNNERTEST_INIT_SQL=\"$(srcdir)/test/etc/checkrunnertest.init.sql\" \
	$(kvQc2_CPPFLAGS) \
	$(cppunit_CFLAGS) \
	-DLOG_CHECK_SCRIPT 
	
kvQc2Test_LDADD = \
	$(kvQc2_LDADD) \
	$(cppunit_LIBS)
	
EXTRA_DIST = \
	test/database/setupdb.sql \
	test/etc/checkrunnertest.init.sql

