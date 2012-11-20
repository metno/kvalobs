
# Usage:
# GTEST_CHECK([compile_location])
# If gtest headers are found, but not libraries, googletest may be 
# automatically compiled in the given location - but you need to provide 
# makefiles/rules for that yourself. The path will be relative to top_builddir 
AC_DEFUN([GTEST_CHECK],
[
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([gtest.h gtest/gtest.h], [have_gtest=true])
gtest_CFLAGS=
AC_CHECK_LIB(gtest, testing::TestResult::Clear(), 
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
