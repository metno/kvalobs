/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvglueParam.cc,v 1.1.6.1 2007/09/27 09:02:46 paule Exp $                                                       

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
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "kvglue.h"

static kvsParamList* allocParamList(long       paramid, 
				    const char *name,
				    const char *unit,
				    long       level_scale);

static void freeParamList(kvsParamList *list);



kvsParamList*
kvsPopParamList(kvsParamList *list)
{
  kvsParamList *tmp;
  
  if(!list)
    return NULL;

  tmp=list->next;

  freeParamList(list);

  return tmp; 
}

void
kvsFreeParamList(kvsParamList *list)
{
  kvsParamList *tmp=list;
  
  while(tmp)
    tmp=kvsPopParamList(tmp);
    
}
                     
kvsParamList*
kvsGetParams()
{
  kvsParamList *head=allocParamList(10, "TA", "m", 0);

  assert(head);
  kvsParamList *tmp=allocParamList(23, "TAM", "m", 0);

  assert(tmp);

  head->next=tmp;
  
  tmp->next=allocParamList(34, "TAN", "m", 0);
  
  assert(tmp->next);
  
  tmp=tmp->next;
  
  
  tmp->next=allocParamList(38, "PP", "hPa", 0);
  
  assert(tmp->next);
  
  tmp=tmp->next;

  return head;

}


static kvsParamList* allocParamList(long       paramid, 
				    const char *name,
				    const char *unit,
				    long       level_scale)
{
  kvsParamList* parList=NULL;
  
  try{
    parList=new kvsParamList;
    parList->param=NULL;
    parList->param=new kvsParam;
    parList->param->name=NULL;
    parList->param->unit=NULL;
    parList->param->name=new char[strlen(name)+strlen(unit)+2];
    strcpy(parList->param->name, name);
    parList->param->unit=&parList->param->name[strlen(name)+1];
    strcpy(parList->param->unit, unit);
    parList->param->paramID=paramid;
    parList->param->level_scale=level_scale;
    parList->next=NULL;

    return parList;
  }
  catch(...){
    if(parList)
      freeParamList(parList);

    return NULL;
  }
}


static void freeParamList(kvsParamList *list)
{
  if(!list)
    return;
  
  delete[] list->param->name;
  delete list->param;
  delete list;
}
  
