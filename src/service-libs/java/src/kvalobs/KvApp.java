/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvApp.java,v 1.1.2.6 2007/09/27 09:02:41 paule Exp $                                                       

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

import java.io.*;
import org.omg.CORBA.*;
import org.omg.CORBA.Object;
import org.omg.PortableServer.*;
import org.omg.CosNaming.*;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;

import CKvalObs.CService.*;
import CKvalObs.CDataSource.*;
import kvalobs.priv.*;

import java.util.*;
import java.lang.*;

public class KvApp
{
    final private int nGetDataThreads=10;
    private CorbaThread   corbaThread;
    //   private KvEventThread kvEventThread;
    private List          dataNotifyList;
    private List          dataList;
    private List          hintList;
    private List          subscriberidList;
    private List          adminList;
    private kvService     service=null;
    private kvServiceExt  serviceExt=null;
    private CKvalObs.CDataSource.Data  dataInput=null;
    private String        kvServer=null;
    private NamingContextExt nameServer=null;
    private KvEventQue       eventQue=null;    
    private ShutdownHook   hook=null; 
    private boolean        isShutdown=false;
    //GetDataThreads is accessed by this thread and getDataThreadManager.
    //All access that add og delete an entry in the array must be
    //synchronized.
    private GetDataThread[]  getDataThreads=new GetDataThread[nGetDataThreads];
    
    private GetDataThreadManager getDataThreadManager=null;
    private PrintWriter          debuglog=null;
    static public KvApp kvApp=null;
    
    
    /**
     * This is the Application (KvApp) class for the Java interface to
     * the kvalobs sevice API. This is a singleton class. This means
     * that it must be one instance only of this class in the program. 
     * It is the users responsibility to ensure this.
     * <p/>
     * If we are NOT using SWING (usingSwing==false) and we use the push
     * interface to kvalobs, ie we are calling at least one of the functions:
     * subscribeKvDataNotifyListener, subscribeKvDataListener  or 
     * subscribeKvHintListener. We have to start our own eventloop. We do that 
     * with a call to the run function. The run functions executes until 
     * the JVM shutsdown. This happens by sending the JVM the one of the 
     * terminating signals, ie SIGTERM, SIGABORT, SIGINT (Ctrl-C), SIGQUIT,..
     * When we receives the interupt we clean up the connection with kvalobs 
     * and exits.
     * <p/>
     * If we use SWING and want the kvalobs events delivered on the SWING 
     * event que set usingSwing=true. In this case dont call run.
     * 
     * @param args the arguments from the command line.
     * @param kvServer_ the name of kvalobs in the CORBA nameserver, 
     *                  ex kvtest (rime), kvalobs (main), ...
     * @param usingSwing true use swing (DONT call run), false execute our own
     *                   mainloop (call run).
     */
    public KvApp(String[] args, 
                 String kvServer_,
                 boolean usingSwing)
    {
        
        for(int i=0; i<args.length; i++){
            if(args[i].equals("--KvDebug")){
                try{
                    debuglog = new PrintWriter(
                            new FileWriter ("KvDebug.log",false)
                    );
                }
                catch(IOException ex){
                    System.out.println("Cant open debuglogfile <KvDebug.log>!");
                    System.exit(1);
                }
                break;
            }
        }
        
        hook=new ShutdownHook(this);
        
        Runtime.getRuntime().addShutdownHook(hook);
        kvServer=kvServer_;
        kvApp=this;
        dataNotifyList=new LinkedList();
        dataList=new LinkedList();
        hintList=new LinkedList();
        subscriberidList=new LinkedList();
        adminList=new LinkedList();
        
        //We set the daemon flag for the CORBA thread and GetDataThread
        //too ensure that they run while the shutdown is in progress and the
        //ShutdownHook is running.
        
        getDataThreadManager=new GetDataThreadManager(getDataThreads);
        getDataThreadManager.setDaemon(true);
        getDataThreadManager.start();
        
        if(!usingSwing){
            eventQue=new KvEventQue();
            corbaThread=new CorbaThread(args, eventQue, this);
        }else{
            corbaThread=new CorbaThread(args, null, this);
        }
        corbaThread.setDaemon(true);
        System.out.println("CORBA thread deamon: "+corbaThread.isDaemon());
        
        corbaThread.start();
        
        while(!corbaThread.isInitilalized());
        
    }
    
    synchronized void removeAllAdmin(){
    	ListIterator   it;
        NamingContextExt ns=resolveNameServer();
    	
    	if(ns==null)
    		return;
    	
        it=adminList.listIterator();
        
        while(it.hasNext()){
            AdminImpl admin=(AdminImpl)it.next();
            String    nsname=admin.getNsname();
    	
            NameComponent name[];

            try {
				name = ns.to_name(nsname);
				ns.unbind(name);
			}
			catch (NotFound ex) {
				System.out.println("CORBA: removeAllAdmin: '"+nsname+
						           "' not registred in CCORBA name server.");
			}
			catch(InvalidName ex){
            	System.out.println("CORBA: removeAllAdmin: Invalid name '"+nsname+"'!");
            }
            catch(Exception ex){
            	System.out.println("CORBA: removeAllAdmin: Unexpected exception!");
            }
		}
			
		adminList.clear();
    }
    
