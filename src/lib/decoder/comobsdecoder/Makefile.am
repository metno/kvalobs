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
comobsdecoder_la_LIBADD = $(top_builddir)/src/lib/miutil/libmiutil.la \
                          $(top_builddir)/src/lib/miconfparser/libmiconfparser.a
comobsdecoder_la_LDFLAGS = -module
