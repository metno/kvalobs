bin_PROGRAMS = aexecd aexecdclt

aexecd_SOURCES = \
	aexecd.cc aexecd.h \
	ExecHelper.cc ExecHelper.h \
	InitLogger.cc InitLogger.h

aexecd_CPPFLAGS = -D_REENTRANT \
                  -I$(top_srcdir)/src/lib 
aexecd_LDFLAGS = -pthread 
aexecd_LDADD = \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la\
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la\
   $(BOOST_THREAD_LIB) \
	-lm 

aexecdclt_SOURCES = aexecdclt.cc

aexecdclt_CPPFLAGS = -D_REENTRANT \
                  -I$(top_srcdir)/src/lib 
aexecdclt_LDFLAGS = -pthread 
aexecdclt_LDADD = \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
	$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la\
	$(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la\
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la\
   $(BOOST_THREAD_LIB) \
	-lm 

#EXTRA_DIST =                
