AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/service-libs  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT


bin_PROGRAMS=stressTestGetData 
noinst_PROGRAMS = getdata

getdata_SOURCES=getdata.cc
getdata_CPPFLAGS = $(AM_CPPFLAGS)
getdata_LDFLAGS =  -pthread 
getdata_LDADD = $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                -lm


stressTestGetData_SOURCES=stressTestGetData.cc
stressTestGetData_CPPFLAGS = $(AM_CPPFLAGS)
stressTestGetData_LDFLAGS =  -pthread  
stressTestGetData_LDADD =  $(top_builddir)/src/service-libs/kvcpp/libkvcpp.la \
                          -lm
 