/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataHelper.java,v 1.1.2.5 2007/09/27 09:02:19 paule Exp $                                                       

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
import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Timestamp;
import metno.kvalobs.kl.KlApp;
import metno.util.MiGMTTime;
import org.apache.log4j.Logger;

import sun.beans.editors.FloatEditor;
import metno.dbutil.DbConnection;
import metno.dbutil.DbConnectionMgr;


/**
 * This class is used to create a message that is to be sendt to the 
 * kldecoder in kvalobs. The data is read from the table KL2KVALOBS 
 * in an open database.
 * 
 * The format on the message to be sendt is.
 * <pre>
 	<paramkode1>[(<sensor1>,<level1>)][,<paramkode2>,...,<paramkodeN(<sensorN,levelN)>]
    YYYYMMDDHHMMSS,<original1i>,<original2i>,...,<originalNi>
    YYYYMMDDHHMMSS,<original1j>,<original2j>,...,<originalNj>
    ...
    YYYYMMDDHHMMSS,<original1n>,<original2n>,...,<originalNn>
 
 	Where the first line is a header that describes the data. The parameters
 	(paramkode) is the name of the parameter, not the paramid.
 	
 	The data in the datasection have the format:
 	original: value[(controlinfo,useinfo)] 
 	
 	Note that if controlinfo and useinfo is given, most of the values 
 	is set to default values before the qa controls, only a few values
 	is accepted and not changed. See the flaggdocument.
  </pre>
 * 
 *  
 * 
 * 
 * @author borgem
 *
 */
public class DataHelper{
	int MAX_COUNT;
		
	static Logger logger = Logger.getLogger(DataHelper.class);
	static final float MISSING_VAL=-32767.0f;
	String tablename="KL2KVALOBS";
	String typeid;
    String params;
    DataToKv dataToKv;
    DbConnection  con;
    int           msgCount;
    int           obsCount;
    static String[] ignoreList={"REG_DATO"};
    
    
    
    public String getTable(){
    	return tablename;
    }

    public String getTypeid(){
    	return typeid;
    }
    
    public boolean ignore(String param){
    	for(int i=0; i<ignoreList.length; i++)
    		if(ignoreList[i].equalsIgnoreCase(param))
    			return true;
    	
    	return false;
    }
    
    public boolean floatIsEq(float f1, float f2){
    	System.out.println("floatIsEq: "+f1 +", "+f2+" dif: "+Math.abs(f1-f2));
    	
    	if(Math.abs(f1-f2)<0.001)
    		return true;
    	else
    		return false;
    }
    
    public DataHelper(DbConnection con, DataToKv dataToKv,  
	                   String typeid){
    	this(con, dataToKv, typeid, null, 20);
    }
    
    public DataHelper(DbConnection con, DataToKv dataToKv,  
                       String typeid, String table){
    	this(con, dataToKv, typeid, table, 20);
    }
    
    /**
     * 
     * @param con A valid DbConnection.
     * @param dataToKv An instancs to a class that implements DataToKv.
     * @param typeid The typeid to the stations we shall send to kvalobs.
     * @param table The table we shall pull the data to kvalobs from. If 
     *         null is given the default table KL2KVALOBS is used.
     * @param max_count The data to be sendt to kvalobs is
     *        split up in max_count chuncks. 
     */
    public DataHelper(DbConnection con, DataToKv dataToKv,  
    		           String typeid, String table, int max_count){
    	params=null;
    	msgCount=0;
    	obsCount=0;
    	
    	if(table!=null)
    		this.tablename=table;
    	
    	if(max_count<1)
    		MAX_COUNT=1;
    	else
    		MAX_COUNT=max_count;
    	
    	this.typeid=typeid;
    	this.dataToKv=dataToKv;
    	this.con=con;
    }
    
    protected String createQuery(Station st){
    	String query=st.query();
    	
    	if(query.length()>0)
    		query=" AND "+query;
    	
   		return "Select * from "+ getTable()+
    				" where typeid="+getTypeid()+ query+
    				" order by stnr,dato";
    }

