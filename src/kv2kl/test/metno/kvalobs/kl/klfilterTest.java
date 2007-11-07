/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: klfilterTest.java,v 1.1.2.5 2007/09/27 09:02:20 paule Exp $                                                       

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

import java.io.File;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import CKvalObs.CService.DataElem;
import metno.dbutil.DbConnectionMgr;
import metno.dbutil.DbConnection;
import metno.util.StringHolder;
import static metno.DbTestUtil.*;
import org.junit.*;
import static org.junit.Assert.*;
import junit.framework.JUnit4TestAdapter;
import org.apache.log4j.Logger;
import org.apache.log4j.PropertyConfigurator;

public class klfilterTest {
    static final String dbdriver="org.hsqldb.jdbcDriver";
    static final String dbconnect="jdbc:hsqldb:tmp/db/db";
    static final String dbpasswd="";
    static final String dbuser="sa"; //Default user in a HSQLDB.
    static final String createFilterTables="src/sql/create_kv2kl_filter_tables.sql";
    DbConnectionMgr mgr=null;
    
    
    
    
    @BeforeClass
    public static void setUpAndLoadTheDb(){
    	PropertyConfigurator.configure("test/metno/kvalobs/kl/klfilterTest_log.conf");
    }

    
    
    
    @Before
    public void setUp(){
    	mgr=fillWithData();
    }
  
    @After
    public void tearDown(){
    	if(mgr!=null){
    		try{
        		mgr.closeDbDriver();
        	}
        	catch(Exception ex){
        		System.out.println("Cant close the DbManager!" +ex.getMessage());
        	}
    	}

		deleteDb("tmp/db");
    	mgr=null;
    }
    
    boolean listTestTables(DbConnectionMgr mgr){
    	boolean ok=true;
    	
    	if(!listDbTable(mgr, "t_kv2klima_filter"))
    		ok=false;
    	
    	if(!listDbTable(mgr, "T_KV2KLIMA_TYPEID_PARAM_FILTER"))
    		ok=false;

    	if(!listDbTable(mgr, "T_KV2KLIMA_PARAM_FILTER"))
    		ok=false;

    	return ok;
    }
    
