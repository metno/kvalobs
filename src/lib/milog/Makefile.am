AM_CPPFLAGS = -I $(top_srcdir)/src/lib -D_REENTRANT

pkglib_LTLIBRARIES = libmilog.la

libmilog_la_SOURCES = Logger.h Logger.cc \
	LogStream.h LogStream.cc \
	Layout.h Layout.cc \
	StdLayout.h StdLayout.cc \
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
	milogtypes.h 
	
# don't install: LoggerImpl.h ThreadLogManager.h types.h
	
SUBDIRS = private thread
