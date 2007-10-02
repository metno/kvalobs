/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CorbaThread.java,v 1.1.2.4 2007/09/27 09:02:41 paule Exp $                                                       

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
import org.omg.CORBA.*;
import org.omg.CORBA.Object;
import org.omg.PortableServer.*;
import org.omg.CosNaming.*;
import kvalobs.*;

public class CorbaThread extends Thread
{
    static public ORB orb=null;
    static public POA poa=null;
    static public kvDataSubscriber          dataSubscriber=null;
    static public kvDataSubscriberExt       dataSubscriberExt=null;
    static public kvDataNotifySubscriber    dataNotifySubscriber=null;
    static public kvDataNotifySubscriberExt dataNotifySubscriberExt=null;
    static public kvHintSubscriber       hintSubscriber=null;
    private String[]                     args=null;
    private boolean                      isInit=false;
    private KvEventQue                   eventQue=null;
    private KvApp                        app=null;
    
    public CorbaThread(String[] args_, KvEventQue eventQue_, KvApp app_)
    {
        args=args_;
        eventQue=eventQue_;
        app=app_;
    }
    
    public void run()
    {
        orb = ORB.init(args, null);
        
        try{
            poa=POAHelper.narrow(orb.resolve_initial_references("RootPOA"));
            poa.the_POAManager().activate();
            
            dataSubscriber= kvDataSubscriberHelper.narrow(
                    poa.servant_to_reference(
                            new kvDataSubscriberImpl(eventQue, 
                                    app)
                    )
            );
            
            dataSubscriberExt= kvDataSubscriberExtHelper.narrow(
                    poa.servant_to_reference(
                            new kvPsDataSubscriberImpl(eventQue, 
                                    app)
                    )
            );
            
            dataNotifySubscriber= kvDataNotifySubscriberHelper.narrow(
                    poa.servant_to_reference(
                            new kvDataNotifySubscriberImpl(eventQue,
                                    app)
                    )
            );
            
            
            dataNotifySubscriberExt= kvDataNotifySubscriberExtHelper.narrow(
                    poa.servant_to_reference(
                            new kvPsDataNotifySubscriberImpl(eventQue,
                                    app)
                    )
            );
            
            hintSubscriber= kvHintSubscriberHelper.narrow(
                    poa.servant_to_reference(
                            new kvHintSubscriberImpl(eventQue,
                                    app)
                    )
            );
            
        }
        catch(Exception ex){
            System.out.println("Kan ikke initialisere CORBA!");
            return;
        }
        
        isInit=true;
        orb.run();
        System.out.println("CORBA thread terminated!");
        
    }
    
    public synchronized boolean isInitilalized()
    {
        return isInit;
    }
    
    public synchronized void shutdown(){
        orb.shutdown(false);
    }
}
