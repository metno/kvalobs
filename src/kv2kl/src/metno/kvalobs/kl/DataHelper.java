package metno.kvalobs.kl;

public class DataHelper {
	IQuery query=null;
	String dbdriver=null;
	CKvalObs.CService.DataElem[] dataElem=null;
	CKvalObs.CService.TextDataElem[] textDataElem=null;
	int index=-1;
	

	public DataHelper( String dbdriver ){
		query = QueryFactory.createQuery( dbdriver );
	}
	
	public DataHelper(CKvalObs.CService.DataElem[] dataElem, 
			          CKvalObs.CService.TextDataElem[] textDataElem,
			          String dbdriver ){
		this( dbdriver );
		init( dataElem, textDataElem );
	}
	
	public void init( CKvalObs.CService.DataElem[] dataElem, 
	                  CKvalObs.CService.TextDataElem[] textDataElem ){
		this.dataElem = dataElem;
		this.textDataElem = textDataElem;
		index = -1;
	}
	
	public String createInsertQuery(){
		if( dataElem != null)
			return query.createDataInsertQuery( dataElem[index] );
		
		if( textDataElem != null)
			return query.createTextDataInsertQuery( textDataElem[index] );
		
		return null;
	}

	public String createUpdateQuery(){
		if( dataElem != null)
			return query.createDataUpdateQuery( dataElem[index] );
		
		if( textDataElem != null)
			return query.createTextDataUpdateQuery( textDataElem[index] );
		
		return null;
	}
   	
	
	public boolean next() {
		index++;
		int l;
		
		if( dataElem != null ) {
			if( index >= dataElem.length ) {
				dataElem = null;
				index=0;
			} else { 
				return true;
			}
		}
		
		if( textDataElem != null ) {
			if( index >= textDataElem.length ) {
				textDataElem = null;
			} else { 
				return  true;
			}
		}
		
		return false;
	}
	
	public int getStationID() throws NoData {
		if( dataElem != null )
			return dataElem[index].stationID;
		
		if( textDataElem != null )
			return textDataElem[index].stationID;
		
		throw new NoData();
	}
	
	public void setStationID( int sid  ){
		if( dataElem != null )
			dataElem[index].stationID = sid;
		
		if( textDataElem != null )
			textDataElem[index].stationID = sid;
	}
	
	public short getTypeID()throws NoData {
		if( dataElem != null )
			return dataElem[index].typeID_;
		
		if( textDataElem != null )
			return textDataElem[index].typeID_;
		
		throw new NoData();
	}

	public short getParamID()throws NoData {
		if( dataElem != null )
			return dataElem[index].paramID;
		
		if( textDataElem != null )
			return textDataElem[index].paramID;
		
		throw new NoData();
	}
	
	public short getLevel() throws NoData{
		if( dataElem != null )
			return dataElem[index].level;
		
		if( textDataElem != null )
			return 0;
		
		throw new NoData();
	}
	
	public String getSensor() throws NoData{
		if( dataElem != null )
			return dataElem[index].sensor;
		
		if( textDataElem != null )
			return null;

		throw new NoData();
	}

	
	public String getObstime() throws NoData{
		if( dataElem != null )
			return dataElem[index].obstime;
		
		if( textDataElem != null )
			return textDataElem[index].obstime;

		throw new NoData();
	}

	public boolean useLevelAndSensor()throws NoData {
		if( dataElem != null )
			return true;
		
		if( textDataElem != null )
			return false;
		
		throw new NoData();
	}
	
}