    synchronized public boolean registerAdmin(String where, IAdmin admin) {
    	NamingContextExt ns=resolveNameServer();
    	
    	if(ns==null)
    		return false;
    	
    	micutil.Admin adm;
    	AdminImpl admImpl=new AdminImpl(admin, where);
    	
    	try{
    		adm= micutil.AdminHelper.narrow(
                      corbaThread.poa.servant_to_reference(admImpl)
                 );
    	}
    	catch(Exception ex){
    		System.out.println("CORBA: Kan ikke initialisere 'Admin'!");
              return false;
    	}
    	
    	NameComponent name[];
    	
    	try{
    		name=ns.to_name(where);
    	}
    	catch(InvalidName ex){
    		System.out.println("CORBA: registerAdmin: Invalid name '"+where+"'!");
    		return false;
    	}
    	catch(Exception ex){
    		System.out.println("CORBA: registerAdmin: Unexpected exception!");
    		return false;
    	}
    	
    	try{
    		ns.bind(name, adm);
    	}
    	catch(AlreadyBound ex){
    		try{
    			ns.rebind(name, adm);
    		}
    		catch(Exception ex1){
    			System.out.println("CORBA: registerAdmin: cant register an 'Admin' interface in the nameserver!");
    			return false;
    		}
    	}
    	catch(Exception ex){
			System.out.println("CORBA: registerAdmin: cant register an 'Admin' interface in the nameserver!");
			return false;
		}
    	
    	adminList.add(admImpl);
    	
    	return true;
	}
    
    synchronized  protected NamingContextExt resolveNameServer()
    {
        try{
            System.out.println("About to resolve the nameserver!");
            nameServer=NamingContextExtHelper.narrow(
                    CorbaThread.orb.resolve_initial_references("NameService")
            );
            System.out.println("The nameserver resolved!");
        }
        catch(Exception ex){
            System.out.println("Can't resolve nameservice!");
            nameServer=null;
            return null;
        }
        
        return nameServer;
    }
    
    synchronized public void setKvServer(String kvserver)
    {
        kvServer=kvserver;
    }
    
    synchronized public String getKvServer()
    {
        return  kvServer;
    }
    
    
    /**
     * onExit is called when the run method is about to return.
     * This function can be used to do some last minute cleanup 
     * before the application terminate.
     */
    protected void onExit(){
    }
    
    
    
    synchronized protected boolean resolveKvService()
    {
        NamingContextExt nc;
        Object           obj=null;
        boolean          resolvedNameServer=false;
        String           server;
        
        if(kvServer!=null){
            server=kvServer+"/kvService";
        }else{
            server="kvalobs/kvService";
        }
        
        System.out.println("Looking up kvService: " + server);
        
        if(nameServer==null){
            if(resolveNameServer()==null)
                return false;
            
            resolvedNameServer=true;
        }
        
        for(int i=0; i<2 && obj==null; i++){
            try{
                obj=nameServer.resolve(
                        nameServer.to_name(server)
                );
                
                if(obj==null){
                    System.out.println("NameServer (1): obj==null");
                }
                
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                System.out.println("NameServer: COMM_FAILURE");
                if(!resolvedNameServer){
                    if(resolveNameServer()==null){
                        nameServer=null;
                        return false;
                    }
                    resolvedNameServer=true;
                    obj=null;
                }else{
                    nameServer=null;
                    return false;
                }
            }
            catch(org.omg.CosNaming.NamingContextPackage.NotFound ex){
                System.out.println("Nameserver: Not found: "+server);
                return false;
            }
            catch(org.omg.CosNaming.NamingContextPackage.InvalidName ex){
                System.out.println("Nameserver: InvalidName: "+server);
                return false;
            }
            catch(Exception ex){
                ex.printStackTrace();
                return false;
            }
        }
        
        System.out.println("NameServer: narrow");
        
        try{
            if(obj==null){
                System.out.println("NameServer: obj==null");
            }
            
            service=kvServiceHelper.narrow(obj);
        }
        catch(Exception ex){
            service=null;
            ex.printStackTrace();
            return false;
        }
        
        return service!=null;
    }

