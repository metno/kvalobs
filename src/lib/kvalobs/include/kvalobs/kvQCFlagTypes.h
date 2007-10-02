/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvQCFlagTypes.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $                                                       

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
#ifndef _kvQCFlagTypes_h
#define _kvQCFlagTypes_h


/* Created by DNMI/FoU/PU: a.christoffersen@dnmi.no
   at Thu Sep  5 15:37:18 2002 */

#include <string>

namespace kvQCFlagTypes {

  /*
    The main QC levels
  */
  const int num_mainqcx = 4;
  
  enum main_qc {
    main_qc1  = 0,
    main_qc2d = 1,
    main_qc2m = 2,
    main_hqc  = 3
  };


  /*
    control-flag for each QC checks, corresponds to
    'controlpart' in kvQcxInfo
  */
  enum c_flags {
    f_fqclevel= 0,
    f_fr=       1,
    f_fcc=      2,
    f_fs=       3,
    f_fnum=     4,
    f_fpos=     5,
    f_fmis=     6,
    f_ftime=    7,
    f_fw=       8,
    f_fstat=    9,
    f_fcp=     10,
    f_fclim=   11,
    f_fd=      12,
    f_fpre=    13,
    f_fcombi=  14,
    f_fhqc=    15
  };
  /*
    legal 'missing' values (QC1-0, f_fmis)
  */
  enum missing_status {
    status_ok = 0,
    status_original_missing = 1,
    status_corrected_missing = 2,
    status_orig_and_corr_missing = 3
  };

}

#endif
