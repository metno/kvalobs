#AM_CPPFLAGS = -I $(top_srcdir)/src/lib

lib_LTLIBRARIES = libkvalobs2_kvskel.la libkvalobs2_kvadmininterface.la
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
nodist_libkvalobs2_kvskel_la_SOURCES = $(idl_sources:.idl=SK.cc)
libkvalobs2_kvskel_la_CPPFLAGS = $(omniORB4_CFLAGS)
libkvalobs2_kvskel_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_kvskel_la_LIBADD = $(omniORB4_LIBS)

libkvalobs2_kvadmininterface_ladir = @includedir@/kvskel
libkvalobs2_kvadmininterface_la_SOURCES = adminInterface.cc
libkvalobs2_kvadmininterface_la_CPPFLAGS = $(omniORB4_CFLAGS)
libkvalobs2_kvadmininterface_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_kvadmininterface_la_LIBADD = $(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
                                $(omniORB4_LIBS)

kvskelincludedir = $(pkgincludedir)/kvskel
kvskelinclude_HEADERS = adminInterface.h $(idl_sources:.idl=.hh)

CLEANFILES = $(idl_sources:.idl=SK.cc) $(idl_sources:.idl=.hh) 


%SK.cc %.hh: $(top_srcdir)/share/kvalobs/idl/%.idl
	$(IDL) -bcxx $<

INSTALL=install -p # tells install to preserve timestamp
