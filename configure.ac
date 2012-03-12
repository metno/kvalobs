AC_PREREQ([2.59])
AC_INIT([kvalobs], [2.4.7], [kvoss@met.no])
AM_INIT_AUTOMAKE
#METNO_SET_VERSION_INFO

#AC_CONFIG_SRCDIR([src/diNetCDF.cc])
#AC_CONFIG_HEADER([include/config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_LIBTOOL
AM_PROG_LEX
AM_PROG_CC_C_O
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_INSTALL

AC_SUBST([OLD_KVALOBSLIBS_SO_VERSION], [2:1:1])
AC_SUBST([KVALOBSLIBS_SO_VERSION], [3:0:0])

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
                 src/lib/decoder/bufrdecoder/Makefile
                 src/lib/decoder/rejectdecoder/Makefile
                 src/lib/decoder/kv2kvdecoder/Makefile
                 src/lib/decoder/dummydecoder/Makefile
                 src/lib/decoder/kldecoder/Makefile
                 src/lib/decoder/autoobsdecoder/Makefile
                 src/lib/decoder/comobsdecoder/Makefile
                 src/lib/decoder/comobsdecoder/test/Makefile
                 src/lib/decoder/synopdecoder/Makefile
                 src/service-libs/Makefile
                 src/service-libs/kvcpp/Makefile
                 src/service-libs/kvcpp/libkvcpp.pc
                 src/service-libs/kvcpp/corba/Makefile
                 src/service-libs/kvcpp/mock/Makefile
                 src/data2kv/Makefile
                 src/kvDataInputd/Makefile
                 src/kvServiced/test/Makefile
                 src/kvServiced/Makefile
                 src/kvQabased_old/Makefile
                 src/kvQabased/Makefile
                 src/kvManagerd/Makefile
                 src/model2kv/Makefile
                 src/kv2metdat/Makefile
                 src/kv2kro2/Makefile
                 src/tools/kvpush/Makefile
                 src/kvalobs_database/Makefile
                 src/kvalobs_database/upgrade/Makefile
                 src/script/Makefile
                 src/script/kv/Makefile
                 src/script/dbadmin/Makefile
                 doc/Makefile
])



PKG_CHECK_MODULES(putools, puTools)
KV_FIND_OMNIORB4
KV_POSTGRESQL
PKG_CHECK_MODULES(sqlite3, sqlite3)
#PKG_CHECK_MODULES(sqlite2, sqlite)
PKG_CHECK_MODULES(libxml2, libxml-2.0)
PKG_CHECK_MODULES(libxmlpp, libxml++-2.6)

PKG_CHECK_MODULES(cppunit, cppunit)
GTEST_CHECK
GMOCK_CHECK


KV_PERL
AX_BOOST_BASE
AX_BOOST_THREAD
if test -z $BOOST_THREAD_LIB; then AC_MSG_ERROR([Unable to find boost thread library]); fi 
AX_BOOST_REGEX
if test -z $BOOST_REGEX_LIB; then AC_MSG_ERROR([Unable to find boost regex library]); fi 
AX_BOOST_FILESYSTEM
if test -z $BOOST_FILESYSTEM_LIB; then AC_MSG_ERROR([Unable to find boost filesystem library]); fi
AX_BOOST_PROGRAM_OPTIONS
if test -z $BOOST_PROGRAM_OPTIONS_LIB; then AC_MSG_ERROR([Unable to find boost program_options library]); fi
 
KV_MODEL2KV

AC_CONFIG_HEADERS(config.h)

AC_OUTPUT
