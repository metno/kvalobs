/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ParamView.cc,v 1.1.6.1 2007/09/27 09:02:47 paule Exp $                                                       

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
#include <sstream>
#include <qbutton.h>
#include <qtable.h>
#include "ParamView.h"

using namespace std;


ParamView::
ParamView(QWidget *parrent, 
	  std::string &name, 
	  kvservice::KvDataList &data_,
	  GUITestApp &app_)
  :QVBox(parrent, name.c_str()), data(data_), app(app_)
{
  QButton   *closeButton;
  QTable    *table;
  kvservice::IKvDataList it;
  int       max=data.size();
  ostringstream ost;
 
  table=new QTable(10, 6, this);

  QHeader *h=table->horizontalHeader();
  h->setLabel(0, "ID");
  h->setLabel(1, "Original");
  h->setLabel(2, "Corrected");
  h->setLabel(3, "Type");
  h->setLabel(4, "UseInfo");
  h->setLabel(5, "CtlInfo");

  it=data.begin();

  CERR("ParamView:  #element=" << max << endl);

  if(max<=0)
    max=10;

  table->setNumRows(max);
  
  for(int i=0; i<max && it!=data.end(); i++, it++){
    table->setText(i, 0, app.param(it->stationID()));
    ost.str("");
    ost << it->original();
    table->setText(i, 1, ost.str().c_str());
    ost.str("");
    ost << it->corrected();
    table->setText(i, 2, ost.str().c_str());
    ost.str("");
    ost << it->typeID();
    table->setText(i, 3, ost.str().c_str());
    table->setText(i, 4, it->useinfo().flagstring().c_str());
    table->setText(i, 5, it->controlinfo().flagstring().c_str());
  }


  closeButton= new QButton(this, "close");

  connect(closeButton, SIGNAL(clicked()),this, SLOT(close()));
}

void
ParamView:: 
close()
{
  emit exit();
}
