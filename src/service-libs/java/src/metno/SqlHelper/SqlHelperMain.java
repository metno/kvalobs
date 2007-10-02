/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SqlHelperMain.java,v 1.1.2.3 2007/09/27 09:02:42 paule Exp $                                                       

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
import metno.util.*;

import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class SqlHelperMain {

	static Logger logger = Logger.getLogger(SqlHelperMain.class);
    
    private static void use(int i) {
        System.out.println(
        "Usage: SqlHelper [-h] [-c configFile] [-f sqlfile]");
        System.exit(i);
    }
	
    public static void main(String[] args)
    {
    	String path=System.getProperties().getProperty("SQLHELPER_HOME");
    	System.out.println("log4j conf: "+path+"/etc/SqlHelper_log.conf");
    	PropertyConfigurator.configure(path+"/etc/SqlHelper_log.conf");
    	
        SqlHelper sqlhelper=null;
        String configfile =path+"/etc/SqlHelper.conf";
        String sqlpropfile=null;
        GetOpt go = new GetOpt("c:f:h");

        File confile=new File(configfile);
        
        if(!confile.exists()){
            logger.fatal("Configfile: "+configfile+" dont exists!");
            System.exit(1);
        }
        
        char c;
        
        while ((c = go.getopt(args)) != GetOpt.DONE) {
            switch(c) {
            case 'h':
                use(0);
                break;
            case 'f':
                sqlpropfile=go.optarg();
                break;
            case 'c':
                configfile= go.optarg();
                break;
            default:
                System.err.println("Unknown option character " + c);
                use(1);
            }
        }
        
        go=null; //We dont need it anymore.

        if(sqlpropfile==null){
            use(1);
        }
        
        logger.info("Reading configuration from: " + configfile);
        logger.info("Reading database commands from propertyfile: "+sqlpropfile);
        
        PropertiesHelper prop=new PropertiesHelper();
        PropertiesHelper propsql=new PropertiesHelper();
        
        try{
            prop.loadFromFile(configfile);
            prop.list(System.out);
            propsql.loadFromFile(sqlpropfile);
            sqlhelper=new SqlHelper(prop);
        }
        catch(Exception e){
            System.out.println("Problems: " + e.getMessage());
            System.exit(1);
        }
       
        try {
            sqlhelper.runSqlFromProp(propsql);
        } catch (Exception e) {
            System.out.println("Errors: "+e.getMessage());
            System.exit(1);
        }
        
        System.exit(0);
        
    }
	
    
}