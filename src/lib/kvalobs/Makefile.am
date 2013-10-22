AM_CPPFLAGS = -I$(top_srcdir)/src/lib \
	-I$(top_builddir)/src/lib \
	-D_REENTRANT

lib_LTLIBRARIES = libkvalobs_kvalobs.la

libkvalobs_kvalobs_la_SOURCES = \
	kvDbGate.cc \
	kvStationInfo.cc \
	kvStationInfoExt.cc \
	kvAlgorithms.cc \
	kvChecks.cc \
	kvParam.cc \
	kvModelData.cc \
	kvData.cc \
	paramlist.cc \
	kvapp.cc \
	bitmanip.cc \
	kvDbBase.cc \
	kvTypes.cc \
	kvRejectdecode.cc \
	kvStation.cc \
	kvStationParam.cc \
	kvQcxInfo.cc \
	kvTimecontrol.cc \
	kvQueries.cc \
	kvStationInfoCommand.cc \
	kvPath.cc \
	kvObsPgm.cc \
	kvTextData.cc \
	kvReferenceStation.cc \
	kvKeyVal.cc \
	kvWorkelement.cc \
	kvServiceElement.cc \
	kvGeneratedTypes.cc \
	kvOperator.cc \
	kvDataOperations.cc \
	kvTextDataOperations.cc \
	kvPsSubscribers.cc \
	kvStationMetadata.cc \
	flag/kvDataFlag.cc \
	flag/kvControlInfo.cc \
	flag/kvUseInfo.cc \
	DataInsertTransaction.cc \
	getLogInfo.cc


libkvalobs_kvalobs_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) \
                         -DPKGLIBDIR="\""$(pkglibdir)"\"" \
                         -DSYSCONFDIR="\""$(sysconfdir)"\"" \
                         -DLIBDIR="\""$(libdir)"\"" \
                         -DBINDIR="\""$(bindir)"\"" \
                         -DDATADIR="\""$(datadir)"\"" \
                         -DLOCALSTATEDIR="\""$(localstatedir)"\"" \
                         -DPREFIX="\""$(prefix)"\"" 

#libkvalobs_kvalobs_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@ $(putools_LDFLAGS)
libkvalobs_kvalobs_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@ 

libkvalobs_kvalobs_la_LIBADD = \
                       $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
                       $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                       $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la \
                       $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
                       $(top_builddir)/src/lib/corbahelper/libkvalobs_corbahelper.la \
                       $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                       $(top_builddir)/src/lib/kvskel/libkvalobs_kvskel.la \
                       $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
				       $(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(omniORB4_LIBS)

kvalobsincludedir = $(pkgincludedir)/kvalobs
kvalobsinclude_HEADERS = \
	kvDbGate.h \
	kvStationInfo.h \
	kvStationInfoExt.h \
	kvAlgorithms.h \
	kvChecks.h \
	kvParam.h \
	kvModelData.h \
	kvData.h \
	paramlist.h \
	kvapp.h \
	bitmanip.h \
	kvDbBase.h \
	kvTypes.h \
	kvRejectdecode.h \
	kvStation.h \
	kvStationParam.h \
	kvQcxInfo.h \
	kvDataFlag.h \
	kvTimecontrol.h \
	kvQueries.h \
	kvStationInfoCommand.h \
	kvPath.h \
	kvObsPgm.h \
	kvTextData.h \
	kvReferenceStation.h \
	kvKeyVal.h \
	kvWorkelement.h \
	kvServiceElement.h \
	kvGeneratedTypes.h \
	kvOperator.h \
	kvDataOperations.h \
	kvTextDataOperations.h \
	kvPsSubscribers.h \
	kvQCFlagTypes.h \
	kvexception.h \
	kvStationMetadata.h \
	DataInsertTransaction.h \
	getLogInfo.h

kvalobsflagincludedir = $(kvalobsincludedir)/flag
kvalobsflaginclude_HEADERS = \
	flag/kvDataFlag.h \
	flag/kvControlInfo.h \
	flag/kvUseInfo.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_kvalobs.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
