bin_PROGRAMS = kvQabased

kvQabased_SOURCES = \
	AdminImpl.cc AdminImpl.h \
	checkrunner.cc checkrunner.h \
	InitLogger.cc InitLogger.h \
	kvalobscache.cc kvalobscache.h \
	kvCronString.cc kvCronString.h \
	kvMetadataTable.cc kvMetadataTable.h \
	kvPerlParser.cc kvPerlParser.h \
	kvQABaseDBConnection.cc kvQABaseDBConnection.h \
	kvQABaseMetadata.cc kvQABaseMetadata.h \
	kvQABaseMeteodata.cc kvQABaseMeteodata.h \
	kvQABaseScriptManager.cc kvQABaseScriptManager.h \
	qabaseApp.cc qabaseApp.h \
	qabaseInputImpl.cc qabaseInputImpl.h \
	QaWorkCommand.cc QaWorkCommand.h \
	QaWorkThread.cc QaWorkThread.h \
	kvQABaseTypes.h \
	kvQabased.cc

kvQabased_CPPFLAGS = \
	-I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-I$(top_builddir)/src/lib/kvskel \
	-I$(top_srcdir)/src/service-libs \
	$(BOOST_CPPFLAGS) \
	$(omniDynamic4_CFLAGS) \
	$(putools_CFLAGS) \
	$(perl_CFLAGS)
	
kvQabased_LDADD = \
	$(top_builddir)/src/lib/milog/libmilog.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs.la \
	$(top_builddir)/src/lib/kvdb/libkvdb.la \
	$(top_builddir)/src/lib/fileutil/libfileutil.la \
	$(top_builddir)/src/lib/miutil/libmiutil.la \
	$(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
	$(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
	$(top_builddir)/src/lib/kvskel/libkvadmininterface.la \
	$(top_builddir)/src/lib/kvskel/libkvskel.la \
	$(top_builddir)/src/lib/dnmithread/libdnmithread.la \
	$(putools_LIBS) \
	$(BOOST_THREAD_LIB) \
	$(BOOST_REGEX_LIB) \
	$(BOOST_FILESYSTEM_LIB) \
	$(omniDynamic4_LIBS) \
	$(perl_LIBS) \
	-ldl