    synchronized protected boolean resolveKvServiceExt()
    {
        NamingContextExt nc;
        Object           obj=null;
        boolean          resolvedNameServer=false;
        String           server;
        
        if(kvServer!=null){
            server=kvServer+"/kvService";
        }else{
            server="kvalobs/kvService";
        }
        
        System.out.println("Looking up kvServiceExt: " + server);
        
        if(nameServer==null){
            if(resolveNameServer()==null)
                return false;
            
            resolvedNameServer=true;
        }
        
        for(int i=0; i<2 && obj==null; i++){
            try{
                obj=nameServer.resolve(
                        nameServer.to_name(server)
                );
                
                if(obj==null){
                    System.out.println("NameServer (1): obj==null");
                }
                
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                System.out.println("NameServer: COMM_FAILURE");
                if(!resolvedNameServer){
                    if(resolveNameServer()==null){
                        nameServer=null;
                        return false;
                    }
                    resolvedNameServer=true;
                    obj=null;
                }else{
                    nameServer=null;
                    return false;
                }
            }
            catch(org.omg.CosNaming.NamingContextPackage.NotFound ex){
                System.out.println("Nameserver: Not found: "+server);
                return false;
            }
            catch(org.omg.CosNaming.NamingContextPackage.InvalidName ex){
                System.out.println("Nameserver: InvalidName: "+server);
                return false;
            }
            catch(Exception ex){
                ex.printStackTrace();
                return false;
            }
        }
        
        System.out.println("NameServer: narrow");
        
        try{
            if(obj==null){
                System.out.println("NameServer: obj==null");
            }
            
            serviceExt=kvServiceExtHelper.narrow(obj);
        }
        catch(Exception ex){
            service=null;
            ex.printStackTrace();
            return false;
        }
        
        return serviceExt!=null;
    }
    
    
    synchronized protected boolean resolveKvDataInput()
    {
        NamingContextExt nc;
        Object           obj=null;
        boolean          resolvedNameServer=false;
        String           server;
        
        if(kvServer!=null){
            server=kvServer+"/kvinput";
        }else{
            server="kvalobs/kvinput";
        }
        
        System.out.println("Looking up kvinput: " + server);
        
        if(nameServer==null){
            if(resolveNameServer()==null)
                return false;
            
            resolvedNameServer=true;
        }
        
        for(int i=0; i<2 && obj==null; i++){
            try{
                obj=nameServer.resolve(
                        nameServer.to_name(server)
                );
                
                if(obj==null){
                    System.out.println("NameServer (1): obj==null");
                }
                
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                System.out.println("NameServer: COMM_FAILURE");
                if(!resolvedNameServer){
                    if(resolveNameServer()==null){
                        nameServer=null;
                        return false;
                    }
                    resolvedNameServer=true;
                    obj=null;
                }else{
                    nameServer=null;
                    return false;
                }
            }
            catch(org.omg.CosNaming.NamingContextPackage.NotFound ex){
                System.out.println("Nameserver: Not found: " +server);
                return false;
            }
            catch(org.omg.CosNaming.NamingContextPackage.InvalidName ex){
                System.out.println("Nameserver: InvalidName: "+server);
                return false;
            }
            catch(Exception ex){
                ex.printStackTrace();
                return false;
            }
        }
        
        System.out.println("NameServer: narrow");
        
        try{
            if(obj==null){
                System.out.println("NameServer: obj==null");
            }
            
            dataInput=DataHelper.narrow(obj);
        }
        catch(Exception ex){
            service=null;
            ex.printStackTrace();
            return false;
        }
        
        return dataInput!=null;
    }
    
    
    
