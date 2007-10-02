/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvglue.h,v 1.1.6.1 2007/09/27 09:02:46 paule Exp $                                                       

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
#ifndef __kvglue_perl_h__
#define __kvglue_perl_h__


#ifdef __cplusplus
extern "C" {
#endif

int
kvsInit(int argn, char **argv);
  
typedef struct kvsParam_ { 
  long paramID;
  char *name;
  char *unit;
  long level_scale;
} kvsParam;
  
typedef  struct kvsParamList_ { 
  kvsParam      *param;
  struct kvsParamList_ *next;
} kvsParamList;
  
/**
 * kvsPopParamList, deletes the element at the front
 * and returns the rest of the list. When the last element 
 * is poped NULL is returned.
 */
kvsParamList*
kvsPopParamList(kvsParamList *list);

void
kvsFreeParamList(kvsParamList *list);
                     
kvsParamList*
kvsGetParams();

#ifdef __cplusplus
}
#endif


#endif
