clean-local:
	rm -rf test/var

TESTS = kvQabasedTest

check_PROGRAMS = kvQabasedTest

kvQabasedTest_SOURCES= \
	runcheck/test/test.cc \
	runcheck/test/kvalobscachetest.cc \
	runcheck/test/checkrunnertest.cc \
	runcheck/test/database/kvalobsdatabase.h \
	runcheck/test/database/kvalobsdatabase.cc \
	$(kvQabased_SOURCES:kvQabased.cc=)

#	-DSETUPDB_SQL=\"$(srcdir)/runcheck/test/database/setupdb.sql\"

SETUP_DB = \"$(srcdir)/runcheck/test/database/setupdb.sql\"
#SETUP_DB = \"$(top_srcdir)/src/kvalobs_database/kvalobs_schema.sql\"
	
kvQabasedTest_CPPFLAGS = \
	-DDBDRIVER=\"$(top_builddir)/src/lib/dbdrivers/.libs/sqlite3driver.so\" \
	-DSETUPDB_SQL=$(SETUP_DB) \
	-DCHECKRUNNERTEST_INIT_SQL=\"$(srcdir)/runcheck/test/share/checkrunnertest.init.sql\" \
	$(kvQabased_CPPFLAGS) \
	$(gtest_CFLAGS) \
	-DLOG_CHECK_SCRIPT 
	
kvQabasedTest_LDADD = \
	$(kvQabased_LDADD) \
	$(gtest_LIBS)
	
EXTRA_DIST = \
	runcheck/test/database/setupdb.sql \
	runcheck/test/share/checkrunnertest.init.sql
