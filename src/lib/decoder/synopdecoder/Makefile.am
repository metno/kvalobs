decoderdir=$(pkglibdir)/decode

decoder_LTLIBRARIES = synop.la
AM_LFLAGS = -PyySynop -olex.yy.c
synop_la_SOURCES = decode.cc  \
                   kvSynopDecoder.cc  kvSynopDecoder.h  \
                   synop.cc  synop.h \
                   synopdecoder.cc  synopdecoder.h  \
                   synopElements.h \
                   synopentry.cc  synopentry.h \
                   synopLexer.h synopLexer.ll
synop_la_CPPFLAGS = -D_REENTRANT \
                    -I$(top_srcdir)/src/lib \
                    -I$(top_srcdir)/src/lib/decoder \
                    $(putools_CFLAGS)
synop_la_LIBADD =                     
synop_la_LDFLAGS = -module