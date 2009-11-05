package metno.kvalobs.kl;
import java.util.*;

public class KvDataSensor {
	int sensor;
	TreeSet<KvDataLevel> container = null;
	
	public KvDataSensor( int sensor )
	{
		this.sensor = sensor;
		try {
			container = new TreeSet<KvDataLevel>(); 
		}
		catch( NullPointerException e) {
			container = null;
		}
	}
	
	public KvDataLevel findLevel( int level ) {
		Iterator<KvDataLevel> it = container.iterator();
		KvDataLevel kvLevel;
		
		while( it.hasNext() ) {
			kvLevel = it.next();
			
			if( kvLevel.getLevel() == level )
				return kvLevel;
		}
		
		return null;
	}
	
	public int getSensor() {
		return sensor;
	}

	public boolean add( KvDataLevel level ) {
		if(container == null )
			return false;
		
		return container.add( level );
	}
	
	Iterator<KvDataLevel> iterator() {
		if( container == null )
			return null;
		
		return container.iterator();
	}
}
