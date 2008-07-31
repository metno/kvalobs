/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvDataListenerMain.java,v 1.1.2.3 2007/09/27 09:02:42 paule Exp $                                                       

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

import kvalobs.*;
import CKvalObs.CService.*;
import java.util.*;
import java.text.*;
import java.io.*;
import metno.util.MiGMTTime;
import metno.util.PropertiesHelper;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class KvDataListenerMain {
	static Logger logger = Logger.getLogger(KvDataListenerMain.class);
	
	static metno.util.PropertiesHelper loadConfFromFile() {
    	String confpath=System.getProperties().getProperty("KVJAVA_CONF");
    	
    	if(confpath==null){
    		logger.warn("Environment variable KVJAVA_CONF is unset, using $HOME/etc!");
    		confpath=System.getProperties().getProperty("user.home");
	    
    		if(confpath==null){
    			logger.fatal("Hmmmm. No 'user.home', exiting!");
    			System.exit(1);
    		}
    		
    		
        	if(confpath.charAt(confpath.length()-1)=='/'){
        		confpath=confpath.substring(0, confpath.length()-1);
        	}
        	
        	confpath = confpath + "/etc";
    	}
	   	
    	if(confpath.charAt(confpath.length()-1)=='/'){
    		confpath=confpath.substring(0, confpath.length()-1);
    	}

    	logger.info("Using <" + confpath + "> as KVJAVA_CONF!");

    	String confFile=confpath+"/kvdatalistener.conf";

    	PropertiesHelper conf=new PropertiesHelper();

    	try {
            conf.loadFromFile(confFile);
        } catch (FileNotFoundException e1) {
            logger.fatal("Cant open configuration file: "+confFile);
            logger.fatal("Reason: "+e1.getMessage());
            System.exit(1);
        } catch (IOException e1) {
            logger.fatal("Error while reading configuration file: "+confFile);
            logger.fatal("Reason: "+e1.getMessage());
            System.exit(1);
        }

        return conf;
    }

	
    public static void main(String[] args)
    {
    	String kvpath=System.getProperties().getProperty("KVJAVA_CONF");
    	
    	if(kvpath==null){
    		System.out.println("FATAL: Propertie KVJAVA_CONF must be set!");
    		System.exit(1);
    	}
    	
    	//Konfigurer loggesystemet, log4j.
    	System.out.println("log4j conf: "+kvpath+"/kvdatalistener_log.conf");
    	PropertyConfigurator.configure(kvpath+"/kvdatalistener_log.conf");
    	
    	metno.util.PropertiesHelper myConf = loadConfFromFile();
    	java.util.Properties corbaConf = null;
    	
    	String corbaNameserver = myConf.getProperty("corba.nameserver");
    	
    	if( corbaNameserver!=null && corbaNameserver.length()>0 ) {
    		corbaConf = new java.util.Properties();
    		corbaConf.setProperty("ORBInitRef.NameService", corbaNameserver );
    	}
    		
    	
    	Param[] param;
    	KvDataListenerApp app=new KvDataListenerApp(args, false, myConf, corbaConf);
    	WhichData[] whichData=new WhichData[1];
    	KvDataSubscribeInfo dataSubscribeInfo=new KvDataSubscribeInfo();
    	KvDataReceiver dataReceiver=new KvDataReceiver(app);
    	String subscriberid=app.getSubscriberid();
    	MiGMTTime now=new MiGMTTime();
    	
    	logger.info("Starting: " +now);
    	
    	if(subscriberid==null){
    		logger.info("No subscriberid, starting as a transient subscriber!");
	      	subscriberid=app.subscribeKvDataListener(dataSubscribeInfo,
    										         dataReceiver);  
    	}else{
    		logger.info("Subscriberid <"+subscriberid+">, starting as a persistent subscriber!");
    		subscriberid=app.registerKvDataListener(subscriberid,
    	  											dataReceiver);  
    	}

    	
      	if(subscriberid==null || subscriberid.length()==0){
      		logger.fatal("Cant subscribe on <KvData>! Exiting !!!!");
      		app.exit();
      	}
    	
    	logger.info("Subscribe on <KvData>, subscriberid <" 
    			           + subscriberid+"> Ctrl+c for aa avslutte!");
    	app.run();

    	logger.info("Prorgram terminate!");
    }
}


	
