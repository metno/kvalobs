/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DataReceiver.java,v 1.6.2.1 2007/09/27 09:02:43 paule Exp $                                                       

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
import kvalobs.*;
import CKvalObs.CService.*;
import java.io.*;
import java.lang.*;
import java.text.*;


public class DataReceiver implements KvDataEventListener {

    DecimalFormat dfm=new DecimalFormat("0.00");
    StringBuffer sb=new StringBuffer(10);
    String  pattern="{0}\t{1}\t{2}\t{3,number}\t{4,number}";
    MessageFormat form=null;

    public DataReceiver(){
	try{
	    form=new MessageFormat(pattern);
	}
	catch(IllegalArgumentException ex){
	    System.out.println("Inavlid pattern: " + ex.getMessage());
	}
    }

    String format(int fieldSize, double number)
    {
	String s=dfm.format(number);
	
	if(s.length()<fieldSize){
	    sb.delete(0, sb.length());
	    
	    for(int i=0; i<(fieldSize-s.length()); i++){
		sb.append(' ');
	    }
	    
	    sb.append(s);
	    return sb.toString();
	}
	
	return s;
	
    }
   

    public void kvDataEvent(KvDataEvent event){
	ObsData[] obsData=event.getObsData();

	System.out.println("DataReceiver: new KvDataEvent!");
	
	if(obsData!=null){
	    for(int i=0; i<obsData.length; i++){
		DataElem[] elem=obsData[i].dataList;
				
		if(elem!=null){

		    if(elem.length>0){
			File of=new File("data/"+Integer.toString(elem[0].stationID)+
					 ".jdat");
			try{
			    System.out.println("stationid: " + elem[0].stationID +
					       " obstime: " + elem[0].obstime +
					       " #params: " + elem.length);
			    
			    FileOutputStream fo=new FileOutputStream(of, true);
			    PrintStream      fout=new PrintStream(fo, true);
			    
			    for(int j=0; j<elem.length; j++){
				Object[] arg={elem[j].obstime,
					      format(10,elem[j].original),
					      format(10,elem[j].corrected),
					      new Short(elem[j].paramID),
					      new Short(elem[j].typeID_)};
				
				fout.println(form.format(arg));
			    }
			    
			    fout.close();
			}catch(FileNotFoundException ex){
			    System.out.println("Cant open file: " +
					       Integer.toString(elem[0].stationID)+
					       ".jdat");
			}
		    }else{
			System.out.println("No data in observation!");
		    }
		}else{
		    System.out.println("Opppsss: NO PARAMS!");
		}
	    }
	}else{
	    System.out.println("Opppsss: NO DATA!");
	}
    }
}



	
