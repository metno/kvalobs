AC_PREREQ([2.59])
AC_INIT([kvalobs], [7.0.2], [kvoss@met.no])
AM_INIT_AUTOMAKE([subdir-objects])
AC_CONFIG_MACRO_DIR([m4])

AC_PROG_LIBTOOL
AC_PROG_LEX
if test "x$LEX" == "x:"; then
	AC_MSG_ERROR([lex/flex not found])
fi

AM_PROG_CC_C_O
AC_PROG_YACC
AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LN_S
AX_CXX_COMPILE_STDCXX_11(noext)

#echo "LIBDIR: ${libdir}"

#AX_SUBDIRS_CONFIGURE( [subdirs], [mandatory arguments], [possibly merged arguments], [replacement arguments], [forbidden arguments])
#AX_SUBDIRS_CONFIGURE( [kvsupport/librdkafka-1.2.1], [--prefix=${libdir}/kvalobs], [], [--prefix=${libdir}/kvalobs], [])

#export PKG_CONFIG_PATH=`pwd`"/kvsupport/build/lib/pkgconfig:${PKG_CONFIG_PATH}"
export PKG_CONFIG_PATH=`pwd`"/kvsupport/build$prefix/lib/kvalobs10/lib/pkgconfig:${PKG_CONFIG_PATH}"
echo "PKG_CONFIG_PATH: '${PKG_CONFIG_PATH}'"
#AC_CONFIG_SUBDIRS([src/lib/libhttpserver/libhttpserver-0.17.5])

AC_SUBST([OLD_KVALOBSLIBS_SO_VERSION], [9:0:0])

#KVALOBSLIBS_BIN_VERSION must match the major version of KVALOBSLIBS_SO_VERSION
AC_SUBST([KVALOBSLIBS_BIN_VERSION], [10])
AC_SUBST([KVALOBSLIBS_SO_VERSION], [10:0:0])

AC_CONFIG_FILES([Makefile
                 kvsupport/Makefile
                 kvconfig
                 share/Makefile
                 share/kvalobs/Makefile
                 share/kvalobs/idl/Makefile
                 src/Makefile
                 src/lib/Makefile
                 src/lib/gtest/Makefile
		             src/lib/json/Makefile
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
                 src/lib/kvsubscribe/Makefile
                 src/lib/decoder/Makefile
                 src/lib/decoder/decoderbase/Makefile
                 src/lib/decoder/decoderbase/test/Makefile
                 src/lib/decoder/bufrdecoder/Makefile
                 src/lib/decoder/rejectdecoder/Makefile
                 src/lib/decoder/kv2kvdecoder/Makefile
                 src/lib/decoder/dummydecoder/Makefile
                 src/lib/decoder/execdecoder/Makefile
                 src/lib/decoder/kldecoder/Makefile
                 src/lib/decoder/kldecoder/test/Makefile
                 src/lib/decoder/kltext/Makefile
                 src/lib/decoder/autoobsdecoder/Makefile
                 src/lib/decoder/comobsdecoder/Makefile
                 src/lib/decoder/comobsdecoder/test/Makefile
                 src/lib/decoder/synopdecoder/Makefile
                 src/lib/decoder/synopdecoder/test/Makefile
                 src/kafka-util/Makefile
                 src/service-libs/Makefile
                 src/service-libs/kvcpp/Makefile
                 src/service-libs/kvcpp/corba/Makefile
                 src/service-libs/kvcpp/sql/Makefile
                 src/service-libs/kvcpp/kafka/Makefile
                 src/service-libs/kvcpp/mock/Makefile
                 src/kvDataInputd/Makefile
                 src/kvQabased/Makefile
                 src/kvManagerd/Makefile
                 src/tools/kv2kvparser/Makefile
                 src/tools/kv2kvDataInputd/Makefile
                 src/tools/kvpull/Makefile
                 src/kvDataInputd/kvdataclt/Makefile
                 src/tools/aexecd/Makefile
                 src/kvalobs_database/Makefile
                 src/kvalobs_database/upgrade/Makefile
                 src/script/Makefile
                 src/script/kv/Makefile
                 src/script/dbadmin/Makefile
                 doc/Makefile
                 src/service-libs/kvcpp/libkvcpp.pc
                 src/lib/kvdb/libkvalobs_kvdb.pc
                 src/lib/decoder/decoderbase/libkvalobs_decoderbase.pc
                 src/lib/decodeutility/libkvalobs_decodeutility.pc
                 src/lib/dnmithread/libkvalobs_dnmithread.pc
                 src/lib/miconfparser/libkvalobs_miconfparser.pc
                 src/lib/miutil/libkvalobs_miutil.pc
                 src/lib/milog/libkvalobs_milog.pc
                 src/lib/corbahelper/libkvalobs_corbahelper.pc
                 src/lib/fileutil/libkvalobs_fileutil.pc
                 src/lib/kvalobs/libkvalobs_kvalobs.pc
                 src/lib/kvskel/libkvalobs_kvskel.pc
                 src/lib/kvskel/libkvalobs_kvadmininterface.pc
                 src/lib/kvsubscribe/libkvsubscribe.pc
                 debian_files/kvalobs-init.postinst
                 debian_files/kvalobs-init.postrm
                 debian_files/kvalobs.install
                 debian_files/kvalobs.postinst
                 debian_files/kvalobs.postrm 
                 debian_files/libkvalobs10.install
                 debian_files/rules
])

