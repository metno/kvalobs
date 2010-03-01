/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataFlag.cc,v 1.27.6.12 2007/09/27 09:02:30 paule Exp $                                                       

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
#include <kvalobs/kvDataFlag.h>
#include <dnmithread/mtcout.h>

using namespace std;
using namespace kvalobs;
using namespace kvQCFlagTypes;

// controlpart --> c_flags map
std::map<int, kvQCFlagTypes::c_flags> kvControlInfo::lockedControlFlags_;
// main_qcx string --> main_qc enum map
std::map<std::string, kvQCFlagTypes::main_qc> kvControlInfo::mainQCXint_;

// list of QcxInfo (from db)
std::list<kvQcxInfo> kvControlInfo::qcxinfolist_;

// medium_qcx --> main_qc enum map
std::map<std::string, kvQCFlagTypes::main_qc> kvControlInfo::mainQCX_;
// medium_qcx --> controlpart map
std::map<std::string, int> kvControlInfo::controlPart_;

/// list of flag values which trigger destruction of 'corrected' (by c_part)
std::map<int, std::set<int> > kvControlInfo::badValues_;


/*
  ------------------------------------------------------------------
  kvDataFlag Constructors
  ------------------------------------------------------------------
*/

kvDataFlag::kvDataFlag()
{
  clear();
}

kvDataFlag::kvDataFlag(const unsigned char f[kvDataFlag::size])
{
  clear();
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = f[i];
}

kvDataFlag::kvDataFlag(const std::string& s)
{
  clear();
  if (s.length() >= kvDataFlag::size) {
    for (int i=0; i<kvDataFlag::size; i++)
      flag_[i] = (unsigned char)(s[i]);
  }
}

kvDataFlag::kvDataFlag(const kvDataFlag& df)
{
  clear();
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = df.flag_[i];
}

kvDataFlag::~kvDataFlag()
{}


// clear flag
void kvDataFlag::clear()
{
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i]= '0';
}

// flag as string
std::string kvDataFlag::flagstring() const
{
  std::string s;
  for (int i=0; i<kvDataFlag::size; i++)
    s+= flag_[i];
  return s;
}

// get one nibble as char
unsigned char kvDataFlag::cflag(const char index) const
{
  if (index < 0 || index >= kvDataFlag::size)
    return '-';

  return flag_[index];
}


// get one nibble
int kvDataFlag::flag(const char index) const
{
  if (index < 0 || index >= kvDataFlag::size)
    return 0;

  return chartoint_(flag_[index]);
}


// set one nibble
void kvDataFlag::set
  (const char index, const int c)
{
  if (index < 0 || index >= kvDataFlag::size ||
      !legal_(c))
    return;

  flag_[index] = inttochar_(c);
}


// Assignment operator
kvDataFlag& kvDataFlag::operator=(const kvDataFlag &rhs)
{
  if (this == &rhs)
    return *this;

  // elementwise copy
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = rhs.flag_[i];

  return *this;
}

// Equality operator
bool kvDataFlag::operator==(const kvDataFlag& rhs) const
{
  for (int i=0; i<kvDataFlag::size; i++)
    if (flag_[i] != rhs.flag_[i])
      return false;
  return true;
}

// ostream operator
std::ostream& kvalobs::operator<<(std::ostream& output,
                                  const kvalobs::kvDataFlag& kd)
{
  output << "[";
  for (int i=0; i<kvDataFlag::size-1; i++) {
    output << kd.cflag(i) << "|";
  }
  output << kd.cflag(kvDataFlag::size-1) << "]";

  return output;
}


// check if legal int
bool kvDataFlag::legal_(const int i)
{
  return (i>= 0 && i<16);
}

// convert a HEX-character (0-9,A-F) to int
int kvDataFlag::chartoint_(const char c) const
{
  const int zv= int('0');
  const int nv= int('9');
  const int av= int('A');
  const int fv= int('F');

  int v= int(c);

  if (v >= zv && v<= nv)
    return v-zv;
  else if (v >= av && v <= fv)
    return v-av+10;

  // illegal character
  return 0;
}

// convert an int to a HEX-character
char kvDataFlag::inttochar_(const int i) const
{
  const int zv= int('0');
  const int av= int('A');

  if (i >= 0 && i <= 9)
    return char(zv+i);
  else if (i >= 10 && i <= 15)
    return char(av+i-10);

  // illegal int
  return '0';
}




