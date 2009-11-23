package metno.kvalobs.kl;

import metno.util.MiTime;

public interface IQuery {
	/*Data*/
	public String createDataUpdateQuery( CKvalObs.CService.DataElem elem );
	public String createDataInsertQuery(CKvalObs.CService.DataElem elem);

	/*TextData*/
	public String createTextDataUpdateQuery( CKvalObs.CService.TextDataElem elem );
	public String createTextDataInsertQuery(CKvalObs.CService.TextDataElem elem);
	
	/*Helper*/
	public String dateString( MiTime time );
}
