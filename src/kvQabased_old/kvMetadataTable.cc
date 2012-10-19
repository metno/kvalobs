/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvMetadataTable.cc,v 1.1.2.3 2007/09/27 09:02:21 paule Exp $                                                       

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

#include "kvMetadataTable.h"
#include <puTools/miString.h>

#include <milog/milog.h>

using namespace miutil;
using namespace std;

bool kvMetadataTable::findEntry(const std::string& name,
				std::string& data)
{
  IDLOGDEBUG("html","kvMetadataTable::findEntry.  we are in var_:" << var_ << endl);
  if (value_.size() == 0) return false;

  // find part of name describing the parameter
  int i1= name.find_last_of('_'); // OBS: No '_' in metadata-name, please!
  miString param= name.substr(0,i1);
  IDLOGDEBUG("html","kvMetadataTable::findEntry. asks for param:" << param << endl);

  // is this the correct metadatatable?
  if (param != var_){
    // check if sensor, level, typeID unspecified
    if (param.contains("&")){
      // level, sensor and typeID specified
      IDLOGDEBUG("html","kvMetadataTable::findEntry. in wrong table:"
		 << var_ << endl);
      return false;
    } else {
      int i2=  var_.find_first_of('&');
      if (var_.substr(0,i2) != param){
	IDLOGDEBUG("html","kvMetadataTable::findEntry. in wrong table:" <<
		   var_.substr(0,i2) << endl);
	return false;
      }
    }
  }
  // get the parameter-independent part
  miString n= name.substr(i1+1,name.length()-i1-1);
  IDLOGDEBUG("html","kvMetadataTable::findEntry. asks for metadata:" << n << endl);
  miString a(n);
  vector<miString> vs1,vs2;
  list<kvMetadataTable::argus> vas;

  // any arguments to name
  if (n.contains("(") && n.contains(")")){
    a= n.substr(n.find_first_of("(")+1,n.length()-1);
    n= n.substr(0,n.find_first_of("("));
    IDLOGDEBUG("html","Found argument:" << a << endl);
    IDLOGDEBUG("html"," name is now:" << n << endl);

    if (a.contains("&"))
      vs1= a.split("&");
    else
      vs1.push_back(a);

    for ( size_t i=0; i<vs1.size(); i++){
      if (vs1[i].contains("=")){
	vs2= vs1[i].split("=");
	vas.push_back(argus(vs2[0],vs2[1]));

      } else {
	IDLOGERROR("html","Illegal argument in kvMetadataTable::findEntry:"
		   << name << endl);
	return false;
      }
    }
  }
  // arguments resolved..

  // check if main metadata-variable exists in table
  if (value_.count(n) == 0){
    IDLOGERROR("html", "kvMetadataTable::findEntry ERROR: entry:"
	       << n << " not found in metadata-table" << endl);
    return false;
  }
  // .. and has it data?
  if (value_[n].size() == 0){
    IDLOGERROR("html", "kvMetadataTable::findEntry ERROR: no data for entry:"
	       << n << " in metadata-table" << endl);
    return false;
  }

  // check arguments - find all legal rows based on arguments
  list<int> lrows, tmprows;
  lrows.push_back(0);
  list<kvMetadataTable::argus>::iterator vp= vas.begin();
  for (int i=0; vp!=vas.end(); i++,vp++){
    if (value_.count(vp->name) == 0){
      IDLOGERROR("html", "kvMetadataTable::findEntry ERROR: argument-type:"
		 << vp->name << " not found in metadata-table" << endl);
      return false;
    }
    // initially: all rows legal
    if (i == 0){
      lrows.clear();
      for (size_t j=0; j<value_[vp->name].size(); j++){
	lrows.push_back(j);
      }
    }
    list<int>::iterator ip= lrows.begin();
    for (; ip != lrows.end(); ip++){
      if (value_[vp->name][*ip] == vp->value){
	tmprows.push_back(*ip);
      }
    }
    lrows= tmprows;
    tmprows.clear();
  }
  // lrows contains all legal rows
  if (lrows.size() == 0){
    IDLOGERROR("html", "kvMetadataTable::findEntry ERROR:"
	       << " no rows matching arguments:"
	       << name << endl);
    return false;

  } else if (lrows.size() > 1){
    IDLOGWARN("html", "kvMetadataTable::findEntry WARNING:"
	      << " ambiguous data for name:"
	      << name << endl);
  }

  // pick the first legal row
  data = value_[n][*(lrows.begin())];

  return true;
}


bool kvMetadataTable::processString(const std::string& param,
				    const std::string& source,
				    std::list<kvMetadataTable>& lmt)
{
  kvMetadataTable kvm;

  kvm.var_ = param;
  //lmt.clear();

  miString s= source;
  vector<miString> vs,names,vs2;
  vs = s.split("\n");
  bool intable = false;
  size_t numc= 0;

  IDLOGDEBUG("html","kvMetadataTable::processString: Number of lines in source:"
	     << vs.size() << endl);

  for (size_t i=0; i<vs.size(); i++){
    miString t = vs[i];
    t.trim();

    if (t.length() == 0){
      // push previous table on stack
      if (intable) lmt.push_back(kvm);
      kvm.value_.clear();
      intable= false;

    } else if (!intable){
      IDLOGDEBUG("html","kvMetadataTable::processString: new table for var:"
		 << param << endl);
      names= t.split(";");
      numc = names.size();
      //       for (int j=0; j<numc; j++){
      // 	names[j] = param + "_" + names[j];
      // 	IDLOGINFO("html","   New var:" << names[j] << endl);
      //       }
      intable= true;

    } else {
      vs2= t.split(";");
      if (vs2.size() != numc){
	IDLOGERROR("html","kvMetadataTable::processString ERROR:"
		   << " too few columns in row for table:"
		   << param << endl);
	return false;
      }
      // fill values
      for (size_t j=0; j<numc; j++){
	miString value= vs2[j];
	std::string::size_type icomment;// remove comments from metadata! (Why...?!?)
	if ( ( icomment = value.find_first_of('#') ) != std::string::npos )
	  value= value.substr(0,icomment);
	value.trim();
	kvm.value_[names[j]].push_back(value);
      }
    }
  }

  // push previous table on stack
  if (intable) lmt.push_back(kvm);

  return true;
}
