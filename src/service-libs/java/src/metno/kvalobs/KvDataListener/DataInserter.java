/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataInserter.java,v 1.1.2.2 2007/09/27 09:02:42 paule Exp $                                                       

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
package metno.kvalobs.KvDataListener;

import org.apache.log4j.Logger;
import CKvalObs.CService.*;


public class DataInserter implements IDataInserter {

    String dataTablename;
    String textDataTablename;
    
    static Logger logger=Logger.getLogger(DataInserter.class);
    
    public DataInserter(String dataTablename, String textDataTablename){
        this.dataTablename=dataTablename;
        this.textDataTablename=textDataTablename;
    }
    
    private String createOracleInsertQuery(DataElem elem){
        logger.debug("Insert into a Oracle database!");
        String query="insert into "+dataTablename+" values ("
                     +elem.stationID+","
                     +"to_date('"+elem.obstime+"','yyyy-mm-dd hh24:mi:ss'),"+
                     +elem.original+","
                     +elem.paramID+","
                     +"to_date('"+elem.tbtime+"','yyyy-mm-dd hh24:mi:ss'),"+
                     +elem.typeID_+","
                     +elem.sensor  +","
                     +elem.level  +","
                     +elem.corrected+","
                     +"'" +elem.controlinfo+"',"
                     +"'"+elem.useinfo +"',"
                     +"'"+elem.cfailed+"')";
        return query;
    }

    private String createOracleInsertQuery(TextDataElem elem){
        logger.debug("Insert into a Oracle database!");
        String query="insert into "+textDataTablename+" values ("
                     +elem.stationID+","
                     +"to_date('"+elem.obstime+"','yyyy-mm-dd hh24:mi:ss'),"
                     +"'" +elem.original+"',"
                     +elem.paramID+","
                     +"to_date('"+elem.tbtime+"','yyyy-mm-dd hh24:mi:ss'),"+
                     +elem.typeID_+")";
        return query;
    }

    

    private String createPgInsertQuery(DataElem elem){
        logger.debug("Insert into a Pg database!");
        String query="insert into " +dataTablename+" values ("
                     +elem.stationID+","
                     +"'"+elem.obstime+"',"
                     +elem.original+","
                     +elem.paramID+","
                     +"'"+elem.tbtime+"',"
                     +elem.typeID_+","
                     +elem.sensor  +","
                     +elem.level  +","
                     +elem.corrected+","
                     +"'" +elem.controlinfo+"',"
                     +"'"+elem.useinfo +"',"
                     +"'"+elem.cfailed+"')";
        return query;
    }

    private String createPgInsertQuery(TextDataElem elem){
        logger.debug("Insert into a Pg database!");
        String query="insert into " +textDataTablename+" values ("
                     +elem.stationID+","
                     +"'"+elem.obstime+"',"
                     +"'"+elem.original+"',"
                     +elem.paramID+","
                     +"'"+elem.tbtime+"',"
                     +elem.typeID_+")";
        return query;
    }


    public String createInsertQuery(DataElem data, String dbdriver){
        if(dbdriver.indexOf("oracle")>-1)
            return createOracleInsertQuery(data);
        else
            return createPgInsertQuery(data);
    }
    
    public String createInsertQuery(TextDataElem data, String dbdriver){
        if(dbdriver.indexOf("oracle")>-1)
            return createOracleInsertQuery(data);
        else
            return createPgInsertQuery(data);
    }

}
