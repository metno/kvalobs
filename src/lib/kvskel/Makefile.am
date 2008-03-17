#AM_CPPFLAGS = -I $(top_srcdir)/src/lib

pkglib_LTLIBRARIES = libkvskel.la libkvadmininterface.la
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
            newfile.idl \
            kvsynopd.idl 


libkvskel_ladir = @includedir@/kvskel
nodist_libkvskel_la_SOURCES = $(idl_sources:.idl=SK.cc)
libkvskel_la_CPPFLAGS = $(omniORB4_CFLAGS)

libkvadmininterface_ladir = @includedir@/kvskel
libkvadmininterface_la_SOURCES = adminInterface.cc
libkvadmininterface_la_CPPFLAGS = $(omniORB4_CFLAGS)

kvskelincludedir = $(pkgincludedir)/kvskel
kvskelinclude_HEADERS = adminInterface.h $(idl_sources:.idl=.hh)

CLEANFILES = $(idl_sources:.idl=SK.cc) $(idl_sources:.idl=.hh) 


%SK.cc %.hh: $(top_srcdir)/share/kvalobs/idl/%.idl
	$(IDL) -bcxx $<

INSTALL=install -p # tells install to preserve timestamp
