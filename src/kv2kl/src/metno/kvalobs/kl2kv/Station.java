/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Station.java,v 1.1.2.2 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kl2kv;

import java.lang.Integer;
import java.util.*;

public class Station {

	int first;
	int last;
	
	public Station(){
		first=0;
		last=0;
	}
	
	public Station(String first) {
		try {
			this.first=Integer.parseInt(first);
		} catch (NumberFormatException e) {
			e.printStackTrace();
			this.first=-1;
		}
		last=0;
	}
	
	public Station(String first, String last){

		try {
			this.first=Integer.parseInt(first);
			this.last=Integer.parseInt(last);
			
			if(this.last<this.first){
				int t=this.last;
				this.last=this.first;
				this.first=t;
			}
		} catch (NumberFormatException e) {
			e.printStackTrace();
			this.first=-1;
			this.last=-1;
		}
	}

	public boolean isInterval(){ return first>0 && last>0;}
	
	public boolean contains(int id){
		if(isInterval())
			return id>=first && id<=last;
		
		return id==first;
	}
	
	public boolean ok(){ return first>-1 && last>-1;}

	public int getFirst(){ return first;}
	public int getLast(){return last;}
	
	public String query(){

		if(last<0 || first<0)
			return null;
		
		if(last==0 && first==0)
			return "";
		
		if(last==0)
			return "stnr="+first;
		
		return "stnr>="+first + " AND stnr<="+last;
	}
	
	public String toString(){
		return new String("["+first+"-"+last+"]");
	}
	
	static public String toString(Station[] stlist){
		String r=null;
		for(Station s : stlist)
			if(r==null)
				r=s.toString();
			else
				r+=","+s.toString();
		
		return r;
	}
	
	/**
	 * Take a list of strings, where each string specifies
	 * a stationid or a stationid interval.
	 * 
	 * Format of the string:
	 * stationspec: stationid | stationinterval
	 * stationinterval: stationid - stationid
	 * 
	 * Ex.
	 *  1. A station 14000
	 *  2. An interval 18700-20000
	 *  
	 * @param st A stationlist
	 * @return A list of stations and stations intervals.
	 */
	public static Station[] stations(List st){
		if(st==null || st.size()==0){
			Station[] stl=new Station[1];
			stl[0]=new Station();
			return stl;
		}
		
		Vector<Station> stv=new Vector<Station>();
		
		for(String s : (List<String>)st){
			String[] e=s.split("-");
		
			if(e.length==0) //s is an empty string
				continue;
		
			if(e.length>2) //An invalid specification
				return null;
		
			Station myst;
			
			if(e.length==1)
				myst=new Station(e[0]);
			else
				myst=new Station(e[0], e[1]);
			
			if(!myst.ok())
				return null;
			
			stv.add(myst);
		}
		
		Station[] myst=new Station[stv.size()];
		
		return stv.toArray(myst);
	}
	
}
