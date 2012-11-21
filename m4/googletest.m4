
# Usage:
# GTEST_CHECK([compile_location])
# If gtest headers are found, but not libraries, googletest may be 
# automatically compiled in the given location - but you need to provide 
# makefiles/rules for that yourself. The path will be relative to top_builddir 
AC_DEFUN([GTEST_CHECK],
[
AC_ARG_WITH([gtest],
    [AS_HELP_STRING([--with-gtest], [Specify google test directory])],
    [gtest_base=${with_gtest}],
    [gtest_base=/usr])

AC_LANG_PUSH(C++)

includes_old="${INCLUDES}"
AS_IF([test "x$gtest_base" = "x/usr"],
    [],
    [gtest_includes="-I${gtest_base}/include"])

INCLUDES="${INCLUDES} ${gtest_includes}"
AC_CHECK_HEADER([gtest/gtest.h],
    [gtest_CFLAGS=${gtest_includes}
    have_gtest=true],
    [AC_MSG_WARN([Unable to find header gtest/gtest.h])])

INCLUDES="${includes_old}"



ldflags_old="${LDFLAGS}"
AS_IF([test "x$gtest_base" = "x/usr"],
    [],
    [gtest_ldflags="-L${gtest_base}/lib"])
LDFLAGS="${LDFLAGS} ${gtest_ldflags}"
OLD_LIBS=${LIBS}
LIBS="${LIBS} -lgtest"
AC_LINK_IFELSE([AC_LANG_PROGRAM([#include <gtest/gtest.h>], [])],
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
LIBS=${OLD_LIBS}
AM_CONDITIONAL(HAVE_GTEST, [test x${have_gtest} = xtrue])
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
