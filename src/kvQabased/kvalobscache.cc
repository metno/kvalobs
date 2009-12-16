/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobscache.cc,v 1.1.2.5 2007/09/27 09:02:21 paule Exp $                                                       

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
#include "kvalobscache.h"
#include "kvQABaseDBConnection.h"
#include <sstream>
#include <list>
#include <kvalobs/kvDbGate.h>
#include <milog/milog.h>

using namespace std;
using namespace kvalobs;


KvalobsCache::KvalobsCache( kvQABaseDBConnection & dbcon )
    : dbcon_( dbcon )
{}


KvalobsCache::~KvalobsCache()
{
  try
  {
    flush();
  }
  catch( std::exception & e )
  {
    IDLOGERROR( "html", e.what() );
  }
}

void KvalobsCache::insert( const kvalobs::kvData & data )
{
  std::pair<Cache::iterator, bool> result = cache_.insert( data );
  if ( not result.second )
  {
//		cache_.erase( result.first -- );
//		cache_.insert( result.first, data );
    cache_.erase( result.first );
    cache_.insert( data );
  }
}

void KvalobsCache::flush()
{
  vector<kvData> toSave;
  toSave.reserve( cache_.size() );
	
  for ( Cache::iterator it = cache_.begin(); it != cache_.end(); ++ it ) {
  	kvData d = * it;
  	kvUseInfo ui = d.useinfo();
  	ui.setUseFlags( d.controlinfo() );
  	d.useinfo( ui );
	toSave.push_back( d );
  }
  
  if ( ! dbcon_.setObservation( toSave.begin(), toSave.end() ) )
    throw std::runtime_error( "Unable to save result in database!" );
    
  cache_.clear();
}

