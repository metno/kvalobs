package metno.kvalobs.kl;

class QueryFactory {
	static public IQuery createQuery( String driver ){
		if(driver.indexOf("oracle")>-1)
    		return new OracleQuery();
    	else
    		return new DefaultQuery();
	}
}