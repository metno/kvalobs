/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: FileUtil.java,v 1.1.2.3 2007/09/27 09:02:43 paule Exp $                                                       

  Copyright (C) 2007 met.no

  Contact information:
  Norwegian Meteorological Institute
  Box 43 Blindern
  0313 OSLO
  NORWAY
  email: kvalobs-dev@met.no

  This file is part of KVALOBS

  KVALOBS is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License as 
  published by the Free Software Foundation; either version 2 
  of the License, or (at your option) any later version.
  
  KVALOBS is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.
  
  You should have received a copy of the GNU General Public License along 
  with KVALOBS; if not, write to the Free Software Foundation Inc., 
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
package metno.util;

import java.io.*;
import java.lang.*;

public class FileUtil {

	/**
	 * Read the contest from a file and return it as a string.
	 * 
	 * @param file2read The file to read.
	 * @return A string on success and null otherwise.
	 */
	static public String readFile2Str(String file2read){
		File f;
		BufferedReader in;
		
		f=new File(file2read);
		
		if(!(f.exists() && f.isFile() && f.canRead()))
				return null;
				
		try {
			in = new BufferedReader(new FileReader(f));
		} catch (FileNotFoundException e) {
			return null;
		}

		int size;
		
		if(f.length()<=Integer.MAX_VALUE)
			size=(int)f.length();
		else
			size=Integer.MAX_VALUE;
		
		StringBuffer sb=new StringBuffer(size);
		char[] buf=new char[256];
		int nRead;
		
		try {
			while((nRead=in.read(buf)) >-1){
				sb.append(buf, 0, nRead);
			}
			
			in.close();
			return sb.toString();
		} catch (IOException e) {
			e.printStackTrace();
			
			try {
				in.close();
			} catch (IOException e1) {
				e1.printStackTrace();
			}	
			
			return null;
		}
	}
	
	
	/**
	 * Writes a string to a file.
	 * 
	 * @param file2write The name of the file to write the string.
	 * @param buf The string to write to the file.
	 * @return true on success and fale ortherwise.
	 */
	static public boolean writeStr2File(String file2write, String buf){
		BufferedWriter out;
				
		try {
			out = new BufferedWriter(new FileWriter(file2write));
			out.write(buf, 0, buf.length());
			out.close();
			return true;
		} catch (IOException e) {
			System.out.println("IOException: FileUtil.writeStr2File: "+e.getMessage());
			return false;
		}
	}

}
