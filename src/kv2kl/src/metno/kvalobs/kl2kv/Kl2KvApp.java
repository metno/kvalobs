/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Kl2KvApp.java,v 1.1.2.6 2007/09/27 09:02:19 paule Exp $                                                       

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

import metno.util.MiGMTTime;
import metno.kvalobs.kl.KlApp;
import metno.util.*;
import kvalobs.*;
import java.sql.*;
import java.io.*;
import java.util.*;

import org.apache.log4j.Logger;

public class Kl2KvApp extends KlApp
{
	static Logger logger=Logger.getLogger(Kl2KvApp.class);
	
    String stations;
    String tablename="KL2KVALOBS";
    
    public Kl2KvApp(String[] args, String conffile, String kvserver, boolean usingSwing){
    	super(args, conffile, kvserver, usingSwing);
    	
    	PropertiesHelper conf=getConf();
    	
    	String tmpTablename=conf.getProperty("kl2kv_table");
    	
    	if(tmpTablename!=null && tmpTablename.length()>0){
    		tablename=tmpTablename;
    	}
    }
    
    public String getTablename(){
    	return tablename;
    }
    
}
