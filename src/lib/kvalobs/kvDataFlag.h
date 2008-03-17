/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvDataFlag.h,v 1.1.2.2 2007/09/27 09:02:29 paule Exp $                                                       

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
#ifndef _kvDataFlag_h
#define _kvDataFlag_h

#include <kvalobs/kvQCFlagTypes.h>
#include <kvalobs/kvQcxInfo.h>
#include <ostream>
#include <string>
#include <map>
#include <list>
#include <set>
/*
   Created by DNMI/FoU/PU: a.christoffersen@met.no
   at Mon Jun 24 08:08:35 2002
*/

namespace kvalobs
{


/**
 * \addtogroup dbutility
 * @{
 */


/**
 * \brief a base class for kvControlInfo and kvUseInfo.
 *
 * 64 bit data flag implemented as 16 characters
 * Each char represents a hex-value: [0 - F]
 *
 * includes subclasses for kvalobs-specific ControlInfo
 * and UseInfo flags.
 *
*/
class kvDataFlag
{
  public:
    enum { size = 16 }; /// number of 4-bit nibbles
  protected:
    /// the actual dataflag
    unsigned char flag_[kvDataFlag::size];

    /// check if legal int
    bool legal_(const int i);
    /// convert a HEX-character (0-9,A-F) to int
    int chartoint_(const char c) const;
    /// convert an int to a HEX-character
    char inttochar_(const int i) const;

  public:
    /// flag is all zero's
    kvDataFlag();
    kvDataFlag(const std::string& s);
    kvDataFlag(const unsigned char f[kvDataFlag::size]);
    kvDataFlag(const kvDataFlag& df);

    virtual ~kvDataFlag();

    /// flag as string
    std::string flagstring() const;
    /// get one nibble as char
    unsigned char cflag(const char index) const;
    /// get one nibble
    int flag(const char index) const;
    /// set one nibble
    void set
      (const char index, const int c);
    /// clear flag
    virtual void clear();

    kvDataFlag& operator=(const kvDataFlag &rhs);
    bool operator==(const kvDataFlag& rhs) const;
    bool operator!=(const kvDataFlag& rhs) const
    {
      return !(*this == rhs);
    }
    friend std::ostream& operator<<(std::ostream& output,
                                    const kvalobs::kvDataFlag& kd);
};


/**
 * \brief  ControlInfo DataFlag
 */
class kvControlInfo : public kvDataFlag
{
  protected:
    /// controlpart --> c_flags map
    static std::map<int, kvQCFlagTypes::c_flags> lockedControlFlags_;
    /// main_qcx string --> main_qc enum map
    static std::map<std::string, kvQCFlagTypes::main_qc> mainQCXint_;

    /// list of QcxInfo (from db)
    static std::list<kvalobs::kvQcxInfo> qcxinfolist_;

    /// medium_qcx --> main_qc enum map
    static std::map<std::string, kvQCFlagTypes::main_qc> mainQCX_;
    /// medium_qcx --> controlpart map
    static std::map<std::string, int> controlPart_;

    /// list of flag values (by c_part) which trigger destruction of 'corrected'
    static std::map<int, std::set<int> > badValues_;

    /// init static structures etc.
    void init_();

  public:
    /// flag is all zero's
    kvControlInfo();
    kvControlInfo(const std::string& s);
    kvControlInfo(const unsigned char f[kvDataFlag::size]);
    kvControlInfo(const kvControlInfo& df);
    kvControlInfo(const kvDataFlag& df);

    /**
     * \brief Fill static structures from list of kvQcxInfo
     *
     * Should be called once in initialisation phase
     */
    void setQcxInfo(const std::list<kvalobs::kvQcxInfo>& qcxi);

    /**
     * \brief get controlflag - based on checktype.
     * \return  false if unknown checktype
    */
    bool getControlFlag(const std::string& medium_qcx,
                        int& control);
    /**
     * \brief set controlflag - based on checktype,
     *
     * optionally calls setFqclevel
     * \return false if unknown checktype
     */
    bool setControlFlag(const std::string& medium_qcx,
                        const int& control,
                        bool setfqcl = true);
    /**
     * \brief force controlflag in specified part of controlinfo
     *
     * optionally calls setFqclevel
     */
    bool setControlFlag(const kvQCFlagTypes::c_flags cf,
                        const int& control,
                        bool setfqcl = true);
    /**
     * \brief set the FqcLevel flag in controlinfo dataflags
     * (Nibble 0)
     *
     * Usually called by setControlFlag()
     */
    void setFqclevel();
    void setFqclevel(const std::string& medium_qcx);

    bool qc1Done() const;
    bool qc2dDone() const;
    bool qc2mDone() const;
    bool qc2Done() const;
    bool hqcDone() const;

    /**
     * \brief convenience-functions to set the missing-flag (QC1-0)
     */
    void MissingFlag(const int& v);

    /**
     * \brief convenience-functions to get the missing-flag (QC1-0)
     */
    int  MissingFlag() const;

    /**
     * \brief check if value for one control part corresponds to a rejected 'corrected'
     */
    bool iznogood(const std::string& medium_qcx) const;

    kvControlInfo& operator=(const kvControlInfo &rhs);
    kvControlInfo& operator=(const kvDataFlag &rhs);
    bool operator==(const kvControlInfo& rhs) const;
    bool operator!=(const kvControlInfo& rhs) const
    {
      return !(*this == rhs);
    }
    friend std::ostream& operator<<(std::ostream& output,
                                    const kvalobs::kvControlInfo& kd);
};


/**
 * \brief  UseInfo DataFlag
 */
class kvUseInfo : public kvDataFlag
{
  public:
    kvUseInfo();
    kvUseInfo(const std::string& s);
    kvUseInfo(const unsigned char f[kvDataFlag::size]);
    kvUseInfo(const kvUseInfo& df);
    kvUseInfo(const kvDataFlag& df);

    /// clear flag
    void clear();

    /**
     * \brief set Useinfo-flags, based on values in a controlflag.
     */
    bool setUseFlags(const kvControlInfo& cinfo);

    /**
     * \brief add 1 to error-count
    */
    void addToErrorCount();

    /**
     * \brief error count
     */
    int ErrorCount() const;

    /**
     * \brief set confidence (0-100)
    */
    void Confidence(const int& c);
    /**
     * \brief get confidence (0-100)
    */
    int Confidence() const;

    /**
     * \brief set HQC observer-id (0 - 255)
    */
    void HQCid(const int& c);
    /**
     * \brief get HQC observer-id
    */
    int HQCid() const;

    kvUseInfo& operator=(const kvUseInfo &rhs);
    kvUseInfo& operator=(const kvDataFlag &rhs);
    bool operator==(const kvUseInfo& rhs) const;
    bool operator!=(const kvUseInfo& rhs) const
    {
      return !(*this == rhs);
    }
    friend std::ostream& operator<<(std::ostream& output,
                                    const kvalobs::kvUseInfo& kd);
};


/** @} */
}

#endif
