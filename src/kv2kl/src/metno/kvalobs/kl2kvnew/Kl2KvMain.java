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
package metno.kvalobs.kl2kvnew;

import kvalobs.*;
import CKvalObs.CService.*;
import java.util.*;
import java.io.File;
import java.text.*;
import java.util.Vector;
import java.util.regex.*;

import javax.print.attribute.standard.MediaSize.Other;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

import metno.kvalobs.kl.*;
import metno.kvalobs.kv2kl.Kv2KlMain;
import metno.util.GetOpt;
import metno.util.GetOptDesc;
import metno.util.MiGMTTime;

public class Kl2KvMain {
    
	static Logger logger = Logger.getLogger(Kl2KvMain.class);
	
	public static void use(int exitcode){
		System.out.println(
		    "Bruk: kl2kv [-h] [-s kvserver] [-c configFile] [-l obstimelist] -t typeidlist stationlist\n\n"+
		    "\t-h Skriv ut denne hjelpeteksten!\n"+
		    "\t-c configFile Angi en alternativ konfigurasjonsfil.\n"+
		    "\t   Default konfigurasjonsfil er: $KVALOBS/etc/kl2kv.conf\n"+
		    "\t-s kvserver Bruk kvserver som kvalobsserver i stedet for den\n"+
		    "\t   som er angitt i konfigurasjonsfilen!\n"+
		    "\t-l \n" +
		    "\t--time-list Kommaseparert liste over obstime som skal brukes. \n" +
		    "\t   Format: fromtime-totime, hvor formatet på fromtime og totime er:\n"+
		    "\t     YYYY-MM-DDThh[:mm:ss] \n"+
		    "\t   to time kan utelates og blir da satt lik fromtime.\n"+
		    "\t \n"+
		    "\t   Eks 1. --time-list '2009-10-1 - 2009-11-1', dette gir alt av data for\n" +
		    "\t        oktober. Legg merke til at apostrof må brukes dersom listen innhoilder\n" +
		    "\t        mellomrom.\n"+
		    "\t   Eks 2. --time-list '2009-10-1T00,2009-10-1T02,2009-10-1T05', dette gir data for\n" +
		    "\t        obstidene 00, 02 og 05 for 1. oktober.\n" +
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
		    "    kl2kv -t 302 18500-8700 17555 1700\n\n"
		);
		
        System.exit(exitcode);
    }
	
	public static List<TimeRange> decodeOptObstime( String optObstime ) 
	{
		if( optObstime == null )
			return null;
		
		
		List<TimeRange> obstimelist = new LinkedList<TimeRange>();
		
		String[] obstimeList = optObstime.split(",");
		
		//Pattern date = Pattern.compile(" *(\\d{4})-(\\d{1,2})-(\\d{1,2})(((:?T)\\:\\d{1,2}){0,3})? *");
		Pattern date = Pattern.compile(" *(\\d{4})-(\\d{1,2})-(\\d{1,2}) *");
		
		for( String obstime : obstimeList ){
			Matcher match = date.matcher( obstime );
			
		}
		return obstimelist;
	}
	
	
    public static void main(String[] args)
    {
    	//Set the default timezone to GMT.
    	TimeZone.setDefault(TimeZone.getTimeZone("GMT"));
    	
    	String kvpath=KlApp.getKvpath();
    	String typeid=null;
    	String optTimeList=null;
    	List<TimeRange> obstimes=null;
    	boolean error=false;
    	
    	GetOptDesc options[] = {
    	            new GetOptDesc('t', "typeid", true),
    	            new GetOptDesc('c', "conf-file", true),
    	            new GetOptDesc('h', "help", false),
    	            new GetOptDesc('s', "kvalobs-server", true),
    	            new GetOptDesc('l', "time-list", true)
    	        };
    	    	
    	if(kvpath==null){
    		System.out.println("FATAL: Propertie KVALOBS must be set!");
    		System.exit(1);
    	}
    	    	
    	Kl2KvApp app;
    	SendData  sendData;          
    	String configfile="kl2kv.conf";
    	MiGMTTime start=new MiGMTTime();
    	
    	GetOpt go = new GetOpt( options );
    	String kvserver=null;
    	
    	
    	Map<String,String> optArgs = go.parseArguments( args );
    	Set<String> optKeys=optArgs.keySet();
    	
         
    	for( String opt : optKeys ) {
             switch( opt.charAt( 0 ) ) {
             case 'h':
                 use(0);
                 break;
             case 'c':
                 configfile= optArgs.get( opt );
                 break;
             case 't':
            	 typeid=optArgs.get( opt );
            	 break;
             case 'l':
            	 optTimeList=optArgs.get( opt );
            	 break;
             case 's':
            	 kvserver=go.optarg();
            	 break;
             default:
             	 logger.fatal("Unknown option character " + opt);
                 use(1);
             }
         }
    	
    	 
    	 if( optTimeList != null ) {
    		 obstimes = TimeDecoder.decodeTimeList( optTimeList );
    		 
    		 if( obstimes == null ) {
    			 System.out.println("Invalid time-list: '"+optTimeList+"'");
    			 System.exit(1);
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
    	 
    	 List<String> stations=go.getFilenameList();
    	
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
    		 if(!sendData.sendDataToKv(type, stationlist, obstimes)){
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


	
