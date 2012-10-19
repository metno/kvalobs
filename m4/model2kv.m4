AC_DEFUN([KV_MODEL2KV],
[
	AC_ARG_ENABLE( 
		[model2kv],
		[AS_HELP_STRING([--enable-model2kv],[Also build model2kv component.])],
		[
			case "${enableval}" in
				yes) model2kv=true ;;
				no)  model2kv=false ;;
				*) AC_MSG_ERROR([bad value ${enableval} for --enable-model2kv]) ;;
			esac
		],
		[model2kv=false]
	)
	AM_CONDITIONAL([BUILD_MODEL2KV], [test x$model2kv = xtrue] ) 

	if test x$model2kv = xtrue; then
		PKG_CHECK_MODULES(libpose,libpose)
	fi
]
)
