AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT
#AM_CPPFLAGS := $(shell pkg-config --cflags libkvcpp)
noinst_PROGRAMS = kvpull

kvpull_SOURCES= \
	kvpull.cc \
	kvpullApp.cc \
	kvpullApp.h

kvpull_CPPFLAGS = $(AM_CPPFLAGS)
 kvpull_LDFLAGS = \
   -R "$(libdir)/@PACKAGE_NAME@/lib" \
 	-R "$(rdkafka_LIBDIR)" \
   -pthread -export-dynamic

#kvpull_LDADD := $(shell pkg-config --libs libkvcpp)

kvpull_LDADD = $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
                $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                $(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
                $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                $(putools_LIBS) $(omniORB4_LIBS) $(libxmlpp_LIBS) \
                $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) \
                -ldl

