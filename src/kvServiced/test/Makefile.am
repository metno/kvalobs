AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_srcdir)/src/service-libs  -I$(top_builddir)/src/lib -I$(top_builddir)/src/lib/kvskel  $(omniORB4_CFLAGS)  $(putools_CFLAGS)  -D_REENTRANT

bin_PROGRAMS=stressTestGetData 
noinst_PROGRAMS =\
	getdata getstationmetadata getstationmetadatakvcpp\
	getworkstatistik

getdata_SOURCES=getdata.cc
getdata_CPPFLAGS = $(AM_CPPFLAGS)
getdata_LDFLAGS =  -pthread 
getdata_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
	$(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
	$(putools_LIBS) \
        $(omniORB4_LIBS) -lm

getstationmetadata_SOURCES=getstationmetadata.cc
getstationmetadata_CPPFLAGS = $(AM_CPPFLAGS)
getstationmetadata_LDFLAGS =  -pthread 
getstationmetadata_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
	$(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
	$(putools_LIBS) \
        $(omniORB4_LIBS) -lm

getstationmetadatakvcpp_SOURCES=getstationmetadatakvcpp.cc
getstationmetadatakvcpp_CPPFLAGS = $(AM_CPPFLAGS)
getstationmetadatakvcpp_LDFLAGS =  -pthread 
getstationmetadatakvcpp_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
	$(putools_LIBS)\
        $(omniORB4_LIBS) -lm

getworkstatistik_SOURCES=getworkstatistik.cc
getworkstatistik_CPPFLAGS = $(AM_CPPFLAGS)
getworkstatistik_LDFLAGS =  -pthread 
getworkstatistik_LDADD = \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
	$(putools_LIBS) $(BOOST_DATE_TIME_LIB) \
        $(omniORB4_LIBS) -lm

stressTestGetData_SOURCES=stressTestGetData.cc
stressTestGetData_CPPFLAGS = $(AM_CPPFLAGS)
#stressTestGetData_LDFLAGS =  -pthread  -rpath $(libdir)/kvalobs
stressTestGetData_LDFLAGS =  -pthread  
stressTestGetData_LDADD =  \
                $(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
                $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                $(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
                $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
				$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
                $(top_builddir)/src/service-libs/kvcpp/libkvalobs2_kvcpp.la \
				$(putools_LIBS)\
				$(BOOST_THREAD_LIB) \
                $(omniORB4_LIBS) -lm
