/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: senddataf.java,v 1.2.2.1 2007/09/27 09:02:43 paule Exp $                                                       

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
import java.io.*;
import kvalobs.*;
import CKvalObs.CService.*;
import CKvalObs.CDataSource.*;

public class senddataf {
    
    public static void main(String[] args)
    {
	int    nationalno=18700;
	int    type=312;
	FileReader in;
	StringBuffer data=new StringBuffer(1024);

	if(args.length<1){
	    System.out.println("Use: senddataf filename!");
	    System.out.println();
	    return;
	}

	System.out.println("Reading from file: " + args[0]);

	File file=new File(args[0]);

	if(!file.isFile() || !file.canRead()){
	    System.out.println(file.getName()+" : is not a regular file or");
	    System.out.println("We have not read access to the file!");
	    return;
	}

	try{
	    in= new FileReader(file);
	}
	catch(java.io.FileNotFoundException ex){
	    System.out.println("File not found: " + file);
	    return;
	}
	

        int c;
	
	try{
	    while ((c =in.read()) != -1)
		//System.out.write(c);
		data.append((char)c);
	    
	    in.close();
	}
	catch(java.io.IOException ex){
	    System.out.println("IOERROR: " + file);
	    
	    try{
		in.close();
	    }
	    catch(java.io.IOException exdummy){
	    }
	    return;
	}

	System.out.println(data.toString());
	
	KvApp app=new KvApp(args, "kv-glacier", false);


	for(int i=0; i<3; i++){
	    Result res=app.sendKlDataToKv(data.toString(),18700 , 412);
	    
	    if(res.res!=EResult.OK){
		System.out.println(res.message);
	    }else{
		System.out.println("OK!");
	    }
	}


	app.exit();
    }
}



	
