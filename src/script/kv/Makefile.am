bin_SCRIPTS = kvstart kvstop kvrestart kvstatus kv2norcom
scriptdir = $(libdir)/kvalobs
script_DATA = tool_funcs.sh

EXTRA_DIST = kvrestart.sh kvstart.sh kvstop.sh kvstatus.sh\
             kvcron.template tool_funcs.sh.in kv2norcom

CLEANFILES = kvstart kvstop kvrestart kvstatus tool_funcs.sh

SED_SUBST = sed s,__KVCONFIG__,$(bindir)/kvconfig,g $< > $@ 

tool_funcs.sh: tool_funcs.sh.in
	$(SED_SUBST)

kvstatus: kvstatus.sh
	$(SED_SUBST)

kvstart: kvstart.sh
	$(SED_SUBST)

kvstop:	kvstop.sh
	$(SED_SUBST)

kvrestart: kvrestart.sh
	$(SED_SUBST)
