nagiosbindir = $(libdir)/nagios/plugins

nagiosbin_SCRIPTS = check_kvprocs

check_kvprocs: check_kvprocs.py
	cp $< $@

	
EXTRA_DIST = check_kvprocs.py
CLEANFILES = check_kvprocs
