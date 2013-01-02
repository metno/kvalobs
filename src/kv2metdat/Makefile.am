AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib $(putools_CFLAGS)  -D_REENTRANT

bin_PROGRAMS = kv2metdat

kv2metdat_SOURCES = kv2metdat.cc 
kv2metdat_CPPFLAGS = $(AM_CPPFLAGS)
kv2metdat_LDFLAGS = -pthread -export-dynamic
kv2metdat_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                  $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                  $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                  $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
		  $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
                  $(putools_LIBS) $(BOOST_THREAD_LIB)  -ldl -lm
