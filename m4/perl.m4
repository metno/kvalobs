#
# SYNOPSIS
#
#   KV_PERL
#
# DESCRIPTION
#
#   Checks for perl, and switches for compiling against perl.
#
#   This macro calls:
#
#     AC_SUBST(PERL)
#     AC_SUBST(perl_CFLAGS)
#     AC_SUBST(perl_LIBS)
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

AC_DEFUN([KV_PERL],
[
	AC_ARG_WITH(
		[perl],
		[AS_HELP_STRING([--with-perl=EXECUTABLE_PATH],[Specify the location of the perl program. The version must greater than 5.8 and less than 6.])],
		[PERL=${with_perl}],
		[AC_PATH_PROG(PERL, perl)]
	)
	AC_SUBST(PERL)

	if test $PERL -a -x $PERL; then
		# TODO: Add a version check here
		perl_CFLAGS=`$PERL -MExtUtils::Embed -e ccopts`
		AC_SUBST(perl_CFLAGS)
		perl_LIBS=`$PERL -MExtUtils::Embed -e ldopts`
		AC_SUBST(perl_LIBS)
	else
		AC_MSG_ERROR([Unable to find perl])
	fi
]
)
