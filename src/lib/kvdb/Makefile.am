pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
AM_CPPFLAGS = -I $(top_srcdir)/src -I $(top_srcdir)/src/lib -D_REENTRANT 

lib_LTLIBRARIES = libkvalobs_kvdb.la

kvdbincludedir =  $(pkgincludedir)/kvdb
kvdbinclude_HEADERS = kvdb.h ConnectionPool.h dbdrivermgr.h transactionhelper.h transaction.h
libkvalobs_kvdb_la_SOURCES = \
	kvdb.h kvdb.cc \
	ConnectionPool.h ConnectionPool.cc \
   dbdrivermgr.h dbdrivermgr.cc \
   Pimpel.h Pimpel.cc \
   transaction.cc transactionhelper.h \
   transactionhelper.cc
libkvalobs_kvdb_la_CPPFLAGS = \
   $(AM_CPPFLAGS) \
   -DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
   -DPKGLIB_DBDIR="\""$(pkglibdir)/db"\""

libkvalobs_kvdb_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_kvdb_la_LIBADD= $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                           $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la

if HAVE_GMOCK
TESTS = libkvdbtest

check_PROGRAMS = libkvdbtest

libkvdbtest_SOURCES = \
	test/kvdbTest.cc \
	kvdb.h kvdb.cc \
    dbdrivermgr.h dbdrivermgr.cc \
    Pimpel.h Pimpel.cc \
    transaction.cc transactionhelper.h \
    transactionhelper.cc

libkvdbtest_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	$(gtest_CFLAGS) \
	-DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
    -DPKGLIB_DBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\""

libkvdbtest_LDADD = \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(libkvalobs_kvdb_la_LIBADD) $(BOOST_SYSTEM_LIB) \
	$(gtest_LIBS)
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_kvdb.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
