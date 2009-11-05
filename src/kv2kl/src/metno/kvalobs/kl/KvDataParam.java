package metno.kvalobs.kl;

public class KvDataParam {
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

}
