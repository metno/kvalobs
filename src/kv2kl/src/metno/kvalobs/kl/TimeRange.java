package metno.kvalobs.kl;

import metno.util.*;
import java.util.*;

import java.util.regex.*;

public class TimeRange {
	MiTime from=null;
	MiTime to=null;
	
	static Pattern datePattern = Pattern.compile(" *(\\d{4})-(\\d{1,2})-(\\d{1,2})((T| +)(\\d{1,2})(:\\d{1,2}){0,2})? *");
	
	public TimeRange( MiTime from, MiTime to )
	{
		if( from == null && to == null )
			return;
		
		this.from = from;
		this.to = to;
		
		if( from == null)
			this.from = to;
		
		if( to == null )
			this.to = from;
		
	}
	
	public TimeRange( MiTime time )
	{
		this.from = time;
		this.to = time;
	}

	public MiTime getFrom() {
		return from;
	}

	public MiTime getTo() {
		return to;
	}
	
	public String toString() {
		return from + " - " + to;
	}
	
	public boolean isEqual() {
		if( to == null || from == null )
			return false;
		
		return to.compareTo(from) == 0;
	}
	
}
