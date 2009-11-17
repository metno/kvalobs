/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KlApp.java,v 1.1.2.5 2007/09/27 09:02:19 paule Exp $                                                       

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

import metno.util.MiGMTTime;
import metno.dbutil.*;
import metno.util.PropertiesHelper;
import kvalobs.*;
import java.sql.*;
import java.io.*;
import java.util.*;
import org.apache.log4j.Logger;
import org.apache.log4j.NDC;

public class KlApp extends KvApp
{
	static Logger logger=Logger.getLogger(KlApp.class);

    DbConnectionMgr conMgr=null;
    PropertiesHelper conf=null;
    String           kvserver;
    File             pidFile=null;
    static String           kvpath=null;
    
    String getConfile(String conf){
    	if(conf==null){
    		logger.fatal("INTERNAL: No configuration file is given! (null)!");
    		System.exit(1);
    	}
    	
    	if(conf.length()==0){
    		logger.fatal("INTERNAL: No configuration file is given!");
    		System.exit(1);
    	}
    
    	String path=KlApp.getKvpath();

    	String confFile=conf;
    	
    	File f=new File(confFile);
    	
    	if(!f.exists()){
    		int i=conf.lastIndexOf('/');
    		String fn;
    		
    		if(i>=0)
    			fn=conf.substring(i+1);
    		else
    			fn=conf;
    		
    		confFile=path+"/etc/"+fn;
    		
    		f=new File(confFile);
    		
    		if(!f.exists()){
    			logger.fatal("Cant find configurationfile: "+confFile);
    			System.exit(1);
    		}
    	}

    	return confFile;
    }
    
    public KlApp(String[] args, String conffile, boolean usingSwing){
    	this(args, conffile, null, usingSwing);
    }
    
    /**
     * 
     * @param args
     * @param conffile The name of the co
     * @param usingSwing
     */
    public KlApp(String[] args, String conffile, String kvserver, boolean usingSwing){
    	super(args, null, usingSwing, null);
    	String confFile=getConfile(conffile);
    	
    	conf=new PropertiesHelper();

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
        
        try {
            conMgr=new DbConnectionMgr(conf);
        } catch (IllegalArgumentException e1) {
            logger.fatal("Missing properties in the configuration file: " + e1.getMessage());
            System.exit(1);
        } catch (ClassNotFoundException e1) {
            logger.fatal("Cant load databasedriver: "+e1.getMessage());
            System.exit(1);
        }

        if(kvserver==null)
        	this.kvserver=conf.getProperty("kvserver");
        else
        	this.kvserver=kvserver;
        
        String myNameserver=null;
        String myNameserverport=null;
       
        String[] tmpArr = this.kvserver.split("@");
        
        if( tmpArr.length > 1) {
        	this.kvserver = tmpArr[0];
        	
        	if(tmpArr[1].length() > 0 ) {
        		tmpArr = tmpArr[1].split( ":" );
        		myNameserver = tmpArr[0];
        		myNameserver.trim();
        		
        		if( tmpArr.length > 1 && tmpArr[1].length() > 0 ) {
        			myNameserverport = tmpArr[1];
        			myNameserverport.trim();
        		}
        			
        		if( myNameserver.isEmpty() ) {
        			myNameserver = null;
        			myNameserverport = null;
        		}
        		
        		if( myNameserverport != null && myNameserverport.isEmpty() )
        			myNameserverport = null;
        	}
        }
        
        if( myNameserver != null ) 
        	this.setNameserver(myNameserver, myNameserverport );
        
        setKvServer(this.kvserver);

    	System.out.println("Database setup: ");
    	System.out.println("     dbuser: " + conMgr.getDbuser());
    	System.out.println("   dbdriver: "+conMgr.getDbdriver());
    	System.out.println("  dbconnect: "+conMgr.getDbconnect());
    	System.out.println("");
    	System.out.println("Using kvalobs server: ");
    	System.out.println("          kvserver: "+this.kvserver);

    	if( myNameserver != null)
    		System.out.println("  CORBA nameserver: "+ myNameserver + (myNameserverport!=null?(":"+myNameserverport):""));
    	
    }

    public void setDbIdleTime(int secs){
    	conMgr.setIdleTime(secs);
    }
    
