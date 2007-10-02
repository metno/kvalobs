/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvPerlParser.h,v 1.8.2.1 2007/09/27 09:02:37 paule Exp $                                                       

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
#ifndef _kvPerlParser_h
#define _kvPerlParser_h

#include <string>
#include <map>

/*  Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
    at Wed May  8 08:58:51 2002
*/

struct interpreter;
typedef struct interpreter PerlInterpreter;

/**
   
  \brief QABase perl-parser

  - accepts a perl-script (optional: parameters to put on the
    perl-stack)
  - execute script in embedded interpreter
  - return values from perl-stack

*/

class kvPerlParser {
protected:
  /// The Perl interpreter instance
  static PerlInterpreter *my_perl;
  
  /// Do the necessary initialisation of Perl before usage
  static void initPerl();

public:
  kvPerlParser();
  ~kvPerlParser();

  /**
     Compile and run the contents of 'script' in the Perl 
     interpreter. Return values from the run are string-double pairs, 
     and are pushed on the 'params' map.
   */
  bool runScript(const std::string& script,
 		 std::map<std::string, double>& params);
		 

  static void freePerl(); /// clean up before termination
};

#endif
