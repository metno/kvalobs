/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvDataReceiver.java,v 1.1.2.3 2007/09/27 09:02:42 paule Exp $                                                       

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

import java.sql.*;
import java.io.*;
import kvalobs.*;
import metno.dbutil.*;
import CKvalObs.CService.*;
import metno.util.MiGMTTime;
import metno.util.StringHolder;
import org.apache.log4j.Logger;

public class KvDataReceiver implements KvDataEventListener {

	KvDataListenerApp app;
    DataInserter      dataInserter;
    
    static PrintWriter fout;
    static Logger logger=Logger.getLogger(KvDataReceiver.class);
    static Logger filterlog=Logger.getLogger("filter");
    
    public static void openfile(String filename) throws IOException{
    	fout = new PrintWriter(new FileWriter (filename,true), true);	
    }

    public KvDataReceiver(KvDataListenerApp app){
    	this.app=app;
        dataInserter=new DataInserter(app.getDataTablename(),
                                      app.getTextDataTablename());
    }

    public void updateElem( DataElem elem, DbConnection dbcon) {
   
    }
    
    public void kvDataEvent(KvDataEvent event) {
    	DbConnection dbconn=app.newDbConnection();
    	String query=null;
    	ObsData[] obsData=event.getObsData();
    	boolean loggedNewObs;
    	StringHolder msg=new StringHolder();
    	
    	logger.debug("DataReceiver (Enter): "+new MiGMTTime());

    	if(dbconn==null){
    		logger.error("No Db connection!"); // 17.01
    		return;
    	}
	
    	try{
    		if(obsData!=null){
    			for(int i=0; i<obsData.length; i++){
    				msg.setValue(null);
    				loggedNewObs=false;
    				DataElem[] elem=obsData[i].dataList;
    				TextDataElem[] textElem=obsData[i].textDataList;
                    
    				if(elem!=null){
    					for(int j=0; j<elem.length; j++){
    						
    						if(!loggedNewObs){
    							loggedNewObs=true;
    							logger.info("New obs: stationid: "+elem[j].stationID+
    										" typid: "+elem[j].typeID_+ 
    										" obstime: " + elem[j].obstime);
    						}
 
    						logger.info("sid: "+elem[j].stationID+
									    " tid: "+elem[j].typeID_+
									    " pid: "+elem[j].paramID+
									    " otime: " + elem[j].obstime+
									    " orig: " +elem[j].original+
									    " cor: " +elem[j].corrected +
									    " cinfo: "+elem[j].controlinfo+
									    " uinfo: "+elem[j].useinfo );
    						
    						query=dataInserter.createInsertQuery(elem[j], dbconn.getDbdriver());
    						logger.debug(query);

                            try{
                                dbconn.exec(query);
                            }
                            catch(SQLException SQLe){
                                logger.error(new MiGMTTime() +": "+ SQLe);
                                logger.error("SQL state: " + SQLe.getSQLState() );
                                
                                if( SQLe.getSQLState().startsWith("23") ) {
                                	updateElem( elem[j], dbconn );
                                }
                            
                                if(query!=null)
                                    fout.println(query);
                            }
                            catch(Exception e){
                                logger.error(new MiGMTTime() + ": " + e);
                            
                                if(query!=null)
                                    fout.println(query);
                            }
    					}
 
    				}else{
    					logger.warn("Opppsss: NO <data> PARAMS!");
    				}
                    
                    if(textElem!=null){
                        for(int j=0; j<textElem.length; j++){
                            if(!loggedNewObs){
                                loggedNewObs=true;
                                logger.info("New obs: stationid: "+textElem[j].stationID+
                                            " typid: "+textElem[j].typeID_+ 
                                            " obstime: " + textElem[j].obstime);
                            }
                             
                            query=dataInserter.createInsertQuery(textElem[j], dbconn.getDbdriver());
                            logger.debug(query);
                
                            try{
                                dbconn.exec(query);
                            }
                            catch(SQLException SQLe){
                                logger.error(new MiGMTTime() +": "+ SQLe);
                            
                                /*if(query!=null)
                                    fout.println(query);
                                */
                            }
                            catch(Exception e){
                                logger.error(new MiGMTTime() + ": " + e);
                            
                               /* if(query!=null)
                                    fout.println(query);
                               */
                            }
                        }
 
                    }
    			}
                
    		}else{
    			logger.warn("Opppsss: NO DATA!");
    		}

    	}
    	catch(Exception e){
    		logger.error(new MiGMTTime() + ": " + e);
        }
    	app.releaseDbConnection(dbconn);

    	logger.debug("DataReceiver (Return): "+new MiGMTTime());
    }
}


	
    
