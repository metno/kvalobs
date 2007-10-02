/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GetDataThread.java,v 1.1.2.3 2007/09/27 09:02:42 paule Exp $                                                       

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

public class GetDataThread extends Thread
{
    private DataIterator it=null;
    private boolean      doExit=false;
    private KvEventQue   eventQue=null;
    private KvApp        app=null;
    private KvDataEventListener listener;
    private boolean       running=true;
    
    public GetDataThread(DataIterator it_, 
                         KvEventQue eventQue_,
                         KvApp        app_,
                         KvDataEventListener listener_ 
    )
    {
        doExit=false;
        eventQue=eventQue_;
        it=it_;
        app=app_;
        listener=listener_;
    }
    
    public synchronized void shutdown(){
        doExit=true;
    }
    
    public synchronized boolean isRunning(){
        return running;
    }
    
    public void run()
    {
        boolean moreData=true;
        
        
        if(it==null){
            System.out.println("GetDataThread: no data iterator!!!!");
            running=false;
            return;
        }
        
        System.out.println("GetDataThread: started!!!!");
        
        ObsDataListHolder dl=new ObsDataListHolder();
        
        while(!doExit && moreData){
            try{
                moreData=it.next(dl);
                
                if(moreData){
                    KvGetDataEvent event=new KvGetDataEvent(this, dl.value, listener);
                    
                    dl.value=null;
                    
                    if(eventQue!=null){
                        eventQue.putEvent(event);
                    }else{
                        //Post to SWING event loop.
                        SwingUtilities.invokeLater(new SwingEvent(app, event));
                    }
                }
                
            }
            catch(Exception ex){
                System.out.println("GetDataThread (next): lost connection to kvalobs!");
                moreData=false;
            }
        }
        
        try{
            it.destroy();
            it=null;
        }
        catch(Exception ex){
            System.out.println("GetDataThread (destroy): lost connection to kvalobs!");
        }
        
        System.out.println("GetDataThread: terminated ....!");
        running=false;
    }
}
