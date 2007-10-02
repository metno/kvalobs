/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: AdminImpl.java,v 1.1.2.3 2007/09/27 09:02:41 paule Exp $                                                       

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
package kvalobs.priv;

import kvalobs.IAdmin;
import micutil.*;

public class AdminImpl extends AdminPOA {
	
	IAdmin admin=null;
	String nsname=null;
	
	public AdminImpl(IAdmin admin, String nsname){
		this.admin=admin;
		this.nsname=nsname;
	}
	
	public boolean ping(){
		return true;
	}
		 	
	public void statusmessage(short details, org.omg.CORBA.StringHolder message){
		if(admin==null)
			message.value="Unkown!";
		else
			message.value=admin.statusmessage(details);
	}

	public boolean shutdown(){
		if(admin==null)
			return false;
		else
			return admin.shutdown();
	}
	
	public String getNsname(){
		return nsname;
	}
}
