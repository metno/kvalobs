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
package metno.kvalobs.kl;

import javax.xml.*;
import javax.xml.stream.*;



import java.io.*;
import java.util.Iterator;

public class Kv2KvDataXml {

	XMLStreamWriter xml;
	
	void write( String element, String attribute, String attributevalue ) throws XMLStreamException
	{
		xml.writeStartElement( element );
		xml.writeAttribute( attribute,  attributevalue );
	}
	
	void write( String element, String attribute, int attributevalue ) throws XMLStreamException
	{
		write( element, attribute, java.lang.Integer.toString(attributevalue) ); 
	}
	
	void writeElem( String element, String value )throws XMLStreamException
	{
		xml.writeStartElement( element );
		xml.writeCharacters( value );
		xml.writeEndElement();
	}
	
	void writeElem( String element, float value )throws XMLStreamException
	{
		writeElem( element, java.lang.Float.toString(value) );
	}
	
	void writeElemVal( String element, int value )throws XMLStreamException
	{
		writeElemVal( element, java.lang.Integer.toString(value)  );	
	}
	
	void writeElemVal( String element, metno.util.MiGMTTime val )throws XMLStreamException
	{
		writeElemVal( element, val.toString(metno.util.MiGMTTime.FMT_COMPACT_TIMESTAMP_1) );	
	}
	
	void endElem( )throws XMLStreamException
	{
		xml.writeEndElement();
	}

	void writeElemVal( String element, String attributevalue )throws XMLStreamException
	{
		write( element, "val", attributevalue );
	}
	
	public Kv2KvDataXml()
	{
		
	}
	
	
	
	public String getXML( KvDataContainer container, boolean overwrite, boolean invalidate ) 
	{
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
		
		ByteArrayOutputStream out  = new ByteArrayOutputStream();
		XMLOutputFactory factory = XMLOutputFactory.newInstance();
		
		try {
			xml = factory.createXMLStreamWriter(out);
			xml.writeStartDocument();

			itStation = container.iterator();
			
			if( itStation == null )
				return null;
			
			if( overwrite )
				write("KvalobsData", "overwrite", "1");
			else
				xml.writeStartElement( "KvalobsData" );

			while( itStation.hasNext() ) {
				station = itStation.next();
				writeElemVal( "station", station.getStation()  );
				
				itType = station.iterator();
				
				if( itType == null ) {
					System.out.println("Kv2KvDataXml::getXML: fatal empty typelist.");
					return null;
				}
				
				while( itType.hasNext() ) {
					type = itType.next();
					writeElemVal( "typeid", type.getType() );
					
					itObstime = type.iterator();
				
					if( itObstime == null ) {
						System.out.println("Kv2KvDataXml::getXML: fatal empty obstimelist.");
						return null;
					}
					
					while( itObstime.hasNext() ) {
						obstime = itObstime.next();
						writeElemVal( "obstime", obstime.getObstime() );
						
						if( invalidate )
							xml.writeAttribute( "invalidate",  "1" );
						
						itSensor = obstime.iterator();
					
						if( itSensor == null ) {
							System.out.println("Kv2KvDataXml::getXML: fatal empty sensorlist.");
							return null;
						}
						
						while( itSensor.hasNext() ) {
							sensor = itSensor.next();
							writeElemVal( "sensor", sensor.getSensor() );
							
							itLevel = sensor.iterator();
						
							if( itLevel == null ) {
								System.out.println("Kv2KvDataXml::getXML: fatal empty levellist.");
								return null;
							}
						
							while( itLevel.hasNext() ) {
								level = itLevel.next();
								writeElemVal( "level", level.getLevel() );
								
								itParam = level.iterator();
							
								if( itParam == null ) {
									System.out.println("Kv2KvDataXml::getXML: fatal empty paramlist.");
									return null;
								}
								
								while( itParam.hasNext() ) {
									param = itParam.next();
									write( "kvdata", "paramid", param.getParamid() );
									writeElem( "original", param.getOriginal() );
									
									if(Math.abs(param.getOriginal() - param.getCorrected() ) > 0.01 )
										writeElem( "corrected", param.getCorrected());
										
									writeElem( "controlinfo", param.getControlinfo()  );
									writeElem( "useinfo", param.getUseinfo() );
									
									if( param.getCfailed() != null && ! param.getCfailed().isEmpty() )
										writeElem("cfailed", param.getCfailed() );
									
									endElem();
								}
								
								endElem();
							}
								
							endElem();
						}
						
						endElem();
					}
					
					endElem();
				}
				
				endElem();
			}

			endElem();
			
			xml.close();
			return out.toString();
		} catch (XMLStreamException e) {
			return null;
		}
	}
	
}
