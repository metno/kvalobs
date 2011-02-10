bin_SCRIPTS = kvstart kvstop kvrestart

EXTRA_DIST = kvrestart.sh kvstart.sh kvstop.sh kvcron.template

CLEANFILES = kvstart kvstop

SED_SUBST = sed s,__KVCONFIG__,$(bindir)/kvconfig,g $< > $@ 

kvstart:	kvstart.sh
	$(SED_SUBST)

kvstop:	kvstop.sh
	$(SED_SUBST)

kvrestart:	kvrestart.sh
	$(SED_SUBST)
	