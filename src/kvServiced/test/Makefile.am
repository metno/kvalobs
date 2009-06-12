AM_CPPFLAGS = -I$(top_srcdir)/src/lib  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT



noinst_PROGRAMS = getdata 

getdata_SOURCES=getdata.cc
getdata_CPPFLAGS = $(AM_CPPFLAGS)
getdata_LDFLAGS =  -pthread 
getdata_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                -lm

 