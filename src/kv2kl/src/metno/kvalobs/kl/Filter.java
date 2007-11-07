/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Filter.java,v 1.1.2.9 2007/09/27 09:02:19 paule Exp $                                                       

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

import java.sql.*;
import metno.dbutil.*;

import java.lang.Math;
import metno.util.MiGMTTime;
import metno.util.StringHolder;
import CKvalObs.CService.*;
import java.util.Date;
import org.apache.log4j.Logger;

public class Filter {

	DbConnection con=null;
	Kv2KlimaFilter dbKv2KlimaFilterElem=null;
	ParamFilter    paramFilter=null;
	boolean       filterEnabled;
	
	static  Logger logger=Logger.getLogger(Filter.class);
	
	protected void addToString(StringHolder sh, String s){
		if(s==null || sh==null)
			return;
		
		if(sh.getValue()==null){
			sh.setValue(s);
		}else{
			sh.setValue(sh.getValue()+" "+s);
		}
	}
	
	protected boolean paramFilter(CKvalObs.CService.DataElem data, 
									Timestamp obstime,
			                        StringHolder msg){
		
		if( paramFilter==null ||
			paramFilter.stationid!=data.stationID ) {
			paramFilter=new ParamFilter(data.stationID, con);
		}
		
		if(!paramFilter.filter(data, obstime)){
			addToString(msg, "[paramFilter] Blocked param: "+data.paramID+
					         " sensor: "+data.sensor+
					         " level: "+data.level); 
			return false;
		}
		
		return true;
	}
	
	
	public boolean inDateInterval(Timestamp fromDate, Timestamp toDate, 
								    Timestamp obstime){
		boolean ret=false;
		
		logger.debug("DEBUG: Filter.inDateInterval: obstime: "+obstime);
		logger.debug("DEBUG: Filter.inDateInterval: "+fromDate +" - " + toDate);
		
		if(fromDate==null){
			if(toDate==null)
				ret=true;
			else if(obstime.compareTo(toDate)<=0)
				ret=true;
		}else if(obstime.compareTo(fromDate)>=0){
			if(toDate==null)
				ret=true;
			else if(obstime.compareTo(toDate)<=0)
				ret=true;
		}
		
		logger.debug("DEBUG: Filter.inDateInterval: obstime in dateinterval: " + ret);
		return ret;
	}
	
	protected boolean doStatusS(Kv2KlimaFilter dbelem, 
            					StringHolder   msg){
		if(dbelem==null)
			return true;
		
		if(dbelem.getTypeid()!=1)
			return true;

		String status=dbelem.getStatus();

		if(status==null)
			return true;

		if(status.length()>0){
			char chStatus=status.charAt(0);

			if(chStatus=='S' || chStatus=='s')
				return true;
		}

		addToString(msg, 
				   "[doStatusS] blocked SYNOP data!");
		return false;
	}

	protected boolean doStatus(Kv2KlimaFilter dbelem, 
            					 StringHolder msg){
		if(dbelem==null)
			return true;

		if(dbelem.getStnr()==0){
			addToString(msg,
			           "[doStatus.MISSING] blocked, not in table T_KV2KLIMA_FILTER or no valid time interval!");
			return false;
		}
			
		
		String status=dbelem.getStatus();

		if(status==null){
			addToString(msg,
	           "[doStatus.MISSING] blocked, not in table T_KV2KLIMA_FILTER or no valid time interval!");
			return false;
		}

		if(status.length()==0)
			return true;
		
		char chStatus=status.charAt(0);

		if(chStatus=='D' || chStatus=='d')
			return true;
			
		addToString(msg,
					"[doStatus.TEST] status=" + chStatus + ", blocked!");

		return false;
	}

	
	
	protected boolean doStatusT(Kv2KlimaFilter dbelem, 
			                         StringHolder msg){
		if(dbelem==null)
			return true;
			
		String status=dbelem.getStatus();
		
		if(status==null)
			return true;
		
		if(status.length()>0){
			char chStatus=status.charAt(0);
				
			if(chStatus=='T' || chStatus=='t'){
				addToString(msg,
						    "[doStatusT] blocked teststation.");
				return false;
			}
		}
			
		return true;
	}
	
	protected void doNyStnr(CKvalObs.CService.DataElem data, 
							Kv2KlimaFilter dbElem,
							StringHolder   msg){
		if(dbElem.getNytt_stnr()!=0){
			if(dbElem.getTypeid()==0 || 
			   dbElem.getTypeid()==Math.abs(data.typeID_)){
				addToString(msg, 
						 "[doNyStnr] Changed stnr to "
						 + dbElem.getNytt_stnr()+".");
			
				data.stationID=dbElem.getNytt_stnr();
			}
		}
	}
	
	
	public Filter(DbConnection con_){
		filterEnabled=true;
		con=con_;
	}
	
	public void setFilterEnabled(boolean enabled){
		filterEnabled=enabled;
	}
	
	public boolean getFilterEnabled(){
		return filterEnabled;
	}
	
	public Kv2KlimaFilter loadFromDb(CKvalObs.CService.DataElem data, 
									  Timestamp obstime){
		int typeid=Math.abs(data.typeID_);
		
		if(dbKv2KlimaFilterElem!=null && 
		   dbKv2KlimaFilterElem.getStnr()==data.stationID &&
		   dbKv2KlimaFilterElem.getTypeid()==typeid ) { 
		   
			return dbKv2KlimaFilterElem;
		}
		
		dbKv2KlimaFilterElem = new Kv2KlimaFilter(data.stationID, typeid, con);
		
		return dbKv2KlimaFilterElem;
	}

	/** Shall we use this observation element. 
	 * 
	 * @param DataElem an observation element.
	 * 
	 * @return true if we shall use this observation element and false otherwise.
	 */
	public boolean filter(CKvalObs.CService.DataElem data, 
			              StringHolder msg){
		Kv2KlimaFilter dbElem;

		if(!filterEnabled)
			return true;
		
		//System.out.println(" -- Filter:  sid: "+data.stationID + " tid: "+data.typeID_
		//		                              + " paramID: "+data.paramID + " obstime: " + data.obstime);
		
		MiGMTTime obstimeTmp=new MiGMTTime();
		
		if(!obstimeTmp.parse(data.obstime)){
			logger.error(new MiGMTTime() 
					     +" ERROR: filter.filter: Cant parse <obstime> '"+data.obstime+"'");
			return true;
		}
				
		Timestamp obstime=new Timestamp(obstimeTmp.getTime().getTime());
		dbElem=loadFromDb(data, obstime);
	
		if( ! dbElem.isOk() ) {
		//There was a problem with loading from the database.
			logger.error(new MiGMTTime() 
		     			+" ERROR: filter.filter: DB error, blocking the observation!");
			return false;
		}

		
		
		if( ! dbElem.hasFilterElements() ){
			logger.info("BLOCKED: No filter data for station: " + data.stationID + " typeid: " +  data.typeID_ );
			return false;
		}
		
		if( ! dbElem.setCurrentFilterElem(obstime) ) {
			logger.info("BLOCKED: No filter element for station that match obstime.");
			return false;
		}

		if(!doStatus(dbElem, msg))
			return false;
		
		doNyStnr(data, dbElem, msg);
		
		if(!paramFilter(data, obstime, msg))
			return false;
		
		return true;
	}
}
