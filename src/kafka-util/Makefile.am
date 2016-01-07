AM_DEFAULT_SOURCE_EXT = .cpp

LDADD = $(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la
AM_CPPFLAGS = 	-I$(top_srcdir)/src \
				-I$(top_srcdir)/src/lib

bin_PROGRAMS = kvsubscribe kvalobs-queue-transfer


kvsubscribe_LDADD = $(LDADD) $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la -lboost_system


kvalobs_queue_transfer_CPPFLAGS = $(AM_CPPFLAGS) -I$(top_srcdir)/src/service-libs/ -I$(top_builddir)/src/lib/ -I$(top_builddir)/src/lib/kvskel
kvalobs_queue_transfer_LDADD = \
	$(LDADD) \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	-lboost_filesystem -lboost_system \
	-lomniDynamic4 -lomniORB4 -lomnithread
