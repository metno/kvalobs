/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Kv2KlMain.java,v 1.1.2.10 2007/09/27 09:02:19 paule Exp $                                                       

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

import kvalobs.*;
import CKvalObs.CService.*;
import java.util.*;
import java.text.*;
import metno.kvalobs.kl.KlApp;
import metno.util.GetOpt;
import metno.util.MiGMTTime;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class Kv2KlMain {
	static Logger logger = Logger.getLogger(Kv2KlMain.class);
	
	public static void use(int exitcode){
		System.out.println("Usage: kv2kl [-s kvserver] [-h] [-c configFile]");
        System.exit(exitcode);
    }

	
    public static void main(String[] args)
    {
    	//Set the default timezone to GMT.
    	TimeZone.setDefault(TimeZone.getTimeZone("GMT"));

    	String kvpath=KlApp.getKvpath();
    	String configfile="kv2kl.conf";
    	Param[] param;
    	Kv2KlApp app;
    	WhichData[] whichData=new WhichData[1];
    	KvDataSubscribeInfo dataSubscribeInfo;
    	KlDataReceiver dataReceiver;
    	KvHintListener hint;
    	
    	if(kvpath==null){
    		System.out.println("FATAL: Propertie KVALOBS must be set!");
    		System.exit(1);
    	}
    	    	
    	String subscriberid;
    	String hintid;
    	MiGMTTime now=new MiGMTTime();
    	GetOpt go = new GetOpt("c:hs:");
    	String kvserver=null;

    	char c;
         
    	while ((c = go.getopt(args)) != GetOpt.DONE) {
             switch(c) {
             case 'h':
                 use(0);
                 break;
             case 'c':
                 configfile= go.optarg();
                 break;
             case 's':
            	 kvserver=go.optarg();
            	 break;
             default:
                 System.err.println("Unknown option character " + c);
             	 logger.fatal("Unknown option character " + c);
                 use(1);
             }
         }

    	System.out.println("Configfile (in): " + configfile );
    	 
    	 int i=configfile.lastIndexOf(".conf");
    	 String kvname;
     	
    	 if(i==-1){
    		 System.out.println("FATAL: the name of the configuration file must end with '.conf' <" + configfile +">");
    		 System.exit(1);
    	 }
        
    	 kvname=configfile.substring(0, i);
    	 i=kvname.lastIndexOf('/');
    	 
    	 if(i>-1){
    		 if(i<kvname.length()){
    			 kvname = kvname.substring(i+1);
    		 }else{
    			 System.out.println("FATAL: the name of the configuration file must on the form 'name.conf' <" + configfile +">");
        		 System.exit(1);
    		 }
    	 }
    	 
    	 
    	 String logfile=kvname+"_log.conf";
    	 
    	//    	Konfigurer loggesystemet, log4j.
     	System.out.println("log4j conf: "+kvpath+"/etc/"+logfile);
     	PropertyConfigurator.configure(kvpath+"/etc/"+logfile);
    	
         go=null; //We dont need it anymore.

         app=new Kv2KlApp(args, configfile, kvserver, false);
         dataSubscribeInfo=new KvDataSubscribeInfo();
         dataReceiver=new KlDataReceiver( app, kvname+".dat" ); 
         hint=new KvHintListener(app);
         
    	logger.info("Starting: " +now);
	
   		now.addHour(-1);
	    
   		whichData[0]=new WhichData(0, StatusId.All,
   				                   now.toString(),
   								   "");	
	    
   		if(!app.getKvData(whichData, dataReceiver)){
   			logger.fatal("getKvData: failed. Exiting !!!!");
   			app.exit();
   			return;
   		}
	
	  
    	logger.info("getKvData: a background thread is started!");

    	subscriberid=app.subscribeKvDataListener(dataSubscribeInfo,
    										     dataReceiver);
    	
	
    	if(subscriberid==null){
    		logger.fatal("Cant subscribe on <KvData>! Exiting !!!!");
    		app.exit();
    	}
    	
    	hintid=app.subscribeKvHintListener(hint);
    	
    	if(hintid==null){
    		logger.fatal("Cant subscribe on <KvHint>! Exiting !!!!");
    		app.exit();
    	}
    	
    	app.setKvIsUp(true);
    	
    	logger.info("Subscribe on <KvData>, subscriberid <" 
    			           + subscriberid+"> Ctrl+c for aa avslutte!");
    	app.run();
    	
    	app.onExit();

    	logger.info("Prorgram terminate!");
    }
}


	
