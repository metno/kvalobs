SUBDIRS =  share src doc

ACLOCAL_AMFLAGS = -I m4

nodist_bin_SCRIPTS = kvconfig

pkgconfigdir = $(libdir)/pkgconfig

install-data-hook:
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/service
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/lib/@PACKAGE_NAME@/service/subscribers
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/run/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(DESTDIR)/$(localstatedir)/log/@PACKAGE_NAME@/html
	$(install_sh) -m 755 -d $(DESTDIR)/$(datadir)/@PACKAGE_NAME@/metadata
	$(install_sh) -m 755 -d $(DESTDIR)/$(sysconfdir)/@PACKAGE_NAME@
	$(install_sh) -m 644 $(top_srcdir)/etc/kv2norcom.conf.template \
	                     $(DESTDIR)/$(sysconfdir)/@PACKAGE_NAME@
	$(install_sh) -m 644 $(top_srcdir)/etc/kv2kvDataInputd.conf.template \
	                     $(DESTDIR)/$(sysconfdir)/@PACKAGE_NAME@
	$(install_sh) -m 644 $(top_srcdir)/etc/kvdataclt.conf.template \
	                     $(DESTDIR)/$(sysconfdir)/@PACKAGE_NAME@
	$(install_sh) -m 644 $(top_srcdir)/etc/norcom2kv.conf.template \
	                     $(DESTDIR)/$(sysconfdir)/@PACKAGE_NAME@
	                     

#pkgconfig_DATA = @PACKAGE_TARNAME@.pc
#$(pkgconfig_DATA): config.status

EXTRA_DIST = etc/kv2norcom.conf.template \
             etc/kv2kvDataInputd.conf.template \
             etc/kvdataclt.conf.template \
             etc/norcom2kv.conf.template

include mk/debian.mk