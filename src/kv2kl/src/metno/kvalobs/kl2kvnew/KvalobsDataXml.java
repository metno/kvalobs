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
package metno.kvalobs.kl2kvnew;

import javax.xml.*;
import javax.xml.stream.*;
import java.io.*;

public class KvalobsDataXml {

	XMLStreamWriter xml;
	
	void write( String element, String attribute, String attributevalue ) throws XMLStreamException
	{
		xml.writeStartElement( element );
		xml.writeAttribute( attribute,  attributevalue );
	}
	
	void writeElem( String element, String value )throws XMLStreamException
	{
		xml.writeStartElement( element );
		xml.writeCharacters( value );
		xml.writeEndElement();
	}
	
	void endElem( )throws XMLStreamException
	{
		xml.writeEndElement();
	}

	void writeElemVal( String element, String attributevalue )throws XMLStreamException
	{
		write( element, "val", attributevalue );
	}
	
	public KvalobsDataXml()
	{
		
	}
	
	
	
	public String getXML() 
	{
		ByteArrayOutputStream out  = new ByteArrayOutputStream();
		XMLOutputFactory factory = XMLOutputFactory.newInstance();
		
		try {
			xml = factory.createXMLStreamWriter(out);
	
			xml.writeStartDocument();
			write("KvalobsData", "overwrite", "1");
			writeElemVal( "station", "70800" );
			writeElemVal( "typeid", "308" );
			writeElemVal( "obstime", "2009-10-02 20:00:00" );
			writeElemVal( "sensor", "0" );
			writeElemVal( "level", "0" );
			write( "kvdata", "paramid", "206" );
			writeElem( "original", "1023");	
			writeElem( "corrected", "1023.5");
			writeElem( "controlinfo", "0000000000000" );
			writeElem( "useinfo", "0000000000000" );
			endElem();
			endElem();
			endElem();
			endElem();
			endElem();
			endElem();
			endElem();
		
			xml.close();
			System.out.print( out.toString() );
		} catch (XMLStreamException e) {
			return null;
		}
		
		return null;
	}
	
}
