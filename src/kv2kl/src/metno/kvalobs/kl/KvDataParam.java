package metno.kvalobs.kl;

import metno.kvalobs.kl.KvDataContainer;

public class KvDataParam implements Comparable<KvDataParam> {
	int paramid;
	float original;
	float corrected;
	String controlinfo;
	String useinfo;
	String cfailed;
	
	public KvDataParam( int paramid, float original, float corrected, 
			                 String controlinfo, String useinfo, String cfailed)
	{
		this.paramid = paramid;
		this.original = original;
		this.corrected = corrected;
		this.controlinfo = controlinfo;
		this.useinfo = useinfo;
		this.cfailed = cfailed;
	}

	public int compareTo( KvDataParam param ) {
		return this.paramid -param.paramid;
	}
	
	public int getParamid() {
		return paramid;
	}

	public float getOriginal() {
		return original;
	}

	public float getCorrected() {
		return corrected;
	}

	public String getControlinfo() {
		return controlinfo;
	}

	public String getUseinfo() {
		return useinfo;
	}

	public String getCfailed() {
		return cfailed;
	}
	
	public boolean isMissingOriginal() {
		int val = Math.round( original );
		
		return val == KvDataContainer.MISSING_VALUE;
	}
	
	public boolean isMissingCorrected() {
		int val = Math.round( corrected );
		
		return val == KvDataContainer.MISSING_VALUE;
	}

}