    /**
     * subscribe to the KvDataNotify event.
     *
     * @param subscribeInfo  which stations are we interested to receives 
     *                       event from.
     * @param listener  the endpoint we want the event delivred to.
     * @return null when we cant register with kvalobs. And a subscriber id
     *         on success.
     * 
     */
    synchronized public String subscribeKvDataNotifyListener(
                                                             KvDataSubscribeInfo subscribeInfo, 
                                                             KvDataNotifyEventListener listener)
    {
        boolean resolvedKvService=false;
        String subscriberid=null;
        
        if(listener==null){
            System.out.println("subscribeKvDataNotifyListener: no listener!)");
            return null;
        }
        
        if(subscribeInfo==null){
            subscribeInfo=new KvDataSubscribeInfo();
        }
        
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("subscribeKvDataNotifyListener: " +
                "Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                subscriberid=service.subscribeDataNotify(
                        subscribeInfo.getInfo(), 
                        CorbaThread.dataNotifySubscriber
                );
                break;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("subscribeKvDataNotifyListener: "+
                        "Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("subscribeKvDataNotifyListener: "+
                    "Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        
        if(subscriberid.length()==0){
            System.out.println("subscribeKvDataNotifyListener: "+
            "cant subscribe to 'dataNotify'");
            return null;
        }
        
        dataNotifyList.add(new SubscriberInfo(subscriberid, listener));
        
        return subscriberid;
    }
    
    /**
     * subscribe to the KvData event.
     *
     * @param subscribeInfo  which stations are we interested to receives 
     *                       event from.
     * @param listener  the endpoint we want the event delivred to.
     * @return null when we cant register with kvalobs. And a subscriber id
     *         on success.
     * 
     */
    synchronized public String subscribeKvDataListener(
                                                       KvDataSubscribeInfo subscribeInfo,
                                                       KvDataEventListener listener)
    {
        boolean resolvedKvService=false;
        String subscriberid=null;
        
        if(listener==null){
            System.out.println("subscribeKvDataListener: no listener!)");	    
            return null;
        }
        
        if(subscribeInfo==null){
            subscribeInfo=new KvDataSubscribeInfo();
        }
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("subscribeKvDataListener: "+
                "Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                subscriberid=service.subscribeData(
                        subscribeInfo.getInfo(), 
                        CorbaThread.dataSubscriber
                );
                break;
                
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("subscribeKvDataListener: "+
                        "Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("subscribeKvDataListener: "+
                    "Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        
        if(subscriberid.length()==0){
            System.out.println("subscribeKvDataListener: "+
            "cant subscribe to 'dataNotify'");
            return null;
        }
        
        dataList.add(new SubscriberInfo(subscriberid, listener));
        
        return subscriberid;
    }
    
  
    
    
    /**
     * subscribe on KvData event.
     * 
     * Use this function to register an listener to kvalobs when
     * we now the subscriberid. The subscriberid must have prevously
     * been set up by an kvalobs administrator. 
     *
     * @param subscriberid  An preallocated subscriberid.
     * @param listener  the endpoint we want the event delivred to.
     * @return null when we cant register with kvalobs. And a subscriber id
     *         on success.
     * 
     */
    synchronized public String registerKvDataListener(
    		String subscriberid,
            KvDataEventListener listener)
    {
        boolean resolvedKvService=false;
                
        if(listener==null){
            System.out.println("registerKvDataListener: no listener!)");	    
            return null;
        }
        
                
        if(serviceExt==null){
            if(!resolveKvServiceExt()){
                System.out.println("registerKvDataListener: "+
                "Cant connect to kvServiceExt!");
                return null;
            }else{
                resolvedKvService=true;
            }
        }
        
        String subid="";
        
        for(int i=0; i<2 && serviceExt!=null; i++){
            try{
                subid=serviceExt.registerData(
                			subscriberid, 
                        CorbaThread.dataSubscriberExt
                );
                break;
                
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvServiceExt()){
                        serviceExt=null;
                        System.out.println("registerKvDataListener: "+
                        "Cant connect to kvServiceExt!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    serviceExt=null;
                    System.out.println("registerKvDataListener: "+
                    "Cant connect to kvServiceExt!");
                    return null;
                }
            }
            catch(Exception ex){
                serviceExt=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        
        if(subid.length()==0){
            System.out.println("registerKvDataListener: "+
            "cant subscribe to 'registerData'");
            return null;
        }
        
        dataList.add(new SubscriberInfo(subscriberid, listener));
        
        return subscriberid;
    }
  
    /**
     * subscribe to the KvHint event. 
     * kvalobs sends a hint when it is started to all who has registred 
     * an interest in this status. It will also send a hint when it is 
     * stopped normaly. If kvalobs is going down abnormally this signal 
     * may or may not be sendt.
     *
     * @param listener  the endpoint we want the event delivred to.
     * @return null when we cant register with kvalobs. And a subscriber id
     *         on success.
     * 
     */
    
    synchronized public String subscribeKvHintListener(KvHintEventListener listener)
    {
        boolean resolvedKvService=false;
        String subscriberid=null;
        
        if(listener==null){
            System.out.println("subscribeKvHintListener: no listener!)");
            return null;
        }
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("subscribeKvHintListener: "+
                "Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                subscriberid=service.subscribeKvHint(CorbaThread.hintSubscriber);
                break;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("subscribeKvHintListener: "+
                        "Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("subscribeKvHintListener: "+
                    "Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        
        if(subscriberid.length()==0){
            System.out.println("subscribeKvHintListener: "+
            "cant subscribe to 'dataNotify'");
            return null;
        }
        
        hintList.add(new SubscriberInfo(subscriberid, listener));
        
        return subscriberid;
    }
    
    synchronized public boolean unsubscribeKv(String subscriberid)
    {
        ListIterator it;
        SubscriberInfo subInfo;
        boolean resolvedKvService=false;
        
        if(subscriberid==null){
            System.out.println("unsubscribeKv: no subscriberid!)");
            return false;
        }
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("unsubscribeKv: "+
                "Cant connect to kvService!");
                return false;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                service.unsubscribe(subscriberid);
                break;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("unsubscribeKv: "+
                        "Cant connect to kvService!");
                        return false;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("unsubscribeKv: "+
                    "Cant connect to kvService!");
                    return false;
                }
            }
            catch(org.omg.CORBA.TRANSIENT e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("unsubscribeKv: "+
                        "Cant connect to kvService!");
                        return false;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("unsubscribeKv: "+
                    "Cant connect to kvService!");
                    return false;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return false;
            }
        }
        
        
        it=dataNotifyList.listIterator();
        
        while(it.hasNext()){
            subInfo=(SubscriberInfo)it.next();
            
            if(subInfo.subscriberid.compareTo(subscriberid)==0){
                it.remove();
                return true;
            }
        }
        
        it=dataList.listIterator();
        
        while(it.hasNext()){
            subInfo=(SubscriberInfo)it.next();
            
            if(subInfo.subscriberid.compareTo(subscriberid)==0){
                it.remove();
                return true;
            }
        }
        
        it=hintList.listIterator();
        
        while(it.hasNext()){
            subInfo=(SubscriberInfo)it.next();
            
            if(subInfo.subscriberid.compareTo(subscriberid)==0){
                it.remove();
                return true;
            }
        }
        
        return true;
    }
    
    synchronized public boolean unsubscribeKvAll()
    {
        ListIterator   it;
        SubscriberInfo subInfo;
        
        it=dataNotifyList.listIterator();
        
        while(it.hasNext()){
            subInfo=(SubscriberInfo)it.next();
            
            if(!unsubscribeKv(subInfo.subscriberid))
                return false;
            else
                it=dataNotifyList.listIterator();
        }
        
        it=dataList.listIterator();
        
        while(it.hasNext()){
            subInfo=(SubscriberInfo)it.next();
            
            if(!unsubscribeKv(subInfo.subscriberid))
                return false;
            else
                it=dataList.listIterator();
        }
        
        it=hintList.listIterator();
        
        while(it.hasNext()){
            subInfo=(SubscriberInfo)it.next();
            
            if(!unsubscribeKv(subInfo.subscriberid))
                return false;
            else
                it=hintList.listIterator();
        }
        
        return true;
        
    }
    
    
    synchronized public void postKvEvent(KvEvent event)
    {
        int i;
        String key;
        String eventName;
        ListIterator it;
        SubscriberInfo info;
        
        eventName=event.toString();
        
        i=eventName.indexOf('@');
        
        if(i<0){
            System.out.println("PostKvEvent: Invalid format missing @.");
            return;
        }
        
        key=eventName.substring(0, i);
        
        if(key.compareTo("KvDataEvent")==0){
            KvDataEventListener dataEventListener;
            
            it=dataList.listIterator();
            
            while(it.hasNext()){
                info=(SubscriberInfo)it.next();
                dataEventListener=(KvDataEventListener)info.listener;
                dataEventListener.kvDataEvent((KvDataEvent)event);
            }
        }else if(key.compareTo("KvDataNotifyEvent")==0 || 
        		  key.compareTo("KvDataNotifyExtEvent")==0){
            KvDataNotifyEventListener dataNotifyEventListener;
            it=dataNotifyList.listIterator();
            
            while(it.hasNext()){
                info=(SubscriberInfo)it.next();
                dataNotifyEventListener=(KvDataNotifyEventListener)info.listener;
                dataNotifyEventListener.kvDataNotifyEvent(
                        (KvDataNotifyEvent)event
                );
            }
        }else if(key.compareTo("KvHintEvent")==0){
            KvHintEventListener hintEventListener;
            KvHintEvent         hint=(KvHintEvent)event;
            
            it=hintList.listIterator();
            
            while(it.hasNext()){
                info=(SubscriberInfo)it.next();
                hintEventListener=(KvHintEventListener)info.listener;
                
                if(hint.getCommingUp())
                    hintEventListener.up();
                else
                    hintEventListener.down();
            }
        }else if(key.compareTo("KvGetDataEvent")==0){
            KvDataEventListener dataEventListener;
            KvGetDataEvent getDataEvent=(KvGetDataEvent)event;
            
            dataEventListener=getDataEvent.getListener();
            
            if(dataEventListener!=null){
                dataEventListener.kvDataEvent((KvDataEvent)event);
            }
        }else{
            System.out.println("PostKvEvent: Unknown KvEvent (" + key +")!");
            return;
        }
    }
    
    public CKvalObs.CService.Station[] getKvStations()
    {
        boolean resolvedKvService=false;
        StationListHolder stationList=new StationListHolder();
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                if(!service.getStations(stationList)){
                    System.out.println("kvService cant deliver the 'stationlist'");
                    return null;
                }else
                    return stationList.value;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        return null;
    }
    
    public CKvalObs.CService.Param[] getKvParams()
    {
        boolean resolvedKvService=false;
        ParamListHolder paramList=new ParamListHolder();
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("getKvParams: Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                if(!service.getParams(paramList)){
                    System.out.println("getKvParams: kvService cant deliver the 'paramlist'");
                    return null;
                }else
                    return paramList.value;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("getKvParams: Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("getKvParams: Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        return null;
    }
    
    public CKvalObs.CService.Types[] getKvTypes()
    {
        boolean resolvedKvService=false;
        TypeListHolder typeList=new TypeListHolder();
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("getKvTypes: Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                if(!service.getTypes(typeList)){
                    System.out.println("getKvTypes: kvService cant deliver the 'typelist'");
                    return null;
                }else
                    return typeList.value;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("getKvTypes: Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("getKvTypes: Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        return null;
    }
    
    /**
     * Get station_param from kvalobs database. 
     *
     * @param stationid The stationid for whihc to et param
     * @param paramid paramid to get. If paramid is less than 0, all
     * paramid will be fetched.
     * @param day Day of year. If day is less than 0: all days.
     *
     * @return An array containing the specified station parameters.
     */
    public CKvalObs.CService.Station_param[] getKvStationParam( int stationid,
                                                                int paramid,
                                                                int day ) {
        boolean resolvedKvService = false;
        Station_paramListHolder spList = new Station_paramListHolder();
        
        if ( service == null ){
            if ( ! resolveKvService() ){
                System.out.println("getKvStationParam: Cant connect to kvService!");
                return null;
            }
            else
                resolvedKvService = true;
        }
        
        for ( int i = 0; i < 2 && service != null; i++ ) { 
            try {
                if ( ! service.getStationParam( spList, stationid, paramid, day ) ) {
                    System.out.println("getKvStationParam: kvService cant deliver the station_param list.");
                    return null;
                } 
                else
                    return spList.value;
            }
            catch( org.omg.CORBA.COMM_FAILURE e ){
                if ( ! resolvedKvService ) {
                    if ( ! resolveKvService() ) {
                        service = null;
                        System.out.println("getKvStationParam: Cant connect to kvService!");
                        return null;
                    }
                    else
                        resolvedKvService = true;
                }
                else {
                    service = null;
                    System.out.println("getKvStationParam: Cant connect to kvService!");
                    return null;
                }
            }
            catch( Exception ex ){
                service = null;
                ex.printStackTrace();
                return null;
            }
        }
        return null;
    }
    
    public RejectedIterator getKvRejectdecode( RejectDecodeInfo decodeInfo ) {
        boolean resolvedKvService = false;
        RejectedIteratorHolder rejectedIterator = new RejectedIteratorHolder();
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("getKvRejectdecode: Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        for ( int i = 0; i < 2 && service != null; i++ ) { 
            try {
                if ( ! service.getRejectdecode( decodeInfo, rejectedIterator ) ) {
                    System.out.println("getKvRejectDecode: kvService cant deliver the Rejectdecode iterator");
                    return null;
                } 
                else
                    return rejectedIterator.value;
            }
            catch( org.omg.CORBA.COMM_FAILURE e ) {
                if ( ! resolvedKvService ) {
                    if ( ! resolveKvService() ) {
                        service = null;
                        System.out.println("getKvRejectDecode: Cant connect to kvService!");
                        return null;
                    }
                    else
                        resolvedKvService = true;
                }
                else {
                    service = null;
                    System.out.println("getKvRejectDecode: Cant connect to kvService!");
                    return null;
                }
            }
            catch( Exception ex ){
                service = null;
                ex.printStackTrace();
                return null;
            }
        }
        return null;
    }
    
    public CKvalObs.CService.Obs_pgm[] getKvObsPgm(int[]stationIDList,
                                                   boolean aUnion)
    {
        boolean resolvedKvService=false;
        Obs_pgmListHolder obs_pgmList=new Obs_pgmListHolder(); 
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("getKvObsPgm: Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        if(stationIDList==null){
            stationIDList=new int[0];
        }
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                if(!service.getObsPgm(obs_pgmList, stationIDList, aUnion)){
                    System.out.println("getKvObsPgm: kvService cant deliver the 'obsPgmList'");
                    return null;
                }else
                    return obs_pgmList.value;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("getKvObsPgm: Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("getKvObsPgm: Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        return null;
        
    }
    
    /**
     * getKvData, starts a background thread that pulls data from kvalobs.
     * Use WhitchData to specify the stations you want data from. A stationid of 0 
     * give you the data from all stations in kvalobs. 
     *
     * You must also specify a KvDataEventListner that the data shold pe posted to.
     *
     * @param whichData  the stations and period we want data for.
     * @param listener  where we want the data delivred!
     * @return true on success and false otherwise.
     */
    
    synchronized public boolean getKvData(CKvalObs.CService.WhichData[] whichData,
                                          KvDataEventListener listener)
    {
        boolean            resolvedKvService=false;
        DataIteratorHolder it=new DataIteratorHolder();
        boolean            ok=false;
        int                threadSlot=-1;
        
        
        if(listener==null){
            System.out.println("getKvData: No listener (listener==null)!");
            return false;
        }
        
        //Find an empty slot in the getDataThreads array.
        
        synchronized(getDataThreads){
            for(int i=0; i<nGetDataThreads; i++){
                if(getDataThreads[i]==null){
                    threadSlot=i;
                    break;
                }
            }
        }
        
        if(threadSlot<0){
            System.out.println("getKvData: to many getKvData in progress!");
            return false;
        }
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("getKvData: Cant connect to kvService!");
                return false;
            }else
                resolvedKvService=true;
        }
        
        for(int i=0; i<2 && service!=null && !ok; i++){
            try{
                if(!service.getData(whichData, it)){
                    System.out.println("getKvData: kvService cant deliver the 'DataIterator'");
                    return false;
                }else{
                    ok=true;
                }
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("getKvData: Cant connect to kvService!");
                        return false;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("getKvData: Cant connect to kvService!");
                    return false;
                }
            }
            catch(Exception ex){
                service=null;
                ex.printStackTrace();
                return false;
            }
        }
        
        if(!ok)
            return false;
        
        try{
            synchronized(getDataThreads){
                getDataThreads[threadSlot]=new GetDataThread(it.value, eventQue, 
                        this, listener);
            }
            
            getDataThreads[threadSlot].start();
            
        }
        catch(IllegalThreadStateException ex){
            //This shoul never happend.
            System.out.println("DEBUG: the getKvData thread is allready running!");
            return true;
        }
        catch(Exception ex){
            System.out.println("getKvData: NO MEMMORY!");
            
            synchronized(getDataThreads){
                getDataThreads[threadSlot]=null;
            }
            
            try{
                it.value.destroy();
            }
            catch(Exception dummy){
            }
            return false;
        }
        
        return true;
    }
    
    
    /**
     * getKvData, start a pull of data from kvalobs. An iterator that is used 
     * to pull the data from kvalobs i returned. Rember to call 'destroy' on the
     * iterator when all the data is received. The iterator consumes reources on the
     * kvalobs server, this resources is released when 'destroy' is called.
     *
     * Use WhitchData to specify the stations you want data from. A stationid of 0 
     * give you the data from all stations in kvalobs. 
     *
     * @param whichData  the stations and period we want data for.
     * @return KvDataIterator to pull the data from kvalobs or null if failed.
     */
    
    public KvDataIterator getKvData(CKvalObs.CService.WhichData[] whichData)
    {
        boolean resolvedKvService=false;
        DataIteratorHolder it=new DataIteratorHolder();
        
        if(service==null){
            if(!resolveKvService()){
                System.out.println("getKvData: Cant connect to kvService!");
                return null;
            }else
                resolvedKvService=true;
        }
        
        for(int i=0; i<2 && service!=null; i++){
            try{
                if(!service.getData(whichData, it)){
                    System.out.println("getKvData: kvService cant deliver the 'DataIterator'");
                    return null;
                }else{
                    return new KvDataIterator(it.value);
                }
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvService){
                    if(!resolveKvService()){
                        service=null;
                        System.out.println("getKvData: Cant connect to kvService!");
                        return null;
                    }else
                        resolvedKvService=true;
                }else{
                    service=null;
                    System.out.println("getKvData: Cant connect to kvService!");
                    return null;
                }
            }
            catch(Exception ex){
                service=null;
                
                if(debuglog!=null){
                    ex.printStackTrace(debuglog);
                    debuglog.flush();
                }else{
                    ex.printStackTrace();
                }
                
                return null;
            }
        }
        
        return null;
    }
    
    /**
     * sendDataToKv, sends data to kvalobs, kvDataInputd. ObsType is used
     * to tell kvalobs what format the data is coded in. It is used by
     * kvDataInputd to select the proper decoder to be used to decode the data.
     * <p\>
     * 
     * The method return a Result. The Result has two fields.
     * <pre>
     * - EResult res
     * - string  message
     *
     *  The value of res:
     *     NODECODER,   there is no decoder for the obsType.
     *                  The observation is not saved to the database. Don't
     *                  mind to retry to send the observation until a
     *                  decoder is written and installed.
     *     DECODEERROR, cant decode the message. The
     *                  message is saved to rejectdecode.
     *     NOTSAVED,    the message is not SAVED to the database,
     *                  if possible try to resend it later, after 
     *                  a delay.
     *     ERROR,       A general error. Look at the 'message'. The
     *                  observation is not saved to the database.
     *     OK           The message is decoded and saved to the 
     *                  database.
     * </pre>
     *  If the value of res is NOT OK a error message is written in message.
     *
     * @param data the data coded in the format given with obsType.
     * @param obsType the format of the data.
     * @return A reference to a Result if we successfully connected to kvinput.
     *         null if we failed to connect with kvinput. kvinput may be down 
     *         or the CORBA nameserver may be down.
     */
    
    protected CKvalObs.CDataSource.Result sendDataToKv(String data, String obsType)
    {
        CKvalObs.CDataSource.Result result=null;
        boolean resolvedKvinput=false;
        //	DataIteratorHolder it=new DataIteratorHolder();
        
        if(dataInput==null){
            if(!resolveKvDataInput()){
                System.out.println("sendDataToKv: Cant connect to kvinput!");
                return null;
            }else
                resolvedKvinput=true;
        }
        
        for(int i=0; i<2 && dataInput!=null; i++){
            try{
                result=dataInput.newData(data, obsType);
                return result;
            }
            catch(org.omg.CORBA.COMM_FAILURE e){
                if(!resolvedKvinput){
                    if(!resolveKvDataInput()){
                        dataInput=null;
                        System.out.println("sendDataToKv: Cant connect to kvinput!");
                        return null;
                    }else
                        resolvedKvinput=true;
                }else{
                    dataInput=null;
                    System.out.println("sendDataToKv: Cant connect to kvinput!");
                    return null;
                }
            }
            catch(Exception ex){
                dataInput=null;
                ex.printStackTrace();
                return null;
            }
        }
        
        return null;
    }
    
    /**
     * sendKlDataToKv, sends data to kvalobs, kvDataInputd. It sends
     * data to be decoded by the 'kldecoder' in kvDataInputd. 
     * <p\>
     * The dataformat is:  
     *  <pre>
     *  pc1[(&lt;sensor>,&lt;level>)],...,pcN[(&lt;sensor>,&lt;level>)]
     *  YYYYMMDDhhmmss,pc1_value,....,pcN_value
     *  YYYYMMDDhhmmss,pc1_value,....,pcN_value
     *  ....
     *  YYYYMMDDhhmmss,pc1_value,....,pcN_value
     *
     *  pc - paramcode, the name of the parameter. An underscore indicate that 
     *                  this is a code value. Suported pc that can have a code 
     *                  value is: HL and VV. The value vil be converted til 
     *                  meter.
     *  If sensor or level is not specified. The default would apply. If both 
     *  shall take the default value, the paranteses can be left out.
     * </pre>
     *
     * The method return a Result. The Result has two fields.
     * <pre>
     * - EResult res
     * - string  message
     *
     *  The value of res:
     *     NODECODER,   there is no decoder for the obsType.
     *                  The observation is not saved to the database. Don't
     *                  mind to retry to send the observation until a
     *                  decoder is written and installed.
     *     DECODEERROR, cant decode the message. The
     *                  message is saved to rejectdecode.
     *     NOTSAVED,    the message is not SAVED to the database,
     *                  if possible try to resend it later, after 
     *                  a delay.
     *     ERROR,       A general error. Look at the 'message'. The
     *                  observation is not saved to the database.
     *     OK           The message is decoded and saved to the 
     *                  database.
     * </pre>
     *  If the value of res is NOT OK a error message is written in message.
     *
     * @param data the data coded in the format expected by the kldecoder.
     * @param nationalid the nationalid to the data.
     * @param typeid     the typeid for the data.
     * @return A reference to a Result if we successfully connected to kvinput.
     *         null if we failed to connect with kvinput. kvinput may be down 
     *         or the CORBA nameserver may be down.
     */
    
    public CKvalObs.CDataSource.Result sendKlDataToKv(String data, 
                                                      long nationalid,
                                                      long typeid)
    {
        String obsType="kldata/nationalnr="+Long.toString(nationalid)+
        "/type="+Long.toString(typeid);
        
        return  sendDataToKv(data, obsType);
    }
    
    
    public void run()
    {
        KvEvent event=null;
        
        if(eventQue==null){
            System.out.println("WARNING: We are using the event que to SWING!");
            return;
        }
        
        
        while(!isShutdown){
            try{
                event=eventQue.getEvent(1000);
                
                if(event==null)
                    continue;
                
                try{
                    System.out.println("KvApp: call postKvEvent!");
                    postKvEvent(event);
                    System.out.println("KvApp: return postKvEvent!");
                }
                catch(Exception ex){
                    if(debuglog!=null){
                        debuglog.println("KvApp::run: Exception in postKvEvent!");
                        ex.printStackTrace(debuglog);
                        debuglog.flush();
                    }else{
                        System.out.println("KvApp::run: Exception in postKvEvent!");
                        ex.printStackTrace();
                    }
                    
                    //throw away all uncaught exception from "user" code.
                }
            }
            catch(Exception ex){
                if(debuglog!=null){
                    debuglog.println("KvApp::run: Exception in getEvent!");
                    ex.printStackTrace(debuglog);
                    debuglog.flush();
                }else{
                    System.out.println("KvApp::run: Exception in getEvent!");
                    ex.printStackTrace();
                }
            }
        }
        
        System.out.println("KvApp::run: terminating the eventque!");
    }
    
    
    public synchronized void exit(){
        
        System.out.println("KvApp.exit() called!");
        
        if(isShutdown)
            return;
        
        System.out.println("KvApp.exit() called (Shutdown)!");
        
        
        unsubscribeKvAll();
        removeAllAdmin();
        corbaThread.shutdown();
        getDataThreadManager.shutdown();
        
        boolean retry=true;
        
        
        while(retry){
            retry=false;
            
            try{
                getDataThreadManager.join();
                System.out.println("exit: getDataThreadManager termineted!");
            }
            catch(InterruptedException e){
                System.out.println("exit (getDataThreadManager): join iterupted!");
                retry=true;
            }
            
        }
        
        retry=true;
        
        while(retry){
            retry=false;
            
            try{
                corbaThread.join();
                System.out.println("exit: CorbaThread termineted!");
            }
            catch(InterruptedException e){
                System.out.println("exit (corbaThread): join iterupted!");
                retry=true;
            }
            
        }
        
        try{
            // It is two scenaries where exit is called.
            //
            // 1. The "user" code call this function.
            // 2. We are called from the ShutdownHook.
            //
            // In 1 we want to remove the hook so that the function is not
            // called a second time from the hook. There is now way to test
            // if we JWM is in shutdown. So we just call removeShutdownHook
            // and let the JVM decide if we can remove the hook. If we cant
            // it will throw IllegalStateException, but that is just fine.
            // 
            if(hook!=null){
                Runtime.getRuntime().removeShutdownHook(hook);
                hook=null;
            }
        }
        catch(IllegalStateException ex){
            System.out.println("IllegalStateException: exit: This is ok!");
            //The JVM is in the process to shutdown. We cant remove the hook,
            //but that is ok.
        }
        catch(SecurityException ex){
            System.out.println("SecurityException: exit: Hmmm!");
            //We are not allowed to do this.
        }
        
        onExit();
        isShutdown=true;
    }
}


