/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kv2kvDecoder.h,v 1.8.2.5 2007/09/27 09:02:29 paule Exp $

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
#ifndef __kvalobs_decoder_kv2kvDecoder_h__
#define __kvalobs_decoder_kv2kvDecoder_h__

#include <decoderbase/decoder.h>
#include <kvalobs/kvDbGate.h>
#include <decodeutility/kvalobsdata.h>
#include <list>
#include <stdexcept>
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>

namespace kvalobs {

namespace decoder {

namespace kv2kvDecoder {

class kv2kvDecoder : public DecoderBase, public boost::noncopyable {
 public:
  kv2kvDecoder(dnmi::db::Connection &con, const ParamList &params,
               const std::list<kvalobs::kvTypes> &typeList,
               const std::string &obsType, const std::string &obs,
               int decoderId = -1);

  virtual ~kv2kvDecoder();

  virtual std::string name() const {
    return "kv2kvDecoder";
  }

  virtual DecodeResult execute(std::string & msg);

 private:
  kvDbGate dbGate;

  void saveInRejectDecode();
  void parse(serialize::KvalobsData & data, const std::string & obs) const;
  void verifyAndAdapt(serialize::KvalobsData & data,
                      std::list<kvalobs::kvData> & out);
  void save2(const std::list<kvalobs::kvData> & dl, const std::list<kvalobs::kvTextData> & tdl);
  //     void save( const serialize::KvalobsData & data );
  void save(const std::list<kvalobs::kvData> & dl,
            const std::list<kvalobs::kvTextData> & tdl);
  void markAsFixed(const serialize::KvalobsData::RejectList & rejectedMesage);

  typedef boost::shared_ptr<const kvalobs::kvData> kvDataPtr;
  kvDataPtr getDbData(const kvalobs::kvData d);
  void verify(const kvalobs::kvData & d, kvDataPtr dbData) const;
  void adapt(kvalobs::kvData & d, kvDataPtr dbData, bool overwrite) const;
  void invalidatePrevious(serialize::KvalobsData & data);
  void invalidatePreviousData(
      serialize::KvalobsData & data,
      const std::list<serialize::KvalobsData::InvalidateSpec> & inv);
  void invalidatePreviousTextData(
      serialize::KvalobsData & data,
      const std::list<serialize::KvalobsData::InvalidateSpec> & inv);

  struct DecoderError : public std::runtime_error {
    const kvalobs::decoder::DecoderBase::DecodeResult res;
    DecoderError(kvalobs::decoder::DecoderBase::DecodeResult res_,
                 const std::string & reason)
        : std::runtime_error(reason),
          res(res_) {
    }
  };

  void setChecked( const std::string &obsType );
  
  serialize::KvalobsData data;
  DecodeResult parseResult_;
  std::string parseMessage_;
  const boost::posix_time::ptime tbtime;
  bool checked_;
};

}

}

}

#endif //__kvalobs_decoder_kv2kvDecoder_h__
