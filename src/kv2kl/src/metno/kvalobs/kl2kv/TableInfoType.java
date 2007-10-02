/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: TableInfoType.java,v 1.1.2.2 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kl2kv;

import java.sql.ResultSet;

public abstract  class TableInfoType{
    final static int MAX_COUNT=20;
    String table;
    String typeid;
    String params;

    public TableInfoType(String table_, String typeid_, String params_){
    	table=table_;
    	typeid=typeid_;
    	params=params_;
    }

    public static TableInfoType createInfoTypeFactory(int typeid){
		switch(typeid){
		case 302:  return new TableInfoType302();
		case 402:  return new TableInfoType402();
		case 404:  return new TableInfoType404();
		case 412:  return new TableInfoType412();
		case 433:  return new TableInfoType433();
		}

		return null;
    }

    public boolean isOk(){
    	return table!=null;
    }

    public String getTable(){
    	return table;
    }
    
    public String getTypeid(){
    	return typeid;
    }

    public String getParams(){
    	return params;
    }

    /**
     * Create a sql query for the selected infoType
     */
    public abstract String createQuery(String whereAfterTypeid);

    /**
     * Creat a string on the format expected by kldecoderen i kvalobs based
     * on the typeInfo.
     */
    public abstract boolean convertToKvDataAndSend(java.sql.ResultSet rs, 
					                                  DataToKv kv);
}
