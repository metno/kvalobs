/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StringUtilTest.java,v 1.1.2.2 2007/09/27 09:02:44 paule Exp $                                                       

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
package metno.util; 

import junit.framework.*;

public class StringUtilTest extends TestCase{
    static final String tmpOut="tmp/tmp_out.prop";
    
    public void setUp(){
        
    }
    
    public void tearDown(){
    }

    
    public void testStringUtilReplace(){
        String input="012345678901234567890123456789";
        String newstr="#######";
        
        String sr=StringUtil.replace(input, 4, 8, newstr);
        assertEquals("0123#######8901234567890123456789", sr);
        
        input="+++${sql}+++";
        newstr="SELECT * FROM T";
        
        sr=StringUtil.replace(input, 3, 9, newstr);
        assertEquals("+++SELECT * FROM T+++", sr);
        
        sr=StringUtil.replace(input, -1, -1, newstr);
        assertEquals(newstr, sr);

        sr=StringUtil.replace(input, -1, 3, newstr);
        assertEquals("SELECT * FROM T${sql}+++", sr);
        
        sr=StringUtil.replace(input, 3, -1, newstr);
        assertEquals("+++SELECT * FROM T", sr);
        
        sr=StringUtil.replace(input, 9, 3, newstr);
        assertEquals("+++SELECT * FROM T+++", sr);
    }
}