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
	
		this.from = from==null?null:new MiTime( from );
		this.to = to==null?null:new MiTime( to );

		if( from == null)
			this.from = this.to;
		else if( to == null )
			this.to = this.from;
		
	}
	
	public TimeRange( MiTime time )
	{
		this.from = new MiTime( time );
		this.to = this.from;
	}

	public MiTime getFrom() {
		return from;
	}

	public MiTime getTo() {
		return to;
	}
	
	public String toString() {
		if( isEqual() )
			return from.toString();
		else
			return from + " - " + to;
	}
	
	public boolean isEqual() {
		if( to == null || from == null )
			return false;
		
		return to.compareTo(from) == 0;
	}
	
}
