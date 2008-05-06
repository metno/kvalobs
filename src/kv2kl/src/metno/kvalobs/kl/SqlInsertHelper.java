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
import CKvalObs.CService.TextDataElem;
import CKvalObs.CService.ObsData;
import metno.dbutil.DbConnectionMgr;
import metno.dbutil.DbConnection;
import metno.util.MiGMTTime;
import metno.util.StringHolder;
import metno.util.IntHolder;

import org.apache.log4j.Logger;

public class SqlInsertHelper {
	static Logger logger=Logger.getLogger(SqlInsertHelper.class);
	static Logger filterlog=Logger.getLogger("filter");
	PrintWriter fout;
	DbConnectionMgr conMgr=null;
	boolean    enableFilter;
	
		
	public SqlInsertHelper(DbConnectionMgr conMgr, String backupfile, boolean enableFilter){
		fout=null;
	   	this.conMgr=conMgr;
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

	
	 public SqlInsertHelper(DbConnectionMgr conMgr, String backupfile){
		 this(conMgr, backupfile, true);
	 }

	 public SqlInsertHelper(DbConnectionMgr conMgr){
		 this(conMgr, null, true);
	 }
	
	 public DbConnection newDbConnection(){
		 try {
			 return conMgr.newDbConnection();
		 } catch (SQLException e) {
			 logger.warn("Cant create a new database connection: "+
					     e.getMessage());
			 return null;
		 }
	 }
	 
	 
	 public void  releaseDbConnection(DbConnection con){
		 String msg;
	        
		 try {
			 conMgr.releaseDbConnection(con);
			 return;
		 } catch (IllegalArgumentException e) {
			 msg=e.getMessage();
		 } catch (IllegalStateException e) {
			 msg=e.getMessage();
		 } catch (SQLException e) {
			 msg=e.getMessage();
		 }

		 logger.warn("Cant release the database connection: "+msg);
	 }
	

	
	protected boolean usetypeid(int  typeid, LinkedList typelist){
		if( typelist == null )
			return true;
		
		if(typelist!=null){
            ListIterator it=typelist.listIterator();
            
            while(it.hasNext()){
                Integer n=(Integer)it.next();
            
                if(typeid==n.shortValue()){
                    return true;
                }
            }
        }
		
		return false;
	}
	

	
	protected boolean doInsertData(DbConnection dbcon, String insertQuery, String updateQuery){
			String query=insertQuery;
			
			if( query == null )
				return true;
			
			logger.debug(query);

			try{
				dbcon.exec(query);
				return true;
			}
			catch(SQLException SQLe){
				String sqlState=SQLe.getSQLState();
						
				if(sqlState!=null && 
				   sqlState.startsWith("23")){
					logger.warn(new MiGMTTime() +": "+ SQLe);
					query = updateQuery;
					return updateData( dbcon, query );
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
	
	protected boolean updateData(DbConnection dbcon, String updateQuery ){
		String query=updateQuery;

		if( query == null )
			return true;
		
		try{
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
   		DbConnection dbconn=newDbConnection();
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
   			
   			DataHelper dh = new DataHelper( dbconn.getDbdriver() );
   			
   			for(int i=0; i<obsData.length; i++){
   				msg.setValue(null);
   				loggedFilter=false;
   				loggedNewObs=false;
                    
   				DataElem[] elem=obsData[i].dataList;
   				dh.init( obsData[i].dataList, obsData[i].textDataList );
   				IntHolder stationID = new IntHolder();
   				
   			
   				while( dh.next() ){
   					if(typeid!=dh.getTypeID() ){
   						typeid=dh.getTypeID();
                        loggedNewObs=false;
   					}
   							
   					if(!usetypeid(dh.getTypeID(), typelist))
   						continue;
                            
   					if(!loggedNewObs){
   						loggedNewObs=true;
   						logger.info("New obs: stationid: "+dh.getStationID()+
   									" typid: "+dh.getTypeID()+ 
   									" obstime: " + dh.getObstime() );
   					}

   					stationID.setValue( dh.getStationID() );

   					filterRet=filter.filter(stationID, dh.getTypeID(), dh.getParamID(), 
   							                dh.getLevel(), dh.getSensor(), dh.useLevelAndSensor(),
   							                dh.getObstime(), msg );
   					
   					//The stationid my have changed
   					dh.setStationID( stationID.getValue() );
   					
					
					if(!loggedFilter && msg.getValue()!=null){
						loggedFilter=true;
   						filterlog.info(dh.getStationID() + 
   								       " " + 
   								       dh.getTypeID() +
							           " " +
								       dh.getObstime() + ": " +
								       msg.getValue());
   					}	
					
   					if(!filterRet)
   						continue;

   					if(!doInsertData( dbconn, dh.createInsertQuery(), dh.createUpdateQuery() ) )
   						ret=false;
   				}
   			}
   		}
   		catch(Exception e){
   			logger.error(new MiGMTTime() + ": " + e);
   		}

   		releaseDbConnection(dbconn);

   		logger.debug("DataReceiver (Return): "+new MiGMTTime());
   		return ret;
   	}
}
