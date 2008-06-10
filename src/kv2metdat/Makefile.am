bin_PROGRAMS = kv2metdat
	
kv2metdat_SOURCES = kv2metdat.cc 
kv2metdat_CPPFLAGS = $(AM_CPPFLAGS)
kv2metdat_LDFLAGS = -pthread -export-dynamic
kv2metdat_LDADD = $(top_builddir)/src/lib/kvalobs/libkvalobs.la \
                  $(top_builddir)/src/lib/kvdb/libkvdb.la \
                  $(top_builddir)/src/lib/fileutil/libfileutil.la \
                  $(top_builddir)/src/lib/dnmithread/libdnmithread.la\
                  $(putools_LIBS) $(BOOST_THREAD_LIB)  -ldl -lm

