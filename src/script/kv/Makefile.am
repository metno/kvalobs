bin_SCRIPTS = kvstart kvstop

EXTRA_DIST = kvstart.sh kvstop.sh kvcron.template save_last_month load_last_month

CLEANFILES = kvstart kvstop

SED_SUBST = sed s,__KVCONFIG__,$(bindir)/kvconfig,g $< > $@ 

kvstart:	kvstart.sh
	$(SED_SUBST)

kvstop:	kvstop.sh
	$(SED_SUBST)
