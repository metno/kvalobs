/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataView.cc,v 1.8.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <mtcout.h>
#include <sstream>
#include <kvQtApp.h>
#include "kvDataView.h"

using namespace std;
using namespace kvservice;

KvDataView::KvDataView(QWidget *parent, const char *name):
  QTable(10, 4, parent, name)
{
  QHeader *h=horizontalHeader();

  h->setLabel(0, "ID");
  h->setLabel(1, "Station");
  h->setLabel(2, "ObsTime");
  h->setLabel(3, "Parameters");
}


void 
KvDataView::update(DataList &data)
{
  int max=data.size();
  std::ostringstream ost;
  IDataList it=data.begin();

  CERR("KvDataView::update:  #element=" << max << endl);

  if(max<=0)
    max=10;

  setNumRows(max);
  
  for(int i=0; i<max && it!=data.end(); i++, it++){
    ost.str("");
    ost << it->data().front().stationID();
    setText(i, 0, ost.str().c_str());
    setText(i, 1, it->stationName().c_str());
    setText(i, 2, it->data().front().obstime().isoTime().c_str());
    
    ost.str("");
    ost << it->data().size();
    setText(i, 3, ost.str().c_str());
  }
}