/*
  ------------------------------------------------------------------
  kvControlInfo Constructors
  ------------------------------------------------------------------
*/

kvControlInfo::kvControlInfo()
    : kvDataFlag()
{
  init_();
}

kvControlInfo::kvControlInfo(const unsigned char f[kvDataFlag::size])
    : kvDataFlag(f)
{
  init_();
}

kvControlInfo::kvControlInfo(const std::string& s)
    : kvDataFlag(s)
{
  init_();
}

kvControlInfo::kvControlInfo(const kvControlInfo& df)
{
  init_();
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = df.flag_[i];
}

kvControlInfo::kvControlInfo(const kvDataFlag& df)
    : kvDataFlag(df)
{
  init_();
}


/*
  Fill static structures from list of kvQcxInfo
  Should be called once in initialisation phase
*/
void kvControlInfo::setQcxInfo(const std::list<kvalobs::kvQcxInfo>& qcxi)
{
  qcxinfolist_= qcxi;

  // reset static structures
  mainQCX_.clear();
  controlPart_.clear();

  std::list<kvalobs::kvQcxInfo>::const_iterator pqi= qcxinfolist_.begin();

  // Loading qcxinfo

  for(;pqi!=qcxinfolist_.end(); pqi++) {

    std::string medium_qcx= pqi->medium_qcx();
    std::string main_qcx=   pqi->main_qcx();

    if (mainQCXint_.count(main_qcx) == 0) {
      CERR("kvControlInfo::setQcxInfo ERROR. " <<
           "unknown main_qcx:"  << main_qcx << ":" << endl);

    } else {
      // combining medium_qcx with main_qcx
      mainQCX_[medium_qcx]  = mainQCXint_[main_qcx];

    }

    // combining medium_qcx with controlpart
    controlPart_[medium_qcx] = pqi->controlpart();
  }

}

// common init
void kvControlInfo::init_()
{
  // controlpart --> c_flags map
  // these parts of the controlflag locked
  if (lockedControlFlags_.size()==0) {
    lockedControlFlags_[f_fr]     = f_fr;
    lockedControlFlags_[f_fcc]    = f_fcc;
    lockedControlFlags_[f_fs]     = f_fs;
    lockedControlFlags_[f_fnum]   = f_fnum;
    lockedControlFlags_[f_fpos]   = f_fpos;
    lockedControlFlags_[f_fmis]   = f_fmis;
    lockedControlFlags_[f_ftime]  = f_ftime;
    lockedControlFlags_[f_fw]     = f_fw;
    lockedControlFlags_[f_fstat]  = f_fstat;
    lockedControlFlags_[f_fcp]    = f_fcp;
    lockedControlFlags_[f_fclim]  = f_fclim;
    lockedControlFlags_[f_fd]     = f_fd;
    lockedControlFlags_[f_fpre]   = f_fpre;
    lockedControlFlags_[f_fcombi] = f_fcombi;
    lockedControlFlags_[f_fhqc]   = f_fhqc;
  }

  // main_qcx string --> main_qc enum map
  if (mainQCXint_.size() == 0) {
    mainQCXint_["QC1"]  = main_qc1;
    mainQCXint_["QC2d"] = main_qc2d;
    mainQCXint_["QC2m"] = main_qc2m;
    mainQCXint_["HQC"]  = main_hqc;
  }


  // list of flag values which trigger destruction of 'corrected' (by c_part)
  // NB: These values should probably be put in a db table...!
  if ( badValues_.size() == 0 ) {
    badValues_[ f_fr     ].insert( 6   );

    badValues_[ f_fcc    ].insert( 0xD );

    badValues_[ f_fpos   ].insert( 6   );

    badValues_[ f_fpre   ].insert( 6   );
    badValues_[ f_fpre   ].insert( 7   );

    badValues_[ f_fcombi ].insert( 9   );
    badValues_[ f_fcombi ].insert( 0xA );
    badValues_[ f_fcombi ].insert( 0xB );

    badValues_[ f_fhqc   ].insert( 0xA );
  }
}


// Assignment operator
kvControlInfo& kvControlInfo::operator=(const kvControlInfo &rhs)
{
  if (this == &rhs)
    return *this;

  // elementwise copy
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = rhs.flag_[i];

  return *this;
}

