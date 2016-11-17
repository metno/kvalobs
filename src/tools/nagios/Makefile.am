bin_PROGRAMS = kvkafkalistener

kvkafkalistener_SOURCES= kvkafkalistener.cpp 

kvkafkalistener_CPPFLAGS = -I$(top_srcdir)/src/lib
kvkafkalistener_LDADD = \
	$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la \
	$(BOOST_PROGRAM_OPTIONS_LIB)


nagiosbindir = $(libdir)/nagios/plugins

PYTHON_FILES = check_kvprocs.py check_kvkafkastatus.py

nagiosbin_SCRIPTS = $(PYTHON_FILES:.py=)

systemddir=$(sysconfdir)/systemd/system/
systemd_DATA = kvkafkalistener.service

kvkafkalistener.service: kvkafkalistener.in.service
	sed s:__BINPATH__:$(bindir): $< > $@


%: %.py
	cp $< $@

EXTRA_DIST = $(PYTHON_FILES) kvkafkalistener.in.service
CLEANFILES = $(nagiosbin_SCRIPTS) kvkafkalistener.service
