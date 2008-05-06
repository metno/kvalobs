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
import metno.util.IntHolder;
import CKvalObs.CService.*;
import java.util.Date;
import java.util.*;
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
	
	protected boolean paramFilter(int stationID, int typeID, int paramID, short level,
			                      String sensor, boolean useLevelAndSensor, 
							      Timestamp obstime,
                                  StringHolder msg){

		if( paramFilter==null ||
			paramFilter.stationid!=stationID ) {
			paramFilter=new ParamFilter(stationID, con);
		}

		if(!paramFilter.filter((short)paramID, (short)typeID, 
				               level, sensor, useLevelAndSensor,
				               obstime)) {
			if( useLevelAndSensor )
				addToString(msg, "[paramFilter] Blocked param: "+paramID+
						         " sensor: "+sensor+
					             " level: "+level);
			else
				addToString(msg, "[paramFilter] Blocked param: "+paramID + 
						         ", No sensor and level." );
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
	
	protected void doNyStnr(IntHolder stationID, int typeID_, 
							Kv2KlimaFilter dbElem,
						    StringHolder   msg){
		if(dbElem.getNytt_stnr()!=0){
			if(dbElem.getTypeid()==0 || 
			   dbElem.getTypeid()==Math.abs(typeID_)){
				addToString(msg, 
							"[doNyStnr] Changed stnr to "+
							dbElem.getNytt_stnr()+".");

				stationID.setValue( dbElem.getNytt_stnr() );
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
	
	public Kv2KlimaFilter loadFromDb( int stationID, int typeID_, 
									  Timestamp obstime){
		int typeid=Math.abs(typeID_);

		if(dbKv2KlimaFilterElem!=null && 
		   dbKv2KlimaFilterElem.getStnr()==stationID &&
		   dbKv2KlimaFilterElem.getTypeid()==typeid ) { 

			return dbKv2KlimaFilterElem;
		}

		dbKv2KlimaFilterElem = new Kv2KlimaFilter(stationID, typeid, con);

		return dbKv2KlimaFilterElem;
	}
	

	
	public boolean filter(IntHolder stationID, short typeID_, short paramID, 
			              short level, String sensor, boolean useLevelAndSensor,
			              String sObstime, StringHolder msg){
		Kv2KlimaFilter dbElem;

		if(!filterEnabled)
			return true;
		
		logger.debug(" -- Filter:  sid: "+stationID.getValue() + " tid: "+typeID_
		             + " paramID: "+paramID + " obstime: " + sObstime);
		
		MiGMTTime obstimeTmp=new MiGMTTime();
		
		if(!obstimeTmp.parse(sObstime)){
			logger.error(new MiGMTTime() 
					     +" ERROR: filter.filter: Cant parse <obstime> '"+sObstime+"'");
			return true;
		}
	
		Timestamp obstime = obstimeTmp.getTimestamp();
		logger.debug(" -- Filter:  Incomming obstime decoded to: " + obstimeTmp + " (GMT) -> Timestamp: " + obstime );
		
		dbElem=loadFromDb( stationID.getValue(), typeID_, obstime);
	
		if( ! dbElem.isOk() ) {
		//There was a problem with loading from the database.
			logger.error(new MiGMTTime() 
		     			+" ERROR: filter.filter: DB error, blocking the observation!");
			return false;
		}

		
		
		if( ! dbElem.hasFilterElements() ){
			logger.info("BLOCKED: No filter data for station: " + stationID.getValue() + " typeid: " +  typeID_ );
			return false;
		}
		
		if( ! dbElem.setCurrentFilterElem(obstime) ) {
			logger.info("BLOCKED: No filter element for station that match obstime.");
			return false;
		}

		if(!doStatus(dbElem, msg))
			return false;
		
		doNyStnr(stationID, typeID_, dbElem, msg);
		
		if(!paramFilter(stationID.getValue(), typeID_, paramID, 
			            level, sensor, useLevelAndSensor,
			            obstime, msg))
			return false;
		
		return true;
	}
	
	
	public boolean filter(CKvalObs.CService.TextDataElem data, 
            StringHolder msg){
		IntHolder stationID = new IntHolder(data.stationID);

		if( filter(stationID, data.typeID_, data.paramID, (short)0, null, 
				false, data.obstime, msg ) ){
			//The stationid may have changed.
			data.stationID = stationID.getValue();
			return true;
		}

		return false;
	}

	
	
	/** Shall we use this observation element. 
	 * 
	 * @param DataElem an observation element.
	 * 
	 * @return true if we shall use this observation element and false otherwise.
	 */
	public boolean filter(CKvalObs.CService.DataElem data, 
			              StringHolder msg){
		IntHolder stationID = new IntHolder(data.stationID);
		
		if( filter(stationID, data.typeID_, data.paramID, data.level, data.sensor, 
				   true, data.obstime, msg ) ){
			//The stationid may have changed.
			data.stationID = stationID.getValue();
			return true;
		}
		
		return false;
		
	}
}