kvControlInfo& kvControlInfo::operator=(const kvDataFlag &rhs)
{
  // elementwise copy
  for (int i=0; i<kvDataFlag::size; i++)
    set
      (i, rhs.flag(i));

  return *this;
}

// Equality operator
bool kvControlInfo::operator==(const kvControlInfo& rhs) const
{
  for (int i=0; i<kvControlInfo::size; i++)
    if (flag_[i] != rhs.flag_[i])
      return false;
  return true;
}

// ostream operator
std::ostream& kvalobs::operator<<(std::ostream& output,
                                  const kvalobs::kvControlInfo& kd)
{
  output << "[";
  for (int i=0; i<kvDataFlag::size-1; i++) {
    output << kd.cflag(i) << "|";
  }
  output << kd.cflag(kvDataFlag::size-1) << "]";

  return output;
}


/*
  Controlinfo-flag routine.
  get controlflag in controlinfo - based on checktype
  return false if unknown checktype
*/
bool kvControlInfo::getControlFlag(const std::string& medium_qcx,
                                   int& control)
{
  if (controlPart_.count(medium_qcx) > 0) {
    control= flag(controlPart_[medium_qcx]);
    return true;
  }
  CERR("kvControlInfo::getControlFlag ERROR. " <<
       "unknown medium_qcx:"  << medium_qcx << endl);
  return false;
}

/*
  Controlinfo-flag routine.
  set controlflag in controlinfo - based on checktype,
  calls setFqclevel!
  return false if unknown checktype
*/
bool kvControlInfo::setControlFlag(const std::string& medium_qcx,
                                   const int& control,
                                   bool setfqcl)
{
  if (controlPart_.count(medium_qcx) > 0) {
    int nibble = controlPart_[medium_qcx];
    set
      (nibble, control);

    if (setfqcl)
      setFqclevel(medium_qcx);
    return true;
  }
  CERR("kvControlInfo::setControlFlag ERROR. " <<
       "unknown medium_qcx:"  << medium_qcx << endl);
  return false;
}

/*
  Controlinfo-flag routine.
  force controlflag in specified part of controlinfo
*/
bool kvControlInfo::setControlFlag(const kvQCFlagTypes::c_flags cf,
                                   const int& control,
                                   bool setfqcl)
{
  int nibble = static_cast<int>(cf);
  set
    (nibble, control);

  if (setfqcl)
    setFqclevel();
  return true;
}


/*
  Controlinfo-flag routine.
  Check if value for one control part in 'bad'-list
*/
bool kvControlInfo::iznogood(const std::string& medium_qcx) const
{
  if ( controlPart_.count(medium_qcx) > 0 ) {
    int index   = controlPart_[medium_qcx];
    int control = flag(index);
    return ( badValues_[index].count(control) == 1 );
  }
  CERR("kvControlInfo::iznogood ERROR. " <<
       "unknown medium_qcx:"  << medium_qcx << endl);
  return false;
}


/*
  Controlinfo-flag routine.
  set the FqcLevel flag in controlinfo dataflags
  (Nibble 0)
*/
void kvControlInfo::setFqclevel()
{
  // REMOVED 2003-12-18 !!!
  return;

  // find completion level for each main QC-type
  bool completed[num_mainqcx];

  for (int i=0; i<num_mainqcx; i++)
    completed[i] = true;

  std::map<std::string, kvQCFlagTypes::main_qc>::const_iterator itr;

  for (itr=mainQCX_.begin(); itr!= mainQCX_.end(); itr++) {

    std::string medium_qcx = itr->first;
    if (controlPart_.count(medium_qcx) > 0) {
      // AND together completion-status for each medium_qcx
      completed[itr->second] &= (flag(controlPart_[medium_qcx]) > 0);

    } else {
      CERR("kvControlInfo::setFqclevel ERROR. " <<
           "unknown medium_qcx:"  << medium_qcx << endl);
    }

  }


  COUT("kvControlInfo::setFqclevel checks completed. " <<
       "qc1:"  << (completed[0]  ? "TRUE " : "FALSE ") <<
       "qc2d:" << (completed[1]  ? "TRUE " : "FALSE ") <<
       "qc2m:" << (completed[2]  ? "TRUE " : "FALSE ") <<
       "hqc:"  << (completed[3]  ? "TRUE " : "FALSE ") << endl);

  int c = 0;
  // final flag is simple bitmask
  for (int i=0; i<num_mainqcx; i++)
    if (completed[i])
      c = c | (1 << i);

  //COUT("kvControlInfo::setFqclevel final flag:" << c << endl);

  set(0, c);
}


