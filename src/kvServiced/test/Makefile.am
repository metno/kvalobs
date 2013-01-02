AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/service-libs  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

EXTRA_PROGRAMS=stressTestGetData \
	getdata getstationmetadata getstationmetadatakvcpp\
	getworkstatistik

getdata_SOURCES=getdata.cc
getdata_CPPFLAGS = $(AM_CPPFLAGS)
getdata_LDFLAGS =  -pthread 
getdata_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(putools_LIBS) \
	$(BOOST_DATE_TIME_LIB) \
        $(omniORB4_LIBS) -lm

getstationmetadata_SOURCES=getstationmetadata.cc
getstationmetadata_CPPFLAGS = $(AM_CPPFLAGS)
getstationmetadata_LDFLAGS =  -pthread 
getstationmetadata_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(putools_LIBS) \
	$(BOOST_DATE_TIME_LIB) \
        $(omniORB4_LIBS) -lm

getstationmetadatakvcpp_SOURCES=getstationmetadatakvcpp.cc
getstationmetadatakvcpp_CPPFLAGS = $(AM_CPPFLAGS)
getstationmetadatakvcpp_LDFLAGS =  -pthread 
getstationmetadatakvcpp_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(putools_LIBS)\
	$(BOOST_DATE_TIME_LIB) \
        $(omniORB4_LIBS) -lm

getworkstatistik_SOURCES=getworkstatistik.cc
getworkstatistik_CPPFLAGS = $(AM_CPPFLAGS)
getworkstatistik_LDFLAGS =  -pthread 
getworkstatistik_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(putools_LIBS) $(BOOST_DATE_TIME_LIB) \
        $(omniORB4_LIBS) -lm

stressTestGetData_SOURCES=stressTestGetData.cc
stressTestGetData_CPPFLAGS = $(AM_CPPFLAGS)
#stressTestGetData_LDFLAGS =  -pthread  -rpath $(libdir)/kvalobs
stressTestGetData_LDFLAGS =  -pthread  
stressTestGetData_LDADD =  \
                $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
				$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                $(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
				$(putools_LIBS)\
				$(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) \
                $(omniORB4_LIBS) -lm
