#
# SYNOPSIS
#
#   KV_FIND_OMNIORB4
#
# DESCRIPTION
#
#   Checks for omniORB4, using pkg-config. Also defines IDL to be the omniidl
#   that belongs to the same omniORB.
#
#   This macro calls:
#
#     AC_SUBST(IDL)
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

AC_DEFUN([KV_FIND_OMNIORB4],
[
	PKG_CHECK_MODULES(omniORB4, omniORB4)
	PKG_CHECK_MODULES(omniDynamic4, omniDynamic4)

	IDL=`pkg-config --variable=omniidl omniORB4`
	if test ! -x $IDL; then
		AC_MSG_ERROR([Unable to locate idl compiler])
	fi

	# Hack: omniORB requires this pythonpath - compilation will fail otherwise	
	IDL="PYTHONPATH=`pkg-config --variable=libdir omniORB4`/python2.4/site-packages/ $IDL"
	
	AC_SUBST(IDL)
])