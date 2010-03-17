sqldir = $(datadir)/kvalobs/db

sql_DATA = pgclean.sql pgclean_mini.sql

SED_SUBST = sed s,KVCONFIG,$(bindir)/kvconfig,g $< > $@ 
	
EXTRA_DIST = pgclean.sql pgclean_mini.sql kvdbadmin.sh

CLEANFILES = kvdbadmin 
