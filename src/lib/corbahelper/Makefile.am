AM_CPPFLAGS = -I $(top_srcdir)/src/lib $(omniORB4_CFLAGS)

lib_LTLIBRARIES = libkvalobs2_corbahelper.la

libkvalobs2_corbahelper_la_SOURCES = corbaApp.cc
libkvalobs2_corbahelper_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@
libkvalobs2_corbahelper_la_LIBADD = $(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
                           $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                           $(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
			   			   $(BOOST_THREAD_LIB) $(omniORB4_LIBS) 
                           
corbahelperincludedir = $(pkgincludedir)/corbahelper
corbahelperinclude_HEADERS = corbaApp.h corbamacros.h
