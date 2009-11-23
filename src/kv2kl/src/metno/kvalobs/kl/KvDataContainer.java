package metno.kvalobs.kl;

import java.sql.ResultSetMetaData;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.util.*;

import javax.xml.stream.XMLStreamException;

import metno.util.MiGMTTime;
import metno.util.MiTime;

public class KvDataContainer {
	static final int MISSING_VALUE=-32767;
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
	int nObservations=0;
	int nMessages=0;

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
			nMessages++;
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
			nObservations++;
			kvParam = new KvDataParam( paramid, original, corrected, controlinfo, useinfo, cfailed );
			kvLevel.add( kvParam );
		} else if( (kvParam.isMissingOriginal() && kvParam.isMissingCorrected()) &&
				   ( (int)Math.round( original) != KvDataContainer.MISSING_VALUE || 
				     (int)Math.round( corrected ) != KvDataContainer.MISSING_VALUE ) ) {
			System.out.println("KvDataContainer::add: Replace [" + stationid + "," +typeid + "," + paramid + "," +level + "," + nSensor + "," + obstime.toString()+"]" );
			kvParam = new KvDataParam( paramid, original, corrected, controlinfo, useinfo, cfailed );
			kvLevel.add( kvParam );
		} else {
			//System.out.println("KvDataContainer::add: ignored duplicate [" + stationid + "," +typeid + "," + paramid + "," +level + "," + nSensor + "," + obstime.toString()+"," +
			//		            + original + "," + corrected + "]" );
		}
	}
	
	
	public KvDataContainer(  ) {
		data = new TreeSet<KvDataStation>(); 
	}
	
	
	
	public boolean add( java.sql.ResultSet rs, boolean disableQC1 ) {
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
    		int iiOriginal;
    		int iiCorrected;
    		char[] buf = new char[16];
    		
    		if( ! doInitialize( rs ) ) {
    			System.out.println("KvDataContainer::add: Missing coloumns in the database.");
    			return false;
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
    				//System.out.println("KvDataContainer::add: sensor is NULL. Seting it to 0." );
    				sensor = "0";
    			}
    			
    			level = rs.getInt(iLevel);
    			
    			corrected = rs.getFloat( iCorrected );
    			
    			if( rs.wasNull() ) {
    				//System.out.println("KvDataContainer::add: corrected is NULL. Setting it to original." );
    				corrected = original;
    			}

    			controlinfo = rs.getString( iControlinfo );
    			
    			if( rs.wasNull() ) {
    				//System.out.println("KvDataContainer::add: controlinfo is NULL." );
    				controlinfo = "0000000000000000";
    			} else {
    				//As a hack we set hqc flag to 3 if the flag is '0'.
    				//If the hqc flag is > 0, we live as it is. 
    				if( controlinfo.length() >= 16 && disableQC1 ) {
    					controlinfo.getChars( 0, 16, buf, 0 );
    					
    					if( buf[15] == '0' ) {
    						buf[15] = '3';
    						controlinfo = new String( buf );
    					}
    				}
    			}
    			
    			
    			// Sett fmis in controlinfo. This should be set, but to be 
    			//compatible with the old kl2kv we set it her if it is not
    			//set.
    			iiOriginal = Math.round( original );
    			iiCorrected = Math.round( corrected );
    			
    			if( iiOriginal == MISSING_VALUE || iiCorrected == MISSING_VALUE ) {
    				if( controlinfo.length() >= 16  ) {
    					controlinfo.getChars( 0, 16, buf, 0 );
    					
    					if( buf[6] == '0' ) {
    						if( iiOriginal == MISSING_VALUE && iiCorrected == MISSING_VALUE )
    							buf[6] = '3';
    						else if( iiOriginal == MISSING_VALUE )
    							buf[6] = '1';
    						else
    							buf[6] = '2';
    						
    						controlinfo = new String( buf );
    					}
    				}
    			}
    			
    			useinfo = rs.getString( iUseinfo );
    			
    			if( rs.wasNull() ) {
    				//System.out.println("KvDataContainer::add: useinfo is NULL." );
    				useinfo = "9999900000000000";
    			}
    			
    			cfailed = rs.getString( iCfailed );
    			
    			if( rs.wasNull() ) {
    				//System.out.println("KvDataContainer::add: cfailed is NULL." );
    				cfailed = "";
    			}
    			
    			obstime=new MiGMTTime(tsobstime);
    			//tmpDate=obstime.toString(MiGMTTime.FMT_COMPACT_TIMESTAMP_1);
    		
    			 add( stationid, typeid, obstime, sensor, level, paramid,
    		    	  original,	corrected, controlinfo,	useinfo, cfailed );
    		}
    		
    		return true;
    	}
    	catch(SQLException SQLe){                
    		System.out.println(SQLe);            
    		return false;
    	}
	}
	
	public int getNumberOfObservations() { return nObservations; }
	public int getNumberOfMessages() { return nMessages; }
	
	public Iterator<KvDataStation> iterator() {
		if( data == null )
			return null;
		
		return data.iterator();
	}
	
	@Override
	public String toString() {
		if( data == null )
			return "";

		StringBuilder s = new StringBuilder();
		KvDataStation station;
		KvDataType type;
		KvDataObstime obstime;
		KvDataSensor sensor;
		KvDataLevel level;
		KvDataParam param;
		Iterator<KvDataStation> itStation;
		Iterator<KvDataType> itType;
		Iterator<KvDataObstime> itObstime;
		Iterator<KvDataSensor> itSensor;
		Iterator<KvDataLevel> itLevel;
		Iterator<KvDataParam> itParam;
		
			
		itStation = data.iterator();

		if( itStation == null ) 
			return "";
				
		while( itStation.hasNext() ) {
			station = itStation.next();
			
			itType = station.iterator();
			
			if( itType == null ) {
				System.err.println("Kv2DataContainer::toString: fatal empty typelist.");
				continue;
			}
				
			while( itType.hasNext() ) {
				type = itType.next();
					
				itObstime = type.iterator();
					
				if( itObstime == null ) {
					System.err.println("Kv2DataContainer::toString: fatal empty obstimelist.");
					continue;
				}
					
				while( itObstime.hasNext() ) {
					obstime = itObstime.next();
					itSensor = obstime.iterator();
							
					if( itSensor == null ) {
						System.err.println("Kv2DataContainer::toString: fatal empty sensorlist.");
						continue;
					}
						
					while( itSensor.hasNext() ) {
						sensor = itSensor.next();
						itLevel = sensor.iterator();
						
						if( itLevel == null ) {
							System.err.println("Kv2DataContainer::toString: fatal empty levellist.");
							continue;
						}
						
						while( itLevel.hasNext() ) {
							level = itLevel.next();
							itParam = level.iterator();
								
							if( itParam == null ) {
								System.err.println("Kv2DataContainer::toString: fatal empty paramlist.");
								continue;
							}
								
							while( itParam.hasNext() ) {
								param = itParam.next();
								s.append( obstime.getObstime().toString()+"," );
								s.append( station.getStation() +"," );
								s.append( type.getType()+"," );
								s.append( param.getParamid()+"," );
								s.append( sensor.getSensor()+"," );
								s.append( level.getLevel() +"," );
								s.append( param.getOriginal()+"," );
								s.append( param.getCorrected()+"," );
								s.append( param.getControlinfo()+"," );
								s.append( param.getUseinfo()+"," );
								s.append( param.getCfailed()+"\n" );
							}									
						}
					}
				}
			}
		}
		
		return s.toString();
	}
}