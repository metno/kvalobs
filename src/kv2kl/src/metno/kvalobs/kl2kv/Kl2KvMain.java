/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Kl2KvMain.java,v 1.1.2.6 2007/09/27 09:02:19 paule Exp $                                                       

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

import kvalobs.*;
import CKvalObs.CService.*;
import java.util.*;
import java.io.File;
import java.text.*;
import java.util.Vector;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import metno.kvalobs.kl.KlApp;
import metno.kvalobs.kv2kl.Kv2KlMain;
import metno.util.GetOpt;
import metno.util.MiGMTTime;

public class Kl2KvMain {
    
	static Logger logger = Logger.getLogger(Kl2KvMain.class);
	
	public static void use(int exitcode){
		System.out.println(
		    "Bruk: kl2kv [-h] [-s kvserver] [-c configFile] -t typeidlist stationlist\n\n"+
		    "\t-h Skriv ut denne hjelpeteksten!\n"+
		    "\t-c configFile Angi en alternativ konfigurasjonsfil.\n"+
		    "\t   Default konfigurasjonsfil er: $KVALOBS/etc/kl2kv.conf\n"+
		    "\t-s kvserver Bruk kvserver som kvalobsserver i stedet for den\n"+
		    "\t   som er angitt i konfigurasjonsfilen!\n"+
		    "\t-t typeidlist Angi en liste av typeider som skal sendes over\n"+
		    "\t   til kvalobs. typeidlist er på formen typeid0,typeid1,..,typeidN\n"+
		    "\t   Det skal ikke være noen mellomrom i listen. Hvis det er \n"+
		    "\t   mellomrom må listen omsluttes av ' tegn, eks '302, 312' \n\n"+
		    "\tstationlist er en liste av stationid som det skal sendes data\n"+
		    "\t   for til kvalobs. Elementene i listen er adskilt med mellomrom\n"+
		    "\t   Man kan angi interval av stationid på formen 18500-18700. I \n"+
		    "\t   interval angivelsen kan det ikke forekomme mellomrom.\n\n"+
		    "  Eks. \n"+
		    "    Hvis man ønsker å sende data for typeid 302 for stasjonene\n"+
		    "    18500 til 18700 samt stasjonene 17555 og 17000 gir man \n"+
		    "    kommandoen:\n\n"+
		    "    kl2kv -t 302 18500-18700 17555 1700\n\n"
		);
		
        System.exit(exitcode);
    }
	
    public static void main(String[] args)
    {
    	//Set the default timezone to GMT.
    	TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
    	
    	String kvpath=KlApp.getKvpath();
    	String typeid=null;
    	boolean error=false;
    	
    	if(kvpath==null){
    		System.out.println("FATAL: Propertie KVALOBS must be set!");
    		System.exit(1);
    	}
    	    	
    	Kl2KvApp app;
    	SendData  sendData;          
    	String configfile="kl2kv.conf";
    	MiGMTTime start=new MiGMTTime();
    	
    	GetOpt go = new GetOpt("t:c:hs:");
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
             case 't':
            	 typeid=go.optarg();
            	 break;
             case 's':
            	 kvserver=go.optarg();
            	 break;
             default:
             	 logger.fatal("Unknown option character " + c);
                 use(1);
             }
         }
    	
    	 int i=configfile.lastIndexOf(".conf");
    	
    	 if(i==-1){
    		 System.out.println("FATAL: the name of the configuration file must end with '.conf' <" + configfile +">");
    		 System.exit(1);
    	 }
        
    	 String logfile=configfile.substring(0, i);
    	 i=logfile.lastIndexOf('/');
    	 
    	 if(i>-1){
    		 if(i<logfile.length()){
    			 logfile.substring(i+1);
    		 }else{
    			 System.out.println("FATAL: the name of the configuration file must on the form 'name.conf' <" + configfile +">");
        		 System.exit(1);
    		 }
    	 }
    	 
    	 logfile+="_log.conf";
    	 
    	//    	Konfigurer loggesystemet, log4j.
     	System.out.println("log4j conf: "+kvpath+"/etc/"+logfile);
     	PropertyConfigurator.configure(kvpath+"/etc/"+logfile);
    	 
    	 List stations=go.getFilenameList();
    	
         go=null; //We dont need it anymore.

         if(typeid==null)
        	 use(1);

     	 String[] typeidlist=typeid.split(",");
         
         app=new Kl2KvApp(args, configfile, kvserver, false);
    	 sendData=new SendData(app);          
    	 
    	 logger.info("Program started at: " + start);

    	 Station stationlist[]=Station.stations(stations);
    	 
    	 if(stationlist==null){
    		 logger.fatal("**** Feil i stasjonslisten ****\n\n");
    		 use(1);
    	 }
    	 
    	 for(String type : typeidlist){
    		 if(!sendData.sendDataToKv(type, stationlist)){
    			 logger.error("Failed to send data for typeid "+
    					       type+ " to kvalobs!");
    		 }else{
    			 logger.info("Data for typeid "+
			             type+ " sendt to kvalobs!");
    		 }
    	 }
    	 
    	 MiGMTTime stop=new MiGMTTime();
    	 
    	 long days, hours, min, secs;
    	 
    	 secs=start.secsTo(stop);
    	 logger.info("secs: "+secs);
    	 days=secs/86400;
    	 hours=(secs-days*86400)/3600;
    	 min=(secs-days*86400-hours*3600)/60;
    	 secs=secs-days*86400-hours*3600-min*60;
    	 
    	 logger.info("kvserver: "+app.getKvServer());
    	 logger.info("Program started at:     " + start);
    	 logger.info("Prorgram terminated at: " + stop);
    	 logger.info("Elapsed time:           "+ 
    	             (days>0?days+" day(s) ":"")+ hours+"h "+min+"m "+secs+"s" );
    	 logger.info("Errors: "+(error?"Yes":"No"));
    	 logger.info("# observations:  "+sendData.getObsCount());
    	 logger.info("# message to kv: "+sendData.getMsgCount());
    }
}


	