    public void setDbTimeToLive(int secs){
    	conMgr.setTimeToLive(secs);
    }
    
    public int getDbIdleTime(){
    	return conMgr.getIdleTime();
    }
    
    public int getDbTimeToLive(){
    	return conMgr.getTimeToLive();
    }
    
    public int checkForConnectionsToClose(){
    	return conMgr.checkForConnectionsToClose();
    }
    

    /**
     * If we have a database connection, relese it. We are about to exit so
     * there should be now users of this connectein so we release it 
     * unconditional.
     */ 
    protected void onExit(){
        conMgr.closeDbDriver();
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
    
    public DbConnectionMgr getConnectionMgr() {
    	return conMgr;
    }
    	
    public PropertiesHelper getConf(){
    	return conf;
    }
    
    public String getKvserver(){
    	return kvserver;
    }
    

	synchronized public static String getKvpath(){

    	if(kvpath!=null)
    		return kvpath;
    	
    	kvpath=System.getProperties().getProperty("KVALOBS");
    	
    	if(kvpath==null){
    		System.out.println("Environment variable KVALOBS is unset, using HOME!");
    		kvpath=System.getProperties().getProperty("user.home");
	    
    		if(kvpath==null){
    			System.out.println("Hmmmm. No 'user.home', exiting!");
    			logger.fatal("Environment variable KVALOBS is unset, using HOME!");
    			System.exit(1);
    		}
    	}
	
    	if(kvpath.charAt(kvpath.length()-1)=='/'){
    		kvpath=kvpath.substring(0, kvpath.length()-1);
    	}

    	System.out.println("Using <" + kvpath + "> as KVALOBS path!");

    	return kvpath;
    }
	
	
	/**
	 * Create a pid file. The filepath and pid must be set as property
	 * in the start script of the application.
	 * 
	 * The path property is PIDFILE and the pid property is USEPID.
	 * 
	 * The property is set with the -D switch on the java commandline.
	 * 
	 * If the pidfile exist or an error occure in the creation of the
	 * pidfile the application terminate.
	 */
	synchronized public void createPidFile(){
		String pid=null;
		String pidFilename=null;
		
    	if(pidFile!=null)
    		return;
    	
    	pidFilename=System.getProperties().getProperty("PIDFILE");
    	
    	if(pidFilename==null){
    		System.out.println("FATAL: Property variable PIDFILE is unset!");
    		logger.fatal("FATAL: Property variable PIDFILE is unset!");
    		System.exit(1);
    	}
	
    	pid=System.getProperties().getProperty("USEPID");
    	
    	if( pid == null ) {
    		System.out.println("FATAL: Property variable USEPID is unset!");
    		logger.fatal("FATAL: Property variable USEPID is unset!");
    		System.exit(1);
    	}

    	pidFile = new File( pidFilename );
    	
    	try {
    		if( ! pidFile.createNewFile() ) {
    			logger.fatal("FATAL: The pidfile '"+pidFilename+"' allready exist!" +
    					     "If an instance of the application is not running"+
    					     " remove the file and try again.");
    			System.exit(1);
    		}
    		
    		FileWriter fw = new FileWriter( pidFile, true );
    		fw.write( pid );
    		fw.close();
    		System.out.println("Writing pidfile '" + pidFilename + "' with pid '"+pid+"'!");
    		logger.info("Writing pidfile '" + pidFilename + "' with pid '"+pid+"'!");
    	}
    	catch( java.io.IOException ex ) {
    		logger.fatal("FATAL: " + ex.getMessage() );
    		System.exit(1);
    	}
    	catch( java.lang.SecurityException ex ) {
    		logger.fatal("FATAL: " + ex.getMessage() );
    		System.exit(1);
    	}
    }

	/**
	 * Remove a previous created pidfile.
	 * 
	 * @see createPidFile
	 */
	synchronized public void removePidFile() {
		try {
			if( pidFile != null ) {
	    			System.out.println("Removing pidfile '" + pidFile.getName() + "!");	
				logger.info("Removing pidfile '" + pidFile.getName() + "!");
				pidFile.delete();
			}
		}
		catch( java.lang.SecurityException ex ) {
			System.out.println("SecurityException: Removing pidfile '" + pidFile.getName() + "!");	
			//NOOP
		}
	}
}
