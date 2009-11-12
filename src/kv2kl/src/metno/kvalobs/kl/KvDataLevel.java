package metno.kvalobs.kl;
import java.util.*;

public class KvDataLevel implements Comparable<KvDataLevel>{
	int level;
	TreeSet<KvDataParam> container = null;
	
	public KvDataLevel( int level )
	{
		this.level = level;
		try {
			container = new TreeSet<KvDataParam>(); 
		}
		catch( NullPointerException e) {
			container = null;
		}
	}
	
	public int compareTo( KvDataLevel level ) {
		return this.level -level.level;
	}

	public KvDataParam findParam( int paramid ) {
		Iterator<KvDataParam> it = container.iterator();
		KvDataParam kvParam;
		
		while( it.hasNext() ) {
			kvParam = it.next();
			
			if( kvParam.getParamid() == paramid )
				return kvParam;
		}
		
		return null;
	}
	
	public int getLevel() {
		return level;
	}

	
	public boolean add( KvDataParam param ) {
		if( container == null )
			return false;
		
		return container.add( param );
	}
	
	Iterator<KvDataParam> iterator() {
		if( container == null )
			return null;
		
		return container.iterator();
	}
}
