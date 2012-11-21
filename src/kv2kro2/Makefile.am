bin_PROGRAMS = kv2kro

kv2kro_SOURCES = GetData.cc GetData.h \
                 App.cc App.h \
                 DataReceiver.cc DataReceiver.h \
                 kv2kro.cc

kv2kro_CPPFLAGS = -D_REENTRANT \
                  -I$(top_builddir)/src/lib \
                  -I$(top_builddir)/src/lib/kvskel \
                  -I$(top_srcdir)/src/lib  \
                  -I$(top_srcdir)/src/service-libs \
                  $(omniORB4_CFLAGS)  \
                  $(putools_CFLAGS)  
kv2kro_LDFLAGS = -pthread 
kv2kro_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
                $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
		$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
		$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
		$(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
		$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
		$(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
		$(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la\
		$(putools_LIBS) \
               $(omniORB4_LIBS) $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
              -lm 

EXTRA_DIST = README delete_oldfiles.pl obsInsert obsInsertTest.pl               
