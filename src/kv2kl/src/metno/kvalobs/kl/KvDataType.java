package metno.kvalobs.kl;
import java.util.*;

public class KvDataType  implements Comparable<KvDataType>{
	int type;
	TreeSet<KvDataObstime> container = null;
	
	public KvDataType( int type )
	{
		this.type = type;
		
		try {
			container = new TreeSet<KvDataObstime>(); 
		}
		catch( NullPointerException e) {
			container = null;
		}
	}
	
	public int compareTo( KvDataType type ) {
		return this.type -type.type;
	}
	
	public KvDataObstime findObstime(  metno.util.MiGMTTime obstime ) {
		Iterator<KvDataObstime> it = container.iterator();
		KvDataObstime kvObstime;
		
		while( it.hasNext() ) {
			kvObstime = it.next();
			
			if( kvObstime.getObstime().compareTo( obstime ) == 0 )
				return kvObstime;
		}
		
		return null;
	}
	
	
	public int getType() {
		return type;
	}

	public boolean add( KvDataObstime obstime ) {
		if(container == null )
			return false;
		
		return container.add( obstime );
	}
	
	Iterator<KvDataObstime> iterator() {
		if( container == null )
			return null;
		
		return container.iterator();
	}
}
