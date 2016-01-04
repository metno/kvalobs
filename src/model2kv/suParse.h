/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: suParse.h,v 1.3.6.2 2007/09/27 09:02:37 paule Exp $                                                       

 Copyright (C) 2007 met.no

 Contact information:
 Norwegian Meteorological Institute
 Box 43 Blindern
 0313 OSLO
 NORWAY
 email: kvalobs-dev@met.no

 This file is part of KVALOBS

 KVALOBS is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License as 
 published by the Free Software Foundation; either version 2 
 of the License, or (at your option) any later version.
 
 KVALOBS is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 General Public License for more details.
 
 You should have received a copy of the GNU General Public License along 
 with KVALOBS; if not, write to the Free Software Foundation Inc., 
 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */
/* A Bison parser, made by GNU Bison 1.875c.  */

/* Skeleton parser for Yacc-like parsing with Bison,
 Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2, or (at your option)
 any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place - Suite 330,
 Boston, MA 02111-1307, USA.  */

/* As a special exception, when this file is copied by Bison into a
 Bison output file, you may use that output file without restriction.
 This special exception was added by the Free Software Foundation
 in version 1.24 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
/* Put the tokens into the symbol table, so that GDB and other debuggers
 know about them.  */
enum yytokentype {
  DATAIN = 258,
  FILETYPE = 259,
  OUTPUT = 260,
  POSNCHANGE = 261,
  PARCHANGE = 262,
  PARDEF = 263,
  MODNAME = 264,
  SUFFTAG = 265,
  QSTRING = 266,
  CMDNO = 267,
  FTYPE = 268
};
#endif
#define DATAIN 258
#define FILETYPE 259
#define OUTPUT 260
#define POSNCHANGE 261
#define PARCHANGE 262
#define PARDEF 263
#define MODNAME 264
#define SUFFTAG 265
#define QSTRING 266
#define CMDNO 267
#define FTYPE 268

#if ! defined (YYSTYPE) && ! defined (YYSTYPE_IS_DECLARED)
#line 15 "suParse.y"
typedef union YYSTYPE {
  int integer;
  enum filetype ftype;
  char* string;
  struct posfile pfile;
  struct bistringlist* bstrl;
  struct bistring* bstr;
} YYSTYPE;
/* Line 1275 of yacc.c.  */
#line 72 "suParse.h"
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yysulval;

