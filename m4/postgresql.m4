#
# SYNOPSIS
#
#   KV_POSTGRESQL
#
# DESCRIPTION
#
#   Checks for postgresql. Allows user to specify path to pg_config
#
#   This macro calls:
#
#     AC_SUBST(postgresql_CFLAGS)
#     AC_SUBST(postgresql_LIBS)
#
# LAST MODIFICATION
#
#   2008-03-05
#
# COPYLEFT
#
#   Copyright (c) 2007 Meteorologisk institutt <diana@met.no>
#
#   This program is free software: you can redistribute it and/or
#   modify it under the terms of the GNU General Public License as
#   published by the Free Software Foundation, either version 3 of the
#   License, or (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
#   General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program. If not, see
#   <http://www.gnu.org/licenses/>.
#

AC_DEFUN([KV_POSTGRESQL],
[
	AC_ARG_WITH([postgresql],
		[AS_HELP_STRING([--with-postgresql=PATH], 
			[Specify the directory in which postgresql is installed (by default, configure searches your PATH). If set, configure will search PATH/bin for pg_config])],
		[PG_CONFIG=${with_postgresql}/bin/pg_config],
		[PG_CONFIG=`which pg_config`]
	)

	if test $PG_CONFIG -a -x $PG_CONFIG; then
		postgresql_CFLAGS=`$PG_CONFIG --includedir`
		AC_SUBST(postgresql_CFLAGS)
		postgresql_LIBS="-L`$PG_CONFIG --libdir` `$PG_CONFIG --libs`"
		AC_SUBST(postgresql_LIBS)
	else
		AC_MSG_ERROR([Unable to locate pg_config - please use --with-postgresql to specify where to find it.])
	fi
])
