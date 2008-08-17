package metno.kvalobs.kvDataInputClt;

import java.io.*;

public class ReadFile {
	String decoder=null;
	String data=null;
	
	public ReadFile( String filename ) throws Exception {
		readDataFile( filename ); 
	}
	
	public ReadFile( ) {
	}
	

	public void readDataFile( String filename ) throws Exception {
	       try {
	    	   BufferedReader reader = new BufferedReader( new FileReader( filename ) );
	    	   
	    	   //Read the decoder part, ignore empty lines at top of file.
	    	   do {
	    		   decoder = reader.readLine();
	    		   decoder = decoder.trim();
	    	   } while( decoder.length() == 0);
	    	   
	    	   StringBuilder sb = new StringBuilder();
	    	   String buf;
	    	   
	    	   while( (buf=reader.readLine()) != null )
	    		   sb.append( buf + "\n" );
	    	   
	    	   data = sb.toString();
	           
	       } catch (FileNotFoundException e) {
	    	   throw new Exception("File not found: " + filename );
	       }
	       catch(IOException e){
	           throw new Exception("Error while reading file '" + filename +"'.");
	       }
	       
	}
	
	public String getDecoder() {
		return decoder;
	}
	
	public String getData() {
		return data;
	}
}
