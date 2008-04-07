AM_CPPFLAGS = -I $(top_srcdir)/src/lib

dbdriverdir = $(pkglibdir)/db

dbdriver_LTLIBRARIES = pgdriver.la dummydriver.la sqlite3driver.la sqlitedriver.la

pgdriver_la_SOURCES = pgdriver.h pgdriver.cc pgsqldb.h pgsqldb.cc
pgdriver_la_CPPFLAGS = $(postgresql_CFLAGS) \
                       -I$(top_srcdir)/src/lib
pgdriver_la_LDFLAGS = $(postgresql_LIBS) -module

dummydriver_la_SOURCES = dummydriver.h dummydriver.cc dummysqldb.h dummysqldb.cc
dummydriver_la_CPPFLAGS =
dummydriver_la_LDFLAGS = -module

sqlite3driver_la_SOURCES = sqlite3driver.h sqlite3driver.cc sqlite3db.h sqlite3db.cc
sqlite3driver_la_CPPFLAGS = $(sqlite3_CFLAGS)
sqlite3driver_la_LDFLAGS = $(sqlite3_LIBS) -module

sqlitedriver_la_SOURCES = sqlitedriver.h sqlitedriver.cc sqlitedb.h sqlitedb.cc
sqlitedriver_la_CPPFLAGS = $(sqlite2_CFLAGS)
sqlitedriver_la_LDFLAGS = $(sqlite2_LIBS) -module
