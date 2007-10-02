/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GetDataThreadManager.java,v 1.1.2.3 2007/09/27 09:02:42 paule Exp $                                                       

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
import CKvalObs.CService.*;
import kvalobs.*;
import javax.swing.*;

public class GetDataThreadManager extends Thread
{
    private GetDataThread[] threads=null;
    private boolean         doExit=false;
    
    public GetDataThreadManager(GetDataThread[] threads_){
        threads=threads_;
    }
    
    public synchronized void shutdown(){
        doExit=true;
    }
    
    
    public void run()
    {
        if(threads==null){
            System.out.println("GetDataThreadManager: No thread array!");
            return;
        }
        
        System.out.println("GetDataThreadManager: Is started!");
        
        while(!doExit){
            try{
                sleep(1000); // sleep 1 second
            }
            catch(InterruptedException e){
                //Do nothing
            }
            
            synchronized(threads){
                for(int i=0; i<threads.length; i++){
                    if(threads[i]!=null){
                        if(!threads[i].isRunning()){
                            boolean retry=true;
                            
                            System.out.println("getDataThreadManager: an KvGetData background thread is terminating!");
                            
                            while(retry){
                                retry=false;
                                
                                try{
                                    threads[i].join();
                                    threads[i]=null;
                                }
                                catch(InterruptedException e){
                                    System.out.println("exit: join iterupted!");
                                    retry=true;
                                }
                            }
                        }
                    }
                }
            }
        }
        
        synchronized(threads){
            for(int i=0; i<threads.length; i++){
                if(threads[i]!=null){
                    threads[i].shutdown();
                }
            }
            
            for(int i=0; i<threads.length; i++){
                if(threads[i]!=null){
                    boolean retry=true;
                    
                    while(retry){
                        retry=false;
                        
                        try{
                            threads[i].join();
                            threads[i]=null;
                        }
                        catch(InterruptedException e){
                            System.out.println("exit: join iterupted!");
                            retry=true;
                        }
                    }
                    
                }
            }
        }
        
        System.out.println("GetDataThreadManager: terminating!");
    }
    
}
