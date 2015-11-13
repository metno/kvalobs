AC_DEFUN([RDKAFKA_CHECK],
[
AC_LANG_PUSH(C++)

AC_CHECK_HEADER([librdkafka/rdkafkacpp.h],
    [rdkafka_CFLAGS=
    have_rdkafka=true],
    [AC_MSG_ERROR([Unable to find header librdkafka/rdkafkacpp.h])])

AC_CHECK_LIB(rdkafka++, main,
	[rdkafka_LIBS="-lrdkafka++ -lrdkafka"],
	[AC_MSG_ERROR([Unable to find or use librdkafka++])],
	[])

AC_SUBST(rdkafka_CFLAGS)
AC_SUBST(rdkafka_LIBS)

AC_LANG_POP
])
