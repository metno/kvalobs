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

testTestingKvApp_LDFLAGS = $(omniORB4_LIBS) $(gtest_LIBS) $(gmock_LIBS) \
                           ../libkvalobs_kvcpp.la \
						   $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                		   $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la		 
