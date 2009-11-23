package metno.kvalobs.kl;

public class QueryFactory {
	static public IQuery createQuery( String driver ){
		if(driver.indexOf("oracle")>-1)
    		return new OracleQuery();
    	else
    		return new DefaultQuery();
	}
}