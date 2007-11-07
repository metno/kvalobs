/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SqlInsertHelper.java,v 1.1.2.7 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kl;

import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.sql.SQLException;
import java.util.LinkedList;
import java.util.ListIterator;

import CKvalObs.CService.DataElem;
import CKvalObs.CService.ObsData;
import metno.dbutil.DbConnection;
import metno.util.MiGMTTime;
import metno.util.StringHolder;

import org.apache.log4j.Logger;

public class SqlInsertHelper {
	static Logger logger=Logger.getLogger(SqlInsertHelper.class);
	static Logger filterlog=Logger.getLogger("filter");
	PrintWriter fout;
	KlApp       klApp;
	boolean    enableFilter;
	
	
	public SqlInsertHelper(KlApp app, String backupfile, boolean enableFilter){
		fout=null;
	   	klApp=app;
	   	this.enableFilter=enableFilter;

	   	if(backupfile!=null){
	   		try {
	   			fout = new PrintWriter(new FileWriter (backupfile,true), true);
	   			fout.println("Starter her:");
	   		}
	   		catch(IOException e){
	   			logger.error(e);
	   		}
	   	}
	 }
	 
	 public SqlInsertHelper(KlApp app, String backupfile){
		 this(app, backupfile, true);
	 }

	 public SqlInsertHelper(KlApp app){
		 this(app, null, true);
	 }
	
	protected String createOracleInsertQuery(DataElem elem){
	   	logger.debug("Insert into a Oracle database!");
	   	String query="insert into kv2klima(stnr,dato,original,kvstamp,paramid,typeid,xlevel,sensor,useinfo,corrected,controlinfo,cfailed) values ("
	   				 +elem.stationID+",to_date('"
	   				 +elem.obstime+"','yyyy-mm-dd hh24:mi:ss'),"
	   				 +elem.original+",to_date('"
	   				 +elem.tbtime+"','yyyy-mm-dd hh24:mi:ss'),"
	   				 +elem.paramID+","
	  				 +elem.typeID_+","
	   				 +elem.level  +","
	   				 +elem.sensor  +",'"
	   				 +elem.useinfo +"',"
	   				 +elem.corrected+",'"
	   				 +elem.controlinfo+"','"
	   				 +elem.cfailed+"')";
	   	return query;
	}

	
	

	protected String createPgInsertQuery(DataElem elem){
	  	logger.debug("Insert into a Pg database!");
	   	String query="insert into kv2klima(stnr,dato,original,kvstamp,paramid,typeid,xlevel,sensor,useinfo,corrected,controlinfo,cfailed) values ("
	   				 +elem.stationID+","
	   				 +"'"+elem.obstime+"',"
	   				 +elem.original+","
	   				 +"'"+elem.tbtime+"',"
	   				 +elem.paramID+","
	   				 +elem.typeID_+","
	   				 +elem.level  +","
	   				 +elem.sensor  +",'"
	   				 +elem.useinfo +"',"
	   				 +elem.corrected+",'"
	   				 +elem.controlinfo+"','"
	   				 +elem.cfailed+"')";
	   	return query;
   }

	protected String createOracleUpdateQuery(DataElem elem){
	   	logger.debug("Update a Oracle database!");
	   	String query="UPDATE kv2klima "+
	   				 "SET " +
	   				 "  original="+elem.original + "," +
	   				 "  kvstamp=to_date('"+elem.tbtime+"','yyyy-mm-dd hh24:mi:ss')," +
	   				 "  useinfo='"+elem.useinfo + "',"+
	   				 "  corrected="+elem.corrected + "," +
	   				 "  controlinfo='"+elem.controlinfo+"'," +
	   				 "  cfailed='"+elem.cfailed+"' " +
	   				 "WHERE " +
	   				 "  stnr=" + elem.stationID + " AND " +
	   				 "  dato=to_date('" + elem.obstime + "','yyyy-mm-dd hh24:mi:ss') AND " +
	   				 "  paramid=" + elem.paramID + " AND " +
	   				 "  typeid=" + elem.typeID_ + " AND " +
	   				 "  xlevel=" + elem.level + " AND " +
	   				 "  sensor=" + elem.sensor;
	   	return query;
	}

	protected String createPgUpdateQuery (DataElem elem){
	   	logger.debug("Update a Pg database!");
	   	String query="UPDATE kv2klima "+
	   				 "SET " +
	   				 "  original="+elem.original + "," +
	   				 "  kvstamp='"+elem.tbtime+"'" +
	   				 "  useinfo='"+elem.useinfo + "',"+
	   				 "  corrected="+elem.corrected + "," +
	   				 "  controlinfo='"+elem.controlinfo+"'," +
	   				 "  cfailed='"+elem.cfailed+"' " +
	   				 "WHERE " +
	   				 "  stnr=" + elem.stationID + " AND " +
	   				 "  dato='" + elem.obstime + "' AND " +
	   				 "  paramid=" + elem.paramID + " AND " +
	   				 "  typeid=" + elem.typeID_ + " AND " +
	   				 "  xlevel=" + elem.level + " AND " +
	   				 "  sensor=" + elem.sensor;
	   	return query;
	}
	
	
	
