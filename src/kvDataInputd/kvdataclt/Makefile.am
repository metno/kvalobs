AM_CPPFLAGS = \
-I$(top_srcdir)/src \
	-I$(top_srcdir)/src/lib \
   $(libhttpserver_CFLAGS) \
   $(omniORB4_CFLAGS)  $(putools_CFLAGS)\
   -D_REENTRANT

bin_PROGRAMS = kvdataclt


#To prevent that we make dependency on a large number of
#of package that we do not need we only compile the files that is needed.
kvdataclt_SOURCES = \
	kvdataclt.cc \
	Option.h

kvdataclt_CPPFLAGS = $(AM_CPPFLAGS) \
                     $(CURL_CFLAGS) \
                     -DPKGLIBDIR="\""$(pkglibdir)"\"" \
                     -DSYSCONFDIR="\""$(sysconfdir)"\"" \
                     -DLIBDIR="\""$(libdir)"\"" \
                     -DBINDIR="\""$(bindir)"\"" \
                     -DDATADIR="\""$(datadir)"\"" \
                     -DLOCALSTATEDIR="\""$(localstatedir)"\"" \
                     -DPREFIX="\""$(prefix)"\"" 

kvdataclt_LDFLAGS = -pthread  
kvdataclt_LDADD = \
	$(top_builddir)/src/lib/kvalobs/.libs/libkvalobs_kvalobs.a\
	$(top_builddir)/src/lib/fileutil/.libs/libkvalobs_fileutil.a\
	$(top_builddir)/src/lib/miconfparser/.libs/libkvalobs_miconfparser.a\
   $(top_builddir)/src/lib/milog/.libs/libkvalobs_milog.a\
	$(top_builddir)/src/lib/miutil/.libs/libkvalobs_miutil.a\
   $(putools_LIBS)\
   $(BOOST_THREAD_LIB)\
   $(BOOST_SYSTEM_LIB)\
   $(CURL_LIBS)\
   $(omniORB4_LIBS) 





