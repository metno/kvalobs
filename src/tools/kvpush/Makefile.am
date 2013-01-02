AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/lib/decoder -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

bin_PROGRAMS = kvpush

kvpush_SOURCES= DataTblView.h DataTblView.cc \
                kvpushApp.h kvpushApp.cc \
                kvpush.cc 

kvpush_CPPFLAGS = $(AM_CPPFLAGS)
kvpush_LDFLAGS = -pthread -export-dynamic
kvpush_LDADD = $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
               $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
               $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
               $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
               $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
               $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
               $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
               $(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
               $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
               $(putools_LIBS) $(omniORB4_LIBS) $(libxmlpp_LIBS) $(BOOST_THREAD_LIB)  -ldl

