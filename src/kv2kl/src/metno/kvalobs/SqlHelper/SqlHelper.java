/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SqlHelper.java,v 1.1.2.2 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.SqlHelper;

import java.util.*;
import java.text.*;
import java.sql.*;
import java.io.*;
import metno.util.MiGMTTime;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class SqlHelper {
	String dbconnect;
    String dbdriver;
    String dblogin;
    String dbpasswd;

    Object dbMutex=new Object();
    DbConnection dbconn=null;
	static Logger logger = Logger.getLogger(SqlHelper.class);
	
    public static void main(String[] args)
    {
    	String kvpath=System.getProperties().getProperty("KVALOBS");
    	System.out.println("log4j conf: "+kvpath+"/etc/SqlHelper_log.conf");
    	PropertyConfigurator.configure(kvpath+"/etc/kv2kl_log.conf");
    	
    	SqlHelper sqlHelper=new SqlHelper();
    	
    	DbConnection con=sqlHelper.newDbConnection();
    	
    	if(con==null){
    		logger.fatal("Cant open a connection to the database!");
    		System.exit(1);
    	}
    	
    	String sql="SELECT * FROM t_kv2klima_filter";
    	
    	logger.debug("sql: " + sql);

    	ResultSet rs=null;
    	
    	try{
    		rs=con.statement.executeQuery(sql);
    		ResultSetMetaData rsMetaData=rs.getMetaData();
    		String data;
    		boolean first;
    		
    		while(rs.next()){
    		
    			first=true;
    			
    			for(int i=1; i<=rsMetaData.getColumnCount(); i++){
    				data=rs.getString(i);
    				
    				if(first){
    					first=false;
    				}else{
    					System.out.print(",");
    				}
    				if(data!=null)
    					System.out.print(data);
    				else
    					System.out.print("\\N");
    				
    			}
    			
    			System.out.println();
    		}
    	}
    	catch(SQLException ex){
    		logger.error(new MiGMTTime() + "SQLEXCEPTION:  " +ex);
    	}

    	if(rs!=null){
    		try{
    			rs.close();
    		}
    		catch(SQLException ex){
    			logger.error("SQLEXCEPTION: When closing the ResultSet!\n"+ex);
    		}
    	}
    	
    	sqlHelper.releaseDbConnection(con);
    	
    }
	
    
    public SqlHelper(){
    	FileInputStream in=null;
    	Properties defconf=new Properties();

    	//Set some default values
    	defconf.setProperty("dbconnect","jdbc:oracle:thin:@euros:1521:dvh");
    	defconf.setProperty("dbdriver", "dbdriver=oracle.jdbc.driver.OracleDrive");
    	defconf.setProperty("dblobin",  "kvalobs");
    	defconf.setProperty("dbpasswd", "obs2005");
    	defconf.setProperty("kvserver", "kvalobs");
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

    	String confFile=kvpath+"/etc/SqlHelper.conf";

    	Properties conf=new Properties(defconf);

    	try{
    		in=new FileInputStream(confFile);
    	}
    	catch(FileNotFoundException ex){
    		logger.error("ERROR: Cant open conf file <"+confFile+">!");
    	}
    	catch(SecurityException ex){
    		logger.error("NOACCESS: We have not permission to read to open the file <"+confFile+"!");
    	}

    	if(in!=null){
    		try{
    			conf.load(in);
    		}
    		catch(Exception ex){
    			logger.fatal("ERROR: Cant load configuration from file <"
    						       +confFile+">!");
		
    			try{
    				in.close();
    			}
    			catch(Exception ex_){
    			}
		
    			System.exit(1);
    		}

    		try{
    			in.close();
    		}
    		catch(Exception ex){
    		}
    	}
	    
    	dblogin =conf.getProperty("dblogin");
    	dbpasswd=conf.getProperty("dbpasswd");
    	dbdriver=conf.getProperty("dbdriver");
    	dbconnect=conf.getProperty("dbconnect");

    	System.out.println("Database setup: ");
    	System.out.println("    dblogin: " + dblogin);
    	System.out.println("   dbpasswd: "+dbpasswd);
    	System.out.println("   dbdriver: "+dbdriver);
    	System.out.println("  dbconnect: "+dbconnect);
    	System.out.println("");

    	try {
    		Class.forName(dbdriver);
    	} catch(Exception e) {
    		logger.fatal("FATAL: cant load the database driver <"+
    						   dbdriver+">!");
    		System.exit(1);
    	}
	
    	logger.info("INFO: Database driver <"+dbdriver+"> loaded!");
    }
    
    public DbConnection newDbConnection(){
    	Connection conn;
    	Statement statement;
    	boolean error=false;

    	synchronized(dbMutex){

    		if(dbconn!=null){
    			if(dbconn.inuse){
    				logger.error("newDbConnection (ERROR): Mismatch " +
    								   "newDbConnection/releaseDbConnection");
    				return null;
    			}

    			dbconn.lastAccess=new MiGMTTime();
    			dbconn.inuse=true;
    			return dbconn;
    		}

    		logger.info("newDbConnection: Creating a new connection to"+
    					" the database!");
	
    		try{
    			conn = DriverManager.getConnection(dbconnect, dblogin, dbpasswd);  
    		}
    		catch(SQLException ex){
    			logger.error(ex);
    			logger.error("DBERROR: cant create a database"+
    					 	 "connection <"+dbconnect+">!");
    			return null;
    		}
    		catch(Exception ex){
    			logger.error(ex);
    			logger.error("DBERROR: (UNEXPECTED??) cant create the data base connection <"+
    							   dbconnect+">!");
    			return null;
    		}
	    
    		if(conn==null){
    			logger.error("DBERROR: (UNEXPECTED???) connection==null!");
    			return null;
    		}
	    
    		try{
    			statement=conn.createStatement();
    		}
    		catch(SQLException ex){
    			logger.error(ex);
    			logger.error("DBERROR: cant create the data base statement!");
		
    			try{
    				conn.close();
    			}
    			catch(Exception ex_){
    			}
    			return null;
    		}
    		catch(Exception ex){
    			logger.error(ex);
    			logger.error("DBERROR: (UNEXPECTED??) cant create the data base statement!");
		
    			try{
    				conn.close();
    			}
    			catch(Exception ex_){
    			}
    			return null;
    		}
	    
    		dbconn=new DbConnection(conn, statement, dbdriver);
    		dbconn.inuse=true;
    		dbconn.lastAccess=new MiGMTTime();
    		return dbconn;
    	}
    }
 
    private void realReleaseDbConnection(DbConnection con){

    	logger.debug("realReleaseDbConnection: called!");
	
    	try{
    		con.statement.close();
    	}
    	catch(Exception ex){
    	}

    	try{
    		con.conn.close();
    	}
    	catch(Exception ex){
    	}
    }

    public void  releaseDbConnection(DbConnection con){

    	synchronized(dbMutex){
    		if(con==dbconn){
    			if(!dbconn.inuse){
    				logger.error("releaseDbConnection (ERROR): Mismatch "
    						    +"newDbConnection/releaseDbConnection");
    				return;
    			}
		 
    			dbconn.inuse=false;
    			dbconn.lastAccess=new MiGMTTime();
    		}else{
    			logger.error("releaseDbConnection (ERROR): The " +
    						 "connection is NOT created by " +
				   			  "newDbConnetion!");
    		}
    	}
    }
}