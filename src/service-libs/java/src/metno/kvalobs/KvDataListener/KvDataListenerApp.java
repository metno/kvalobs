/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvDataListenerApp.java,v 1.1.2.4 2007/09/27 09:02:42 paule Exp $                                                       

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

import metno.util.MiGMTTime;
import metno.dbutil.*;
import metno.util.PropertiesHelper;
import kvalobs.*;
import java.sql.*;
import java.io.*;
import java.util.*;
import org.apache.log4j.Logger;
import org.apache.log4j.NDC;

public class KvDataListenerApp extends KvApp
{
	static Logger logger=Logger.getLogger(KvDataListenerApp.class);

    DbConnectionMgr conMgr=null;
    Timer        dbCleanupTimer;
    MiGMTTime    lastAccess;
    String       dataTablename;
    String       textDataTableName;
    String       subscriberid=null;
    
    class DbCleanup extends TimerTask{
        KvDataListenerApp app;
	
    	public DbCleanup(KvDataListenerApp app_){
    		app=app_;
    	}

    	public void run(){
    		int n;
            NDC.push("DbCleanup");
    		logger.debug("DbCleanup: running db connection cleanup!");
            n=app.conMgr.checkForConnectionsToClose();
            
            if(n>0)
                logger.info("Idle connections closed: "+n);
    	}
    }

    public KvDataListenerApp(String[] args, boolean usingSwing){
    	super(args, null, usingSwing);
    	
    	for(int i=0; i<args.length; i++){
    		if(args[i].compareTo("--subid")==0){
    			i++;
    			
    			if(i<args.length){
    				subscriberid=args[i];
    			}
    			
    			break;
    		}
    	}
   	
    	String kvpath=System.getProperties().getProperty("KVALOBS");
	
    	if(kvpath==null){
    		logger.warn("Environment variable KVALOBS is unset, using HOME!");
    		kvpath=System.getProperties().getProperty("user.home");
	    
    		if(kvpath==null){
    			logger.fatal("Hmmmm. No 'user.home', exiting!");
    			System.exit(1);
    		}
    	}
	   	
    	if(kvpath.charAt(kvpath.length()-1)=='/'){
    		kvpath=kvpath.substring(0, kvpath.length()-1);
    	}

    	logger.info("Using <" + kvpath + "> as KVALOBS path!");

    	String confFile=kvpath+"/etc/kvdatalistener.conf";

    	PropertiesHelper conf=new PropertiesHelper();

    	try {
            conf.loadFromFile(confFile);
        } catch (FileNotFoundException e1) {
            logger.fatal("Cant open configuration file: "+confFile);
            logger.fatal("Reason: "+e1.getMessage());// TODO Auto-generated catch block
            System.exit(1);
        } catch (IOException e1) {
            logger.fatal("Error while reading configuration file: "+confFile);
            logger.fatal("Reason: "+e1.getMessage());// TODO Auto-generated catch block
            System.exit(1);
        }
        
        try {
            conMgr=new DbConnectionMgr(conf);
        } catch (IllegalArgumentException e1) {
            logger.fatal("Missing properties in the configuration file: " + e1.getMessage());
            System.exit(1);
        } catch (ClassNotFoundException e1) {
            logger.fatal("Cant load databasedriver: "+e1.getMessage());
            System.exit(1);
        }

        if(subscriberid==null){
        	subscriberid=conf.getProperty("subscriberid");
        	
        	if(subscriberid!=null && subscriberid.length()==0)
        		subscriberid=null;
        }
        
        conMgr.setIdleTime(300); //5 minutter
        conMgr.setTimeToLive(3600); //1 time
        
        setKvServer(conf.getProperty("kvserver"));

    	System.out.println("Database setup: ");
    	System.out.println("     dbuser: " + conMgr.getDbuser());
    	System.out.println("   dbdriver: "+conMgr.getDbdriver());
    	System.out.println("  dbconnect: "+conMgr.getDbconnect());
    	System.out.println("");
    	System.out.println("Receiving data from kvalobs server:");
    	System.out.println("   kvserver: "+conf.getProperty("kvserver"));
    	System.out.println("   subscriberid: "+(subscriberid==null?"(N/A)":subscriberid));
    	
    	//Run as a deamon, ie terminate when the application 
        //terminate.
        dbCleanupTimer=new Timer(true); 
        dbCleanupTimer.schedule(new DbCleanup(this), 60000, 60000);
        lastAccess=new MiGMTTime();
        
        dataTablename=conf.getProperty("data.tablename", "data");
        textDataTableName=conf.getProperty("text_data.tablename", "text_data");
    }


    /**
     * If we have a database connection, relese it. We are about to exit so
     * there should be now users of this connectein so we release it 
     * unconditional.
     */ 
    protected void onExit(){
        conMgr.closeDbDriver();
    }
    
    public String getSubscriberid(){ return subscriberid;}
    public String getDataTablename(){ return dataTablename; }
    public String getTextDataTablename(){ return textDataTableName;}
    
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
    
    synchronized void updateLastAccess(){
        lastAccess =new MiGMTTime();
    }
    
    synchronized MiGMTTime getLastAccess(){
        return lastAccess;
    }
}
