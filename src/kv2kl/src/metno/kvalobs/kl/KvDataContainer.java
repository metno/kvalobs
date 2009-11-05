package metno.kvalobs.kl;

import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.*;

import metno.util.MiGMTTime;

public class KvDataContainer {
	boolean overwrite;
	TreeSet<KvDataStation> data;
	int iStationid; 
	int iObstime;    
	int iOriginal; 
	int iParamid; 
	int iTypeid; 
	int iSensor;
	int iLevel; 
	int iCorrected;
	int iControlinfo;
	int iUseinfo;
	int iCfailed;

	KvDataStation findStation( int stationid ) {
		Iterator<KvDataStation> it = data.iterator();
		KvDataStation station;
		
		while( it.hasNext() ) {
			station = it.next();
			
			if( station.getStation() == stationid )
				return station;
		}
		
		return null;
	}
	
	boolean doInitialize( java.sql.ResultSet rs ) throws  SQLException
	{
		ResultSetMetaData md=rs.getMetaData();
		int N = md.getColumnCount();
		
		iStationid = 0; 
		iObstime = 0;    
		iOriginal = 0; 
		iParamid = 0; 
		iTypeid = 0; 
		iSensor = 0;
		iLevel = 0; 
		iCorrected = 0;
		iControlinfo = 0;
		iUseinfo = 0;
		iCfailed = 0;
		
		String colName;
		
		for( int n=1; n<=N; n++ ) {
			colName = md.getColumnName( n );

			if( colName.compareToIgnoreCase("stationid" ) == 0 )
				iStationid = n;
			else if( colName.compareToIgnoreCase("obstime" ) == 0 )
				iObstime = n;
			else if( colName.compareToIgnoreCase("original" ) == 0 )
				iOriginal = n;
			else if( colName.compareToIgnoreCase("paramid" ) == 0 )
				iParamid = n;
			else if( colName.compareToIgnoreCase( "typeid" ) == 0 )
				iTypeid = n;
			else if( colName.compareToIgnoreCase( "sensor" ) == 0 )
				iSensor = n;
			else if( colName.compareToIgnoreCase("level" ) == 0 )
				iLevel = n;
			else if( colName.compareToIgnoreCase("xlevel" ) == 0 )
				iLevel = n;
			else if( colName.compareToIgnoreCase("corrected" ) == 0 )
				iCorrected = n;
			else if( colName.compareToIgnoreCase("controlinfo" ) == 0 )
				iControlinfo = n;
			else if( colName.compareToIgnoreCase("useinfo" ) == 0 )
				iUseinfo = n;
			else if( colName.compareToIgnoreCase("cfailed" ) == 0 )
				iCfailed = n;
		}
			
		if( iStationid == 0 || iObstime == 0 || iOriginal == 0 || iParamid == 0 ||
			iTypeid == 0 || iSensor == 0 || iLevel == 0 || iCorrected == 0 ||
			iControlinfo == 0 || iUseinfo == 0 || iCfailed == 0 )
			return false;
		
		return true;
		
	}

	void add( int stationid,
    		  int typeid,
    		  MiGMTTime obstime,
    		  String sensor,
    	      int level,
    		  int paramid,
    		  float original,
    		  float corrected,
    		  String controlinfo,
    		  String useinfo,
    		  String cfailed )
	{
		KvDataStation station = findStation( stationid );
		
		if( station == null ) {
			station = new KvDataStation( stationid );
			data.add( station );
		}
		
		KvDataType type = station.findType( typeid );
		
		if( type == null ) {
			type = new KvDataType( typeid );
			station.add( type );
		}
		
		KvDataObstime otime = type.findObstime( obstime );
		
		if( otime == null ) {
			otime = new KvDataObstime(obstime, false );
			type.add( otime );
		}
		
		if( sensor.isEmpty() )
			sensor = "0";
		
		int nSensor;
		
		try {
			nSensor = java.lang.Integer.parseInt( sensor );
		}
		catch( NumberFormatException ex ) {
			System.out.println("KvDataContainer::add: sensor not a valid number.");
			nSensor = 0;
		}
		
		KvDataSensor kvSensor = otime.findSensor( nSensor );
		
		if( kvSensor == null ) {
			kvSensor = new KvDataSensor( nSensor );
			otime.add( kvSensor );
		}
		
		KvDataLevel kvLevel = kvSensor.findLevel( level );
		
		if( kvLevel == null ) {
			kvLevel = new KvDataLevel( level );
			kvSensor.add( kvLevel ); 
		}
		
		KvDataParam kvParam = kvLevel.findParam( paramid );
		
		if( kvParam == null  ) {
			kvParam = new KvDataParam( paramid, original, corrected, controlinfo, useinfo, cfailed );
			kvLevel.add( kvParam );
		} else {
			System.out.println("KvDataContainer::add: ignored duplicate [" + stationid + "," +typeid + "," + paramid + "," +level + "," + nSensor + "," + obstime.toString()+"]" );
		}
	}
	
	
	public KvDataContainer( boolean overwrite ) {
		this.overwrite = overwrite;
		data = new TreeSet<KvDataStation>(); 
	}
	
	
	
	public void add( java.sql.ResultSet rs ) {
    	try{
    		Timestamp tsobstime;
    		//String tmpDate;
    		int stationid;
    		int typeid;
    		MiGMTTime obstime;
    		String sensor;
    		int level;
    		int paramid;
    		float original;
    		float corrected;
    		String controlinfo;
    		String useinfo;
    		String cfailed;
    		
    		if( ! doInitialize( rs ) ) {
    			System.out.println("KvDataContainer::add: Missing coloumns in the database.");
    			return;
    		}
    		
    		while(rs.next()){
    			
    			stationid =  rs.getInt( iStationid);
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: stationid is NULL." );
    				continue;
    			}
    				
    			tsobstime=rs.getTimestamp( iObstime );
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: obstime is NULL." );
    				continue;
    			}
    			
    			original = rs.getFloat( iOriginal);
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: original is NULL." );
    				continue;
    			}
    			
    			paramid = rs.getInt( iParamid);
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: paramid is NULL." );
    				continue;
    			}
    			
    			typeid = rs.getInt(iTypeid);
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: typeid is NULL." );
    				continue;
    			}
    			
    			sensor = rs.getString( iSensor );
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: sensor is NULL. Settinin it to 0." );
    				sensor = "0";
    			}
    			
    			level = rs.getInt(iLevel);
    			
    			corrected = rs.getFloat( iCorrected );
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: corrected is NULL. Setting it to original." );
    				corrected = original;
    			}

    			controlinfo = rs.getString( iControlinfo );
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: controlinfo is NULL." );
    				continue;
    			}
    			
    			useinfo = rs.getString( iUseinfo );
    			
    			if( rs.wasNull() ) {
    				System.out.println("KvDataContainer::add: useinfo is NULL." );
    				continue;
    			}
    			
    			cfailed = rs.getString( iCfailed );
    			
    			
    			obstime=new MiGMTTime(tsobstime);
    			//tmpDate=obstime.toString(MiGMTTime.FMT_COMPACT_TIMESTAMP_1);
    		}

    	}
    	catch(SQLException SQLe){                
    		System.out.println(SQLe);            
    		return ;
    	}
	}
	
	Iterator<KvDataStation> iterator() {
		if( data == null )
			return null;
		
		return data.iterator();
	}
}
