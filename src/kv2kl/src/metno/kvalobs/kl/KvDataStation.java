package metno.kvalobs.kl;
import java.util.*;

public class KvDataStation implements Comparable<KvDataStation> {
	int station;
	TreeSet<KvDataType> container = null;
	
	public KvDataStation( int station )
	{
		this.station= station;

		try {
			container = new TreeSet<KvDataType>(); 
		}
		catch( NullPointerException e) {
			container = null;
		}
	}
	
	public int compareTo( KvDataStation station ) {
		return this.station -station.station;
	}
	
	public KvDataType findType( int typeid ) {
		Iterator<KvDataType> it = container.iterator();
		KvDataType type;
		
		while( it.hasNext() ) {
			type = it.next();
			
			if( type.getType() == typeid )
				return type;
		}
		
		return null;
	}
	
	public int getStation() {
		return station;
	}

	public boolean add( KvDataType type ) {
		if(container == null )
			return false;
		
		return container.add( type );
	}
	
	Iterator<KvDataType> iterator() {
		if( container == null )
			return null;
		
		return container.iterator();
	}
}