/*
  Controlinfo-flag routine.
  set the FqcLevel flag in controlinfo dataflags
  (Nibble 0)
*/
void kvControlInfo::setFqclevel(const std::string& medium_qcx)
{
  if (mainQCX_.count(medium_qcx) > 0) {
    int c = flag(0);
    int mqc= int(mainQCX_[medium_qcx]);
    c |= (1 << mqc);

//    COUT("kvControlInfo::setFqclevel final flag:" << c << endl);

    set
      (0, c);
    return;
  }

  CERR("kvControlInfo::setFqclevel ERROR. " <<
       "unknown medium_qcx:"  << medium_qcx << endl);
}

bool kvControlInfo::qc1Done() const
{
  return flag( f_fr ) or flag( f_fcc ) or flag( f_fs ) or flag( f_fnum ) or
      flag( f_fpos ) or flag( f_fcp ) or flag( f_fd ) or flag( f_fpre ) or
      flag( f_fcombi );
}

bool kvControlInfo::qc2dDone() const
{
  return false;//flag( f_fs ) or flag( f_ftime ) or flag( f_fw ) or flag( f_fstat );
}

bool kvControlInfo::qc2mDone() const
{
  return false;//flag( f_fclim ) or flag( f_fd );
}

bool kvControlInfo::qc2Done() const
{
  return qc2dDone() or qc2mDone();
}

bool kvControlInfo::hqcDone() const
{
  return flag( f_fhqc );
}


/*
  convenience-functions to set/get the missing-flag (QC1-0)
*/
void kvControlInfo::MissingFlag(const int& v)
{
  if (v >= kvQCFlagTypes::status_ok &&
      v <= kvQCFlagTypes::status_orig_and_corr_missing) {
    set
      (f_fmis, v);

  } else
    CERR("kvControlInfo::MissingFlag ERROR. " <<
         "illegal missing-value:"  << v << endl);
}

int kvControlInfo::MissingFlag() const
{
  return flag(f_fmis);
}



/*
  ------------------------------------------------------------------
  kvUseInfo Constructors
  ------------------------------------------------------------------
*/

kvUseInfo::kvUseInfo()
    : kvDataFlag()
{
  clear();
}

kvUseInfo::kvUseInfo(const unsigned char f[kvDataFlag::size])
    : kvDataFlag(f)
{}

kvUseInfo::kvUseInfo(const std::string& s)
    : kvDataFlag(s)
{}

kvUseInfo::kvUseInfo(const kvUseInfo& df)
{
  clear();
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = df.flag_[i];
}

kvUseInfo::kvUseInfo(const kvDataFlag& df)
    : kvDataFlag(df)
{}

// clear flag
void kvUseInfo::clear()
{
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i]= (i < 5 ? '9' : '0');
}

// Assignment operator
kvUseInfo& kvUseInfo::operator=(const kvUseInfo &rhs)
{
  if (this == &rhs)
    return *this;

  // elementwise copy
  for (int i=0; i<kvDataFlag::size; i++)
    flag_[i] = rhs.flag_[i];

  return *this;
}

kvUseInfo& kvUseInfo::operator=(const kvDataFlag &rhs)
{
  // elementwise copy
  for (int i=0; i<kvDataFlag::size; i++)
    set
      (i, rhs.flag(i));

  return *this;
}

// Equality operator
bool kvUseInfo::operator==(const kvUseInfo& rhs) const
{
  for (int i=0; i<kvControlInfo::size; i++)
    if (flag_[i] != rhs.flag_[i])
      return false;
  return true;
}

// ostream operator
std::ostream& kvalobs::operator<<(std::ostream& output,
                                  const kvalobs::kvUseInfo& kd)
{
  output << "[";
  for (int i=0; i<kvDataFlag::size-1; i++) {
    output << kd.cflag(i) << "|";
  }
  output << kd.cflag(kvDataFlag::size-1) << "]";

  return output;
}



