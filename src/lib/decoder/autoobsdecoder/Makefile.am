pkglibdir = $(libdir)/@PACKAGE_NAME@@KVALOBSLIBS_BIN_VERSION@
decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = autoobs.la
autoobs_la_SOURCES = autoobs.cc  autoobs.h \
                     autoobsdecoder.cc autoobsdecoder.h \
                     convert.cc  convert.h  \
                     DontDecodeParams.cc  \
                     NameConvertDef.cc  \
                     SplitDef.cc 
autoobs_la_CPPFLAGS = -D_REENTRANT \
                      -I$(top_srcdir)/src/lib  \
                      -I$(top_srcdir)/src/lib/decoder \
                      $(putools_CFLAGS) $(libxmlpp_CFLAGS)
autoobs_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
                    $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                    $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                    $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                    $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
		    		$(top_builddir)/src/lib/miutil/libkvalobs_miutil.la 
autoobs_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
