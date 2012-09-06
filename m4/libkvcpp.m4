#
# SYNOPSIS
#
#   LIBKVCPP
#
# DESCRIPTION
#
#   Checks for libkvcpp, using pkg-config. Also define KVIDLDIR. KVIDLDIR is the
#   path to the CORBA IDL files that defines the interface to kvalobs.
#
#   This macro calls:
#
#     AC_SUBST(KVIDLDIR)
#
# LAST MODIFICATION
#
#   2010-04-16
#
# COPYLEFT
#
#   Copyright (c) 2007 Meteorologisk institutt <kvoss@met.no>
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

AC_DEFUN([LIBKVCPP],
[
    PKG_CHECK_MODULES(kvcpp, libkvcpp)

	KVIDLDIR=`pkg-config --variable=idldir libkvcpp`
	if test ! -x $KVIDLDIR; then
		AC_MSG_ERROR([Unable to locate the idl files to KVALOBS])
	fi
	
	AC_SUBST(KVIDLDIR)
])


AC_DEFUN([LIBKVCPP2],
[
    PKG_CHECK_MODULES(kvcpp, libkvcpp2)

    KVIDLDIR=`pkg-config --variable=idldir libkvcpp2`
    if test ! -x $KVIDLDIR; then
        AC_MSG_ERROR([Unable to locate the idl files to KVALOBS])
    fi
    
    AC_SUBST(KVIDLDIR)
])
