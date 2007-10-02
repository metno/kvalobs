/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: TableInfoType412.java,v 1.1.2.2 2007/09/27 09:02:19 paule Exp $                                                       

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

public class TableInfoType412 extends TableInfoType {
    
    public TableInfoType412(){
	super("KL2KVALOBS", "412","TA,TAN_12,TAX_12,TW,UU,DD,FF,RR_12,RR_24,SA,EM,NN,HL,VV,V1,V2,V3,WW,V4,V5,V6,V7,W1,W2,FX,NH,CL,CM,CH,OT_24" );
    }

    public String createQuery(String stations){
	if(stations!=null)
	    return "Select stnr,to_char(dato,'yyyymmddhh24mi'),"+
		getParams() +" from "+ getTable()+
		" where typeid="+getTypeid()+" and stnr in ("+stations+")"+
		 " order by stnr,dato";
	else
	    return "Select stnr,to_char(dato,'yyyymmddhh24mi'),"+
		getParams() +" from "+ getTable()+
		" where typeid="+getTypeid() + " order by stnr,dato";
    }


    public boolean convertToKvDataAndSend(java.sql.ResultSet rs, 
					  DataToKv kvalobs){
	try{
	    int nColumns=rs.getMetaData().getColumnCount();
	    int stationid=-1;
	    int prevStationid=-1;
	    int count=0;
	    String tmp;
	    String data=getParams()+"\n";

	    while(rs.next()){
		stationid = rs.getInt(1);
		
		if((stationid!=prevStationid && prevStationid!=-1) ||
		   count>MAX_COUNT){
		    System.out.println(prevStationid+"("+getTypeid()+") ["+
				       data+"]");
		    if(!kvalobs.sendData(data, prevStationid, 
					 Integer.parseInt(getTypeid()))){
			System.out.println("Cant send data to kvalobs!");
			return false;
		    }
		    data=getParams()+"\n";;
		    count=0;
		}

		prevStationid=stationid;

		data += rs.getString(2);
			
		for (int i = 3; i<=nColumns; i++) {
		    data+=",";
		    tmp=rs.getString(i);
		    
		    if(!rs.wasNull())
			data+=tmp;
		}
		count++;
		data+="\n";
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
    
