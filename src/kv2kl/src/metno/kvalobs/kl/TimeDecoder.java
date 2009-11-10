package metno.kvalobs.kl;

import java.util.LinkedList;
import java.util.List;

import metno.util.MiGMTTime;
import metno.util.MiTime;

public class TimeDecoder {
	
	
	/**
	 * Decodes a comma separated list of times an returns a list of TimeRange. The
	 * from and to fields is equal.
	 * 
	 * @param timeListIn
	 * @return A list of TimeRange. 
	 */
	public static List<TimeRange> decodeTimeList( String timeListIn ) 
	{
		List<TimeRange> times = new LinkedList<TimeRange>();
		MiTime fromTime;
		MiTime toTime;
		String[] timeList = timeListIn.split(",");
		StringBuilder remaining = new StringBuilder();
		
		for( String timeString : timeList ){
			toTime = null;
			fromTime = MiGMTTime.parseOptHms( timeString, remaining );
			
			System.out.println("timeString: '" + timeString +"' remaining: '" + remaining+"'");
			
			if( fromTime == null ) {
				System.out.println("Inavalid timespec: '" + timeString +"'.");
				return null;
			}

			if( remaining.length() > 2 && remaining.charAt(0) == '-' ) {
				toTime = MiGMTTime.parseOptHms( remaining.substring( 1 ), null );
				
				if( toTime == null ) {
					System.out.println("Inavalid timespec range: '" + timeString +"'.");
					return null;
				}
			}
			times.add( new TimeRange(fromTime, toTime ) );
		}
		
		return times;
		
	}
	
	/**
	 * Create a TimeRange from a 'fromTime' and 'toTime' given as
	 * strings. If both fromTime and toTime is null, null is returned.
	 * 
	 * If one of them is null, from and to will be equal in the returned TimeRange. 
	 * 
	 * @param fromTime A valid timestamp or null. 
	 * @param toTime A valid timestamp or null.
	 * @return TimeRange. 
	 */
	public static TimeRange decodeToTimeRange( String fromTime, String toTime ) 
	{
	
		if( fromTime == null && toTime == null ) 
			return null;

		
		MiTime from;
		MiTime to;
		
		if( fromTime == null )
			fromTime = toTime;
		
		if( toTime == null )
			toTime = fromTime;
		
		from = MiGMTTime.parseOptHms( fromTime, null );
		to = MiGMTTime.parseOptHms( toTime, null );
		
		//TODO: This should be an exception instead of returning null. 
		if( from == null || to == null )
			return null;
		
		return new TimeRange( from, to );
		
	}

}