    /**
     * Before this method is called must createParams has been called 
     * at least one time. 
     * 
     * @return The header section to the message that is to be sendt
     * to kvalobs.
     */
    protected String getParams(){
    	return params;
    }
    
    
    /**
     * createParams creates a header for the message sendt to kvalobs.
     * The precipitation elements is added as the last element.
     * 
     * The creation of the header must match the data creation in
     * convertToKvDataAndSend. 
     * 
     * @param md Metadata
     * @return A string to use as a header in the message to kvalobs.
     * @throws SQLException
     */
    protected String createParams(ResultSetMetaData md) throws SQLException{
    	boolean first=true;
    	boolean hasRR24=false;

    	for(int i=4; i<=md.getColumnCount(); i++){
    		if(ignore(md.getColumnName(i)))
				continue;
				
    		if(md.getColumnName(i).equalsIgnoreCase("RR_24") ||
    		   md.getColumnName(i).equalsIgnoreCase("CONTROLINFO") ||
    		   md.getColumnName(i).equalsIgnoreCase("USEINFO")){
    			hasRR24=true;
    			continue;
    		}
    		
    		if(first){
    			params=md.getColumnName(i).toUpperCase();
    			first=false;
    		}else
    			params+=","+md.getColumnName(i).toUpperCase();
    	}
    	
    	if(hasRR24){
    		if(first)
    			params="RR_24";
    		else
    			params+=",RR_24";
    	}
    	
    	return params;
    }
    
