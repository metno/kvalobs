pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@

SUBDIRS =  kvsupport share src doc

ACLOCAL_AMFLAGS = -I m4

nodist_bin_SCRIPTS = kvconfig

pkgconfigdir = $(libdir)/pkgconfig

install-data-hook:
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/run
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/decoders
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/service
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/service/subscribers
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/@PACKAGE_NAME@/html
	$(install_sh) -m 755 -d $(DESTDIR)/$(pkglibdir)/lib
	$(install_sh) -m 755 -d $(DESTDIR)/$(datadir)/@PACKAGE_NAME@/metadata
	cd $(DESTDIR)/$(bindir); rm -f sync_norcom_files; $(LN_S) $(bindir)/kvsync_files sync_norcom_files
	cp -d $(libhttpserver_LIBDIR)/lib*.so* $(DESTDIR)/$(pkglibdir)/lib


pkgsysconfdir = $(sysconfdir)/@PACKAGE_NAME@
pkgsysconf_DATA = \
	etc/kv2norcom.conf.template \
	etc/sync_norcom_files.conf.template \
	etc/kv2kvDataInputd.conf.template \
	etc/kvdataclt.conf.template \
	etc/norcom2kv.conf.template \
	etc/aexecd.conf.template \
	etc/stinfosys.conf \
	etc/stinfosys_params.csv.default
	
#pkgconfig_DATA = @PACKAGE_TARNAME@.pc
#$(pkgconfig_DATA): config.status

EXTRA_DIST = \
	$(top_srcdir)/kvsupport/*.tar* \
	etc/kv2norcom.conf.template \
	etc/sync_norcom_files.conf.template \
  etc/kv2kvDataInputd.conf.template \
  etc/kvdataclt.conf.template \
  etc/norcom2kv.conf.template \
  etc/aexecd.conf.template \
  etc/stinfosys.conf \
  etc/stinfosys_params.csv.default
	
include mk/debian.mk
#include kvsupport/Makefile.mk

