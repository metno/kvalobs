bin_PROGRAMS = kv2kvparser

kv2kvparser_SOURCES = \
  kv2kvparser.cc
  
kv2kvparser_CPPFLAGS = -D_REENTRANT \
                  -I$(top_srcdir)/src \
                  -I$(top_builddir)/src/lib \
                  -I$(top_builddir)/src/lib/kvskel \
                  -I$(top_srcdir)/src/lib  \
                  -I$(top_srcdir)/src/service-libs \
                  $(libxmlpp_CFLAGS) \
                  $(putools_CFLAGS)  
kv2kvparser_LDFLAGS = -pthread 
kv2kvparser_LDADD = \
		$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
		$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
		$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
		$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
		$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
		$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
		$(putools_LIBS) $(BOOST_THREAD_LIB) $(BOOST_FILESYSTEM_LIB) $(BOOST_SYSTEM_LIB) \
		-lm 

#EXTRA_DIST =                
