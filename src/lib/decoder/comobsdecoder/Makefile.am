SUBDIRS=test
decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = comobsdecoder.la

comobsdecoder_la_SOURCES = comobsdecode.cc  comobsdecode.h \
                           comobsentry.cc  comobsentry.h \
                           ConfInfo12.cc ConfInfo12.h \
                           decodeddata.cc decodeddata.h \
                           decodeutil.cc decodeutil.h \
                           sms12.cc  sms12.h \
                           sms2.cc  sms2.h \
                           smsbase.cc  smsbase.h \
                           smsdata.cc  smsdata.h \
                           smsmelding.cc  smsmelding.h \
                           smsmeldingparser.cc smsmeldingparser.h
                                             
comobsdecoder_la_CPPFLAGS = -D_REENTRANT \
                            -I$(top_srcdir)/src/lib \
                            -I$(top_srcdir)/src/lib/decoder \
                            $(putools_CFLAGS) $(libxmlpp_CFLAGS)

comobsdecoder_la_LIBADD = $(top_builddir)/src/lib/decoder/decoderbase/libkvalobs2_decoderbase.la \
                          $(top_builddir)/src/lib/decodeutility/libkvalobs2_decodeutility.la \
                          $(top_builddir)/src/lib/kvalobs/libkvalobs2_kvalobs.la \
                          $(top_builddir)/src/lib/milog/libkvalobs2_milog.la \
                          $(top_builddir)/src/lib/kvdb/libkvalobs2_kvdb.la \
			  			  $(top_builddir)/src/lib/miutil/libkvalobs2_miutil.la \
                          $(top_builddir)/src/lib/miconfparser/libkvalobs2_miconfparser.la \
                          $(libxml2_LIBS)

comobsdecoder_la_LDFLAGS = -module
