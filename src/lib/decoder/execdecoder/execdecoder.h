/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dummydecoder.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __kvalobs_decoder_dummydecoder_h__
#define  __kvalobs_decoder_dummydecoder_h__

#include <decoderbase/decoder.h>

namespace kvalobs{
namespace decoder{
namespace execdecoder{

/**
 * \addtogroup execdecode
 *
 * @{
 */


/**
 * \brief implements the interface  DecoderBase.
 *
 * ExecDecoder, call an external program to decode the received data.
 * The external program writes the decoded data to a file in the kldata
 * format. The decoder redirect to the kldata decoder. The kldata loads
 * the data into the database and the result returned to the data sender
 * is from kldata.
 *
 * The program to decode the data is configured in the configuration file
 * kvalobs.conf in the section:
 *
 *    kvDataInputd {
 *       ExecDecoder {
 *           aexecd = ("localhost", 6666)
 *           bindir="/home/borgem/projects/build/kvalobs_1"
 *           loglevel=debug
 *
 *           decoders {
 *              bufr {
 *                 decoder="mytest.sh"
 *              }
 *           }
 *       }
 *    }
 */
class ExecDecoder : public DecoderBase{
    ExecDecoder();
    ExecDecoder(const ExecDecoder &);
    ExecDecoder& operator=(const ExecDecoder &);

protected:
    std::string getDecoderProg();
    std::string getDecoderName();
    std::string getBindir();
    std::string loglevel();
    bool getAexecd( std::string &host, int &port );
    bool createInputFile( const std::string &filename );
    int  getProgTimeout( int defaultTimeout=10 );
    int  runProg( const std::string &cmd, const std::string &logfile );

    DecoderBase::DecodeResult  doRedirect( const std::string &kvdata, std::string &msg  );

public:
    ExecDecoder(dnmi::db::Connection     &con,
                const ParamList        &params,
                const std::list<kvalobs::kvTypes> &typeList,
                const std::string &obsType,
                const std::string &obs,
                int   decoderId=-1);

   virtual ~ExecDecoder();

   virtual std::string name()const;

   virtual DecodeResult execute(std::string &msg);
};

/** @} */
}
}
}

#endif
