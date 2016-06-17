SUBDIRS=test
decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = comobsdecoder.la

comobsdecoder_la_SOURCES = comobsdecode.cc  comobsdecode.h \
                           comobsentry.cc  comobsentry.h \
                           ConfInfo12.cc ConfInfo12.h \
                           decodeddata.cc decodeddata.h \
                           decodeutil.cc decodeutil.h \
                           sms12.cc  sms12.h \
                           sms13.cc  sms13.h \
                           sms2.cc  sms2.h \
                           smsbase.cc  smsbase.h \
                           smsdata.cc  smsdata.h \
                           smsmelding.cc  smsmelding.h \
                           smsmeldingparser.cc smsmeldingparser.h
                                             
comobsdecoder_la_CPPFLAGS = -D_REENTRANT \
                            -I$(top_srcdir)/src/lib \
                            -I$(top_srcdir)/src/lib/decoder \
                            $(putools_CFLAGS) $(libxmlpp_CFLAGS)

comobsdecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs_decoderbase.la \
                          $(top_builddir)/src/lib/decodeutility/libkvalobs_decodeutility.la \
                          $(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
                          $(top_builddir)/src/lib/milog/libkvalobs_milog.la \
                          $(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
			  			  		  $(top_builddir)/src/lib/miutil/libkvalobs_miutil.la \
                          $(top_builddir)/src/lib/miconfparser/libkvalobs_miconfparser.la \
                          $(libxml2_LIBS)

comobsdecoder_la_LDFLAGS = -module -version-info @KVALOBSLIBS_SO_VERSION@