/*
  set Useinfo-flags, based on values in a controlflag.
*/
bool kvUseInfo::setUseFlags(const kvControlInfo& cinfo)
{
  int ui[kvDataFlag::size];

  for (int i=0; i<kvDataFlag::size; i++)
    ui[i] = flag(i);

  const int fqclevel = cinfo.flag(f_fqclevel);
  const int fr       = cinfo.flag(f_fr);
  const int fcc      = cinfo.flag(f_fcc);
  const int fs       = cinfo.flag(f_fs);
  const int fnum     = cinfo.flag(f_fnum);
  const int fpos     = cinfo.flag(f_fpos);
  const int fmis     = cinfo.flag(f_fmis);
  const int ftime    = cinfo.flag(f_ftime);
  const int fw       = cinfo.flag(f_fw);
  const int fstat    = cinfo.flag(f_fstat);
  const int fcp      = cinfo.flag(f_fcp);
  const int fclim    = cinfo.flag(f_fclim);
  const int fd       = cinfo.flag(f_fd);
  const int fpre     = cinfo.flag(f_fpre);
  const int fcombi   = cinfo.flag(f_fcombi);
  const int fhqc     = cinfo.flag(f_fhqc);

  //IDLOGDEBUG("html", "setUseFlags: fhqc=" << fhqc << endl <<
  //           "qclevel=" << qclevel<< endl);


  // useinfo(0) : "Kontrollniv� passert"
  if ( cinfo.qc1Done() and cinfo.qc2Done() and cinfo.hqcDone() )             ui[ 0 ] = 1;
  else if ( ! cinfo.qc1Done() and cinfo.qc2Done() and cinfo.hqcDone() )      ui[ 0 ] = 2;
  else if ( cinfo.qc1Done() and ! cinfo.qc2Done() and cinfo.hqcDone() )      ui[ 0 ] = 3;
  else if ( ! cinfo.qc1Done() and ! cinfo.qc2Done() and cinfo.hqcDone() )    ui[ 0 ] = 4;
  else if ( cinfo.qc1Done() and cinfo.qc2Done() and ! cinfo.hqcDone() )      ui[ 0 ] = 5;
  else if ( ! cinfo.qc1Done() and cinfo.qc2Done() and ! cinfo.hqcDone() )    ui[ 0 ] = 6;
  else if ( cinfo.qc1Done() and ! cinfo.qc2Done() and ! cinfo.hqcDone() )    ui[ 0 ] = 7;
  else                                                                       ui[ 0 ] = 9;



  // useinfo(1) : "Avvik fra normert observasjon"
  // NB: After useinfo[6] and useinfo[7]
  ui[ 1 ] = 9;
  //if ( cinfo.qc1Done() )
  {
    if (fmis == 1 || fmis == 3)
      ui[1] = 8;

    else if ( (fd  ==  0 || fd == 1 ) &&
               ui[7] == 0 )
      ui[1] = 0;

    else if ( (fd  ==  0 || fd ==1 ) &&
               ui[7]  > 0 )
      ui[1] = 1;

    else if ( fd == 3    &&
              ui[7] == 0 )
      ui[1] = 2;

    else if ( (fd == 2 || fd == 6 || fd == 7 || fd == 0xA || fd == 0xB) &&
               ui[7] == 0 )
      ui[1] = 3;

    else if ( fd == 3   &&
              ui[7] > 0 )
      ui[1] = 4;

    else if ( (fd == 2 || fd == 6 || fd == 7 || fd == 0xA || fd == 0xB) &&
               ui[7] > 0 )
      ui[1] = 5;
  }

  // useinfo(2) : "Kvalitetsniv� for originalverdi"
  ui[2]= 9;
  //if ( cinfo.qc1Done() or cinfo.qc2Done() or cinfo.hqcDone() )
  {
    if ( fmis == 1 || fmis == 3 )
      ui[2] = 9;
    
    else if ( fhqc == 1 || fhqc == 2 )
      ui[2]= 0;

    else if ( fr         == 6    ||
	      fcc        >= 0xA  ||
	      fcp        >= 0xA  ||
	      fs         >= 9    ||
          fnum       == 6    ||
	      fpos       >= 4    ||
          fd         == 2    ||
          fd         >= 6    ||
	      fpre       >= 4    ||
	      fw         == 3    ||
	      fclim      == 3    ||
	      fcombi     >= 9    ||
	      fhqc       >= 6 )
      ui[2]= 3;

    else if ( ((fr == 4 || fr == 5) && fcombi != 2)  ||
              fcc   == 3   ||
              fcc   == 4   ||
              fcc   == 6   ||
              fcc   == 7   ||
              fcp   == 3   ||
              fcp   == 4   ||
              fcp   == 6   ||
              fcp   == 7   ||
              fs    == 3   ||
              fnum  == 4   ||
              fnum  == 5   ||
              fpos  == 3   ||
              fstat == 2   ||
              fd    == 3 )
      ui[2]= 2;

    else if (fr    == 2    ||
             fr    == 3    ||
             fcc   == 2    ||
             fcp   == 2    ||
             fs    == 2    ||
             fs    == 4    ||
             fs    == 5    ||
             fs    == 7    ||
             fnum  == 2    ||
             fnum  == 3    ||
             fw    == 2    ||
             fclim == 2    ||
             fcombi== 2)
      ui[2]= 1;

    else if (fr    == 1    ||
             fcc   == 1    ||
             fcp   == 1    ||
             fs    == 1    ||
             fnum  == 1    ||
             fpos  == 1    ||
             fstat == 1    ||
             fclim == 1    ||
             fd    == 1    ||
             fcombi== 1 )
      ui[2]= 0;
  }

  // useinfo(3) : "Originalverdi korrigert"
  ui[ 3 ] = 9;
  //if ( cinfo.qc1Done() or cinfo.qc2Done() or cinfo.hqcDone() )
  {
  	if ( fmis == 3 )
  		ui[ 3 ] = 9;
  		
    else if ( fhqc == 6 )
    	ui[ 3 ] = 5;
    	
    else if ( fd > 5 )
    	ui[ 3 ] = 6;
    	
    else if ( fhqc == 5 )
    	ui[ 3 ] = 2;
    	
    else if ( fhqc == 7 )
    	ui[ 3 ] = 1;

  	else if ( fhqc == 1 ||
  			  fhqc == 2 )
  		 ui[ 3 ] = 0;

    else if ( fmis == 1 )
    	ui[ 3 ] = 4;
    	
    else if ( fcc   == 0xA ||
    		  fcc   == 0xB ||
    		  fcp   == 0xA ||
    		  fcp   == 0xB ||
    		  fs    == 9   ||
    		  fs    == 0xA ||
    		  fpos  == 4   ||
    		  fpre  == 4   ||
    		  fw    == 3   ||
    		  fclim == 3   ||
    		  ( fnum == 6 && fmis == 0 ) )
    	ui[ 3 ] = 3;

    else if ( fr     == 6   ||
    		  fcc    == 0xD ||
    		  fpos   == 6   ||
    		  fpre   >= 6   ||
    		  fcombi >= 9   ||
    		  fhqc   == 0xA )
    	ui[ 3 ] = 8;

	else
		ui[ 3 ] = 0;
  }


  // useinfo(4) : "Viktigste kontrollmetode"
  ui[ 4 ] = 9;
  // NB: After useinfo[2]
  //if ( cinfo.qc1Done() or cinfo.qc2Done() or cinfo.hqcDone() )
  {
	if ( ui[ 2 ] == 0 )
		ui[ 4 ] = 0;
		
	else if ( fhqc >= 5 )
		ui[ 4 ] = 9;

	else if ( fd == 2 || fd >= 6 || fr == 7)
		ui[ 4 ] = 9;

	else if ( fr    <  7 &&
			  fr    >  1 &&
			  fcc   <= 1 &&
			  fcp   <= 1 &&
			  fs    <= 1 &&
			  fnum  <= 1 &&
			  fpos  <= 1 &&
			  ftime <= 1 &&
			  fw    <= 1 &&
			  fstat <= 1 &&
			  fclim <= 1 )
		ui[ 4 ] = 1;

	else if ( ( fcc   ==   2 || 
			    fcc   ==   3 ||
			    fcc   ==   6 ||
				fcc   ==   9 ||
			    fcc   == 0xA ||
			    fcc   == 0xD ||
			    fcp   ==   2 || 
			    fcp   ==   3 ||
			    fcp   == 0xA ) &&
			  fs    <= 1 &&
			  fnum  <= 1 &&
			  fpos  <= 1 &&
			  ftime <= 1 &&
			  fw    <= 1 &&
			  fstat <= 1 &&
			  fclim <= 1 ) 
		ui[ 4 ] = 2;

	else if ( ( fs > 1 || fpos > 1 ) &&
			  fnum  <= 1 &&
			  ftime <= 1 &&
			  fw    <= 1 &&
			  fstat <= 1 &&
			  fclim <= 1 )
		ui[ 4 ] = 3;

	else if ( ( fcc ==   4 || 
			    fcc ==   7 || 
			    fcc == 0xB ||
			    fcp ==   4 ||
			    fcp ==   7 ||
			    fcp == 0xB ) &&
			  fnum  <= 1 &&
			  ftime <= 1 &&
			  fw    <= 1 &&
			  fstat <= 1 &&
			  fclim <= 1 )
		ui[ 4 ] = 4;

	else if ( ( fw > 1 || fclim > 1 ) &&
			  fnum  <= 1 &&
			  ftime <= 1 &&
			  fstat <= 1 )
		ui[ 4 ] = 5;

	else if ( ftime  > 1 &&
			  fnum  <= 1 &&
			  fstat <= 1 )
		ui[ 4 ] = 6;

	else if ( fnum > 1 &&
			  fstat <= 1 )
		ui[ 4 ] = 7;

	else if ( fstat > 1 )
		ui[ 4 ] = 8;
  }


  // useinfo( 6) : "observasjonstid i forhold til normert tidsintervall"
  // useinfo( 7) : "Forsinkelse"
  // useinfo( 8) : "Konfidens"
  // useinfo( 9) : "----"----"
  // useinfo(13) : "HQC-operat�rens identifikator"
  // useinfo(14) : "-------------"---------------"


  // useinfo(15) : "Tester som har gitt utslag (cfailed)"
  ui[15] = 0;
  if (fr    > 1)
    ui[15]++;
  if (fcc   > 1)
    ui[15]++;
  if (fcp   > 1)
    ui[15]++;
  if (fs    > 1)
    ui[15]++;
  if (fnum  > 1)
    ui[15]++;
  if (fpos  > 1)
    ui[15]++;
  if (fw    > 1)
    ui[15]++;
  if (fstat > 1)
    ui[15]++;
  if (fclim > 1)
    ui[15]++;
  if (fpre  > 1)
    ui[15]++;

  // Finally- set the useflags
  for (int i=0; i<kvDataFlag::size; i++)
    set(i, ui[i]);

  return true;
}


