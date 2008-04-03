/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: ParamFilter.java,v 1.1.2.5 2007/09/27 09:02:19 paule Exp $                                                       

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
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.Collections;
import java.util.HashMap;
import java.util.LinkedList;
import java.util.ListIterator;
import java.lang.Short;
import java.lang.Integer;
import java.lang.Comparable;
import metno.dbutil.DbConnection;


import metno.util.MiGMTTime;

import org.apache.log4j.Logger;


/**
 * The ParamFilter class is a helper class used to filter data on parameter.
 * 
 * It use two helper tables from the database it is connected to. These tables are
 * T_KV2KLIMA_TYPEID_PARAM_FILTER and T_KV2KLIMA_PARAM_FILTER. The first table, 
 * T_KV2KLIMA_TYPEID_PARAM_FILTER, contains values that is common for all typeids. 
 * The seccond table, T_KV2KLIMA_PARAM_FILTER, contains values that is specific for one 
 * stationid/typeid combination. Values in T_KV2KLIMA_PARAM_FILTER, may add or remove 
 * to the values in the T_KV2KLIMA_TYPEID_PARAM_FILTER table. If a paramid is negativ 
 * it is removed from the filter and a postiv paramid add to the filter. A timespan must
 * be given for when the values is valid. The timespan is given with a from date (fdato)
 * and a to date (tdato). Note that for the add/remove to function as specified the 
 * fdato and tdato must match exactly. 
 * 
 * If a typeid in the filter table is negativ, ie less than 0 it only blocks inncomming
 * data with a negativ typeid. While a positiv typeid in the filtertables blocks both
 * data with nagativ and positiv typeids.
 * 
 * The fdato and tdato is valid for the timespan [fdato,tdato].
 * 
 * <b>Examle of filter setup</b><br/>
 * If you generally dont want data for params SA (112) and SD (18) for typeid 302,
 * but you want SD for station 18700. <br> 
 * 
 * <pre>
     insert into  T_KV2KLIMA_TYPEID_PARAM_FILTER VALUES(302, 112, 0, 0, NULL, NULL); 
     insert into  T_KV2KLIMA_TYPEID_PARAM_FILTER VALUES(302,  18, 0, 0, NULL, NULL);
   
     insert into  T_KV2KLIMA_PARAM_FILTER VALUES(18700, 302, -18, 0, 0, NULL, NULL);
 * </pre>
 *  And if you want to block RR_X (117) from station 18700, you add this line to the table
 *   T_KV2KLIMA_PARAM_FILTER VALUES.
 * <pre>  
     insert into  T_KV2KLIMA_PARAM_FILTER VALUES(18700, 302, 117, 0, 0, NULL, NULL);
 * </pre>  
 *   
 * @author borgem
 *
 */
public class ParamFilter {

	static  Logger logger=Logger.getLogger(ParamFilter.class);

	int stationid;
	HashMap types;
	DbConnection con;
	
	static class ParamElem implements Comparable{
		final static long MILIS_IN_YEAR=31536000000L;
		final static long MIN_YEAR=-1900*MILIS_IN_YEAR;
		final static long MAX_YEAR=6000*MILIS_IN_YEAR;
		int typeid;
		int paramid;
		int sensor;
		int level;
		Timestamp fdato;
		Timestamp tdato;
			
		ParamElem(int typeid,
				  int paramid, 
				  int sensor, 
				  int level,
				  Timestamp fdato,
				  Timestamp tdato){
			this.typeid=typeid;
			this.paramid=paramid;
			this.sensor=sensor;
			this.level=level;
			
			if(fdato==null)
				this.fdato=new Timestamp(MIN_YEAR);
			else
				this.fdato=fdato;
			
			if(tdato==null)
				this.tdato=new Timestamp(MAX_YEAR);
			else
				this.tdato=tdato;
			
		}
		
		ParamElem(int typeid,
				  int paramid, 
				  int sensor, 
				  int level){
			this.typeid=typeid;
			this.paramid=paramid;
			this.sensor=sensor;
			this.level=level;
			this.fdato=new Timestamp(MIN_YEAR);
			this.tdato=new Timestamp(MAX_YEAR);
		}

		ParamElem(java.sql.ResultSet rs){
			init(rs);
		}
		
		boolean init(java.sql.ResultSet rs){
			try{
				typeid  = rs.getInt("typeid");
				paramid = rs.getInt("paramid");
				sensor  = rs.getInt("sensor");
				level   = rs.getInt("xlevel");
				fdato   = rs.getTimestamp("fdato");
				tdato   = rs.getTimestamp("tdato");
				
				if(fdato==null)
					this.fdato=new Timestamp(MIN_YEAR);
				
				if(tdato==null)
					this.tdato=new Timestamp(MAX_YEAR);
				
				return true;
			}
			catch(SQLException ex){
				logger.error("SQL problems: cant fetch data from a resultset!\n"+ex
						      + " SQLState: "+ex.getSQLState());
				typeid=0;
				paramid=0;
				sensor=0;
				level=0;
			}
			
			return false;
		}
		
		boolean isOk(){ return paramid!=0; }

		@Override
		public String toString(){
			return "{ tid:"+typeid+", pid:"+paramid+", sen:"+sensor+", lev:"+level+",fdate:"+fdato+", tdate:"+tdato+"}";
		}
		
		public int compareTo(Object obj) {
			ParamElem e=(ParamElem)obj;
			
			if( typeid<e.typeid)
				return -1;
			
			if( typeid>e.typeid)
				return 1;
			
			if(paramid<e.paramid)
				return -1;
			
			if(paramid>e.paramid)
				return 1;
			
			if(sensor<e.sensor)
				return -1;
			
			if(sensor>e.sensor)
				return 1;
			
			if(level<e.level)
				return -1;
			
			if(level>e.level)
				return 1;

			return fdato.compareTo(e.fdato);
		}
	}
	

