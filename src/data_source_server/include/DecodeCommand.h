/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: DecodeCommand.h,v 1.11.2.2 2007/09/27 09:02:16 paule Exp $                                                       

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
#ifndef ___DecodeCommand_h__
#define ___DecodeCommand_h__

#include <dnmithread/CommandQue.h>
#include <decoderbase/decoder.h>
#include <puTools/miString>


/**
 * \addtogroup kvDatainputd
 * @{
 */

/**
 * \brief This is the message that is passed to the
 * work threads.
 */
 
class DecodeCommand : public dnmi::thread::CommandBase{
  DecodeCommand();
  DecodeCommand(const DecodeCommand &);
  DecodeCommand& operator=(const DecodeCommand &);

  kvalobs::decoder::DecoderBase  *decoder;
  kvalobs::decoder::DecoderBase::DecodeResult result;
  miutil::miString               msg;
  dnmi::thread::CommandQue       resQue;
  
  
  friend class DataSrcApp;
  
  kvalobs::decoder::DecoderBase* getDecoder(){ return decoder;}

  ~DecodeCommand();
  DecodeCommand(kvalobs::decoder::DecoderBase *decoder_);
  
 public:
  kvalobs::decoder::DecoderBase::DecodeResult getResult()const{ return result;}
  
  miutil::miString getMsg()const{ return msg;}

  kvalobs::kvStationInfoList& getInfoList();

  DecodeCommand *wait(int timeoutInSecond=0);
  void       signal();
  bool       executeImpl();
  void       debugInfo(std::iostream &info);
};

/** @} */

#endif
