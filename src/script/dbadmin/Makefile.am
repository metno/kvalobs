sqldir = $(datadir)/kvalobs/db

sql_DATA = cleansynopdb.sql generated_synop.sql pgclean.sql

bin_SCRIPTS = kvdbadmin kvsynopdbadmin

SED_SUBST = sed s,KVCONFIG,$(bindir)/kvconfig,g $< > $@ 

kvdbadmin: kvdbadmin.sh
	$(SED_SUBST)
	
kvsynopdbadmin: kvsynopdbadmin.sh 
	$(SED_SUBST)
	
	
EXTRA_DIST = cleansynopdb.sql generated_synop.sql pgclean.sql kvdbadmin.sh kvsynopdbadmin.sh

CLEANFILES = kvdbadmin kvsynopdbadmin