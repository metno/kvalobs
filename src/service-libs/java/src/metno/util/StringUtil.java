/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: StringUtil.java,v 1.1.2.2 2007/09/27 09:02:43 paule Exp $                                                       

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

import java.lang.StringBuffer;

public class StringUtil {

    /**
     * Replace a range in the input string with newstr.
     * The range is specified with the index begin and end.
     * The range replaced is begin to end-1. 
     * 
     * If end is -1 or greater than input length the end of the string 
     * starting at begin is replaced with newstr.
     * 
     * @param input The string to replace content in
     * @param begin The start index of what to replace.
     * @param end   The end index of what to replace.  
     * @param newstr Replace with this string.
     * @return the input string with the range [begin, end] replaced with 
     *          newstr.
     */
    public static String replace(String input, 
                                 int begin, 
                                 int end, 
                                 String newstr){
        
        if(begin<0)
            begin=0;
        
        if(end<0)
            end=input.length();
        
        if(begin==end)
            return input;
            
        if(begin>=input.length())
            return input;
        
        StringBuffer sb=new StringBuffer(input);
                        
        if(begin>end){
            int tmp=begin;
            begin=end;
            end=tmp;
        }
            
        sb.replace(begin, end, newstr);
        
        return sb.toString();
    }
    
}
