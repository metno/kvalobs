AM_CPPFLAGS = -I $(top_srcdir)/src/lib -D_REENTRANT

lib_LTLIBRARIES = libkvalobs_milog.la

milogincludedir =$(pkgincludedir)/milog
nobase_miloginclude_HEADERS = Logger.h \
	LogStream.h \
	Layout.h \
	StdLayout.h \
	StdLayout1.h \
	FLogStream.h  \
	TraceLayout.h \
	LogManager.h \
	HtmlStream.h \
	HtmlLayout.h \
	StdErrStream.h \
	miloghelper.h \
	milog.h macros.h \
	milogtypes.h \
	utility.h \
	private/LoggerImpl.h private/ThreadLogManager.h private/types.h \
	thread/DummyThread.h thread/PThread.h thread/Thread.h

#milogprivateincludedir =$(pkgincludedir)/milog/private
#milogprivateinclude_HEADERS = private/LoggerImpl.h private/ThreadLogManager.h private/types.h


libkvalobs_milog_la_SOURCES = Logger.h Logger.cc \
	LogStream.h LogStream.cc \
	Layout.h Layout.cc \
	StdLayout.h StdLayout.cc \
	StdLayout1.h StdLayout1.cc \
	FLogStream.h FLogStream.cc \
	LoggerImpl.cc \
	TraceLayout.h TraceLayout.cc \
	LogManager.h LogManager.cc \
	ThreadLogManager.cc \
	HtmlStream.h HtmlStream.cc \
	HtmlLayout.h HtmlLayout.cc \
	StdErrStream.h StdErrStream.cc \
	PThread.cc \
	miloghelper.h miloghelper.cc \
	milog.h macros.h \
	milogtypes.h \
	utility.h utility.cc 
	

# don't install: LoggerImpl.h ThreadLogManager.h types.h

SUBDIRS = private thread

libkvalobs_milog_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@ -pthread
libkvalobs_milog_la_LIBADD = -lpthread
