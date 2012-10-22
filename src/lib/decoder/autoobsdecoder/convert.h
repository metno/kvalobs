/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: convert.h,v 1.14.2.4 2007/09/27 09:02:23 paule Exp $                                                       

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
#ifndef __convert_h__
#define __convert_h__

#include <iostream>
#include <limits.h>
#include <string>
#include <exception>
#include <vector>
#include <puTools/miTime.h>
#include <kvalobs/paramlist.h>
#include <kvalobs/kvData.h>
namespace kvalobs{
namespace decoder{
namespace autoobs{

/**
 * \addtogroup decodeautoobs.
 *
 * @{
 */

/**
 * \brief convert from autoobs param name too kvalobs param name.
 */
struct NameConvertDef{
   const char *oldName;
   const char *newName;
};

/**
 * \brief A name
 */
struct NameDef{
   const char *name;
};

/**
 * \brief AutoObs parameters that shall be splitted up in one or more
 *        kvalobs parameteres.
 */
struct SplitDef
{
   ///Kvalobs parameter name.
   const char *id;
   /// An index into the AutoObs parameter data where a element start
   char index;
   ///The size of the data that starts at index
   char size;
};

/**
 *  \brief A referanse to a SplitDef, from AutoObs param name.
 */
struct SplitData
{
   ///AutoObs param name.
   const char *name;

   ///A referanse to a corresponding kvalobs param name.
   SplitDef *def;
};

/**
 * \brief find a SplitDef that decode a AutoObs parameter.
 *
 * \param name AutoObs parameter name.
 * \param[out] totSize the expected size of the AutoObs parameter data.
 */
SplitDef *findSplitDef(const std::string &name, int &totSize);

/**
 * \brief convert a AutoObs parameter name to a kvalobs parameter name
 *        without spliting.
 */
std::string convertName(const std::string &nameToConvert);
//     bool        textParam(const std::string &paramName);

/**
 * \brief excpetion
 */
class BadFormat : public std::exception{
   std::string reason;
public:
   explicit BadFormat(const std::string &reason_):reason(reason_){
   }

   virtual ~BadFormat()throw(){
   }

   const char *what()const throw(){ return reason.c_str();}
};

/**
 * \brief exception.
 */
 class UnknownParam : public std::exception{
    std::string reason;
 public:
    explicit UnknownParam(const std::string &reason_):reason(reason_){
    }

    virtual ~UnknownParam()throw(){
    }

    const char *what()const throw(){ return reason.c_str();}
 };

 /**
  * \brief Helper class to hold data elements while decoding a message.
  */
 class DataElem{
    DataElem();
    int          id_;
    std::string  val_;
    int          sensorno_;
    int          heigth_;
    int          mod_;

 public:
    DataElem(int id,
             const std::string &val,
             int sensorno=1,
             int height=0,
             int mod=0);
    DataElem(const DataElem &p)
    :id_(p.id_), val_(p.val_),
     sensorno_(p.sensorno_),heigth_(p.heigth_),mod_(p.mod_){
    }

    bool valid()const{ return id_>-1;}

    DataElem& operator=(const DataElem &p);
    int         id()const      { return id_;}
    int         sensorno()const{ return sensorno_;}
    bool        fVal(float &f)const;
    std::string sVal()const   { return val_;}
    int         height()const  { return heigth_;}
    int         mod()const     { return mod_;}
 };

 /**
  * \brief The class that controlls the decoding of autoobs data
  *        to kvalobs data.
  */
 class DataConvert{
 public:
    struct RRRtr{
       int RRR; //RRR fra _RRRtr
       int tr;  //tr fra _RRRtr
       int rt;  //Rt fra _4RtWdWdWd
       int ir;
       bool RR_N; //Has at least one RR_N parameter,
       //where N is 1,2,3,6,9,12,15,18 or 24.

