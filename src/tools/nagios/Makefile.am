bin_PROGRAMS = kvkafkalistener

kvkafkalistener_SOURCES= kvkafkalistener.cpp 

kvkafkalistener_CPPFLAGS = -I$(top_srcdir)/src/lib
kvkafkalistener_LDADD = \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(BOOST_PROGRAM_OPTIONS_LIB)


nagiosbindir = $(libdir)/nagios/plugins

nagiosbin_SCRIPTS = check_kvprocs check_kvkafkastatus

check_kvprocs: check_kvprocs.py
	cp $< $@

check_kvkafkastatus: check_kvkafkastatus.py
	cp $< $@

	
EXTRA_DIST = $(nagiosbin_SCRIPTS)
CLEANFILES = $(nagiosbin_SCRIPTS:.py=)