    /**
     * This function creates a message to send to kvalobs. The data
     * is read from the resultset.
     * 
     * The creation of the data section must match the header that 
     * is created in createParams.
     * @param rs The dataset to send to kvalobs.
     * @return true on success.
     */
    protected boolean convertToKvDataAndSend(java.sql.ResultSet rs){
    	try{
    		ResultSetMetaData md=rs.getMetaData();
    		int nColumns=md.getColumnCount();
    		int stationid=-1;
    		int prevStationid=-1;
    		int count=0;
    		boolean hasData;
    		String tmp;
    		String data=createParams(md)+"\n";
    		String tmpData=null;
    		Timestamp tsobstime;
    		MiGMTTime obstime;
    		String RR24;
    		String cinfo_RR24;
    		String uinfo_RR24;
    		
    		while(rs.next()){
    			
    			stationid =  rs.getInt(1);

    			if((stationid!=prevStationid && prevStationid!=-1 && count>0) ||
    			    count>=MAX_COUNT){
    				System.out.println("DEBUG:"+prevStationid+"("+getTypeid()+") ["+
                                       data+"]");
    				if(!dataToKv.sendData(data, prevStationid, 
    				                     Integer.parseInt(getTypeid()))){
    					System.out.println("Cant send data to kvalobs!");
    					return false;
    				}
    				msgCount++;
    				data=getParams()+"\n";
    				count=0;
    			}

    			
    			prevStationid=stationid;
    			hasData=false;
    			
    			tsobstime=rs.getTimestamp(2);
    			
    			if(rs.wasNull()){
    				logger.error("No timestamp given (SQL NULL)!");
    				continue;
    			}
    			
    			obstime=new MiGMTTime(tsobstime);
    			tmpData=obstime.toString(MiGMTTime.FMT_COMPACT_TIMESTAMP_1);

    			hasData=false;
    			RR24="";
    			cinfo_RR24="";
    			uinfo_RR24="";
    			int i;
    			boolean missing=false;
    			
    			for (i = 4; i<=nColumns; i++) {
    				if(ignore(md.getColumnName(i)))
    					continue;

    				if(md.getColumnName(i).equalsIgnoreCase("RR_24")){
    					RR24=rs.getString(i);
    					
    					if(rs.wasNull())
    						RR24="";
    					else{
    						float f=Float.parseFloat(RR24);
    						
    						if(floatIsEq(f, MISSING_VAL)){
    							missing=true;
    						}
    					}
    				}else if(md.getColumnName(i).equalsIgnoreCase("CONTROLINFO")){
    					cinfo_RR24=rs.getString(i);
    					
    					if(rs.wasNull() || cinfo_RR24.length()!=16){
    						if(missing)
    							cinfo_RR24="0000003000000000";
    						else
    							cinfo_RR24="";
    					}else{
    						if(missing){
   								char buf[]=cinfo_RR24.toCharArray();
    							
   								if(buf.length!=16){
   									logger.warn("controlinfo (RR_24). Wrong size '"+buf.length+"' expected length 16.");
   									cinfo_RR24=null;
   								}else{
   									buf[6]='3';
   									cinfo_RR24=new String(buf);
   								}
    						}
    					}
    				}else if(md.getColumnName(i).equalsIgnoreCase("USEINFO")){
    					uinfo_RR24=rs.getString(i);
    					
    					if(rs.wasNull())
    						uinfo_RR24="";
    				}else{
    					tmp=rs.getString(i);
		        				
    					if(rs.wasNull()){
    						tmp="";
    					}else if(tmp.length()>0){
    						try {
								float f=Float.parseFloat(tmp);
			
								if(floatIsEq(f, MISSING_VAL))
										tmp=tmp+"(0000003000000000,)";
							} catch (NumberFormatException e) {
								logger.debug("NumberFormatException: '"+tmp+ "' Not a number (Float)");
							}
    					}
    					
    					tmpData+=","+tmp;

						if(tmp.length()>0)
							hasData=true;
    				}
    			}

                if(i>4){
                	tmpData+=",";
                	if(RR24.length()>0){
                		tmpData+=RR24+"("+cinfo_RR24+","+uinfo_RR24+")";
                		hasData=true;
                	}
   				}
    			
    			if(hasData){
    				data+=tmpData+"\n";
    				count++;
    				obsCount++;
    			}else{
        			System.out.println("NODATA: "+stationid+" ("+getTypeid()+") obstime: "+rs.getString(2));
        		}
    		}
	    
    		if(count>0){
    			System.out.println(stationid+"("+getTypeid()+") ["+
    							   data+"]");
    			if(!dataToKv.sendData(data, stationid, 
				   Integer.parseInt(getTypeid()))){
    				System.out.println("Cant send data to kvalobs!");
    				return false;
    			}
    			
    			msgCount++;
    		}
	    
    		return true;
    	}
    	catch(SQLException SQLe){                
    		System.out.println(SQLe);            
    		return false;
    	}
    }
    
    public boolean sendDataToKv(Station station){
    	System.out.println("Running for typeid: " + typeid);
    	System.out.println("table:       " + tablename);
    	System.out.println("Station(s): "+station);

    	logger.info("Running for typeid: " + typeid);
    	logger.info("table:       " + tablename);
    	logger.info("Station(s):  " + station);
    	
    	if(con==null){
    		System.out.println("No valid (null) connection to the db!");
     	    logger.error("No valid (null) connection to the db!");
     	    return false;
    	}
    	
    	String query=createQuery(station);
    	
    	System.out.println("Query:[" + query +"]");

    	if(query==null){
    	    System.out.println("Cant create query!!!");
    	    logger.error("Cant create query!!!");
    	    
    	    return false;
    	}

    	ResultSet rs;
    	
    	try{
    	    rs=con.execQuery(query);
    	}
    	catch(SQLException ex){
    	    System.out.println("Cant execute query:");
    	    System.out.println("[" +query+"]");
    	    System.out.println("Reason: "+ex);
    	    logger.error("Cant execute query:");
    	    logger.error("[" +query+"]");
    	    logger.error("Reason: "+ex);

    	    return false;
    	}
    	    
           
    	boolean res=convertToKvDataAndSend(rs);

    	return res;
    }
    
    public int getObsCount(){ return obsCount; }
    public int getMsgCount(){ return msgCount; }
}
