AM_CPPFLAGS = -I $(top_srcdir)/src/lib

lib_LTLIBRARIES = libkvalobs_miconfparser.la

libkvalobs_miconfparser_la_SOURCES = valelement.h valelement.cc \
	confsection.h confsection.cc \
	trimstr.h trimstr.cc \
	confparser.h confparser.cc \
	miconfparser.h \
	confexception.h ConfParserLexer.ll

libkvalobs_miconfparser_la_LDFLAGS = -version-info @KVALOBSLIBS_SO_VERSION@ \
	@BOOST_LDFLAGS@
libkvalobs_miconfparser_la_LIBADD = @BOOST_SYSTEM_LIB@
AM_LFLAGS = -PyyConfParser -olex.yy.c

miconfparserincludedir = $(pkgincludedir)/miconfparser
miconfparserinclude_HEADERS = \
	valelement.h \
	confsection.h \
	trimstr.h \
	confparser.h \
	miconfparser.h \
	confexception.h


EXTRA_PROGRAMS = miconfparse 
miconfparse_SOURCES = miconfparse.cc
miconfparse_LDADD = libkvalobs_miconfparser.la

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libkvalobs_miconfparser.pc
$(pkgconfig_DATA): $(top_builddir)/config.status
