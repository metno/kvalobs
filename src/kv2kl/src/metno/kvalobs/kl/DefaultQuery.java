package metno.kvalobs.kl;

import org.apache.log4j.Logger;

public class DefaultQuery implements IQuery{
	static Logger logger=Logger.getLogger(DefaultQuery.class);
	
	public DefaultQuery(){
	}
	
	public String createDataUpdateQuery( CKvalObs.CService.DataElem elem ){
		logger.debug("Update a SQL92 database!");
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
	
	public String createDataInsertQuery( CKvalObs.CService.DataElem elem){
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
		return null;
	}
	
	public String createTextDataInsertQuery(CKvalObs.CService.TextDataElem elem){
		return null;
	}
	
}
