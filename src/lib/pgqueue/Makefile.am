pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
AM_CPPFLAGS = -I $(top_srcdir)/src -I $(top_srcdir)/src/lib -D_REENTRANT 

lib_LTLIBRARIES = libkvalobs_pgqueue.la

kvdbincludedir =  $(pkgincludedir)/pgqueue
kvdbinclude_HEADERS = pgqueue.h
libkvalobs_pgqueue_la_SOURCES = \
	pgqueue.h pgqueue.cpp
libkvalobs_pgqueue_la_CPPFLAGS = \
   $(AM_CPPFLAGS) \
   $(postgresql_CFLAGS) \
   -DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
   -DPKGLIB_DBDIR="\""$(pkglibdir)"\""

libkvalobs_pgqueue_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_pgqueue_la_LIBADD= $(postgresql_LIBS)

bin_PROGRAMS = kvproducer kvconsumer

kvproducer_SOURCES = pgqueue_producer.cpp
kvproducer_LDADD = libkvalobs_pgqueue.la

kvconsumer_SOURCES = pgqueue_consumer.cpp
kvconsumer_LDADD = libkvalobs_pgqueue.la

# if HAVE_GMOCK
# TESTS = libkvdbtest

# check_PROGRAMS = libkvdbtest

# libkvdbtest_SOURCES = \
# 	test/kvdbTest.cc \
# 	kvdb.h kvdb.cc \
#     dbdrivermgr.h dbdrivermgr.cc \
#     Pimpel.h Pimpel.cc \
#     transaction.cc transactionhelper.h \
#     transactionhelper.cc

# libkvdbtest_CPPFLAGS = \
# 	$(AM_CPPFLAGS) \
# 	$(gtest_CFLAGS) \
# 	-DKVALOBSLIBS_SO_VERSION="\""@KVALOBSLIBS_SO_VERSION@"\"" \
#     -DPKGLIB_DBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\""

# libkvdbtest_LDADD = \
# 	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
# 	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
# 	$(libkvalobs_kvdb_la_LIBADD) $(BOOST_SYSTEM_LIB) \
# 	$(gtest_LIBS)
# endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_pgqueue.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
