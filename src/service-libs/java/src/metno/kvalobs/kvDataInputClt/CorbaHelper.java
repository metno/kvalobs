package metno.kvalobs.kvDataInputClt;

import org.omg.CORBA.*;
import org.omg.CORBA.Object;
import org.omg.PortableServer.*;
import org.omg.CosNaming.*;
import org.omg.CosNaming.NamingContextPackage.AlreadyBound;
import org.omg.CosNaming.NamingContextPackage.CannotProceed;
import org.omg.CosNaming.NamingContextPackage.InvalidName;
import org.omg.CosNaming.NamingContextPackage.NotFound;

import CKvalObs.CDataSource.*;

public class CorbaHelper {
	ORB orb=null;
	NamingContextExt nameServer=null;
	String nameServerLocation;
	
	public CorbaHelper( String[] args ) {
		
		boolean hasPort = false;
		boolean hasORBInit = false;
		String myArg[] = null;
		String cnameserver=System.getProperty("corba.nameserver", "corbans.oslo.dnmi.no:2809" );
		
		int i = cnameserver.indexOf(":");
				
		if( i>=0 )
			hasPort=true;
		
		for( String elem : args ){
			if( elem.startsWith( "-ORBInitRef" ) ) {
				hasORBInit = true;
				break;
			}
		}
		
		if( hasORBInit ) {
			myArg = args;
		} else {
			myArg = new String[args.length+2];
			
			int n; 
			for( n=0; n<args.length; ++n )
				myArg[n] = args[n];
			
			myArg[n] = "-ORBInitRef";
			n++;
			
			if( hasPort )
				nameServerLocation = "NameService=corbaloc:iiop:"+cnameserver + "/NameService";
			else
				nameServerLocation ="NameService=corbaloc:iiop:"+cnameserver + ":2809/NameService";
			
			myArg[n] = nameServerLocation;
		}
		
		try {
			orb = ORB.init( myArg, null );
			resolveNameServer();
	            
		} catch ( Exception e ) {
			System.err.println( "Exception in INSClient " + e );
			System.exit( 1 );
		}
	}

	
	synchronized  protected void resolveNameServer()
    {
		try{
            nameServer=NamingContextExtHelper.narrow(
            		orb.resolve_initial_references( "NameService" )
            	);
		}
        catch(Exception ex){
        	ex.printStackTrace();
            System.out.println("Can't resolve nameservice '" + nameServerLocation + "'.");
            System.exit( 1 );
        }
    }

	synchronized public CKvalObs.CDataSource.Data resolveDataInput() {
	     NamingContextExt nc;
	     Object           obj=null;
	     String           server;
	     String kvServer=System.getProperty( "kvserver" );
	        
	     if( kvServer==null || kvServer.length() == 0 ) {
	    	 System.out.println("The kvalobsserver is missing.");
	    	 System.out.println("Set the property kvserver: ex. -Dkvserver=kvalobs");
	    	 System.exit( 1 );
	     }
	        
	     server=kvServer+"/kvinput";
	        
	     System.out.println("Looking up kvinput: " + server);
	        
	     try{
	    	 obj = nameServer.resolve(
	    			 nameServer.to_name(server)
	    	       );
	                
	    	 if(obj==null){
	    		 System.out.println("NameServer (1): obj==null");
	    		 System.exit( 1 );
	    	 }
	                
	     }
	     catch(org.omg.CORBA.COMM_FAILURE e){
	    	 System.out.println("NameServer: COMM_FAILURE");
	    	 System.exit( 1 );
	     }
	     catch(org.omg.CosNaming.NamingContextPackage.NotFound ex){
	    	 System.out.println("Nameserver: Not found: " +server);
	    	 System.exit( 1 );
	     }
	     catch(org.omg.CosNaming.NamingContextPackage.InvalidName ex){
	    	 System.out.println("Nameserver: InvalidName: "+server);
	    	 System.exit( 1 );
	     }
	     catch(Exception ex){
	    	 ex.printStackTrace();
	    	 System.exit( 1 );
	     }
	        
	     try{
	    	 CKvalObs.CDataSource.Data dataInput = DataHelper.narrow(obj);
	    	 return dataInput;
	     }
	     catch(Exception ex){
	    	 ex.printStackTrace();
	    	 System.exit( 1 );
	     }
	     
	     //Should never be reached.
	     return null;
	}
}
