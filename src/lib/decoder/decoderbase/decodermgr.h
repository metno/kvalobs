/* -*- c++ -*-
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: decodermgr.h,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $

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
#ifndef __dnmi_decoder_DecoderMgr_h__
#define __dnmi_decoder_DecoderMgr_h__

#include "decoder/decoderbase/decoder.h"
#include "fileutil/dso.h"
#include "kvalobs/kvTypes.h"
#include "kvdb/kvdb.h"
#include "miconfparser/miconfparser.h"
#include <boost/noncopyable.hpp>
#include <list>
#include <string>

/**
 * \addtogroup kvdecoder
 *
 * @{
 */

extern "C" {
typedef kvalobs::decoder::DecoderBase *(*decoderFactory)(
    dnmi::db::Connection &con_, const ParamList &params,
    const std::list<kvalobs::kvTypes> &typeList, int decoderId_,
    const std::string &observationType_, const std::string &observation_);

typedef void (*releaseDecoderFunc)(kvalobs::decoder::DecoderBase *decoder);

typedef std::list<std::string> (*getObsTypes)();
typedef std::list<std::string> (*getObsTypesExt)(
    miutil::conf::ConfSection *theKvConf);

typedef void (*setKvConf)(kvalobs::decoder::DecoderBase *decoder,
                          miutil::conf::ConfSection *theKvConf);
}

namespace kvalobs {

namespace decoder {

/**
 * \brief DecoderMgr is responsible for loading of decoders.
 */
class DecoderMgr {
  struct DecoderItem : public boost::noncopyable {
    decoderFactory factory;
    releaseDecoderFunc releaseFunc;
    setKvConf setConf;
    dnmi::file::DSO *dso;
    time_t modTime;
    int decoderCount;
    int decoderId;
    std::list<std::string> obsTypes;

    DecoderItem(decoderFactory factory_, releaseDecoderFunc releaseFunc_,
                setKvConf setKvConf_, dnmi::file::DSO *dso_, time_t mTime)
        : factory(factory_), releaseFunc(releaseFunc_), setConf(setKvConf_),
          dso(dso_), modTime(mTime), decoderCount(0), decoderId(-1) {}

    ~DecoderItem() { delete dso; }
  };

  typedef std::list<DecoderItem *> DecoderList;
  typedef std::list<DecoderItem *>::iterator IDecoderList;
  typedef std::list<DecoderItem *>::const_iterator CIDecoderList;

  DecoderList decoders;
  std::string decoderPath;
  std::string soVersion;
  miutil::conf::ConfSection *theKvConf;

public:
  DecoderMgr(const std::string &decoderPath_,
             miutil::conf::ConfSection *theKvConf);
  DecoderMgr() : theKvConf(0) {};
  ~DecoderMgr();

  std::string fixDecoderName(const std::string &driver);

  void setTheKvConf(miutil::conf::ConfSection *theKvConf);
  void setDecoderPath(const std::string &decoderPath_);

  /**
   * returns true when all decoder has a decoderCount of 0.
   */
  bool readyForUpdate();
  void updateDecoders(miutil::conf::ConfSection *theKvConf);
  void updateDecoders();

  DecoderBase *findDecoder(dnmi::db::Connection &connection,
                           const ParamList &params,
                           const std::list<kvalobs::kvTypes> &typeList,
                           const std::string &obsType, const std::string &obs,
                           std::string &errorMsg);

  void releaseDecoder(DecoderBase *dec);

  int numberOfDecoders() const { return decoders.size(); }
  void obsTypes(std::list<std::string> &list);

private:
  void clearDecoders();
};

/** @} */
} // namespace decoder
} // namespace kvalobs

#endif
