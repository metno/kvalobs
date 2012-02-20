bin_SCRIPTS = kvstart kvstop kvrestart
scriptdir = $(libdir)/kvalobs
script_DATA = tool_funcs.sh

EXTRA_DIST = kvrestart.sh kvstart.sh kvstop.sh kvcron.template\
             tool_funcs.sh

CLEANFILES = kvstart kvstop kvrestart

SED_SUBST = sed s,__KVCONFIG__,$(bindir)/kvconfig,g $< > $@ 

kvstart:	kvstart.sh
	$(SED_SUBST)

kvstop:	kvstop.sh
	$(SED_SUBST)

kvrestart:	kvrestart.sh
	$(SED_SUBST)
