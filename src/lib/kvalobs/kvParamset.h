/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvParamset.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvParamset_h
#define _kvParamset_h 

#include <vector>
#include <map>

/* Dette er ingen databaseconnection- men et bibliotek */


namespace kvalobs {

  /**
   * \addtogroup kvinternalhelpers
   * @{
   */

  /**
   * \brief a kvParamset
   *
   * \todo Noen som f�ler seg kallet til � dokumentere dette.
   */
    class kvParamset {

     private:
        std::map< int, std::vector<int> > m_int_paramset;
	std::map< int, std::vector< miutil::std::string> > m_str_paramset;
	std::map< int, std::vector<int> > m_int_param;

	void add_inverse( int paramsetid, std::vector<int> vi );
 
      public:
        kvParamset();
	
	/**
	 * \brief returnerer en vector av parametere som dette
	 * paramstetet inneholder
	 * 
	 * \note get_param og  get_param_str er egentlig samme funksjone, 
	 * de returnerer bare verdien som forskjellig datatype - henholdsvis 
	 * som en vector<int> og en vector<string>
	 * get_param og get_paramset er omvendte funksjoner
	 */
	std::vector<int> get_param( int paramsetid );
	   
	/**
	 * \brief  returnerer en vector av parametere som dette
	 *  paramsetet inneholder.
	 * 
	 * \note get_param og  get_param_str er egentlig samme funksjone, 
	 * de returnerer bare verdien som forskjellig datatype - henholdsvis 
	 * som en vector<int> og en vector<string>
	 *  get_param og get_paramset er omvendte funksjoner
	 */
	std::vector< miutil::std::string> get_param_str( int paramsetid );
	
	/**
	 * \brief returnerer en vector av paramset som inneholder mengden
	 *  av de paramsetene som parameteren paramid er medlem av.
	 */
	std::vector<int> get_paramset( int paramid );


   };
    /** @} */

}


#endif



