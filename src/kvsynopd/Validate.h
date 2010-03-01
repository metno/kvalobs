/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ValidData.h,v 1.2.2.3 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __validdata_h__
#define __validdata_h__

#include "Data.h"

namespace kvdatacheck{

class Validate {

	int  flag2int(char c);
	bool check_fr(kvalobs::kvControlInfo &f,  int paramid);
	bool check_fcc(kvalobs::kvControlInfo &f, int paramid);
	bool check_fcp(kvalobs::kvControlInfo &f, int paramid);
	bool check_fs(kvalobs::kvControlInfo &f,  int paramid);
	bool check_fnum(kvalobs::kvControlInfo &f,int paramid);
	bool check_fpos(kvalobs::kvControlInfo &f,int paramid);
	bool check_fmis(kvalobs::kvControlInfo &f,int paramid);
	bool check_useinfo1(kvalobs::kvUseInfo  &f,int paramid);

	bool (Validate::*validate) ( const Data &data );

	bool validDataUseOnlyControlInfo( const Data &data );
	bool validDataUseOnlyUseInfo( const Data &data );
	bool validDataCombineControlAndUseInfo( const Data &data );
	bool validDataNoCheck( const Data &data );

public:
	enum HowToValidate { UseOnlyControlInfo, UseOnlyUseInfo, CombineControlAndUseInfo, NoCheck };
	Validate();
	Validate( HowToValidate howToValidate );

	bool operator()( const Data &data );

};

}


#endif
