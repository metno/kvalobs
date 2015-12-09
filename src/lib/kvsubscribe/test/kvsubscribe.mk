if HAVE_GMOCK

check_PROGRAMS = kvsubscribetest
TESTS = $(check_PROGRAMS)

kvsubscribetest_CPPFLAGS = $(AM_CPPFLAGS) $(gmock_CFLAGS) $(gtest_CFLAGS)
kvsubscribetest_LDFLAGS = -pthread
kvsubscribetest_LDADD = libkvalobs_kvsubscribe.la $(BOOST_DATE_TIME_LIB) $(gmock_LIBS) $(gtest_LIBS)

endif # HAVE_GMOCK

kvsubscribetest_SOURCES = \
    test/test.cpp \
    test/NotificationTest.cpp

EXTRA_DIST = $(kvsubscribetest_SOURCES)
