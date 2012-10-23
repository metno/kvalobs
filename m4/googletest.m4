AC_DEFUN([GTEST_CHECK],
[
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([gtest.h gtest/gtest.h], [have_gtest=true])
gtest_CFLAGS=
gtest_LIBS=-lgtest
AM_CONDITIONAL(HAVE_GTEST, [test x${have_gtest} != x])

AC_SUBST(gtest_CFLAGS)
AC_SUBST(gtest_LIBS)

AC_LANG_POP
])

AC_DEFUN([GMOCK_CHECK],
[
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([gmock.h gmock/gmock.h], [have_gmock=true])
gmock_CFLAGS=
gmock_LIBS="-lgmock -lgtest"
AM_CONDITIONAL(HAVE_GMOCK, [test x${have_gmock} != x])

AC_SUBST(gmock_CFLAGS)
AC_SUBST(gmock_LIBS)

AC_LANG_POP
])
