decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = bufrdecoder.la

noinst_PROGRAMS = bufrdecoder

bufrdecoder_SOURCES = kvParamdefs.h kvParamdefs.cc\
					  BufrMessage.cc BufrMessage.h \ 
					  BufrDecodeBase.cc BufrDecodeBase.h \
					  BufrDecodeKvResult.cc BufrDecodeKvResult.h \
					  BufrDecodeSynoptic.cc BufrDecodeSynoptic.h \
                      bufrdecoder.cc
                      
bufrdecoder_CPPFLAGS = -I$(top_srcdir)/src/lib \
                       $(putools_CFLAGS) $(AM_CPPFLAGS)
bufrdecoder_LDADD = $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
				    $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
					$(putools_LIBS)\
					-L$(HOME)/lib -lbufr -lgfortran \
                    -lm

bufrdecoder_la_SOURCES = bufrdecodeentry.cc bufrdecodeentry.h \
						 BufrMessage.cc BufrMessage.h 
						                           
bufrdecoder_la_CPPFLAGS = -D_REENTRANT \
                            -I$(top_srcdir)/src/lib \
                            -I$(top_srcdir)/src/lib/decoder \
                            $(putools_CFLAGS) $(libxmlpp_CFLAGS)

bufrdecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
                          $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                          $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                          $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                          $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
			  			  $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                          $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                          $(putools_LIBS) $(libxml2_LIBS)

bufrdecoder_la_LDFLAGS = -module
