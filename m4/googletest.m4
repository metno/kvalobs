
# Usage:
# GTEST_CHECK([compile_location])
# If gtest headers are found, but not libraries, googletest may be 
# automatically compiled in the given location - but you need to provide 
# makefiles/rules for that yourself. The path will be relative to top_builddir 
AC_DEFUN([GTEST_CHECK],
[
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([gtest/gtest.h], [have_gtest=true])
gtest_CFLAGS=

OLD_LIBS=$LIBS
LIBS=-lgtest
OLD_LDFLAGS=$LDFLAGS
LDFLAGS=-pthread
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <gtest/gtest.h>], [testing::AssertionResult r(true)])],
	[gtest_LIBS=-lgtest],
	[
	if test $1; then
		AC_MSG_NOTICE([Unable to find precompiled googletest libraries - must compile own version])
		must_compile_gtest=true
		gtest_LIBS="-L\$(top_builddir)/$1 -lgtest"
	else
		AC_MSG_WARN([Found headers but no precompiled googletest libraries - uanble to use googletest])
		have_gtest=false
	fi
])
LIBS=$OLD_LIBS
LDFLAGS=$OLD_LDFLAGS
AM_CONDITIONAL(HAVE_GTEST, [test x${have_gtest} != x])
AM_CONDITIONAL(MUST_COMPILE_GTEST, [test x${must_compile_gtest} = xtrue])

AC_SUBST(gtest_CFLAGS)
AC_SUBST(gtest_LIBS)

AC_LANG_POP
])

AC_DEFUN([GMOCK_CHECK],
[
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([gmock.h gmock/gmock.h], [have_gmock=true])
gmock_CFLAGS=
gmock_LIBS="-lgmock \$(gtest_LIBS)"
AM_CONDITIONAL(HAVE_GMOCK, [test x${have_gmock} != x])

AC_SUBST(gmock_CFLAGS)
AC_SUBST(gmock_LIBS)

AC_LANG_POP
])
