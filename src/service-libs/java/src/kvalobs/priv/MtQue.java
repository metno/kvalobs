/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MtQue.java,v 1.1.2.4 2007/09/27 09:02:42 paule Exp $                                                       

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
package kvalobs.priv;

import java.util.*;


public class MtQue
{
    private List que;
    
    public MtQue()
    {
        que=new LinkedList();
    }
    
    public Object getObject(int waitInMillisecond)
    {
        synchronized(que){
            if(!que.isEmpty()){
                System.out.println("MtQue::getObject: que.size="+que.size());
            }
            
            if(que.isEmpty()){
                try{
                    que.wait(waitInMillisecond);
                }
                catch(IllegalArgumentException e){
                    System.out.println("MtQue::getObject: invalid timeout value. (" +
                            waitInMillisecond +")");
                }
                catch(IllegalMonitorStateException e){
                    System.out.println("MtQue::getObject: Not owner of the monitor (Mutex)");
                }
                catch(InterruptedException e){
                    System.out.println("MtQue::getObject: Interupted");
                }
                catch(Exception ex){
                    System.out.println("MtQue::getObject: Unexpected exception!");
                }
                
                if(que.isEmpty())
                    return null;
            }
            
            try{
                return que.remove(0);
            }
            catch(Exception ex){
                System.out.println("MtQue::getObject: No objects in que.!!!");
                return null;
            }
        }
    }
    
    public void putObject(Object obj)
    {
        if(obj==null){
            System.out.println("MtQue::putObject: obj==null" );
            return;
        }
        
        synchronized(que){
            if(!que.isEmpty()){
                System.out.println("MtQue::putObject: que.size="+que.size());
            }
            
            
            try{
                que.add(obj);
                que.notifyAll();
            }
            catch(IllegalMonitorStateException ex){
                System.out.println("MtQue::putObject: IllegalMonitorStateException");
            }
            catch(UnsupportedOperationException ex){
                System.out.println("MtQue::putObject: UnsupportedOperationException");
            }
            catch(ClassCastException ex){
                System.out.println("MtQue::putObject: ClassCastException");
                
            }
            catch(NullPointerException ex){
                System.out.println("MtQue::putObject: NullPointerException");
            }
            catch(IllegalArgumentException ex){
                System.out.println("MtQue::putObject: IllegalArgumentException");
            }
            catch(Exception e){
                e.printStackTrace();
            }
        }
    }
    
    public synchronized int size() {
			return que.size();
	}
}
