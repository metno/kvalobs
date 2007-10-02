/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Main.java,v 1.1.2.10 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kv2klgetdata;

import java.util.LinkedList;
import java.util.List;
import java.util.ListIterator;
import metno.kvalobs.kl.KlApp;
import kvalobs.*;
import CKvalObs.CService.*;
import metno.util.*;
import metno.kvalobs.kl.SqlInsertHelper;
import metno.util.MiGMTTime;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class Main {
	static Logger logger=Logger.getLogger(Main.class);
	
	static void use(int no){
        System.out.println("\n\n  kv2klgetdata -f fromdate [-s kvserver] [-c conffile] [-t todate] \n");
        System.out.println("         [-i typeidlist]  [-df] stnr1 stnr2 .. stnrN");
        System.out.println("\n   kv2klgetdata henter data fra kvalobs og fyller opp i klimadatabasen.");
        System.out.println("   Dataene filtreres med filterdata fra T_KV2KLIMA_FILTER,   t_kv2klima_param_filter");
        System.out.println("   og t_kv2klima_typeid_param_filter .");
        System.out.println("");
        System.out.println("   -nf Deaktiver filteret, dvs si at filteret ikke skal brukes!");
        System.out.println("   -dd Data skal ikke lagres til databasen!");
        System.out.println("   -c conffile Angi en alternativ konfigurationfil");
        System.out.println("               Default konfigurasjonsfil er: $KVALOBS/etc/kv2kl.conf");
        System.out.println("               Dvs. samme file som kv2kl bruker!");
        System.out.println("   -s kvserver Bruk kvserver som kvalobsserver istedet for den som");
        System.out.println("               er angitt i konfigurasjonsfilen!");
        System.out.println("   -f fromdate Hent data fra og med denne observasjonsterminen.");
        System.out.println("   -t todate hent data til og med denne observasjonsterminen.");
        System.out.println("             Hvis -t ikke angis hentes data frem til nåtid.");
        System.out.println("   -i typeid En liste av typeid som skal hentes.");
        System.out.println("             Hvis -i ikke angis lagres data for alle typeid.");
        System.out.println("   stnr1 stnr2 .. stnrN er stasjonene man ønsker å hente data");
        System.out.println("   for. Hvis man ikke angir noen stasjoner hentes data for alle");
        System.out.println("   stasjonene.");
        System.out.println("\n  Datoformatet til fromdate og todate er:");
        System.out.println("  'YYYY-MM-DD hh:mm:ss'."); 
        System.out.println("  Fnuttene rundt må være med!");
        System.out.println("\n  Foramatet til typeid er: ");
        System.out.println("  'typeid1 typeid2 .. typeidN'");
        System.out.println("  Fnuttene må være med hvis det angis mer enn en typeid");
        System.out.println("\n\n");
        
        System.exit(no);
        
	}
    
    static String datecheck(String date){
        if(date==null)
            return "";
        
        MiGMTTime t=new MiGMTTime();
        
        if(!t.parse(date))
            return null;
        
        return t.toString();
    }
    
    static LinkedList createtTypelist(String typelist){
        if(typelist==null)
            return null;
        
        String[] res=typelist.split(" |,");
        
        LinkedList ll=new LinkedList();
        
        for(int i=0; i<res.length; i++){
            try {
                ll.add(new Integer(Integer.parseInt(res[i])));
            } catch (NumberFormatException e) {
                //Just ignore exceptions
            }
        }
        
        return ll;
        
    }
	
    public static void main(String[] args)
    {	
    	String conffile="kv2kl.conf";
    	String kvpath=KlApp.getKvpath();
	
    	if(kvpath==null){
    		System.out.println("FATAL: Propertie KVALOBS must be set!");
    		System.exit(1);
    	}
	    	
    	Param[] param;
    	SqlInsertHelper insertstmt;
    	App app;
    	String kvserver=null;
    	
    	GetOptDesc myopt[]={ 	
    			new GetOptDesc('h', null, false),
    			new GetOptDesc('t', null, true),
    			new GetOptDesc('f', null, true),
    			new GetOptDesc('i', null, true),
    			new GetOptDesc('c', null, true),
    			new GetOptDesc('s', null, true),
    			new GetOptDesc('n', "nf", false),
    			new GetOptDesc('d', "dd", false)
    	};
    	
    	GetOpt opt=new GetOpt(myopt);
    	char  c;
    	boolean disableFilter=false;
    	boolean saveData=true;
    	String toDate=null;
    	String fromDate=null;
    	String typeid=null;
        List<String> list=null;
        LinkedList<Integer> stationList=null;
        LinkedList typelist=null;
        WhichData[] whichData=null;
        int        nObservations=0;
        int        nObsMsg=0;
        int        nIterations=0;
        
    	while ((c=opt.getopt(args)) != GetOpt.DONE) {
    		switch(c) {
             case 'h':
                 use(0);
                 break;
             case 't':
            	 toDate = opt.optarg();
                 break;
             case 'f':
                 fromDate=opt.optarg();
                 break;
             case 'i':
                 typeid=opt.optarg();
                 break;
             case 'c':
            	 conffile=opt.optarg();
            	 break;
             case 's':
            	 kvserver=opt.optarg();
            	 break;
             case 'n':
            	 disableFilter=true;
            	 break;
             case 'd':
            	 saveData=false;
            	 break;
             default:
                 logger.error("Unknown option character " + c);
                 use(1);
            }
        }

    	int ii=conffile.lastIndexOf(".conf");
     	
    	if(ii==-1){
    		System.out.println("FATAL: the name of the configuration file must end with '.conf' <" + conffile +">");
    		System.exit(1);
    	}
        
    	String logfile=conffile.substring(0, ii);
    	ii=logfile.lastIndexOf('/');
    	 
    	if(ii>-1){
    		if(ii<logfile.length()){
    			logfile.substring(ii+1);
    		}else{
    			System.out.println("FATAL: the name of the configuration file must on the form 'name.conf' <" + conffile +">");
        		System.exit(1);
    		}
    	}
    	 
    	logfile+="_log.conf";
    	 
    	//    	Konfigurer loggesystemet, log4j.
     	System.out.println("log4j conf: "+kvpath+"/etc/"+logfile);
     	PropertyConfigurator.configure(kvpath+"/etc/"+logfile);
    	 
    	 
    	list=(List<String>)opt.getFilenameList();
    	 
    	//We dont need opt and myopt anymore.
    	opt=null;
    	myopt=null;
    	 
    	app=new App(args, conffile, kvserver);
    	 
        toDate=datecheck(toDate);
        fromDate=datecheck(fromDate);
         
        if(fromDate==null){
        	logger.error("ERROR: Ugyldig fromdate!\n");
            use(1);
        }
         
        if(toDate==null ){
        	logger.error("ERROR: Ugyldig todate!\n");
            use(1);
        }
         
        if(fromDate.length()==0){
        	logger.error("ERROR: fromdate mÃ¥ angis!\n");
        	use(1);
        }
         
        stationList=new LinkedList<Integer>();
         
        for (String s  : list) {
        	try {
        		stationList.add(Integer.parseInt(s));
            } catch (NumberFormatException e) {
                e.printStackTrace();
            }
        }

        typelist=createtTypelist(typeid);
         
        logger.info("Options: ");
        logger.info("toDate: " + toDate );
        logger.info("fromDate: " + fromDate );
        logger.info("Stations: "+stationList);
        logger.info("typeid:   "+typelist);
        logger.info("kvserver: "+kvserver);
        logger.info("disable filter: "+disableFilter);
         
        if(stationList.size()==0){
        	whichData=new WhichData[1];
            whichData[0]=new WhichData(0, StatusId.All,
            						   fromDate, 
            						   toDate);
        }else{
            whichData=new WhichData[stationList.size()];
             
            int k=0;
            for(Integer i : stationList)
                whichData[k++]=new WhichData(i, StatusId.All,
                                             fromDate, toDate);
             
             
            for(int i=0; i<whichData.length; i++)
            	logger.info("WhichData["+i+"] ["+
                         whichData[i].stationid+","+
                         whichData[i].fromObsTime+","+
                         whichData[i].toObsTime+"]");
       }
    	

       if(disableFilter)
    	   insertstmt=new SqlInsertHelper(app, null, false);
       else
    	   insertstmt=new SqlInsertHelper(app);
         
       MiGMTTime start=new MiGMTTime();	
         
       KvDataIterator it=app.getKvData(whichData);

       if(it==null){
    	   logger.fatal("Cant connect to the kvalobs server: "+app.getKvServer());
           System.exit(1);
       }
	
       logger.info("Connected to kvalobs server: "+app.getKvServer());
	
       try {
    	   ObsData[] datalist=it.next();
			
    	   if(datalist!=null){
    		   nObsMsg+=datalist.length;
			
    		   for(ObsData od : datalist)
    			   nObservations+=od.dataList.length+od.textDataList.length;
			
    		   while(datalist!=null){
    			   nIterations++;
				
    			   if(saveData)
    				   insertstmt.insertData(datalist,typelist);
				
    			   datalist=it.next();	
    		   }
    	   }
		} catch (KvNoConnection e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
		finally{
			it.destroy();
		}
	
    	app.exit();
    	
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
   	 	logger.info("# iterations:   "+nIterations);	
   	 	logger.info("# obsMsg:       "+nObsMsg);
   	 	logger.info("# observations: "+nObservations);
    }
}


	
