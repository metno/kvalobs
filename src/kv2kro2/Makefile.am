pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
bin_PROGRAMS = kv2kro

kv2kro_SOURCES = GetData.cc GetData.h \
                 App.cc App.h \
                 DataReceiver.cc DataReceiver.h \
                 kv2kro.cc

kv2kro_CPPFLAGS = -D_REENTRANT \
				  -I$(top_srcdir)/src \
                  -I$(top_builddir)/src/lib \
                  -I$(top_builddir)/src/lib/kvskel \
                  -I$(top_srcdir)/src/lib  \
                  -I$(top_srcdir)/src/service-libs \
                  $(omniORB4_CFLAGS)  \
                  $(putools_CFLAGS)  
kv2kro_LDFLAGS = -pthread 
kv2kro_LDADD = \
		$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
		$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
		$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
		$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
		$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
		$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
		$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
		$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
		$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
		$(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la\
		$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la\
		$(putools_LIBS) \
		$(omniORB4_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB) \
		-lm 

EXTRA_DIST = README delete_oldfiles.pl obsInsert obsInsertTest.pl               
