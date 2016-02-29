bin_PROGRAMS = kvkafkalistener

kvkafkalistener_SOURCES= kvkafkalistener.cpp 

kvkafkalistener_CPPFLAGS = -I$(top_srcdir)/src/lib
kvkafkalistener_LDADD = \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(BOOST_PROGRAM_OPTIONS_LIB)


nagiosbindir = $(libdir)/nagios/plugins

PYTHON_FILES = check_kvprocs.py check_kvkafkastatus.py

nagiosbin_SCRIPTS = $(PYTHON_FILES:.py=)

%: %.py
	cp $< $@

EXTRA_DIST = $(PYTHON_FILES)
CLEANFILES = $(nagiosbin_SCRIPTS)
