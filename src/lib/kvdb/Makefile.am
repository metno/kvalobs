AM_CPPFLAGS = -I $(top_srcdir)/src/lib

pkglib_LTLIBRARIES = libkvdb.la

libkvdb_la_SOURCES = kvdb.h kvdb.cc dbdrivermgr.h dbdrivermgr.cc transactionhelper.h transactionhelper.cc
libkvdb_la_LIBADD= $(top_builddir)/src/lib/fileutil/libfileutil.la 
                   