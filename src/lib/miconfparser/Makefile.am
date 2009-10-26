AM_CPPFLAGS = -I $(top_srcdir)/src/lib -fPIC

pkglib_LIBRARIES = libmiconfparser.a

libmiconfparser_a_SOURCES = valelement.h valelement.cc \
	confsection.h confsection.cc \
	trimstr.h trimstr.cc \
	confparser.h confparser.cc \
	miconfparser.h \
	confexception.h ConfParserLexer.ll

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
miconfparse_LDADD = libmiconfparser.a

# Does not work
#EXTRA_PROGRAMS = miconfparser
#miconfparser_SOURCES = miconfparser.cc
#miconfparser_LDADD = libmiconfparser.a
