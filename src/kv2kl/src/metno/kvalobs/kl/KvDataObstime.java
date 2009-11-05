package metno.kvalobs.kl;
import java.util.*;

import metno.util.*;

public class KvDataObstime {
	MiGMTTime obstime;
	boolean invalidate;
	TreeSet<KvDataSensor> container = null;
	
	public KvDataObstime( MiGMTTime obstime, boolean invalidate )
	{
		this.obstime = obstime;
		this.invalidate = invalidate;
		
		try {
			container = new TreeSet<KvDataSensor>(); 
		}
		catch( NullPointerException e) {
			container = null;
		}
	}
	
	public KvDataSensor findSensor( int sensor ) {
		Iterator<KvDataSensor> it = container.iterator();
		KvDataSensor kvSensor;
		
		while( it.hasNext() ) {
			kvSensor = it.next();
			
			if( kvSensor.getSensor() == sensor )
				return kvSensor;
		}
		
		return null;
	}

	
	public MiGMTTime getObstime() {
		return obstime;
	}

	public boolean isInvalidate() {
		return invalidate;
	}

	public boolean add( KvDataSensor sensor ) {
		if(container == null )
			return false;
		
		return container.add( sensor );
	}
	
	Iterator<KvDataSensor> iterator() {
		if( container == null )
			return null;
		
		return container.iterator();
	}
}
