/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Kv2KlimaFilter.java,v 1.1.2.4 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kl;
import metno.dbutil.*;
import metno.util.MiGMTTime;
import java.sql.*;
import java.util.*;
import org.apache.log4j.Logger;

/** This class is a proxy for a row in the table T_KV2KLIMA_FILTER in the
 * klima database.
 * 
 * @author borgem
 */
public class Kv2KlimaFilter {

	int       stnr;
	int       typeid;
	FilterElem currentElem=null;
	LinkedList<FilterElem> filterElemList = new LinkedList<FilterElem>();
	static  Logger logger=Logger.getLogger(Filter.class);
	
	static class FilterElem implements Comparable{
		final static long MILIS_IN_YEAR=31536000000L;
		final static long MIN_YEAR=-1900*MILIS_IN_YEAR;
		final static long MAX_YEAR=6000*MILIS_IN_YEAR;
		
		String    status;
		int       nytt_stnr;
		Timestamp fdato;
		Timestamp tdato;
			
		FilterElem(String status, 
				   int nytt_stnr, 
				   Timestamp fdato,
				   Timestamp tdato){
			init(status, nytt_stnr, fdato, tdato);
		}
		
		FilterElem(String status, 
				   int nytt_stnr ){
			init(status, nytt_stnr, null, null);
		}

		FilterElem(java.sql.ResultSet rs) throws SQLException {
			init(rs);
		}
		
		void init(String status, 
				  int nytt_stnr, 
				  Timestamp fdato,
				  Timestamp tdato){
			this.status=status;
			this.nytt_stnr=nytt_stnr;
						
			if(fdato==null)
				this.fdato=new Timestamp(MIN_YEAR);
			else
				this.fdato=fdato;
			
			if(tdato==null)
				this.tdato=new Timestamp(MAX_YEAR);
			else
				this.tdato=tdato;
		}
		
		void init(java.sql.ResultSet rs) throws SQLException {
			
				try {
					status=rs.getString("status");
					fdato=rs.getTimestamp("fdato");
					tdato=rs.getTimestamp("tdato");
					nytt_stnr=rs.getInt("nytt_stnr");
				} catch (SQLException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
					throw e;
				}
				
				init( status, nytt_stnr, fdato, tdato );
		}

		@Override
		public String toString(){
			return "{ S: "+status+", N_No: "+nytt_stnr+", fdato: "+fdato+", tdato: "+tdato+"}";
		}
		
		public int compareTo(Object obj) {
			FilterElem e=(FilterElem)obj;
			
			return fdato.compareTo(e.fdato);
		}
	}
	
	public Kv2KlimaFilter(){
		stnr=0;
		typeid=0;
		
		filterElemList.add( new FilterElem(null, 0 ) );
		currentElem = filterElemList.getFirst();
	}

	/**
	 * Create an instance with stnr and typeid set. 
	 * status, fdato and tdato is set to null.
	 */
	public Kv2KlimaFilter(int stnr, int typeid){
		this.stnr=stnr;
		this.typeid=typeid;
		filterElemList.add( new FilterElem(null, 0 ) );
	}
	
	public Kv2KlimaFilter(int stnr, int typeid, DbConnection con){
		FilterElem filterElem;
		
		this.stnr=stnr;
		this.typeid=typeid;
		
		String stmt="SELECT * FROM T_KV2KLIMA_FILTER "+
					"  WHERE stnr="+ stnr + " AND typeid="+typeid;

		logger.debug("Filter lookup: " + stmt);
		
		ResultSet rs=null;

		try{
			rs=con.execQuery(stmt);
	
			while(rs.next()){
				filterElem= new FilterElem(rs);
				logger.debug("Adding filter: "+filterElem);
				filterElemList.add( filterElem);
			}
		}
		catch(SQLException ex){
			logger.error(new MiGMTTime() + "- EXCEPTION: CTOR: " +ex
					     + " SQLState: "+ex.getSQLState());
			stnr=0;
		}
		
		if( rs!=null ) {
			try{
				rs.close();
			}
			catch(SQLException ex){
				logger.error(new MiGMTTime() + "- EXCEPTION: CTOR (close): " +ex
						     + " SQLState: "+ex.getSQLState());
			}
		}
		
		String debugStr="";
				
		for( FilterElem e : filterElemList ) {
			debugStr+="\n"+e;
		}
		logger.debug("Filterlist: "+debugStr);
	}
		
	public boolean isOk(){ return stnr!=0;}
	
	/**
	 * setCurrentFilterElem finds a timeperiod from the list of time
	 * periods that is defined for the station and set it to the active 
	 * filter for this instance. 
	 * 
	 * If no period is found false is returned.
	 *  
	 * @param obstime the obstime to chevk.
	 * @return true if a timeperiod is defined for the obstime and false 
	 *         otherwise.
	 */
	public boolean setCurrentFilterElem(Timestamp obstime){
		if( obstime==null ) {
			logger.error("INVALID  obstime (null)");
			return false;
		}	
		
		for( FilterElem e : filterElemList ) {
			if( obstime.compareTo(e.fdato) >= 0 && obstime.compareTo(e.tdato)<=0){
				currentElem = e;
				
				logger.debug("Current filter element (true): " + currentElem);
				return true;
			}
		}
		currentElem = filterElemList.getFirst();
		logger.debug("No valid filter element (false): " + this);
		return false;
	}
	
	@Override
	public String toString(){
		String msg="{ stnr: " + stnr + ", typeid: "+typeid + "\n" +
		                      "  FilterList: ";
		
		if( filterElemList==null)
				msg += "(null)";
		else if(filterElemList.size()==0)
				msg+= "(empty)";
		else {
			boolean first=true;
			for( FilterElem e : filterElemList ) {
				if( first )
					first=false;
				else
					msg+="\n              ";
				msg+=e;
			}
		}
		                       
		return msg;		
	}

	
	public int       getStnr(){   return stnr;}
	public String    getStatus(){ return currentElem.status;}
	public Timestamp getFdato(){  return currentElem.fdato;}
	public Timestamp getTdato(){  return currentElem.tdato;}
	public int       getTypeid(){ return typeid;}
	public int       getNytt_stnr(){ return currentElem.nytt_stnr;}
	public boolean   hasFilterElements() { return filterElemList.size()!=0;}
}
