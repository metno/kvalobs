pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
sqldir = $(datadir)/kvalobs/db

sql_DATA = pgclean.sql pgclean_mini.sql
bin_SCRIPTS = kvdbadmin

SED_SUBST = sed s,KVCONFIG,$(bindir)/kvconfig,g $< > $@ 

kvdbadmin: kvdbadmin.sh
	$(SED_SUBST)
	
EXTRA_DIST = pgclean.sql pgclean_mini.sql kvdbadmin.sh

CLEANFILES = kvdbadmin 
