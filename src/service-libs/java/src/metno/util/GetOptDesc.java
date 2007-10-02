/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: GetOptDesc.java,v 1.1.2.2 2007/09/27 09:02:43 paule Exp $                                                       

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

/** A GetOptDesc describes one argument that may be accepted by the program.
 *  @author Ian F. Darwin, http://www.darwinsys.com/
 */
public class GetOptDesc {
	/** The short-form option letter */
	protected char argLetter;
	/** The long-form option name */
	protected String argName;
	/** True if this option needs an argument after it */
	protected boolean takesArgument;

	/** Construct a GetOpt option.
	 * @param ch The single-character name for this option.
	 * @param nm The word name for this option.
	 * @param ta True if this option requires an argument after it.
	 */
	public GetOptDesc(char ch, String nm, boolean ta) {
		if (!Character.isLetter(ch) && !Character.isDigit(ch)) {
			throw new IllegalArgumentException(ch + ": not letter or digit");
		}
		argLetter = ch;
		argName   = nm;	// may be null, meaning no long name.
		takesArgument = ta;
	}
}
