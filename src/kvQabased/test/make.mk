clean-local:
	rm -rf test/var

TESTS = kvQabasedTest

check_PROGRAMS = kvQabasedTest

kvQabasedTest_SOURCES= \
	test/test.cc \
	test/kvalobscachetest.cc \
	test/checkrunnertest.cc \
	test/database/kvalobsdatabase.h \
	test/database/kvalobsdatabase.cc \
	$(kvQabased_SOURCES:kvQabased.cc=)

#	-DSETUPDB_SQL=\"$(srcdir)/test/database/setupdb.sql\"

SETUP_DB = \"$(srcdir)/test/database/setupdb.sql\"
#SETUP_DB = \"$(top_srcdir)/src/kvalobs_database/kvalobs_schema.sql\"
	
kvQabasedTest_CPPFLAGS = \
	-DDBDRIVER=\"$(top_builddir)/src/lib/dbdrivers/.libs/sqlite3driver.so\" \
	-DSETUPDB_SQL=$(SETUP_DB) \
	-DCHECKRUNNERTEST_INIT_SQL=\"$(srcdir)/test/share/checkrunnertest.init.sql\" \
	$(kvQabased_CPPFLAGS) \
	$(gtest_CFLAGS) \
	-DLOG_CHECK_SCRIPT 
	
kvQabasedTest_LDADD = \
	$(kvQabased_LDADD) \
	$(gtest_LIBS)
	
EXTRA_DIST = \
	test/database/setupdb.sql \
	test/share/checkrunnertest.init.sql
