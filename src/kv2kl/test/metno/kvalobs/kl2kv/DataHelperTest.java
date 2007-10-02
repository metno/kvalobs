/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataHelperTest.java,v 1.1.2.3 2007/09/27 09:02:20 paule Exp $                                                       

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

import static metno.DbTestUtil.*;
import metno.kvalobs.kl2kv.*;
import java.io.File;
import java.sql.ResultSet;
import java.sql.ResultSetMetaData;
import java.sql.Timestamp;
import java.util.Collections;
import java.util.LinkedList;
import java.lang.Short;
import CKvalObs.CService.DataElem;
import metno.dbutil.DbConnectionMgr;
import metno.dbutil.DbConnection;
import metno.util.MiGMTTime;
import metno.util.StringHolder;
import metno.util.FileUtil;
import org.junit.*;
import static org.junit.Assert.*;
import junit.framework.JUnit4TestAdapter;

public class DataHelperTest {
    static final String dbdriver="org.hsqldb.jdbcDriver";
    static final String dbconnect="jdbc:hsqldb:tmp/db/db";
    static final String dbpasswd="";
    static final String dbuser="sa"; //Default user in a HSQLDB.
    static final String create_kv2kvalobs="src/sql/create_kl2kvalobs.sql";
	static DbConnectionMgr mgr=null;
    
    @BeforeClass
    public static void setUpAndLoadTheDb(){
        deleteDb("tmp/db");
        
        try {
            mgr=new DbConnectionMgr(dbdriver, dbuser, dbpasswd, dbconnect, 10);
            
            if(!runSqlFromFile(mgr, create_kv2kvalobs)){
            	mgr.closeDbDriver();
            	fail("Cant execute SQL from file: "+create_kv2kvalobs);
            	return;
            }
            
            //Load the database with test data from a file.
        	assertTrue("Cant load: test/metno/kvalobs/kl2kv/insert_into_kl2kvalobs.sql", 
        			runSqlFromFile(mgr, "test/metno/kvalobs/kl2kv/insert_into_kl2kvalobs.sql"));
        	
        	listKl2Kvalobs(mgr);
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
    
    
        
    static boolean listKl2Kvalobs(DbConnectionMgr mgr){
    	return listDbTable(mgr,"KV2KVALOBS");
    }

    

    
    @Test
    public void createMsgFromDb(){
    	DbConnection con=null;
    	DataToKvForTest dataToKv=new DataToKvForTest();
    	
    	
    	try{
    		con=mgr.newDbConnection();
    	
    		DataHelper dh=new DataHelper(con, dataToKv, "330", null, 3);
    		boolean ret=dh.sendDataToKv(new Station());
    	
    		System.out.println("Size: "+dataToKv.theData.size());
    		assertTrue(dataToKv.theData.size()==4);
    		assertTrue(ret);
    		assertEquals(dataToKv.theData.get(0),
    					 "18500/330\n"+
    			         "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
    				     "200606260700,3,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
    			         "200606260800,4,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n"+
    			         "200606260900,5,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n");
    	
    		assertEquals(dataToKv.theData.get(1),
    					 "18500/330\n"+
    					 "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
    					 "200606261000,6,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n"+
    					 "200606261100,7,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
			         	 "200606261200,8,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n");

    		assertEquals(dataToKv.theData.get(2),
    					 "18500/330\n"+
    					 "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
    					 "200606261300,9,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
		             	"200606261400,10,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n");
    	
    		assertEquals(dataToKv.theData.get(3),
			         	 "18700/330\n"+
			         	 "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
			         	 "200606260700,1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
			         	 "200606260800,2,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n");

    	
    		dataToKv.clear();
    		ret=dh.sendDataToKv(new Station("18700"));
    		assertTrue(dataToKv.theData.size()==1);
    		assertTrue(ret);
    		assertEquals(dataToKv.theData.get(0),
    					 "18700/330\n"+
    			         "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
                         "200606260700,1,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
                         "200606260800,2,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n");

    		dataToKv.clear();
    		ret=dh.sendDataToKv(new Station("18500"));
    		assertTrue(ret);
    		System.out.println(dataToKv.theData.size());
    		assertTrue(dataToKv.theData.size()==3);
    		assertEquals(dataToKv.theData.get(0),
    			     	 "18500/330\n"+
    			         "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
    				     "200606260700,3,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
    			         "200606260800,4,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n"+
    			         "200606260900,5,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n");
    	
    		assertEquals(dataToKv.theData.get(1),
			         	 "18500/330\n"+
			         	 "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
			         	 "200606261000,6,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n"+
			         	 "200606261100,7,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
			         	 "200606261200,8,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n");

    		assertEquals(dataToKv.theData.get(2),
		             	 "18500/330\n"+
		             	 "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
		             	 "200606261300,9,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n"+
		             	 "200606261400,10,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n");
    	
    	
    		dataToKv.clear();
    		dh=new DataHelper(con, dataToKv, "310", null, 3);
    		ret=dh.sendDataToKv(new Station("18500"));
    		assertTrue(ret);
    		assertTrue(dataToKv.theData.size()==2);
    		
    		
    		dataToKv.clear();
    		dh=new DataHelper(con, dataToKv, "302", null, 5);
    		ret=dh.sendDataToKv(new Station("100"));
    		assertTrue(ret);
    		System.out.println(dataToKv.theData.size());
    		assertTrue(dataToKv.theData.size()==1);
    		assertEquals(dataToKv.theData.get(0),
    			     	 "100/302\n"+
    			         "V4S,V5S,V6S,V4,V5,V6,SA,SD,ITR,RR_X,KLSTART,KLOBS,RR_01,V1,V2,V3,WW,W1,W2,UU,TA,TAX_12,TAN_12,DD,FF,NN,NH,CL,CM,CH,RR_12,HL,VV,V7,DX,DG,KLFX,KLFG,FG_010,FF_1,FX_1,FG_1,SG,EM,OT_24,EV,TW,FX,FG,PO,PR,AA,PP,FX_6,FX_12,OT_1,BT,FM,QO,QOX,RA,RR_1,TD,TAM,TAN,TAX,TG,TGM,TGN,TGX,UM,RR_24\n"+
    				     "200606260600,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-32767(0000003000000000,)\n"+
    			         "200606270600,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-32767.0(0000003000000000,)\n"+
    			         "200606280600,,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,4(0000000000000000,1000000000000000)\n"+
    			         "200606290600,,,,,,,,,,,200601020000,200601080000,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,-32767(0123453789012345,1000000000000000)\n"+
    			         "200606300600,,,,,,,,,,-32767(0000003000000000,),,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\n");
    	
    		
    	}
    	catch(Exception ex){
    		ex.printStackTrace();
    	}
    	finally{
    		if(con!=null){
    			try{
    				mgr.releaseDbConnection(con);
    			}catch(Exception ex){
    				ex.printStackTrace();
    			}
    		}
    	}
    }
        
    public static junit.framework.Test suite() { 
        return new JUnit4TestAdapter(DataHelperTest.class); 
    }


    
}
