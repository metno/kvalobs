AM_DEFAULT_SOURCE_EXT = .cpp

LDADD = $(top_builddir)/src/lib/kvsubscribe/libkvalobs_kvsubscribe.la
AM_CPPFLAGS = 	-I$(top_srcdir)/src \
				-I$(top_srcdir)/src/lib \
				$(rdkafka_CFLAGS) \
				-DVERSION=\"PACKAGE_STRING\"

bin_PROGRAMS = kvsubscribe kvalobs-queue-transfer

kvsubscribe_LDFLAGS = \
	-R "$(libdir)/@PACKAGE_NAME@/lib" -R "$(rdkafka_LIBDIR)" 

kvsubscribe_LDADD = \
	$(LDADD) \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(rdkafka_LIBS) \
	$(BOOST_FILESYSTEM_LIB) \
	-lboost_program_options -lboost_system


kvalobs_queue_transfer_CPPFLAGS = \
	$(AM_CPPFLAGS) \
	-I$(top_srcdir)/src/service-libs/ \
	-I$(top_builddir)/src/lib/ \
	-I$(top_builddir)/src/lib/kvskel \
	$(rdkafka_CFLAGS)

kvalobs_queue_transfer_LDFLAGS = \
	-R "$(libdir)/@PACKAGE_NAME@/lib" -R "$(rdkafka_LIBDIR)"
	
kvalobs_queue_transfer_LDADD = \
	$(LDADD) \
	$(top_builddir)/src/service-libs/kvcpp/libkvalobs_kvcpp.la \
	$(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(rdkafka_LIBS) \
	-lboost_filesystem -lboost_system \
	-lomniDynamic4 -lomniORB4 -lomnithread
