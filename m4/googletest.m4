AC_DEFUN([GTEST_CHECK],
[
AC_ARG_WITH(
	[gtest],
	AS_HELP_STRING([--with-gtest=PATH],
	[Specify install location for googletest.]),
	[GTEST_CONFIG_PATH="${with_gtest}/bin"}],
	[GTEST_CONFIG_PATH="${PATH}"]
)
AC_PATH_PROG(GTEST_CONFIG, [gtest-config], [no], ${GTEST_CONFIG_PATH})

if test -x ${GTEST_CONFIG}; then
   gtest_CFLAGS=`${GTEST_CONFIG} --cppflags`
   gtest_LIBS="`${GTEST_CONFIG} --ldflags` `${GTEST_CONFIG} --libs`"
fi

AM_CONDITIONAL([HAVE_GTEST], [test -x ${GTEST_CONFIG}])

AC_SUBST(gtest_CFLAGS)
AC_SUBST(gtest_LIBS)
])



AC_DEFUN([GMOCK_CHECK],
[
AC_ARG_WITH(
	[gmock],
	AS_HELP_STRING([--with-gmock=PATH],
	[Specify install location for googlemock.]),
	[GMOCK_CONFIG_PATH="${with_gmock}/bin"}],
	[GMOCK_CONFIG_PATH="${PATH}"]
)
AC_PATH_PROG(GMOCK_CONFIG, [gmock-config], [no], ${GMOCK_CONFIG_PATH})

if test -x ${GMOCK_CONFIG}; then
   gmock_CFLAGS=`${GMOCK_CONFIG} --cppflags`

   GMOCK_LIBRARY_DIR=`${GMOCK_CONFIG} --libdir`
   if test x != x$GMOCK_LIBRARY_DIR; then
   	gmock_LIBS="-L${GMOCK_LIBRARY_DIR} `${GMOCK_CONFIG} --libs`"
   else
    gmock_LIBS="`${GMOCK_CONFIG} --libs`"
   fi
fi

AM_CONDITIONAL([HAVE_GMOCK], [test -x ${GMOCK_CONFIG}])

AC_SUBST(gmock_CFLAGS)
AC_SUBST(gmock_LIBS)
])
