AM_CPPFLAGS = -DUSE_OPENSSL -I $(top_srcdir)/src/lib $(omniORB4_CFLAGS) $(OpenSSL_CFLAGS)

pkglib_LTLIBRARIES = libcorbahelper.la

libcorbahelper_la_SOURCES = corbaApp.cc
libcorbahelper_la_LIBADD = $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                           $(top_builddir)/src/lib/milog/libmilog.la \
                           $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
			   $(BOOST_THREAD_LIB) $(omniORB4_LIBS) -lomnisslTP4 $(OpenSSL_LIBS)
                           
corbahelperincludedir = $(pkgincludedir)/corbahelper
corbahelperinclude_HEADERS = corbaApp.h corbamacros.h
