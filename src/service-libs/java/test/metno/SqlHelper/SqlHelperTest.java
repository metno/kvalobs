/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: SqlHelperTest.java,v 1.1.2.2 2007/09/27 09:02:44 paule Exp $                                                       

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

import metno.util.*;
import junit.framework.TestCase;

public class SqlHelperTest extends TestCase {

    static final String SQL="SELECT * FROM data WHERE stationid=${$2} AND typeid=${$3} ORDER BY obstime, stationid, typeid, paramid, sensor, level;";
    static final String OFILE="testresults/${$2}-${$3}.dat";
    static final String S18700T3="select.18700.3.ofile";
    static final String S18700T6="select.18700.6.ofile";
    PropertiesHelper prop;
        
    public void setUp(){
        prop=new PropertiesHelper();
        
        prop.setProperty("sql", SQL);
        prop.setProperty("ofile", OFILE);
        prop.setProperty(S18700T3, "${sql}");
        prop.setProperty(S18700T6, "${sql}");
    }
    
    public void tearDown(){
    }    

    public void testSelectInfoParserParse(){

        SelectInfoParser p=new SelectInfoParser(prop);
      
        SelectInfo si=p.parse(S18700T3);
        
        assertNotNull(si);
        
        assertEquals("testresults/18700-3.dat", si.filename);
        assertEquals("SELECT * FROM data WHERE stationid=18700 AND typeid=3 ORDER BY obstime, stationid, typeid, paramid, sensor, level;", si.select);
    }
    
    public void testSelectInfoParserNext(){

        SelectInfoParser p=new SelectInfoParser(prop);
      
        SelectInfo si;
        int        cnt=0;
        
        si=p.next();
        
        while(si!=null){
            cnt++;
            si=p.next();
        }
        
        assertTrue(cnt==2);
        
        //assertEquals("testresults/18700-3.dat", si.filename);
        //lsassertEquals("SELECT * FROM data WHERE stationid=18700 AND typeid=3 ORDER BY obstime, stationid, typeid, paramid, sensor, level;", si.select);
    }

}
