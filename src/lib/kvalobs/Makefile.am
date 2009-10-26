AM_CPPFLAGS = -DUSE_KVALOBS_ENV -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib -D_REENTRANT

pkglib_LTLIBRARIES = libkvalobs.la

libkvalobs_la_SOURCES = \
	kvDbGate.cc \
	kvStationInfo.cc \
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
	kvDataFlag.cc \
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
	kvPsSubscribers.cc


libkvalobs_la_CPPFLAGS = $(AM_CPPFLAGS) $(putools_CFLAGS) \
                         -DPKGLIBDIR="\""$(pkglibdir)"\"" \
                         -DSYSCONFDIR="\""$(sysconfdir)"\"" \
                         -DLIBDIR="\""$(libdir)"\"" \
                         -DBINDIR="\""$(bindir)"\"" \
                         -DDATADIR="\""$(datadir)"\"" \
                         -DLOCALSTATEDIR="\""$(localstatedir)"\"" \
                         -DPREFIX="\""$(prefix)"\"" 
libkvalobs_la_LDFLAGS = $(putools_LDFLAGS)
#libkvalobs_la_LIBS = $(top_builddir)/src/lib/miutil/libmiutil.a
libkvalobs_la_LIBADD = $(top_builddir)/src/lib/miutil/libmiutil.la \
                       $(top_builddir)/src/lib/fileutil/libfileutil.la \
                       $(top_builddir)/src/lib/milog/libmilog.la \
                       $(top_builddir)/src/lib/dnmithread/libdnmithread.la \
                       $(top_builddir)/src/lib/kvdb/libkvdb.la \
                       $(top_builddir)/src/lib/corbahelper/libcorbahelper.la \
                       $(top_builddir)/src/lib/miconfparser/libmiconfparser.a \
                       $(top_builddir)/src/lib/kvskel/libkvskel.la \
		       $(BOOST_THREAD_LIB) $(omniORB4_LIBS)

kvalobsincludedir = $(pkgincludedir)/kvalobs
kvalobsinclude_HEADERS = \
	kvDbGate.h \
	kvStationInfo.h \
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
	kvexception.h
