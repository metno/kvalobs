/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SqlHelper.java,v 1.1.2.3 2007/09/27 09:02:42 paule Exp $                                                       

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
package metno.SqlHelper;

import java.util.*;
import java.text.*;
import java.sql.*;
import java.io.*;
import java.lang.*;
import metno.util.*;
import metno.dbutil.*;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;


public class SqlHelper {

    DbConnectionMgr dbmgr;
	static Logger logger = Logger.getLogger(SqlHelper.class);
	static final  String NULL="\\N";
    void throwException(String msg)throws Exception{
        logger.fatal(msg);
        throw new Exception(msg);
    }
    
    public SqlHelper(PropertiesHelper prop)throws Exception{
        try {
            dbmgr=new DbConnectionMgr(prop);
        } catch (IllegalArgumentException e) {
            throwException(e.getMessage());
        } catch (ClassNotFoundException e) {
            throwException(e.getMessage());
        }

        //Dont cache connections.
        dbmgr.setTimeToLive(0);
        
    	System.out.println("Database setup. ");
        System.out.println("         dbdriver: "+dbmgr.getDbdriver());
    	System.out.println("           dbuser: "+dbmgr.getDbuser());
    	System.out.println("        dbconnect: "+dbmgr.getDbconnect());
        System.out.println(" dbmaxconnections: "+dbmgr.getMaxconnections());
    	System.out.println("---------------------------------------------");
	
        logger.info("dbdriver: "+dbmgr.getDbdriver());
        logger.info("dbuser: " + dbmgr.getDbuser());
        logger.info("dbconnect: "+dbmgr.getDbconnect());
        logger.info("dbmaxconnections: "+dbmgr.getMaxconnections());
    }
   
    public void runSqlFromProp(PropertiesHelper prop)throws Exception{
        SelectInfoParser parser=new SelectInfoParser(prop);
        SelectInfo si;
        
        si=parser.next();
        
        while(si!=null){
            File file=new File(si.filename);
            FileOutputStream fos=null;
            PrintStream ps=null;
            
            try {
                fos=new FileOutputStream(file);
                ps=new PrintStream(fos, true);
                runSql(si.select, ps);
                
            } catch (FileNotFoundException e) {
                e.printStackTrace();
            }
            catch(Exception e){
                e.printStackTrace();
                ps.close();
            }
            
            si=parser.next();
        }
    }
    
    
    public void runSql(String sqlstatement, PrintStream out)throws Exception{
        String errormsg=null;
        DbConnection con=dbmgr.newDbConnection();
        ResultSet rs=null;

        if(con==null){
            throwException("Cant create a connection to the database!");
        }
       
        try{
            rs=con.execQuery(sqlstatement);
            ResultSetMetaData rsMetaData=rs.getMetaData();
            String data;
            boolean first;
        
            while(rs.next()){
                first=true;
                
                
                for(int i=1; i<=rsMetaData.getColumnCount(); i++){
                    data=rs.getString(i);
                
                    if(rs.wasNull())
                        data=NULL;
                    
                    if(first){
                        first=false;
                    }else{
                        out.print(",");
                    }
        
                    out.print(data);
                }
            
                out.println();
            }
        }
        catch(SQLException ex){
            errormsg=ex.getMessage();
            logger.error(new MiGMTTime() + "SQLEXCEPTION:  " +ex);
            logger.error("QUERY: "+sqlstatement);
        }

        if(rs!=null){
            try{
                rs.close();
            }   
            catch(SQLException ex){
                errormsg=ex.getMessage();
                logger.error("SQLEXCEPTION: When closing the ResultSet!\n"+ex);
            
            }
        }
    
    
        try {
            dbmgr.releaseDbConnection(con);
        } catch (Exception e) {
            errormsg=e.getMessage();
        }
    
        if(errormsg!=null){
            throwException(errormsg);
        }
    }
    
    
}