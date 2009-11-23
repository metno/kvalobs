package metno.kvalobs.kl;

import org.apache.log4j.Logger;
import metno.kvalobs.kl.*;
import metno.util.MiTime;

public class OracleQuery implements IQuery {
	static Logger logger=Logger.getLogger(SqlInsertHelper.class);
	
	public OracleQuery(){
	}
	
	public String createDataUpdateQuery( CKvalObs.CService.DataElem elem ){
		logger.debug("Update a Oracle database!");
	   	String query="UPDATE kv2klima "+
	   				 "SET " +
	   				 "  original="+elem.original + "," +
	   				 "  kvstamp=to_date('"+elem.tbtime+"','yyyy-mm-dd hh24:mi:ss')," +
	   				 "  useinfo='"+elem.useinfo + "',"+
	   				 "  corrected="+elem.corrected + "," +
	   				 "  controlinfo='"+elem.controlinfo+"'," +
	   				 "  cfailed='"+elem.cfailed+"' " +
	   				 "WHERE " +
	   				 "  stnr=" + elem.stationID + " AND " +
	   				 "  dato=to_date('" + elem.obstime + "','yyyy-mm-dd hh24:mi:ss') AND " +
	   				 "  paramid=" + elem.paramID + " AND " +
	   				 "  typeid=" + elem.typeID_ + " AND " +
	   				 "  xlevel=" + elem.level + " AND " +
	   				 "  sensor=" + elem.sensor;
	   	return query;
	}
	
	public String createDataInsertQuery( CKvalObs.CService.DataElem elem){
		logger.debug("Insert data into a Oracle database!");
	   	String query="insert into kv2klima(stnr,dato,original,kvstamp,paramid,typeid,xlevel,sensor,useinfo,corrected,controlinfo,cfailed) values ("
	   				 +elem.stationID+",to_date('"
	   				 +elem.obstime+"','yyyy-mm-dd hh24:mi:ss'),"
	   				 +elem.original+",to_date('"
	   				 +elem.tbtime+"','yyyy-mm-dd hh24:mi:ss'),"
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
		logger.debug("Update textData in a Oracle database!");
	   	String query="UPDATE T_TEXT_DATA "+
	   				 "SET " +
	   				 "  original='"+elem.original + "'," +
	   				 "  tbtime=to_date('"+elem.tbtime+"','yyyy-mm-dd hh24:mi:ss')" +
	   				 "WHERE " +
	   				 "  stationid=" + elem.stationID + " AND " +
	   				 "  obstime=to_date('" + elem.obstime + "','yyyy-mm-dd hh24:mi:ss') AND " +
	   				 "  paramid=" + elem.paramID + " AND " +
	   				 "  typeid=" + elem.typeID_;
	   	return query;
	}
	
	public String createTextDataInsertQuery(CKvalObs.CService.TextDataElem elem){
		logger.debug("Insert textData into a Oracle database!");
	   	String query="insert into T_TEXT_DATA(stationid,obstime,original,paramid,tbtime,typeid) values ("
	   				 +elem.stationID+",to_date('"
	   				 +elem.obstime+"','yyyy-mm-dd hh24:mi:ss'),'"
	   				 +elem.original+"',"
	   				 +elem.paramID+",to_date('"
	   				 +elem.tbtime+"','yyyy-mm-dd hh24:mi:ss'),"
	   				 +elem.typeID_
	   				 +")";
	   	return query;
	}
	
	public String dateString( MiTime time ) {
		return "to_date('" + time.toString(MiTime.FMT_ISO) + "','yyyy-mm-dd hh24:mi:ss')";
	}
}
