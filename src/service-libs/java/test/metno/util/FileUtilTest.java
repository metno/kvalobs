/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: FileUtilTest.java,v 1.1.2.3 2007/09/27 09:02:44 paule Exp $                                                       

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

public class FileUtilTest extends TestCase{
    static final String inFile="tmp/inFile.txt";
    static final String outFile="tmp/outFile.txt";
    static final String readTstFile="test/metno/util/readtest.txt";

    
    //filecontent must be equal to the content in the file test/metno/util/readtest.txt.
    static String filecontent="#Dette er en text file for testing av \n"+ 
    						   "#FileUtil.readFile2Str  \n"+
    						   "linej 3\n"+
    						   "linje4";
   
    public void setUp(){
        
    }
    
    public void tearDown(){
    }

    
    public void testReadFile(){
        String buf=FileUtil.readFile2Str(readTstFile);

        assertNotNull("Failed to read file: "+readTstFile, buf);
        assertEquals("Read content differ from expected content", filecontent, buf);
    }
    
    public void testWriteFile(){
    	boolean res=FileUtil.writeStr2File(outFile, filecontent);
    	
    	assertTrue("Failed to write file: "+outFile, res);
    	
    	String buf=FileUtil.readFile2Str(outFile);

        assertNotNull("Failed to read file: "+outFile, buf);
        assertEquals("Read content differ from expected content", filecontent, buf);

    }
}