/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ParamFilterTest.java,v 1.1.2.5 2007/09/27 09:02:20 paule Exp $                                                       

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

import static metno.DbTestUtil.*;
import java.io.File;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.Timestamp;
import java.util.Collections;
import java.util.LinkedList;
import java.lang.Integer;
import CKvalObs.CService.DataElem;
import metno.dbutil.DbConnectionMgr;
import metno.dbutil.DbConnection;
import metno.kvalobs.kl.ParamFilter;
import metno.util.MiGMTTime;
import metno.util.StringHolder;
import metno.util.FileUtil;
import org.junit.*;
import static org.junit.Assert.*;
import junit.framework.JUnit4TestAdapter;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class ParamFilterTest {
    static final String dbdriver="org.hsqldb.jdbcDriver";
    static final String dbconnect="jdbc:hsqldb:tmp/db/db";
    static final String dbpasswd="";
    static final String dbuser="sa"; //Default user in a HSQLDB.
    static final String createKv2KlFilter="src/sql/create_kv2kl_filter_tables.sql";
	static DbConnectionMgr mgr=null;
    
    @BeforeClass
    public static void setUpAndLoadTheDb(){
    	
    	PropertyConfigurator.configure("test/metno/kvalobs/kl/ParamFilterTest_log.conf");
    	
        deleteDb("tmp/db");
        
        try {
            mgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, dbconnect, 10);
            
            if(!runSqlFromFile(mgr, createKv2KlFilter)){
            	mgr.closeDbDriver();
            	fail("Cant execute SQL from file: "+createKv2KlFilter);
            	return;
            }
            
            //Load the database with test data from a file.
        	assertTrue("Cant load: test/metno/kvalobs/kl/insert_into_typeid_param_filter.sql", 
        			runSqlFromFile(mgr, "test/metno/kvalobs/kl/insert_into_typeid_param_filter.sql"));
        	
        	listKlimaTypeidParamFilter(mgr);
        	listKlimaParamFilter(mgr);
        }catch(Exception ex) {
            if(mgr!=null)
                try{
                    mgr.closeDbDriver();
                }
                catch(Exception e){
                }
            
            fail("Cant load the database with testdata!");
        }
    	
    }
    
    public void setUp(){
    }
  
    public void tearDown(){
    }
    
    
        
    static boolean listKlimaTypeidParamFilter(DbConnectionMgr mgr){
    	return listDbTable(mgr,"T_KV2KLIMA_TYPEID_PARAM_FILTER");
    }

    
    static boolean listKlimaParamFilter(DbConnectionMgr mgr){
    	return listDbTable(mgr,"T_KV2KLIMA_PARAM_FILTER");
    }

    
    
    
    DataElem getDataElem(int sid, int tid, String obstime, 
        		            int paramid, int sensor, int level){
        
        return new DataElem(
                            sid, obstime, 1.2, (short)paramid,
                            "2006-03-09 18:00:00", 
                            (short)tid, Integer.toString(sensor), (short)level, 1.2, 
                            "0123456789012345", "0123456789012345",
                            ""); 
    }
    
    boolean doFilter(ParamFilter pf, DataElem de){
    	MiGMTTime obstimeTmp=new MiGMTTime();
		
		if(!obstimeTmp.parse(de.obstime)){
			System.out.println("ERROR: filter.filter: Cant parse <obstime> '"+de.obstime+"'");
			return false;
		}
				
		Timestamp obstime=new Timestamp(obstimeTmp.getTime().getTime());
		
		return pf.filter(de, obstime);
    }
    
    @Test
    public void loadFromDb(){
    	DbConnection con=null;
    	
    	try{
    		con=mgr.newDbConnection();
    	
    		assertNotNull("Cant create an dbconnection!", con);
    	
    		LinkedList tl302=new LinkedList();
    		tl302.add(new ParamFilter.ParamElem(302,112,0,0));
    		Collections.sort(tl302);
    	
    		LinkedList tl330=new LinkedList();
    		tl330.add(new ParamFilter.ParamElem(330,104,1,0));
    		tl330.add(new ParamFilter.ParamElem(330,106,0,0));
    		tl330.add(new ParamFilter.ParamElem(330,125,0,0));
    		Collections.sort(tl330);
    	
    		ParamFilter pf=new ParamFilter(18700, con);
    	    	
    		LinkedList paramList=pf.loadFromDb((short)302);
    	
    		System.out.println(pf.types);
    		assertEquals(paramList.toString(), tl302.toString());
    		assertEquals(pf.types.size(), 1);
    	
    		paramList=pf.loadFromDb((short)330);
    		System.out.println(pf.types);
    		assertEquals(paramList.toString(), tl330.toString());
    		assertEquals(pf.types.size(), 2);
    	
    		paramList=pf.loadFromDb((short)312);
    		System.out.println(pf.types);
    		assertTrue(paramList.size()==0);
    		assertEquals(pf.types.size(), 3);
    		
    		Object obj=pf.types.get(new Integer((short)312));
    		
    		assertNotNull(obj);
    		LinkedList	list=(LinkedList)obj;
    		assertTrue(list.size()==0);

    		paramList=pf.loadFromDb((short)308);
    		System.out.println(pf.types);
    		assertTrue(paramList.size()==2);
    		assertTrue(pf.types.size()==4);
    		
    		mgr.releaseDbConnection(con);
    	}
    	catch(Exception e){
    		e.printStackTrace();
    		fail("Unexpected exception!");
    	}
    }
    
        
    @Test
    public void filter(){
        StringHolder msg=new StringHolder();
        DbConnection con=null;
        boolean         ret;
        
        assertNotNull(mgr);
        //assertTrue(listKv2KlimaFilter(mgr));
        
        try{
           con=mgr.newDbConnection();
        
           assertNotNull("Cant create an dbconnection!", con);
        
           ParamFilter pf=new ParamFilter(18700, con);
    
           ret=doFilter(pf, getDataElem(18700,302,"2006-06-14 12:00:00", 108, 0,0));
   		   System.out.println("filter 1\n"+pf.types+"\n\n");
           assertTrue(ret);
       
           ret=doFilter(pf, getDataElem(18700,302,"2006-06-14 12:00:00", 112,0,0));
   		   System.out.println("filter 2\n"+pf.types+"\n\n");
           assertFalse(ret);
           
           ret=doFilter(pf, getDataElem(18700,330,"2006-06-14 12:00:00", 104,1,0));
   		   System.out.println("filter 3\n"+pf.types+"\n\n");
           assertFalse(ret);
           
           ret=doFilter(pf, getDataElem(18700,312,"2006-06-14 12:00:00", 104,1,0));
   		   System.out.println("filter 4\n"+pf.types+"\n\n");
           assertTrue(ret);
           
           ret=doFilter(pf, getDataElem(18700,308, "2006-06-14 12:00:00", 22,0,0));
   		   System.out.println("filter 5\n"+pf.types+"\n\n");
           assertTrue(ret);
           
           ret=doFilter(pf, getDataElem(18700,308, "2006-06-14 12:00:00", 109, 0,0));
   		   System.out.println("filter 6\n"+pf.types+"\n\n");
           assertFalse(ret);
           
           ret=doFilter(pf, getDataElem(18700, 3,"2005-03-09 18:00:00", 104, 0, 0));
   		   System.out.println("filter 7\n"+pf.types+"\n\n");
           assertFalse(ret);
     	
       	   ret=doFilter(pf, getDataElem(18700, 3,"2006-03-09 18:00:00", 104, 0, 0));
   		   System.out.println("filter 8\n"+pf.types+"\n\n");
       	   assertFalse(ret);

       	   
           ret=doFilter(pf, getDataElem(18700, 3,"2005-03-09 18:00:00", 110, 0, 0));
  		   System.out.println("filter 9\n"+pf.types+"\n\n");
           assertTrue(ret);
           
           
           pf=new ParamFilter(87110, con);
           ret=doFilter(pf, getDataElem(87110, 1, "2006-09-08 06:00:00", 211, 0, 0));
 		   System.out.println("filter 10\n"+pf.types+"\n\n");
           assertFalse(ret);
           
        }
        catch(Exception e){
        	fail("Unexpected exception!");
        }
    }
    
    
    public static junit.framework.Test suite() { 
        return new JUnit4TestAdapter(ParamFilterTest.class); 
    }


    
}
