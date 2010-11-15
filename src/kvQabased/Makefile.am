bin_PROGRAMS = kvQabased_old

kvQabased_old_SOURCES = \
	AdminImpl.cc AdminImpl.h \
	InitLogger.cc InitLogger.h \
	kvCronString.cc kvCronString.h \
	kvMetadataTable.cc kvMetadataTable.h \
	kvQABaseDBConnection.cc kvQABaseDBConnection.h \
	qabaseApp.cc qabaseApp.h \
	qabaseInputImpl.cc qabaseInputImpl.h \
	QaWorkCommand.cc QaWorkCommand.h \
	QaWorkThread.cc QaWorkThread.h \
	kvQABaseTypes.h \
	kvQabased.cc

kvQabased_old_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	-I$(top_srcdir)/src/service-libs \
	$(BOOST_CPPFLAGS) \
	$(omniDynamic4_CFLAGS) \
	$(putools_CFLAGS) \
	$(perl_CFLAGS)
	
kvQabased_old_LDFLAGS = -pthread -export-dynamic	
kvQabased_old_LDADD = \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvadmininterface.la \
	$(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
	$(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
	$(putools_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(omniDynamic4_LIBS) \
	$(perl_LIBS) \
	-ldl

include runcheck/make.mk
include test/make.mk