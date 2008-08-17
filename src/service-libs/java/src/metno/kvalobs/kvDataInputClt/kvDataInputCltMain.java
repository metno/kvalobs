package metno.kvalobs.kvDataInputClt;

import CKvalObs.CDataSource.*;

public class kvDataInputCltMain {

	/**
	 * @param args
	 */
	public static void main(String[] args) {
		if( args.length == 0 ) {
			System.out.println("Use: kvDataInputClt [ORB-options] datafile");
			System.exit( 1 );
		}
		
		if( args[args.length-1].startsWith("-") ) {
			System.out.println("Use: kvDataInputClt [ORB-options] datafile");
			System.exit( 1 );
		}
		
		
		ReadFile dataToSend=null;
		
		try {
			dataToSend = new ReadFile( args[args.length-1] );
		}
		catch( Exception ex ) {
			System.out.println( ex );
			System.exit( 1 );
		}

		System.out.println( "Decoder: [" + dataToSend.getDecoder() + "]" );
		System.out.println( "Data: [" );
		System.out.println( dataToSend.getData() );
		System.out.println( "]" );
		
		CorbaHelper corbaHelper=new CorbaHelper( args );
		
		Data kvdata = corbaHelper.resolveDataInput();
		
		try {
			Result res=kvdata.newData( dataToSend.getData(), dataToSend.getDecoder() );
			
			if( res.res != EResult.OK ) {
				System.out.println("Decode error: " + res.message );
				System.exit( 1 );
			}
		}
		catch( Exception ex ) {
			System.out.println( ex );
			System.exit( 1 );
		}
	}

}
