AM_CPPFLAGS = -I $(top_srcdir)/src/lib -D_REENTRANT 

lib_LTLIBRARIES = libkvalobs2_kvdb.la

kvdbincludedir =  $(pkgincludedir)/kvdb
kvdbinclude_HEADERS = kvdb.h dbdrivermgr.h transactionhelper.h transaction.h
libkvalobs2_kvdb_la_SOURCES = kvdb.h kvdb.cc \
                             dbdrivermgr.h dbdrivermgr.cc \
                             Pimpel.h Pimpel.cc \
                             transaction.cc transactionhelper.h \
                             transactionhelper.cc
libkvalobs2_kvdb_la_CPPFLAGS = \
   $(AM_CPPFLAGS) \
   -DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
   -DPKGLIB_DBDIR="\""$(pkglibdir)/db"\""

libkvalobs2_kvdb_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_kvdb_la_LIBADD= $(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la 


if HAVE_GTEST

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
	$(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
	$(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
	$(libkvalobs2_kvdb_la_LIBADD) \
	$(putools_LIBS) \
	$(gtest_LIBS)

endif