/*
  add 1 to error-count
*/
void kvUseInfo::addToErrorCount()
{
  int c= flag(15)+1;
  if (c > 15)
    c= 15;
  set
    (15, c);
}

/*
  error count
*/
int kvUseInfo::ErrorCount() const
{
  return flag(15);
}


/*
  Useinfo: set confidence (0-100)
*/
void kvUseInfo::Confidence(const int& c)
{
  if (c < 0 || c > 100) {
    CERR("kvUseInfo::Confidence ERROR setting illegal confidence:"
         << c << std::endl);
    return;
  }
  int i1 = c / 16;
  int i2 = c % 16;

  // confidence lies in useflag, nibbles 8 and 9
  set
    (8, i1);
  set
    (9, i2);
}

/*
  Useinfo: get confidence (0-100)
*/
int kvUseInfo::Confidence() const
{
  // confidence lies in useflag, nibbles 8 and 9
  int i1= flag(8);
  int i2= flag(9);

  return i1*16 + i2;
}

/*
  Useinfo: set HQC observer-id (0 - 255)
*/
void kvUseInfo::HQCid(const int& c)
{
  if (c < 0 || c > 255) {
    CERR("kvUseInfo::HQCid ERROR setting illegal id:"
         << c << std::endl);
    return;
  }
  int i1 = c / 16;
  int i2 = c % 16;

  // HQCid lies in useflag, nibbles 13 and 14
  set
    (13, i1);
  set
    (14, i2);
}

/*
  Useinfo: get HQC observer-id
*/
int kvUseInfo::HQCid() const
{
  // HQCid lies in useflag, nibbles 13 and 14
  int i1= flag(13);
  int i2= flag(14);

  return i1*16 + i2;
}

