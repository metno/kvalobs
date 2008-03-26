AC_PREREQ([2.59])
AC_INIT([kvalobs], [2.0.0], [kvoss@met.no])
AM_INIT_AUTOMAKE
#METNO_SET_VERSION_INFO

#AC_CONFIG_SRCDIR([src/diNetCDF.cc])
#AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_LIBTOOL
AM_PROG_LEX
AC_PROG_YACC

AC_CONFIG_FILES([Makefile
				kvconfig
				share/Makefile
				share/kvalobs/Makefile
				share/kvalobs/idl/Makefile
                src/Makefile
                src/lib/Makefile
                src/lib/kvskel/Makefile
                src/lib/miutil/Makefile
                src/lib/decodeutility/Makefile
                src/lib/dbdrivers/Makefile
                src/lib/corbahelper/Makefile
                src/lib/milog/Makefile
                src/lib/milog/private/Makefile
                src/lib/milog/thread/Makefile
                src/lib/miconfparser/Makefile
                src/lib/dnmithread/Makefile
                src/lib/fileutil/Makefile
                src/lib/kvdb/Makefile
                src/lib/kvalobs/Makefile
                src/lib/decoder/Makefile
                src/lib/decoder/decoderbase/Makefile
                src/lib/decoder/rejectdecoder/Makefile
                src/lib/decoder/kv2kvdecoder/Makefile
                src/lib/decoder/dummydecoder/Makefile
                src/lib/decoder/kldecoder/Makefile
                src/lib/decoder/autoobsdecoder/Makefile
                src/lib/decoder/comobsdecoder/Makefile
                src/lib/decoder/synopdecoder/Makefile
                src/service-libs/Makefile
                src/service-libs/kvcpp/Makefile
                src/service-libs/kvcpp/kvcpp.pc
                src/service-libs/kvcpp/corba/Makefile
                src/kvDataInputd/Makefile
                src/kvServiced/Makefile
                src/kvQabased/Makefile
                src/kvManagerd/Makefile
                src/norcom2kv/Makefile
                src/kvAgregated/Makefile
                src/kvsynopd/Makefile                
                src/model2kv/Makefile
                src/kv2kro2/Makefile
                src/kvalobs_database/Makefile
])


PKG_CHECK_MODULES(putools, putools)
KV_FIND_OMNIORB4
KV_POSTGRESQL
PKG_CHECK_MODULES(sqlite3, sqlite3)
PKG_CHECK_MODULES(sqlite2, sqlite)
PKG_CHECK_MODULES(libxmlpp, libxml++-1.0)
PKG_CHECK_MODULES(cppunit, cppunit)
KV_PERL
AX_BOOST_BASE
AX_BOOST_THREAD
AX_BOOST_REGEX
AX_BOOST_FILESYSTEM
KV_MODEL2KV

AC_CONFIG_HEADERS(config.h)

AC_OUTPUT