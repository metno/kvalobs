/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: KvDataSubscribeInfo.java,v 1.1.2.3 2007/09/27 09:02:41 paule Exp $                                                       

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
package kvalobs;

import CKvalObs.CService.*;
import java.util.*;



public class KvDataSubscribeInfo{
    List              stationList_;
    List              qcList_;
    StatusId          statusid_;
    
    
    DataSubscribeInfo getInfo()
    {
        int[]        stList;
        QcId[]       qcIdList;
        int          i;
        ListIterator it;
        
        stList=new int[stationList_.size()];
        it=stationList_.listIterator();
        i=0;
        
        while(it.hasNext()){
            stList[i]=((Integer)it.next()).intValue();
            i++;
        }
        
        qcIdList=new QcId[qcList_.size()];
        it=qcList_.listIterator();
        i=0;
        
        while(it.hasNext()){
            qcIdList[i]=(QcId)it.next();
            i++;
        }
        
        return new DataSubscribeInfo(stList, statusid_, qcIdList);
        
    }
    
    public KvDataSubscribeInfo()
    {
        stationList_=new LinkedList();
        qcList_=new LinkedList();
        statusid_=StatusId.All;
    }
    
    
    public void setStatusId(CKvalObs.CService.StatusId statusid)
    {
        statusid_=statusid;
    }
    
    public void addStationId(int stationid)
    {
        stationList_.add(new Integer(stationid));
    }
    
    public void addQcId(CKvalObs.CService.QcId qcId)
    {
        qcList_.add(qcId);
    }
}
