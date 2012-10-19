AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

bin_PROGRAMS = kvpush

kvpush_SOURCES= DataTblView.h DataTblView.cc \
                kvpushApp.h kvpushApp.cc \
                kvpush.cc 

kvpush_CPPFLAGS = $(AM_CPPFLAGS)
kvpush_LDFLAGS = -pthread -export-dynamic
kvpush_LDADD = $(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
               $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
               $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
               $(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
               $(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la\
               $(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
               $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
               $(top_builddir)/src/lib/kvskel/libkvalobs2_kvadmininterface.la \
               $(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
               $(putools_LIBS) $(omniORB4_LIBS) $(libxmlpp_LIBS) $(BOOST_THREAD_LIB)  -ldl

