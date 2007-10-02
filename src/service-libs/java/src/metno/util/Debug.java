/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: Debug.java,v 1.1.2.2 2007/09/27 09:02:43 paule Exp $                                                       

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

/** Utilities for debugging
 * @author	Ian Darwin, http://www.darwinsys.com/
 */
public class Debug {
	/** Static method to see if a given category of debugging is enabled.
	 * Enable by setting e.g., -Ddebug.fileio to debug file I/O operations.
	 * For example:<br/>
	 * if (Debug.isEnabled("fileio"))<br/>
	 * 	System.out.println("Starting to read file " + fileName);
	 */
	public static boolean isEnabled(String category) {
		return System.getProperty("debug." + category) != null;
	}

	/** Static method to println a given message if the
	 * given category is enabled for debugging, as reported by isEnabled.
	 */
	public static void println(String category, String msg) {
		if (isEnabled(category))
			System.out.println(msg);
	}
	/** Static method to println an arbitrary Objecct if the given
	 * category is enabled for debugging, as reported by isEnabled.
	 */
	public static void println(String category, Object stuff) {
		println(category, stuff.toString());
	}
}