	protected String createInsertQuery(DataElem data, String dbdriver){
    	if(dbdriver.indexOf("oracle")>-1)
    		return createOracleInsertQuery(data);
    	else
    		return createPgInsertQuery(data);
	}

	protected String createUpdateQuery(DataElem data, String dbdriver){
   		if(dbdriver.indexOf("oracle")>-1)
   			return createOracleUpdateQuery(data);
   		else
   			return createPgUpdateQuery(data);
	}

	protected boolean usetypeid(DataElem elem, LinkedList typelist){
		if( typelist == null )
			return true;
		
		if(typelist!=null){
            ListIterator it=typelist.listIterator();
            
            while(it.hasNext()){
                Integer n=(Integer)it.next();
            
                if(elem.typeID_==n.shortValue()){
                    return true;
                }
            }
        }
		
		return false;
	}
	
	
	protected boolean doInsertData(DbConnection dbcon, DataElem elem){
			String query=createInsertQuery(elem, dbcon.getDbdriver());
			
			logger.debug(query);

			try{
				dbcon.exec(query);
			}
			catch(SQLException SQLe){
				String sqlState=SQLe.getSQLState();
						
				if(sqlState!=null && 
				   sqlState.startsWith("23")){
					logger.warn(new MiGMTTime() +": "+ SQLe);
					String updateQuery=createUpdateQuery(elem, dbcon.getDbdriver());
					
					return updateData(dbcon, elem);
				}else{
					logger.error(new MiGMTTime() +": "+ SQLe);
				}
			}
			catch(Exception e){
	   			logger.error(new MiGMTTime() + ": " + e);
			}
			
			if(query!=null && fout!=null)
				fout.println(query);
				
			return false;
	}
	
	protected boolean updateData(DbConnection dbcon, DataElem elem){
		String query=null;

		try{
			query=createUpdateQuery(elem, dbcon.getDbdriver());
			
			dbcon.exec(query);
			return true;
		}catch(SQLException SQLe){
			logger.error(new MiGMTTime() +": "+ SQLe + "\n" + query);
		}
		catch(Exception e){
   			logger.error(new MiGMTTime() + ": " + e);
		}
		
		if(query!=null && fout!=null)
			fout.println(query);
		
		return false;
	}

   	public boolean insertData(ObsData[] obsData, LinkedList typelist){
   		DbConnection dbconn=klApp.newDbConnection();
   		Filter filter=new Filter(dbconn); 
   		boolean loggedFilter;
   		boolean loggedNewObs;
        int     typeid=0;
   		StringHolder msg=new StringHolder();
   		boolean  filterRet;
   		boolean  ret=true;

   		if(!enableFilter)
   			filter.setFilterEnabled(false);
   		
   		logger.debug("InsertData (Enter): "+new MiGMTTime());

   		if(dbconn==null){
   			logger.error("No Db connection!"); 
   			return false;
   		}	

   		try{
   			if(obsData==null){
   				logger.warn("Opppsss: NO DATA!");
   				return true;
   			}
   		
   			for(int i=0; i<obsData.length; i++){
   				msg.setValue(null);
   				loggedFilter=false;
   				loggedNewObs=false;
                    
   				DataElem[] elem=obsData[i].dataList;
	    
   				if(elem==null){
   					logger.warn("Opppsss: NO PARAMS!");
   					continue;
   				}
   			
   				for(int j=0; j<elem.length; j++){
   					if(typeid!=elem[j].typeID_){
   						typeid=elem[j].typeID_;
                        loggedNewObs=false;
   					}
   							
   					if(!usetypeid(elem[j], typelist))
   						continue;
                            
   					if(!loggedNewObs){
   						loggedNewObs=true;
   						logger.info("New obs: stationid: "+elem[j].stationID+
   									" typid: "+elem[j].typeID_+ 
   									" obstime: " + elem[j].obstime);
   					}

   					filterRet=filter.filter(elem[j], msg);
					
					if(!loggedFilter && msg.getValue()!=null){
						loggedFilter=true;
   						filterlog.info(elem[j].stationID + 
   								       " " + 
   								       elem[j].typeID_ +
							           " " +
								       elem[j].obstime + ": " +
								       msg.getValue());
   					}	
					
   					if(!filterRet)
   						continue;

   					if(!doInsertData(dbconn, elem[j]))
   						ret=false;
   				}
   			}
   		}
   		catch(Exception e){
   			logger.error(new MiGMTTime() + ": " + e);
   		}

   		klApp.releaseDbConnection(dbconn);

   		logger.debug("DataReceiver (Return): "+new MiGMTTime());
   		return ret;
   	}
   
   
}
