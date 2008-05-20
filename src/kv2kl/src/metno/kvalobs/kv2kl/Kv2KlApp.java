/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Kv2KlApp.java,v 1.1.2.11 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kv2kl;

import metno.util.MiGMTTime;
import metno.dbutil.*;
import metno.kvalobs.kl.KlApp;
import metno.util.PropertiesHelper;
import metno.util.FileUtil;
import kvalobs.*;
import java.sql.*;
import java.io.*;
import java.util.*;
import org.apache.log4j.Logger;
import org.apache.log4j.NDC;

public class Kv2KlApp extends KlApp
{
	static Logger logger=Logger.getLogger(Kv2KlApp.class);

    //DbConnectionMgr conMgr=null;
    Timer        dbCleanupTimer=null;
    Timer        isUpTimer=null;
    boolean     kvIsUp;
    MiGMTTime    kvLastContact=null;
    String       kvIsUpLogFile=null;
    boolean     isUpTaskIsRunning=false;
        
    class DbCleanup extends TimerTask{
    	Kv2KlApp app;
	
    	public DbCleanup(Kv2KlApp app_){
    		app=app_;
    	}

    	public void run(){
    		int n;
            NDC.push("DbCleanup");
    		logger.debug("DbCleanup: running db connection cleanup!");
    		n=app.checkForConnectionsToClose();
            
            if(n>0)
                logger.info("Idle connections closed: "+n);
            
            NDC.pop();
    	}
    }

    class IsUp extends TimerTask{
    	Kv2KlApp app;
    	String   filename;
	
    	public IsUp(Kv2KlApp app_, String filename){
    		app=app_;
    		this.filename=filename;
    	}

    	public void run(){
    		int n;
            NDC.push("IsUp");
    		logger.debug("Updating the file '"+filename+"'!");
    		MiGMTTime now=new MiGMTTime();
    		MiGMTTime lastContact=app.getLastKvContact();
    		boolean isUp=app.getKvIsUp();
    		
    		String buf=null;
    
    		app.setIsUpTimeIsRunning(true);	
			
    		
    		if(!isUp){
    			
    			if(lastContact==null) //In startup.
    				buf="Startup: "+now;
    			else
    				buf="Down: "+lastContact;
    			
    			logger.debug(buf);
    		}else{
    			long s=now.secsTo(lastContact);
    			
    			if(Math.abs(s)<3600){
    				buf="Up: "+lastContact;
    				logger.debug(buf);
    			}else{
    				logger.warn("Last contact from kvalobs: "+s+ " s since!");
    			}
    		}
    		
    		if(buf!=null){
    			if(!FileUtil.writeStr2File(filename, buf)){
    				logger.warn("Cant update the file '"+filename+"'!");
    			}
    		}
    		
    		app.setIsUpTimeIsRunning(false);
    		
    		NDC.pop();
    	}
    }

    
    public Kv2KlApp(String[] args, String configfile, String kvserver, 
    			    boolean usingSwing){
    	super(args, configfile, kvserver, false);

    	PropertiesHelper prop=getConf();
        String admin=prop.getProperty("admin");
        setDbIdleTime(300); //5 minutter
        setDbTimeToLive(3600); //1 time
        
        if(admin!=null)
        	admin=admin.trim();
        
        //Run the timer as a deamon, ie terminate the timer
        //when the application is about to terminate.
        dbCleanupTimer=new Timer(true);
        dbCleanupTimer.schedule(new DbCleanup(this), 60000, 60000);
        
        
        //Try to register an Admin interface in the CORBA name server.
        //The interface is registred in the same context as the kavlobs
        //server we are set up to receice data from. If we succeed an
        //timerthread is started that update a logfile every 15th seconds
        //with status information.
        if(admin!=null && admin.length()>0){
        	kvIsUpLogFile=getKvpath()+"/var/log/kv2kl_"+admin+".log";
        
        	String kvServer=getKvserver();
        	
        	if(kvServer!=null){
        		String nsname=kvServer+"/"+admin;
        		
        		Admin myAdmin=new Admin();
        		
        		if(registerAdmin(nsname, myAdmin)){
        			logger.info("Registred a 'Admin' interface in CORBA name service: '"+nsname+"'.");
        			isUpTimer=new Timer(true);
        	
        			//Run every 15th second.
        			isUpTimer.schedule(new IsUp(this, kvIsUpLogFile), 0, 15000);
        		}else{
        			logger.warn("Cant registred a 'Admin' interface in CORBA name service: '"+nsname+"'.");
        		}
        	}
        }
    }

    synchronized void setIsUpTimeIsRunning(boolean f){
    	isUpTaskIsRunning=f;
    }
    
    synchronized boolean getIsUpTimeIsRunning(){
    	return isUpTaskIsRunning;
    }
    
    synchronized public void setKvIsUp(boolean isUp){
    	kvLastContact=new MiGMTTime();
    	kvIsUp=isUp;
    }
    
    synchronized public boolean getKvIsUp(){
    	return kvIsUp;
    }
    
    synchronized public void updateLastKvContact(){
    	kvIsUp=true;
    	kvLastContact=new MiGMTTime();
    }
    
    synchronized public MiGMTTime getLastKvContact(){
    	return kvLastContact;
    }
    
    protected void onExit(){
    	if(isUpTimer!=null){
    		isUpTimer.cancel();
    	
    		while(getIsUpTimeIsRunning());
    	
    		String buf="Stopped: "+new MiGMTTime();
    	
    		if(!FileUtil.writeStr2File(kvIsUpLogFile, buf)){
    			logger.warn("Cant update the file '"+kvIsUpLogFile+"'!");
    		}
    	}
    	removePidFile();
    	logger.info("Prorgram terminate!");
    }
}

