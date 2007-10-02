/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: TableInfoType402.java,v 1.1.2.6 2007/09/27 09:02:19 paule Exp $                                                       

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

import java.sql.ResultSet;
import java.sql.SQLException;

public class TableInfoType402 extends TableInfoType {
    
    public TableInfoType402(){
    	super("KL2KVALOBS", "402", "RR_24,SA,SD,V4,V5,V6,V4S,V5S,V6S,KLOBS,KLSTART");
    }

    public String createQuery(String stations){
    	if(stations!=null)
    		return "Select stnr,to_char(dato,'yyyymmddhh24mi'),"+
    				"controlinfo,useinfo,"+ getParams() +" from "+ getTable()+
    				" where typeid="+getTypeid()+" and stnr in ("+stations+")"+
    				" order by stnr,dato";
    	else
    		return "Select stnr,to_char(dato,'yyyymmddhh24mi'),"+
    			    "controlinfo,useinfo,"+ getParams() +" from "+ getTable()+
    			    " where typeid="+getTypeid() + " order by stnr,dato";
    }

    public boolean convertToKvDataAndSend(java.sql.ResultSet rs, 
					  DataToKv kvalobs){
    	try{
    		int nColumns=rs.getMetaData().getColumnCount();
    		int stationid=-1;
    		int prevStationid=-1;
    		int count=0;
    		boolean hasData;
    		String tmp;
    		String data=getParams()+"\n";
    		String tmpData=null;
	    
    		while(rs.next()){
    			
    			stationid =  rs.getInt(1);

    			if((stationid!=prevStationid && prevStationid!=-1 && count>0) ||
    			    count>MAX_COUNT){
    				System.out.println(prevStationid+"("+getTypeid()+") ["+
                                       data+"]");
    				if(!kvalobs.sendData(data, prevStationid, 
    				                     Integer.parseInt(getTypeid()))){
    					System.out.println("Cant send data to kvalobs!");
    					return false;
    				}
    				data=getParams()+"\n";
    				count=0;
    			}

    			prevStationid=stationid;
    			hasData=false;

    			tmpData = rs.getString(2);
    			tmpData+=",";
		
    			String cinfo=rs.getString(3);
    			
    			if(rs.wasNull())
    				cinfo="";
		
    			String uinfo=rs.getString(4);
    			
    			if(rs.wasNull())
    				uinfo="";
		
    			String RR24=rs.getString(5);
    			
    			if(rs.wasNull())
    				RR24="";
    				
   				if(RR24.length()>0){
   					tmpData+=RR24+"("+cinfo+","+uinfo+")";
   					hasData=true;
   				}
		
    			for (int i = 6; i<=nColumns; i++) {
    				tmp=rs.getString(i);
		        				
    				if(rs.wasNull())
    					tmp="";
    				
    				tmpData+=","+tmp;
    				
    				if(tmp.length()>0)
    					hasData=true;
    			}
		
    			
    			if(hasData){
    				data+=tmpData+"\n";
    				count++;
    			}else{
        			System.out.println("NODATA: "+stationid+" ("+getTypeid()+") obstime: "+rs.getString(2));
        		}
    		}
	    
    		if(count>0){
    			System.out.println(stationid+"("+getTypeid()+") ["+
    							   data+"]");
    			if(!kvalobs.sendData(data, stationid, 
				   Integer.parseInt(getTypeid()))){
    				System.out.println("Cant send data to kvalobs!");
    				return false;
    			}
    		}
	    
    		return true;
    	}
    	catch(SQLException SQLe){                
    		System.out.println(SQLe);            
    		return false;
    	}
    }
}
