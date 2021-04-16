decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = kltextdecoder.la

kltextdecoder_la_SOURCES = kltext.h kltext.cc kltextdecoder.cc  kltextdecoder.h  kltextentry.cc  kltextentry.h
kltextdecoder_la_CPPFLAGS = -D_REENTRANT -I$(top_srcdir)/src -I$(top_srcdir)/src/lib  -I$(top_srcdir)/src -I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS)
kltextdecoder_la_LIBADD = \
                 $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
		         $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                 $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la 
kltextdecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@


# if HAVE_GTEST
# check_PROGRAMS = kltest
# TESTS = $(check_PROGRAMS)
  

# kltest_SOURCES=\
# 	kltext.cc \
# 	test/testheader.h \
# 	test/kltest.cc

# kltest_CPPFLAGS =\
# 	-I$(top_srcdir)/src -I$(top_srcdir)/src/lib $(gtest_CFLAGS) \
# 	-I$(top_srcdir)/src/lib/decoder $(putools_CFLAGS) \
# 	-DTESTDIR="\""$(top_srcdir)/src/lib/decoder/kltext/test"\"" \
# 	-DDBDIR="\""$(top_builddir)/src/lib/dbdrivers/.libs"\"" 
# kltest_LDFLAGS = -pthread -export-dynamic
# kltest_LDADD = \
# 	$(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
#    $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
#    $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
#    $(top_builddir)/src/lib/dnmithread/libkvalobs_dnmithread.la\
#    $(top_builddir)/src/lib/fileutil/libkvalobs_fileutil.la \
#    $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
#    $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
#    $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
#    $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
#    $(putools_LIBS) $(libxml2_LIBS) \
# 	$(BOOST_THREAD_LIB) $(BOOST_DATE_TIME_LIB) $(BOOST_SYSTEM_LIB) \
# 	$(gtest_LIBS) \
# 	-lm -ldl
# endif