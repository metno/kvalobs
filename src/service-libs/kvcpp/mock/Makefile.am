
if HAVE_GMOCK
libmockkvappincludedir =  $(pkgincludedir)/kvcpp/mock
libmockkvappinclude_HEADERS = \
	FakeKvApp.h \
	MockKvApp.h

TESTS = testTestingKvApp

check_PROGRAMS = testTestingKvApp

testTestingKvApp_SOURCES = \
	FakeKvApp.h \
	MockKvApp.h \
	test.cc

testTestingKvApp_CPPFLAGS = -I$(top_srcdir)/src \
							-I$(top_srcdir)/src/lib  \
							-I$(top_srcdir)/src/service-libs \
                            -I$(top_builddir)/src/lib \
                            -I$(top_builddir)/src/lib/kvskel \
                            $(putools_CFLAGS) $(gtest_CFLAGS) $(gmock_CFLAGS)

testTestingKvApp_LDADD = $(gtest_LIBS) $(gmock_LIBS) \
   ../libkvalobs_kvcpp.la \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(BOOST_SYSTEM_LIB) \
	$(putools_LIBS) 

endif # HAVE_GMOCK
