/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: PropertiesHelper.java,v 1.1.2.3 2007/09/27 09:02:43 paule Exp $                                                       

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
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;

public class PropertiesHelper extends Properties {
	
    String filename;
    
	public PropertiesHelper(){
	    
	}
    
    public PropertiesHelper(Properties defaults){
        super(defaults);
    }
	
	public PropertiesHelper(String filename){
        this.filename=filename;
    }
    
    
    public int countSubkeys(String key, StringHolder sh){
        int cnt=1;
        int i=0;
        
        key=key.trim();
        
        while(key.length()>0 && key.charAt(0)=='.' )
            key=key.substring(1);
        
        while(key.length()>0 && key.charAt(key.length()-1)=='.' )
            key=key.substring(0,key.length()-1);

        sh.setValue(key);
        
        if(key.length()==0)
            return 0;
        
        i=key.indexOf('.', 0);
        
        while(i>=0){
            cnt++;
            i=key.indexOf('.', i+1);
        }

        return cnt;
    }
    
    /**
     * Assume a key on the form k1.k2.k3 and return a sublement
     * of the key. Subelements of this key is: k1, k2 and k3.
     * The subelement of index 1 is k2, index 2 is k2, etc.
     * 
     * An index of 0 is returns the entire key, ie k1.k2.k3.
     * 
     * @param key A key on the form k1.k2.k3.k4 .....
     * @param index An index of n'te subelement of the key.
     * @return The n'te subelement of the key.
     *         If the index is out of range null is returned.
     */
    public String getSubkey(String key, int index){
        int cnt;
        int i;
        StringHolder sh=new StringHolder();
        
        cnt=countSubkeys(key, sh);
        
        key=sh.getValue();
        
        if(index==0)
            return key;
        
        if(index<1 || index>cnt)
            return null;

        i=-1;

        for(int ii=0; ii<index-1; ii++)
            i=key.indexOf('.', i+1);

        if(i>-1)
            key=key.substring(i+1);
        
        i=key.indexOf('.');
        
        if(i<0)
            return key;
        
        key=key.substring(0, i);
        
        return key.trim();
    }
	

	public Properties loadFromFile(String filename)
        throws FileNotFoundException, IOException{
        
        File f=new File(filename);
        FileInputStream fin=new FileInputStream(f);
        
        load(fin);
		fin.close();
        return this;
	}
	
    public boolean save(){
        return false;
    }
    
	public void saveToFile(String filename)
        throws FileNotFoundException, IOException{

        File f=new File(filename);
        FileOutputStream fout=new FileOutputStream(f);
        
        store(fout, "WARNING Dont create comment in this file, they will be lost!");
        fout.close();
	}
    
    /**
     * Same as apply(String key, String Value), but we look
     * up the value for the key in the propertys.
     * 
     * @param key to look up in the property.
     * @return a string on success and null on failure.
     */
	
    public String apply(String key){
        String val=getProperty(key);
        
        return apply(key, val);
    }
    
    /**
     * apply susbtitues some keys in the value string with
     * data from the Properties. Keys are on the form: 
     * ${key}. Where key must be the name of a property. 
     * A special form of key are $n where n is an number.
     * This number reference an subkey in the key argument
     * to the method. The key must be on the form k1.k2.k3 etc.
     *     
     * <pre>
     *     Eksempel.
     *     
     *     We have the folowing propertys.
     *     
     *     key1=val1
     *     key2=${$1}-${$2}.dat
     *     
     *     if we call apply with the following key and value.
     *     
     *     key=file.1870
     *     value=${key1}
     *     
     *     The method will return the string 'val1'.
     *     
     *     If we call it with the value=${key2} it will return
     *     the string 'file-1870.dat'.
     * </pre>
     * 
     * @param key
     * @param value a string.
     * @return a string on success and null on failure.
     */
	public String apply(String key, String value){
        int ii;
        String k;
        String val2;
        int keyIndex;
        
        if(value==null)
            return null;
        
		int i=value.indexOf("${");
        
        while(i>-1){
            ii=value.indexOf('}', i);
        
            if(ii==-1){
                //Throw an format exception.
                return null;
            }
        
            k=value.substring(i+2, ii);
            k=k.trim();
        
            if(k.startsWith("$")){
                if(k.length()<2){
                    //Throw an format error.
                }
            
                keyIndex=Integer.parseInt(k.substring(1));

                val2=getSubkey(key, keyIndex);
            
                if(val2==null){
                    //Throw an index exception.
                    return null;
                }
            }else{
                val2=getProperty(k);
        
                if(val2==null){
                    //Throw an form of property error.
                    return null;
                }
            }
        
            value=StringUtil.replace(value, i, ii+1, val2);
            i=value.indexOf("${");
        }
        
        return value;
	}
	
	public String apply(String key, Properties prop){
		return null;
	}
}
