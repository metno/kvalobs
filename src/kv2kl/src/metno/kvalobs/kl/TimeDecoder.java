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
	
	/**
	 * ensureResololution split all TimeRange in the list times where the difference between
	 * fromtime and totime is greater than the resolutionInHours. On return all TimeRange in the
	 * times list has a difference equal or less than  resolutionInHours. The number of elements
	 * in the times list will be greater or equal to the length of the in comming list. The in comming
	 * times list is unchanged.
	 *  
	 * 
	 * @param times List of TimeRange to split.
 	 * @param resolutionInHours The maximum difference between fromTime and toTime.
	 * @return List<TimeRange> A list of TimeRange with a differens less than or equal to r
	 *         resolutionInHours.
	 */
	public static List<TimeRange> ensureResolution( List<TimeRange> times, int resolutionInHours )
	{
		List<TimeRange> retTimes = new LinkedList<TimeRange>();
		MiTime fromTime;
		MiTime toTime;
		MiTime newToTime;
		MiTime newFromTime;
		
		for( TimeRange time : times ){
			fromTime = time.getFrom();
			toTime = time.getTo();
			
			if( fromTime == null || toTime == null ) {
				if( fromTime == null )
					fromTime = toTime;
				else 
					toTime = fromTime;
				
				retTimes.add( new TimeRange( fromTime ,toTime ) );
				continue;
			}

			newToTime = new MiTime( fromTime );
			newFromTime = new MiTime( fromTime );
			
			while( true ) {
				newToTime.addHour( resolutionInHours );

				if( newToTime.compareTo( toTime ) < 0 ) {
					retTimes.add(  new TimeRange( newFromTime, newToTime ) );
					newFromTime.set( newToTime );
					newFromTime.addHour( 1 );
				} else {
					retTimes.add(  new TimeRange( newFromTime, toTime ) );
					break;
				}
			}
		}
		
		return retTimes;		
	}
}
