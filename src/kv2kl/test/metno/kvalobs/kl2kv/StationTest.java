/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StationTest.java,v 1.1.2.2 2007/09/27 09:02:20 paule Exp $                                                       

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

import java.util.*;
import metno.kvalobs.kl2kv.Station;
import org.junit.*;
import static org.junit.Assert.*;
import junit.framework.JUnit4TestAdapter;

public class StationTest {

	public StationTest() {
	}

	@Test
	public void parse(){
		Station[] st;

		ArrayList<String> list=new ArrayList<String>();

		st=Station.stations(list);
		
		System.out.println(Station.toString(st));
		assertTrue(st.length==1);
		assertEquals("[0-0]", st[0].toString());
		
		list.add("18700");
		st=Station.stations(list);
		System.out.println(Station.toString(st));
		assertTrue(st.length==1);
		assertEquals("[18700-0]", st[0].toString());
		System.out.println("Qyery: [" +st[0].query()+"]");
		assertEquals("stnr=18700", st[0].query());
		

		list.add("18700-18500");
		st=Station.stations(list);
		System.out.println(Station.toString(st));
		assertTrue(st.length==2);
		assertEquals("[18700-0]", st[0].toString());
		System.out.println("Qyery: [" +st[0].query()+"]");
		assertEquals("stnr=18700", st[0].query());

		assertEquals("[18500-18700]", st[1].toString());
		System.out.println("Qyery: [" +st[1].query()+"]");
		assertEquals("stnr>=18500 AND stnr<=18700", st[1].query());

		
	}
	
	public static junit.framework.Test suite() { 
        return new JUnit4TestAdapter(StationTest.class); 
    }

	
}