    DbConnectionMgr fillWithData(){
        DbConnectionMgr mgr=null;
        
        
        String insert="insert into t_kv2klima_filter VALUES("+
        			  "18700,'T',NULL,NULL,308,NULL);"+
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18700,'D',NULL,NULL,3,NULL);"+
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18700,'D',NULL,NULL,6,NULL);"+
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18700,'D',NULL,NULL,10,18701);"+ 
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18500,'D',NULL,'2006-01-01 03:00:00',10,NULL);"+ 
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18500,'D','2006-02-01 03:00:00', '2006-02-20 12:00:00',10,NULL);"+
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18500,'D','2006-02-20 14:00:00',NULL,10,NULL);" +
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18600,'D',NULL,'2006-01-01 03:00:00',10,NULL);" +
        			  "insert into t_kv2klima_filter VALUES("+
        			  "18800,'D','2006-01-01 03:00:00', NULL, 10,NULL);";

        

        
        deleteDb("tmp/db");
        
        try {
            mgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, dbconnect, 1);
        }catch(Exception ex) {
            System.out.println("Cant create connection: " +ex.getMessage());
            return null;
        }

        boolean ok=true;
        

        if(ok && !runSqlFromFile(mgr, createFilterTables)){
        	System.out.println("Cant execute SQL from file: "+createFilterTables);
        	ok=false;
        }

        
        if(ok && !runSqlFromString(mgr, insert)){
        	System.out.println("Cant insert data into the table: T_KV2KLIMA_FILTER");
        	ok=false;
        }

        if(ok && !runSqlFromFile(mgr, "test/metno/kvalobs/kl/insert_into_typeid_param_filter.sql"))
        	ok=false;
        
    
        if(!ok){
        	try{
        		mgr.closeDbDriver();
        	}
        	catch(Exception ex){
        		System.out.println("Cant close the DbManager!" +ex.getMessage());
        	}
        	
        	fail("Cant load the database with testdata!");
        	return null;
        }
        
        return mgr;
    }
    
    DataElem 
        fillWithStationData(int sid, int tid, String obstime){
        
        return new DataElem(
                            sid, obstime, 1.2, (short)211,
                            "2006-03-09 18:00:00", 
                            (short)tid, "0", (short)0, 1.2, 
                            "0123456789012345", "0123456789012345",
                            ""); 
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

    
    @Test
    public void missingTableEntryForType(){
        StringHolder msg=new StringHolder();
        DbConnection con=null;
        boolean         ret;

        System.out.println("Test: missingTableEntryForType");
        assertNotNull(mgr);
        assertTrue(listTestTables(mgr));
        
        try{
           con=mgr.newDbConnection();
        }
        catch(Exception e){
            fail("Unexpected exception!");
        }
        
        assertNotNull(con);
        
        Filter filter=new Filter(con);
        
        ret=filter.filter(fillWithStationData(18700, 6, "2006-03-09 18:00:00"), msg);
        assertTrue(ret);       
        
        ret=filter.filter(fillWithStationData(18700, 6, "2006-03-09 18:00:00"), msg);
        assertTrue(ret);       

        ret=filter.filter(fillWithStationData(18700, 1, "2006-03-09 18:00:00"), msg);
        
        assertFalse(ret);       

        ret=filter.filter(fillWithStationData(18700, 1, "2006-03-09 18:00:00"), msg);
        
        assertFalse(ret);
        
        ret=filter.filter(fillWithStationData(18700, 6, "2006-03-09 18:00:00"), msg);
        assertTrue(ret);       

        ret=filter.filter(fillWithStationData(18700, 6, "2006-03-09 18:00:00"), msg);
        assertTrue(ret); 
		  
		ret=filter.filter(fillWithStationData(18500, 10, "2005-12-09 18:00:00"), msg);
        assertTrue(ret); 

        ret=filter.filter(fillWithStationData(18500, 10, "2006-01-01 03:00:00"), msg);
        assertTrue(ret);
		        
        ret=filter.filter(fillWithStationData(18500, 10, "2006-1-1 04:00:00"), msg);
        assertFalse(ret);

        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-1 02:00:00"), msg);
        assertFalse(ret);
        
        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-1 03:00:00"), msg);
        assertTrue(ret);
        
        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-18 02:00:00"), msg);
        assertTrue(ret);

        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-20 12:00:00"), msg);
        assertTrue(ret);

        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-20 13:00:00"), msg);
        assertFalse(ret);
        
        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-20 14:00:00"), msg);
        assertTrue(ret);
        
        ret=filter.filter(fillWithStationData(18500, 10, "2006-2-20 11:00:00"), msg);
        assertTrue(ret);
        
        ret=filter.filter(fillWithStationData(18500, 10, "2008-2-20 11:00:00"), msg);
        assertTrue(ret);

        ret=filter.filter(fillWithStationData(18600, 10, "2003-2-20 11:00:00"), msg);
        assertTrue(ret);
        
        ret=filter.filter(fillWithStationData(18600, 10, "2006-1-1 3:00:00"), msg);
        assertTrue(ret);

        ret=filter.filter(fillWithStationData(18600, 10, "2006-1-1 4:00:00"), msg);
        assertFalse(ret);
        
        ret=filter.filter(fillWithStationData(18600, 10, "2007-1-1 4:00:00"), msg);
        assertFalse(ret);
        
        ret=filter.filter(fillWithStationData(18800, 10, "2006-1-1 2:00:00"), msg);
        assertFalse(ret);
                        
        ret=filter.filter(fillWithStationData(18800, 10, "2006-1-1 3:00:00"), msg);
        assertTrue(ret);

        ret=filter.filter(fillWithStationData(18800, 10, "2007-1-1 4:00:00"), msg);
        assertTrue(ret);
                
        try{
           mgr.releaseDbConnection(con);
        }
        catch(Exception e){
             fail("Unexpected exception!");
        }

    }
    
    @Test
    public void paramFilter(){
        StringHolder msg=new StringHolder();
        DbConnection con=null;
    	boolean ret;
 
        System.out.println("Test: paramFilter!");
    	assertNotNull(mgr);
        assertTrue(listTestTables(mgr));
        
        try{
           con=mgr.newDbConnection();
        }
        catch(Exception e){
            fail("Unexpected exception!");
        }
        
        assertNotNull(con);
        
        Filter filter=new Filter(con);
    	
    	ret=filter.filter(getDataElem(18700, 3,"2006-03-09 18:00:00", 88, 0, 0),
        				  msg);
        assertTrue(ret);

    	
        ret=filter.filter(getDataElem(18700, 3,"2001-03-09 18:00:00", 104, 0, 0),
                msg);
        assertFalse(ret);
        

    	ret=filter.filter(getDataElem(18700, 3,"2005-01-02 23:00:00", 104, 0, 0),
                  msg);

      	assertFalse(ret);

      	ret=filter.filter(getDataElem(18700, 3,"2005-01-03 00:00:00", 104, 0, 0),
                msg);

    	assertFalse(ret);
    	
    	ret=filter.filter(getDataElem(18700, 3,"2005-01-03 01:00:00", 104, 0, 0),
                msg);

    	assertTrue(ret);

    	ret=filter.filter(getDataElem(18700, 3,"2005-01-13 23:00:00", 104, 0, 0),
                msg);

    	assertTrue(ret);
    	
    	ret=filter.filter(getDataElem(18700, 3,"2005-01-31 23:00:00", 104, 0, 0),
                msg);

    	assertTrue(ret);

    	ret=filter.filter(getDataElem(18700, 3,"2005-02-01 00:00:00", 104, 0, 0),
                msg);

    	assertFalse(ret);
    	
    	ret=filter.filter(getDataElem(18700, 3,"2005-010-02 23:00:00", 104, 0, 0),
                msg);

    	assertFalse(ret);
    	
    	ret=filter.filter(getDataElem(18700, 3,"2006-01-01 00:00:00", 104, 0, 0),
                msg);

    	assertFalse(ret);
    	

    	ret=filter.filter(getDataElem(18700, 3,"2006-01-01 01:00:00", 104, 0, 0),
                msg);

    	assertTrue(ret);

    	ret=filter.filter(getDataElem(18700, 3,"2006-01-01 02:00:00", 104, 0, 0),
                msg);

    	assertFalse(ret);
    	
        ret=filter.filter(getDataElem(18700, 3,"2006-03-09 18:00:00", 104, 0, 0),
				          msg);
    	assertFalse(ret);
    	    	
        ret=filter.filter(getDataElem(18700, 3,"2005-01-03 00:00:00", 110, 0, 0),
    	                  msg);
        assertFalse(ret);
         
        //test nagativ typeid. Should be accepted as abs(typeid).
        ret=filter.filter(getDataElem(18700, -3,"2005-01-03 00:00:00", 110, 0, 0),
                          msg);
        
        System.out.println(" ret : " +ret);
        assertFalse(ret);
      	
      	//Test the upper limit of [fdato,tdato>, the tdato part.
      	
      	ret=filter.filter(getDataElem(18700, 3,"2005-01-03 00:00:00", 104, 0, 0),
    	                  msg);
        assertFalse(ret);
        
        ret=filter.filter(getDataElem(18700, 3,"2005-01-02 23:59:59", 104, 0, 0),
  	          			  msg);
        assertFalse(ret);
        
        
        //Test the lower limit of [fdato,tdato>. The fdato part.

        ret=filter.filter(getDataElem(18700, 3, "2005-01-03 00:00:00", 111, 0, 0),
  	          			  msg);
        assertFalse(ret);
      
        ret=filter.filter(getDataElem(18700, 3,"2005-01-02 23:59:59", 111, 0, 0),
	          			  msg);
        assertTrue(ret);

        
        try{
            mgr.releaseDbConnection(con);
        }
        catch(Exception e){
            fail("Unexpected exception!");
        }
    }
    
    @Test
    public void testStation(){
        StringHolder msg=new StringHolder();
        DbConnection con=null;
        boolean         ret;
        
        System.out.println("Test: testStation");
        assertNotNull(mgr);
        assertTrue(listTestTables(mgr));
        
        try{
           con=mgr.newDbConnection();
        }
        catch(Exception e){
            fail("Unexpected exception!");
        }
        
        assertNotNull(con);
        
        Filter filter=new Filter(con);
        
        ret=filter.filter(fillWithStationData(18700, 308,
                                              "2006-03-09 18:00:00"),
                           msg);
        assertFalse(ret);
        assertNotNull(filter.dbKv2KlimaFilterElem);
        assertNotNull(filter.dbKv2KlimaFilterElem.getStatus());
        assertTrue(filter.dbKv2KlimaFilterElem.getStatus().length() != 0 );
        assertTrue(filter.dbKv2KlimaFilterElem.getStatus().charAt(0)=='T');
        ret=filter.filter(fillWithStationData(18700, 308,
                                              "2006-03-09 18:00:00"),
                                               msg);
        assertFalse(ret);

        
        ret=filter.filter(fillWithStationData(18700, 6,
                                               "2006-03-09 18:00:00"),
                                               msg);
        assertTrue(ret);       

        ret=filter.filter(fillWithStationData(18700, 6,
                                              "2006-03-09 18:00:00"),
                                              msg);
        try{
           mgr.releaseDbConnection(con);
        }
        catch(Exception e){
             fail("Unexpected exception!");
        }

    }
    
    @Test	
    public void nyttStNr(){
        StringHolder msg=new StringHolder();
        DbConnection con=null;
        boolean         ret;
        
        System.out.println("Test: nyttStNr");
        assertNotNull(mgr);
        assertTrue(listTestTables(mgr));
        
        try{
           con=mgr.newDbConnection();
        }
        catch(Exception e){
            fail("Unexpected exception!");
        }
        
        assertNotNull(con);
        
        Filter filter=new Filter(con);
        DataElem de=fillWithStationData(18700, 10,
                                        "2006-03-09 18:00:00");
        ret=filter.filter(de, msg);
        assertTrue(ret);
        assertTrue(de.stationID==18701);

        filter=new Filter(con);
        de=fillWithStationData(18700, -10,
                               "2006-03-09 18:00:00");
        ret=filter.filter(de, msg);
        assertTrue(ret);
        assertTrue(de.stationID==18701);
        
        try{
           mgr.releaseDbConnection(con);
        }
        catch(Exception e){
             fail("Unexpected exception!");
        }

    }

    public static junit.framework.Test suite() { 
        return new JUnit4TestAdapter(klfilterTest.class); 
    }
    
}
