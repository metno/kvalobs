/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MiTime.java,v 1.1.2.4 2007/09/27 09:02:43 paule Exp $                                                       

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
package metno.util;


import java.util.*;
import java.util.regex.Pattern;
import java.util.regex.Matcher;
import java.text.*;

public class MiTime{
    Calendar cal;

    /**yyyy-MM-dd HH:mm:ss*/
    public static final String FMT_ISO="yyyy-MM-dd HH:mm:ss";

    /**yyyy-MM-dd HH:mm:ss z*/
    public static final String FMT_ISO_z="yyyy-MM-dd HH:mm:ss z";
    
    /**yyyyMMddHHmmss
    public static final String FMT_COMPACT_TIMESTAMP="yyyyMMddHHmmss";
    /**yyyyMMddHHmm*/
    public static final String FMT_COMPACT_TIMESTAMP_1="yyyyMMddHHmm";
    /**yyyyMMddHH*/
    public static final String FMT_COMPACT_TIMESTAMP_2="yyyyMMddHH";
    
    static Pattern datePattern = Pattern.compile("^ *(\\d{4})-(\\d{1,2})-(\\d{1,2})((T| +)(\\d{1,2})(:\\d{1,2}){0,2})? *");
    
    public MiTime(){
        cal=Calendar.getInstance();
    };
    
    public MiTime(TimeZone zone){
        cal=Calendar.getInstance(zone);
    }
    
    public MiTime(MiTime miTime){
        cal=(Calendar)miTime.cal.clone();
    };
    
    /**
     * Converts from GMT to localtime
     */
    public MiTime(MiGMTTime gmtTime){
        cal=(Calendar)gmtTime.cal.clone();
        cal.setTimeZone(Calendar.getInstance().getTimeZone());
    };
    
    
    public MiTime(Calendar c){
        cal=(Calendar)c.clone();
    };
    
    public MiTime(Date d){
        cal=Calendar.getInstance();
        cal.setTime(d);
    }
    
    public MiTime(int year, int month, int day){
        cal=Calendar.getInstance();
        cal.set(year, month-1, day, 0, 0, 0);
    }
    
    public MiTime(int year, int month, int day, int hour, int min, int sec){
        cal=Calendar.getInstance();
        cal.set(year, month-1, day, hour, min, sec);
    }
    
    public void addSec(int sec){
        cal.add(Calendar.SECOND, sec);
    }
    
    public void addHour(int hour){
        cal.add(Calendar.HOUR, hour);
    }
    
    public void addMin(int min){
        cal.add(Calendar.MINUTE, min);
    }
    
    public void addDay(int days){
        cal.add(Calendar.HOUR, days*24);
    }
    
    public long daysTo(MiTime date){
        long d1=cal.getTime().getTime();
        long d2=date.getTime().getTime();
        
        return (d2-d1)/86400000;
    }
    
    public long secsTo(MiTime date){
        long d1=cal.getTime().getTime();
        long d2=date.getTime().getTime();
        
        return (d2-d1)/1000;
    }
    
    public long hoursTo(MiTime date){
        long d1=cal.getTime().getTime();
        long d2=date.getTime().getTime();
        
        return (d2-d1)/3600000;
    }
    
    public long minTo(MiTime date){
        long d1=cal.getTime().getTime();
        long d2=date.getTime().getTime();
        
        return (d2-d1)/60000;
    }
    
    public int compareTo(MiTime date){
        return cal.getTime().compareTo(date.getTime());
    }
    
    public int compareTo(Calendar date){
        return cal.getTime().compareTo(date.getTime());
    }
    
    public Date getTime(){
        return cal.getTime();
    }
    
    public Calendar getCalendar(){
        return (Calendar)cal.clone();
    }
    
    public void set(int year, int month, int day){
        cal.set(year, month-1, day, 0, 0, 0);
    }
    
    public void set(int year, int month, int day, int hour, int min, int sec){
        cal.set(year, month-1, day, hour, min, sec);
    }
    
    public void set(MiGMTTime gmtTime){
        TimeZone zone=cal.getTimeZone();
        cal=(Calendar)gmtTime.cal.clone();
        cal.setTimeZone(zone);
    }
    
    public void set(MiTime miTime){
        cal=(Calendar)miTime.cal.clone();
    }
    
