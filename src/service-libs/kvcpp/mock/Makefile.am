
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

testTestingKvApp_CPPFLAGS = -I$(srcdir)/../.. -I$(top_srcdir)/src/lib/  \
                            -I$(top_builddir)/src/lib \
                            -I$(top_builddir)/src/lib/kvskel \
                            $(putools_CFLAGS) $(gtest_CFLAGS) $(gmock_CFLAGS)

testTestingKvApp_LDADD = $(gtest_LIBS) $(gmock_LIBS) \
   ../libkvalobs2_kvcpp.la \
	$(top_builddir)/src/service-libs/kvcpp/corba/libcorbakvapp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs2_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs2_fileutil.la \
	$(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvskel.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs2_corbahelper.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs2_dnmithread.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs2_kvadmininterface.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
	$(putools_LIBS) 

endif # HAVE_GMOCK
