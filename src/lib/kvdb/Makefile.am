AM_CPPFLAGS = -I $(top_srcdir)/src/lib -D_REENTRANT


lib_LTLIBRARIES = libkvalobs_kvdb.la

kvdbincludedir =  $(pkgincludedir)/kvdb
kvdbinclude_HEADERS = kvdb.h dbdrivermgr.h transactionhelper.h transaction.h
libkvalobs_kvdb_la_SOURCES = kvdb.h kvdb.cc \
                             dbdrivermgr.h dbdrivermgr.cc \
                             Pimpel.h Pimpel.cc\
                             transaction.cc transactionhelper.h transactionhelper.cc
libkvalobs_kvdb_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_kvdb_la_LIBADD= $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la 
