package metno.kvalobs.kl;

import java.lang.Exception;

public class NoData extends Exception{
	public NoData(){
		super( "EXCEPTION: No more data");
	}
	
	public NoData( String msg ){
		super( msg );
	}
	
}
