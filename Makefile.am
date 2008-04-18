SUBDIRS = share src

ACLOCAL_AMFLAGS = -I m4

nodist_bin_SCRIPTS = kvconfig

pkgconfigdir = $(libdir)/pkgconfig

install-data-hook:
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/agregate
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/kvsynop
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/kvsynop/debug
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/log
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/log/html
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/log/kvsynop
	$(install_sh) -m 755 -d $(localstatedir)/@PACKAGE_NAME@/run
	
#pkgconfig_DATA = @PACKAGE_TARNAME@.pc
#$(pkgconfig_DATA): config.status
