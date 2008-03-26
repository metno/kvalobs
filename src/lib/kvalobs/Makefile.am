AM_CPPFLAGS = -I$(top_srcdir)/src/lib -I$(top_builddir)/src/lib -D_REENTRANT

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
libkvalobs_la_LIBS = ../miutil/libmiutil.a

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
 