AM_CPPFLAGS = -I $(top_srcdir)/src/lib

dbdriverdir = $(pkglibdir)/db

#dbdriver_LTLIBRARIES = pgdriver.la dummydriver.la sqlite3driver.la sqlitedriver.la
dbdriver_LTLIBRARIES = pgdriver.la dummydriver.la sqlite3driver.la 
pgdriver_la_SOURCES = pgdriver.h pgdriver.cc pgsqldb.h pgsqldb.cc
pgdriver_la_CPPFLAGS= $(postgresql_CFLAGS) \
                      -I$(top_srcdir)/src/lib
pgdriver_la_LDFLAGS= -module -version-info @KVALOBSLIBS_SO_VERSION@
pgdriver_la_LIBADD = $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
		               $(postgresql_LIBS)

dummydriver_la_SOURCES = dummydriver.h dummydriver.cc dummysqldb.h dummysqldb.cc
dummydriver_la_CPPFLAGS = -I$(top_srcdir)/src/lib
dummydriver_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
dummydriver_la_LIBADD = $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 


sqlite3driver_la_SOURCES = sqlite3driver.h sqlite3driver.cc sqlite3db.h sqlite3db.cc
sqlite3driver_la_CPPFLAGS = $(sqlite3_CFLAGS) \
                            -I$(top_srcdir)/src/lib
sqlite3driver_la_LDFLAGS =  -module -version-info @KVALOBSLIBS_SO_VERSION@
sqlite3driver_la_LIBADD = $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                          $(sqlite3_LIBS)

#sqlitedriver_la_SOURCES = sqlitedriver.h sqlitedriver.cc sqlitedb.h sqlitedb.cc
#sqlitedriver_la_CPPFLAGS = $(sqlite2_CFLAGS)  \
#                          -I$(top_srcdir)/src/lib
#sqlitedriver_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
#sqlitedriver_la_LIBADD = $(sqlite2_LIBS)
