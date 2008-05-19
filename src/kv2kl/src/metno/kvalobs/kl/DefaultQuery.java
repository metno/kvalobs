package metno.kvalobs.kl;

import org.apache.log4j.Logger;

public class DefaultQuery implements IQuery{
	static Logger logger=Logger.getLogger(DefaultQuery.class);
	
	public DefaultQuery(){
	}
	
	public String createDataUpdateQuery( CKvalObs.CService.DataElem elem ){
		logger.debug("Update data in a SQL92 database!");
		String query="UPDATE kv2klima "+
			 "SET " +
			 "  original="+elem.original + "," +
			 "  kvstamp='"+elem.tbtime+"'" +
			 "  useinfo='"+elem.useinfo + "',"+
			 "  corrected="+elem.corrected + "," +
			 "  controlinfo='"+elem.controlinfo+"'," +
			 "  cfailed='"+elem.cfailed+"' " +
			 "WHERE " +
			 "  stnr=" + elem.stationID + " AND " +
			 "  dato='" + elem.obstime + "' AND " +
			 "  paramid=" + elem.paramID + " AND " +
			 "  typeid=" + elem.typeID_ + " AND " +
			 "  xlevel=" + elem.level + " AND " +
			 "  sensor=" + elem.sensor;
		return query;
	}
	
	public String createDataInsertQuery( CKvalObs.CService.DataElem elem ){
		logger.debug("Insert into a SQL92 database!");
		String query="insert into kv2klima(stnr,dato,original,kvstamp,paramid,typeid,xlevel,sensor,useinfo,corrected,controlinfo,cfailed) values ("
				 +elem.stationID+","
				 +"'"+elem.obstime+"',"
				 +elem.original+","
				 +"'"+elem.tbtime+"',"
				 +elem.paramID+","
				 +elem.typeID_+","
				 +elem.level  +","
				 +elem.sensor  +",'"
				 +elem.useinfo +"',"
				 +elem.corrected+",'"
				 +elem.controlinfo+"','"
				 +elem.cfailed+"')";
		return query;
	}
	
	public String createTextDataUpdateQuery( CKvalObs.CService.TextDataElem elem ){
		logger.debug("Update textData in a SQL92 database!");
		String query="UPDATE T_TEXT_DATA "+
			 "SET " +
			 "  original='"+elem.original + "'," +
			 "  tbtime='"+elem.tbtime+"' " +
			 "WHERE " +
			 "  stationid=" + elem.stationID + " AND " +
			 "  obstime='" + elem.obstime + "' AND " +
			 "  paramid=" + elem.paramID + " AND " +
			 "  typeid=" + elem.typeID_; 
		return query;
	}
	
	public String createTextDataInsertQuery( CKvalObs.CService.TextDataElem elem ) {
		logger.debug("Insert textData into a SQL92 database!");
		String query="INSERT INTO T_TEXT_DATA(stationid,obstime,original,paramid,tbtime,typeid) "+
		             "values ("
				 	     +elem.stationID+","
				         +"'"+elem.obstime+"','"
				         +elem.original+"',"
				         +elem.paramID+","
				         +"'"+elem.tbtime+"',"
				         +elem.typeID_
				     +")";
		return query;
	}
	
}