#src/tools/kvpush/Makefile

#libcurl, sets CURL_LIBS and CURL_CFLAGS.
AX_LIB_CURL([7.15.5])

PKG_CHECK_MODULES(putools, puTools)
KV_FIND_OMNIORB4
KV_POSTGRESQL
#PKG_CHECK_MODULES(sqlite3, sqlite3)
#sqlite3_CFLAGS=

PKG_CHECK_MODULES(libxml2, libxml-2.0)
PKG_CHECK_MODULES(libxmlpp, libxml++-2.6)

#RDKAFKA_CHECK
old_PKG_CONFIG=${PKG_CONFIG}
echo "old_PKG_CONFIG: '${PKG_CONFIG}'"
PKG_CONFIG="${PKG_CONFIG} --define-prefix "
PKG_CHECK_MODULES( [rdkafka], [rdkafka++], [], [true])
PKG_CHECK_MODULES( [rdkafkac], [rdkafka], [], [true])
PKG_CHECK_VAR(rdkafka_LIBDIR, rdkafka++, libdir)

#Use local build of sqlite3
PKG_CHECK_MODULES(sqlite3, sqlite3)


echo "rdkafka_LIBS '$rdkafka_LIBS'"
echo "rdkafka_CFLAGS '$rdkafka_CFLAGS'"

#libhttpserver
PKG_CHECK_MODULES( [libhttpserver], [libhttpserver >= 0.17.0], [], [true] )
PKG_CHECK_VAR(libhttpserver_LIBDIR, libhttpserver, libdir)
echo "libhttpserver_LIBDIR '$libhttpserver_LIBDIR'"
PKG_CONFIG=${old_PKG_CONFIG}

#Test, cppunit and gtest
PKG_CHECK_MODULES(cppunit, cppunit)
AC_PATH_PROG(CMAKE, cmake, false)
GMOCK_DIST_CHECK([src/lib/gtest])


#Perl
AC_CHECK_LIB(perl, [main], [], [AC_MSG_ERROR([no libperl found])])

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
AX_BOOST_DATE_TIME
if test -z $BOOST_DATE_TIME_LIB; then AC_MSG_ERROR([Unable to find boost date_time library]); fi
AX_BOOST_SYSTEM
if test -z $BOOST_SYSTEM_LIB; then AC_MSG_ERROR([Unable to find boost system library]); fi

KV_MODEL2KV

AC_CONFIG_HEADERS(config.h)

AC_OUTPUT
