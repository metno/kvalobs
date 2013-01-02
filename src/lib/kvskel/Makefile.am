#AM_CPPFLAGS = -I $(top_srcdir)/src/lib

lib_LTLIBRARIES = libkvalobs_kvskel.la libkvalobs_kvadmininterface.la
idl_sources=admin.idl \
            KeyVal.idl \
            datasource.idl \
            commonStationInfo.idl \
            managerInput.idl \
            qabase.idl \
            kvServiceCommon.idl \
            kvDataNotifySubscriber.idl \
            kvDataSubscriber.idl \
            kvHintSubscriber.idl \
            kvService.idl \
            newfile.idl 


libkvskel_ladir = @includedir@/kvskel
nodist_libkvalobs_kvskel_la_SOURCES = $(idl_sources:.idl=SK.cc)
libkvalobs_kvskel_la_CPPFLAGS = $(omniORB4_CFLAGS)
libkvalobs_kvskel_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_kvskel_la_LIBADD = $(omniORB4_LIBS)

libkvalobs_kvadmininterface_ladir = @includedir@/kvskel
libkvalobs_kvadmininterface_la_SOURCES = adminInterface.cc
libkvalobs_kvadmininterface_la_CPPFLAGS = $(omniORB4_CFLAGS)
libkvalobs_kvadmininterface_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_kvadmininterface_la_LIBADD = $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                                $(omniORB4_LIBS)

kvskelincludedir = $(pkgincludedir)/kvskel
kvskelinclude_HEADERS = adminInterface.h $(idl_sources:.idl=.hh)

CLEANFILES = $(idl_sources:.idl=SK.cc) $(idl_sources:.idl=.hh) 


%SK.cc %.hh: $(top_srcdir)/share/kvalobs/idl/%.idl
	$(IDL) -bcxx $<

INSTALL=install -p # tells install to preserve timestamp

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_kvskel.pc libkvalobs_kvadmininterface.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