       RRRtr():RRR(INT_MAX), tr(-1), rt(0), ir(-1), RR_N(false){}
       RRRtr(const RRRtr &r):RRR(r.RRR), tr(r.tr), rt(r.rt), ir(r.ir), RR_N(false){}
       RRRtr& operator=(const RRRtr &r){
          if(this!=&r){
             RRR=r.RRR;
             tr=r.tr;
             rt=r.rt;
             ir=r.ir;
             RR_N=r.RR_N;
          }

          return *this;
       }

       float  RR(int &paramid, const miutil::miTime &obstime);
    };

    struct SaSdEmEi {
       std::string sa;
       std::string sd;
       std::string em;
       std::string ei;
       bool  hasSa;
       bool  hasSd;
       bool  hasEm;
       bool  hasEi;

       SaSdEmEi()
       :hasSa( false ), hasSd( false ),
        hasEm( false ), hasEi( false )
       {}

       SaSdEmEi( const SaSdEmEi &cc )
       : sa(cc.sa), sd(cc.sd),em(cc.em), ei(cc.ei),
         hasSa( cc.hasSa ), hasSd( cc.hasSd ),
         hasEm( cc.hasEm ), hasEi( cc.hasEi )
       {}

       SaSdEmEi& operator=(const SaSdEmEi &rhs ) {
          if( this != &rhs ) {
             sa = rhs.sa;
             sd = rhs.sd;
             em = rhs.em;
             ei = rhs.ei;
             hasSa = rhs.hasSa;
             hasSd = rhs.hasSd;
             hasEm = rhs.hasEm;
             hasEi = rhs.hasEi;
          }
          return *this;
       }

       static bool dataSa( kvData &data, const SaSdEmEi &sa, const kvData &saSdEmTemplate );
       static bool dataSd( kvData &data, const SaSdEmEi &sd, const kvData &saSdEmTemplate );
       static bool dataEm( kvData &data, const SaSdEmEi &em, const kvData &saSdEmTemplate );
       static bool dataEi( kvData &data, const SaSdEmEi &ei, const kvData &saSdEmTemplate );

    };

 private:
    DataConvert();
    DataConvert(const DataConvert&);
    DataConvert& operator=(const DataConvert&);

    bool allCh(const std::string &val, char ch);

    bool allSlash(const std::string &val);

    bool decodeSpDef(std::vector<DataElem>  &data,
                     SplitDef               *spDef,
                     const std::string      &param,
                     const std::string      &val,
                     int                    sensor,
                     int                    height,
                     int                    mod);


    /**
     * \exception BadFormat, UnknownParam
     */
    void decodeVal(std::vector<DataElem>  &data,
                   const miutil::miTime   &obsTime,
                   const std::string      &param,
                   const std::string      &val,
                   int                    sensor,
                   int                    height,
                   int                    mod);

    bool decodeParam(const std::string &param,
                     std::string       &name,
                     int               &sensor,
                     int               &height,
                     int               &mod);


    miutil::miTime convertToMiTimeFromHHMM(const miutil::miTime &obst,
                                           const std::string &val);

    ParamList &paramList;

    bool  hasRRRtr_;
    RRRtr RRRtr_;
    bool hasSa, hasSd, hasEm, hasEi; //Is the station setup with Sa, SD, Em and/or Ei.
    SaSdEmEi saSdEm_;
    std::string logid;

 public:
    DataConvert(ParamList &p, const std::string &logid );

    void setSaSdEmEi( const std::string &sa_sd_em);

    /**
     * \exception BadFormat, UnkownParameter
     */
    std::vector<DataElem> convert(const std::string &param,
                                  const std::string &val,
                                  const miutil::miTime &obsTime);

    void resetRRRtr(){ hasRRRtr_=false; RRRtr_=RRRtr();}
    bool hasRRRtr(RRRtr &rr){ rr=RRRtr_;
    return hasRRRtr_ && !rr.RR_N;}

    void resetSaSdEm(){ saSdEm_ = SaSdEmEi(); }
    bool hasSaSdEmEi( SaSdEmEi &saSdEm );
 };

 /** @} */
}
}
}

#endif