	/**
	 * Remove or add an ParamElement to a list. If an paramid
	 * is negativ, remove the element from the list if an element
	 * exist in the list. Elements that is greater than zero is 
	 * added to the list if it not allready exist in the list.
	 * 
	 * The paramid to the elemet is given with the absolute value
	 * of paramid.
	 * 
	 * @param list The list toa add or remove an element from.
	 * @param pe The ParamElement to add/remove to the list.
	 */
	void addOrRemoveFromList(LinkedList list, ParamElem pe){
		boolean remove=false;
		int     pid=Math.abs(pe.paramid);
		
		if(pe.paramid<0)
			remove=true;
		
		ListIterator it=list.listIterator(0);
		
		while(it.hasNext()){
			ParamElem param=(ParamElem)it.next();
			
			if(param.typeid==pe.typeid &&
			   param.paramid==pid &&
			   param.sensor==pe.sensor &&
			   param.level==pe.level &&
			   param.fdato.compareTo(pe.fdato)==0 &&
			   param.tdato.compareTo(pe.tdato)==0){
				if(remove)
					it.remove();
				
				return;
			}
		}
		
		if(!remove)
			list.add(pe);
	}
	
	LinkedList loadFromDb(short type){
		int  typeid = Math.abs(type);
		LinkedList list=new LinkedList();
		boolean error=false;

		if(types==null)
			types=new HashMap();
		
		String stmt="SELECT * FROM T_KV2KLIMA_TYPEID_PARAM_FILTER "+
					"  WHERE abs(typeid)="+typeid;

		logger.debug("TypeidParamFilter::loadFromDb: query: " + stmt);
		
		ResultSet rs=null;
		
		try{
			rs=con.execQuery(stmt);
	
			while(rs.next()){
				ParamElem pe=new ParamElem(rs);
					
				if(!pe.isOk()){
					logger.error("Failed to initialize an ParamElem from the database!");
					continue;
				}
				logger.debug("TypeidParamFilter::loadFromDb: add element: " + pe);
				list.add(pe);
			}
		}
		catch(SQLException ex){
			logger.error(new MiGMTTime() + " - EXCEPTION: loadFromDb: " +ex + " SQLState: "+ex.getSQLState());
			error=true;
		}
		
		if(rs!=null){
			try{
				rs.close();
			}
			catch(SQLException ex){
				logger.error("Filter.loadFromDb: Exception when closing the ResultSet!\n"+ex 
						     + " SQLState: "+ex.getSQLState());
			}
		}

		if(error)
			return null;
		
		stmt="SELECT * FROM T_KV2KLIMA_PARAM_FILTER "+
		     "  WHERE abs(typeid)="+typeid+" AND stnr="+stationid;

		logger.debug("ParamFilter lookup: " + stmt);

		rs=null;

		try{
			rs=con.execQuery(stmt);

			while(rs.next()){
				ParamElem pe=new ParamElem(rs);
		
				if(!pe.isOk()){
					logger.error("Failed to initialize an ParamElem from the database!");
					continue;
				}
				logger.debug("TypeidParamFilter::loadFromDb: addOrRemove element: " + pe);
				addOrRemoveFromList(list, pe);
			}
		}
		catch(SQLException ex){
			logger.error(new MiGMTTime() + " - EXCEPTION: loadFromDb: " +ex
					     + " SQLState: "+ex.getSQLState());
			error=true;
		}

		if(rs!=null){
			try{
				rs.close();
			}
			catch(SQLException ex){
				logger.error("Filter.loadFromDb: Exception when closing the ResultSet!\n"+ex
						      + " SQLState: "+ex.getSQLState());
			}
		}
		Collections.sort(list);
		types.put(new Integer(typeid), list);

		return list;
	}
	
	public ParamFilter(int stationid, DbConnection con) {
		this.stationid=stationid;
		this.con=con;
		types=null;
	}

	boolean filter(CKvalObs.CService.DataElem data, Timestamp obstime){
		LinkedList params=null;
			
		if(types==null){
			params=loadFromDb(data.typeID_);
			
			if(params==null){
				System.out.println("ParamFilter: Unexpected null list!");
				return true;
			}
		}
		
		Object obj=types.get(new Integer( Math.abs( data.typeID_) ) );
		
		if(obj==null)
			params=loadFromDb(data.typeID_);
		else
			params=(LinkedList)obj;
		
		if(params==null || params.size()==0)
			return true;
		
		ListIterator it=params.listIterator(0);
		
		while(it.hasNext()){
			ParamElem param=(ParamElem)it.next();

			logger.debug("-- ParamFilter: ParamElem: " + param);
			
			if( param.paramid == data.paramID &&
			    param.level   == data.level   &&
			    Math.abs(param.typeid)  == Math.abs(data.typeID_) ){
				
				//Negative typeid in the filter tables effect only
				//negative types.
				if( param.typeid < 0 && data.typeID_ > 0 )
					continue;
				
				int s;
				
				try{
					s=Integer.parseInt(data.sensor);

					if(param.sensor==s){
						if(obstime.compareTo(param.fdato)>=0 && 
						    obstime.compareTo(param.tdato)<=0)
							return false;
					}
				}
				catch(NumberFormatException ex){
					ex.printStackTrace();
					logger.error("parseInt: the sensor value is not a number ("+param.sensor+")");
					return true;
				}
				
			}
		}
		
		return true;
	}
	
}
