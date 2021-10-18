pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
AM_CPPFLAGS = -I $(top_srcdir)/src/lib $(omniORB4_CFLAGS)

lib_LTLIBRARIES = libkvalobs_corbahelper.la

libkvalobs_corbahelper_la_SOURCES = corbaApp.cc
libkvalobs_corbahelper_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs_corbahelper_la_LIBADD = $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
                           $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                           $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
			   			   $(BOOST_THREAD_LIB) $(omniORB4_LIBS)

corbahelperincludedir = $(pkgincludedir)/corbahelper
corbahelperinclude_HEADERS = corbaApp.h corbamacros.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_corbahelper.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
