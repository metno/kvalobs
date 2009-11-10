package metno.kvalobs.kl;
	
import java.util.LinkedList;
import java.util.List;
import java.util.regex.*;

import metno.kvalobs.kl.TimeDecoder;
import metno.util.MiGMTTime;

	


public class dateregex {

		
	public static void main( String[] args ) {
		System.out.println("args[0: '"+args[0] +"'");
		//TimeRange time = decodeObstimes( args[0], null );
		List<TimeRange> times = TimeDecoder.decodeTimeList( args[0] );
		
		/*
		if( time != null ) 
			System.out.println("time: " + time );
		*/
		
		if( times != null ) {
			for( TimeRange time : times ) {
				System.out.println("time: " + time );
			}
		}
		
	}
}