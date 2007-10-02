/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PropertiesHelperTest.java,v 1.1.2.4 2007/09/27 09:02:44 paule Exp $                                                       

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

import java.util.Properties;
import java.util.Date;
import junit.framework.*;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
//import metno.util.*;

public class PropertiesHelperTest extends TestCase{
    static final String fileIn="tmp/test_in.prop";
    static final String fileOut="tmp/test_out.prop";
    static final String tmpOut="tmp/tmp_out.prop";
    
    public void setUp(){
        File fin=new File(fileIn);
        FileOutputStream fsout=null;
        
        Properties prop=new Properties();
      
        try{
            fsout=new FileOutputStream(fin);
        }
        catch (FileNotFoundException e) {
            System.out.println("Cant create file: " +fin.toString()+e.toString());
        }
        
        prop.setProperty("sql", "SELECT * FROM T");
        prop.setProperty("18700.3","${sql}");
        prop.setProperty("mysql", "${sql}");
        
        try {
            prop.store(fsout, "#Test " + new Date());   
        } catch (IOException e) {
            System.out.println("Cant write to file: " + fin.toString()+e.toString());
        }
        
        try {
            fsout.close();
        } catch (IOException e) {
            // TODO Auto-generated catch block
            System.out.println("Cant close the file: " + fin);
        }
        
    }
    
    public void tearDown(){
        try{
            File f=new File(fileIn);
            
            f.delete();
            
            f=new File(fileOut);
            f.delete();
            
            f=new File(tmpOut);
            f.delete();
        }
        catch (Exception e) {
            System.out.println("Unexpected exception in tearDown!");
        }
    }

    
    public void testCountSubkeys(){
        String key="k1.k2.k3";
        StringHolder sh=new StringHolder();
        PropertiesHelper prophelper=new PropertiesHelper();
        
        int cnt=prophelper.countSubkeys(key, sh);
        assertEquals(3, cnt);
        assertEquals(key, sh.getValue());
        
        key="k1";
        cnt=prophelper.countSubkeys(key, sh);
        assertEquals(1, cnt);
        assertEquals(key, sh.getValue());
    }
    
    public void testGetSubkey(){
        String key="k1.k2.k3";
        String subkey;
        PropertiesHelper prophelper=new PropertiesHelper();
        
        subkey=prophelper.getSubkey(key, 0);
        assertEquals(key, subkey);
        
        subkey=prophelper.getSubkey(key, 1);
        assertEquals("k1", subkey);

        subkey=prophelper.getSubkey(key, 2);
        assertEquals("k2", subkey);

        subkey=prophelper.getSubkey(key, 3);
        assertEquals("k3", subkey);

        subkey=prophelper.getSubkey(key, 4);
        assertNull( subkey);

        key="k1";
        subkey=prophelper.getSubkey(key, 1);
        assertEquals(key, subkey);
        
        subkey=prophelper.getSubkey(key, 2);
        assertNull( subkey);
    }
    
    public void testLoadFromFile(){
        PropertiesHelper prophelper=new PropertiesHelper();
        
        try{
            prophelper.loadFromFile(fileIn);
        }
        catch(FileNotFoundException e){
            fail("Expected to openfile: " + fileIn);
        }
        catch (IOException e){
            fail("Unexpected IOException: " + e.getMessage());
        }
        
        try{
            prophelper.loadFromFile("This_file_dont_exsist");
        }
        catch(FileNotFoundException e){
        }
        catch (IOException e){
            fail("Unexpected IOException: " + e.getMessage());
        }
        
        assertTrue(true);
    }
        
    public void testSaveToFile(){
        PropertiesHelper prophelper=new PropertiesHelper();
        
        prophelper.setProperty("hei", "prop1");
        
        try{
            prophelper.saveToFile(fileOut);
            
        }catch(IOException e){
            fail();
        }
        
        assertTrue(true);
    }
    

    
    public void testApply(){
        PropertiesHelper prop=new PropertiesHelper();

        prop.setProperty("sql1", "SELECT * FROM T WHERE stnr=${$1} "+
                         "AND typeid=${$2}");      
        prop.setProperty("sql", "SELECT * FROM T");
        prop.setProperty("18700.3","${sql}");
        prop.setProperty("mysql", "${sql}");
        prop.setProperty("18700.4","${$1}");
        prop.setProperty("18700.5","${$2}");
        prop.setProperty("18700.6","${$1} ${$2}");
        prop.setProperty("18700.7","${sql1}");
      
        String val=prop.apply("mysql");
        assertTrue(val!=null);
        assertEquals("SELECT * FROM T", val);
        
        val=prop.apply("18700.4");
        assertTrue(val!=null);
        assertEquals("18700", val);

        val=prop.apply("18700.5");
        assertTrue(val!=null);
        assertEquals("5", val);

        val=prop.apply("18700.6");
        assertTrue(val!=null);
        assertEquals("18700 6", val);
  
        val=prop.apply("18700.7");
        assertTrue(val!=null);
        assertEquals(val, "SELECT * FROM T WHERE stnr=18700 AND typeid=7", val);
    }
    
    /*
    public void testApply2(){
        assertTrue(prophelper.apply("key2", prop)!=null);
    } */
}