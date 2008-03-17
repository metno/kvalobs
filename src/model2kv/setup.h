/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: setup.h,v 1.1.6.1 2007/09/27 09:02:37 paule Exp $                                                       

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
/* setup.h -- definitions for matters relating to the input (as read
   by the parser) */

#ifndef _setup_h
#define _setup_h


#include <libpose/psSubstitute.h>
#include <libpose/psRead.h>
#include <libpose/pose.h>

#define _defaultoutfile "pose.output"
#define _backupextension "~"

#ifdef __cplusplus
#define _cplusplus_clean_setup
extern "C" {
#endif

struct globalinput {
  int    nfiles;
  struct posfile* posfilelist;
  char*  outfile;
  char*  pardeffile;
  bool   makeoutputbackup;

  struct substitutions sub;
};

extern struct globalinput setup;

struct nescessaryinput {
  bool datafile;
  bool outputfile;
};

extern struct nescessaryinput inputlist;

extern char** yysuargv;
extern int    yysuargc;

extern FILE* yysuin;

extern int parseinput();

extern int yysulex();
extern int yysuparse();
  
extern void yysuerror(const char*);
  extern void yysuwarning(const char*);
  
#ifdef _cplusplus_clean_setup
#ifndef _clean_setup
#define _clean_setup

  void set_cl(int c, char ** v){
    yysuargc=c;
    yysuargv=v;
  }

  void clean_setup(){
    int i,j;
    for (i=0; i<setup.nfiles; i++) {
      free(setup.posfilelist[i].name);
      if (setup.posfilelist[i].modname)
	free(setup.posfilelist[i].modname);
      if (setup.posfilelist[i].parchanges) {
	for (j=0; j<setup.posfilelist[i].parchanges->entries; j++) {
	  free(setup.posfilelist[i].parchanges->list[j]->changeto);
	  free(setup.posfilelist[i].parchanges->list[j]->changefrom);
	}
	free(setup.posfilelist[i].parchanges->list);
	free(setup.posfilelist[i].parchanges);
      } 
    }
    free(setup.posfilelist);
  };
#endif
#endif






#ifdef __cplusplus
}
#endif

#endif
