/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KlDataReceiver.java,v 1.1.2.8 2007/09/27 09:02:19 paule Exp $                                                       

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
package metno.kvalobs.kv2kl;

import java.io.*;
import kvalobs.*;
import metno.kvalobs.kl.KlApp;
import metno.kvalobs.kl.SqlInsertHelper;
import CKvalObs.CService.*;
import org.apache.log4j.Logger;

public class KlDataReceiver implements KvDataEventListener {

	Kv2KlApp app;
	SqlInsertHelper insertstmt;
    static Logger logger=Logger.getLogger(KlDataReceiver.class);
    static Logger filterlog=Logger.getLogger("filter");
    
    public KlDataReceiver(Kv2KlApp app, String backupfile){
    	this.app=app;
    	insertstmt=new SqlInsertHelper(app, backupfile);
    }
 
    public void kvDataEvent(KvDataEvent event) {
    	ObsData[] obsData=event.getObsData();
    	app.updateLastKvContact();
    	insertstmt.insertData(obsData, null);
    }
}


	
    
