/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: senddata.java,v 1.3.2.2 2007/09/27 09:02:43 paule Exp $                                                       

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
import CKvalObs.CDataSource.*;

public class senddata {
    
    public static void main(String[] args)
    {
	String data=null;
	int    nationalno=18700;
	int    type=402;

	
	data="RR_24,SA,SD,V4,V5,V6\n"+
	    "200504291200,,,,7,,\n"+
	    "200504300700,0.4(0000000000010000),-1,-1,,,\n"+
	    "200505010700,-1(0000000100000000,1111111111111101),-1,-1,,,\n"+
	    "200505020700,0.1,-1,-1,7,,\n"+
	    "200505030700,0.0,-1,-1,7,,\n"+
	    "200505031200,,,,7,,\n"+
	    "200505040700,0.3,-1,-1,7,,\n"+
	    "200505050700,0.3,-1,-1,7,,\n"+
	    "200505051800,,,,8,,\n"+
	    "200505051200,,,,7,,\n"+
	    "200505060700,1.1,-1,-1,7,,\n";

	if(args.length<1)
	    System.exit(1);

	System.out.println("Connectiong to kvalobs server <"+args[0]+">!");
	

	KvApp app=null;
	Result res=null;

	

	try{
	    app=new KvApp(args, args[0], false);
	    res=app.sendKlDataToKv(data, nationalno, type);
	}
	catch(Exception e){
	    System.out.println("Cant connect to <"+args[0]+">");
	    app.exit();
	}

	if(res.res!=EResult.OK){
	    System.out.println(res.message);
	}else{
	    System.out.println("OK!");
	}

	app.exit();
    }
}



	
