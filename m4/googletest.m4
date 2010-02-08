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
   gtest_LIBS=`${GTEST_CONFIG} --libs`
fi

AM_CONDITIONAL([HAVE_GTEST], [test -x ${GTEST_CONFIG}])

AC_SUBST(gtest_CFLAGS)
AC_SUBST(gtest_LIBS)
])