    public void set(Date d){
        cal.setTime(d);
    }
    
    public void set(Calendar c){
        cal=(Calendar)c.clone();
    }
    
    /**
     * Create a string representation of the date in
     * the given format, fmt.
     * 
     * @param fmt The format we want the string in.
     * @return An string in the format given by fmt.
     * 
     * @see java.text.SimpleDateFormat for an description of fmt.
     */
    public String toString(String fmt){
        SimpleDateFormat format=new SimpleDateFormat(fmt);
        format.getCalendar().setTimeZone(cal.getTimeZone());
        
        return format.format(cal.getTime());
    }
    
    public String toString(){
        return toString(FMT_ISO_z);
    }

    /**
     * 
     * @param timestamp time as a string
     * @param fmt The format of the string.
     * @return True if the timestamp could be parsed as a timestamp
     *          in accordance with fmt. False otherwise.
     *          
     * @see java.text.SimpleDateFormat for an description of fmt.
     */
    public boolean parse(String timestamp, String fmt){
        ParsePosition    pos; 
        SimpleDateFormat format;
        
        try{
            pos= new ParsePosition(0);
            format=new SimpleDateFormat(fmt);
            
            format.setTimeZone(cal.getTimeZone());
            
            Date date=format.parse(timestamp, pos);
            
            if(date==null)
                return false;
            
            set(date);
            return true;
        }
        catch(NullPointerException ex){
        }
        
        return false;
    }

    
    public boolean parse(String timestamp){
        return parse(timestamp, FMT_ISO);
    }
    
    static MiTime parseOptHms( String time, MiTime miTime, StringBuilder remaining ) {
    	int year, month, day, hour, min, second;

    	if( miTime == null ) {
			System.out.println( "No MiTime template (miTime==null)!");
			return null;
		} 
		
		if( time == null ) {
			System.out.println( "No time string (time==null)!");
			return null;
		}
		
		if( remaining != null )
			remaining.delete( 0, remaining.length() );
			
		Matcher match = datePattern.matcher( time );
		
		hour=0;
		min=0;
		second=0;	
		System.out.println("Match: '"+ time + "'");
			
		if( ! match.lookingAt() ) {
			System.out.println("NOT a valid timestamp, '" + time + "'!");
			return null;
		}
		
		if( remaining != null ) {
			remaining.delete( 0, remaining.length() );
			String matched = match.group();
			String notMatched = time.substring( matched.length() );
			notMatched = notMatched.trim();
			remaining.append( notMatched ); 
		}
		
		year = Integer.parseInt( match.group( 1 ) );
		month = Integer.parseInt( match.group( 2 ) );
		day = Integer.parseInt( match.group( 3 ) );
		time = match.group( 4 );
				
		if( time != null ) {
			String hms[] = time.substring( 1 ).split( ":" );
				
			if( hms.length > 0 )
				hour = Integer.parseInt( hms[0] );
				
			if( hms.length > 1 )
				min = Integer.parseInt( hms[1] );
						
			if( hms.length > 2 )
				second = Integer.parseInt( hms[2] );
		}
		
		if( month < 1 || month > 12 ) {
			System.out.println(" Invalid month: '" + month + "' must be in range [1,12]." );
			return null;
		}
				
		if( day < 1 || day > 31 ) {
			System.out.println(" Invalid day: '" + day + "' must be in range [1,31]." );
			return null;
		}

		if( hour<0 || hour>23 ){
			System.out.println(" Invalid hour: '" + hour + "' must be in range [0,23]." );
			return null;
		}
				
		if( min<0 || min>23 ){
			System.out.println(" Invalid min: '" + min + "' must be in range [0,59]." );
			return null;
		}
				
		if( second<0 || second>23 ){
			System.out.println(" Invalid second: '" + second + "' must be in range [0,59]." );
			return null;
		}
				
		miTime.set( year, month, day, hour, min, second );

		return miTime;
    }
    
    /**
     * Parse time format with optional hour, min and second. Missing
     * hour, min and second is set to 0.
     * 
     *   
     * @param time
     * @return
     */
    public static MiTime parseOptHms( String time, StringBuilder remaining ) 
	{
    	MiTime miTime=new MiTime();
    	
    	return parseOptHms( time, miTime, remaining );	
    }
	
    
}
