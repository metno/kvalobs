AC_DEFUN([DOCBOOK_TO_MAN],
[
AC_PATH_PROG(XMLTO, [xmlto], [no])

if test x${XMLTO} = x; then
	AC_MSG_ERROR([Unable to find xmlto])
fi
])