/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvDataIterator.java,v 1.1.2.3 2007/09/27 09:02:41 paule Exp $                                                       

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
package kvalobs;

import CKvalObs.CService.*;


public class KvDataIterator
{
    private DataIterator it;
    
    KvDataIterator(DataIterator it_)
    {
        it=it_;
    }
    
    /**
     * next returnerer en ObsData[]. Og null n�r alle dataene er hentet.
     *
     * @exception KvNoConnection hvis kontakten med kvalobs er mistet.
     */ 
    public CKvalObs.CService.ObsData[] next() throws KvNoConnection
    {
        ObsDataListHolder dl=new ObsDataListHolder();
        
        if(it==null){
            System.out.println("KvDataIterator: no iterator!!!!");
            return null;
        }
        
        try{
            if(it.next(dl)){
                return dl.value;
            }else{
                it.destroy();
                it=null;
                return null;
            }
        }
        catch(Exception ex){
            throw new KvNoConnection();
        }
    }
    
    /**
     * Kall destroy n�r KvDataIteratoren ikke trengs lenger dvs n�r alle
     * data er hentet.
     */
    
    public void destroy()
    {
        try{
            if(it!=null)
                it.destroy();
        }
        catch(Exception ex){
            System.out.println("KvDataIterator.destroy:"
                    +"Lost connection to kvalobs!");
        }
        
        it=null;
    }
    
}
