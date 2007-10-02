/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: MessageQue.h,v 1.1.6.1 2007/09/27 09:02:46 paule Exp $                                                       

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
#ifndef __MessageQue_h__
#define __MessageQue_h___
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition.hpp>
#include <deque>
#include <exception>


class MessageQueCancel : public std::exception{
  std::string reason;
 public:
  MessageQueCancel():reason("MessageQueCancel Exception!"){
  }
  ~MessageQueCancel()throw(){}
  
  const char *what()const throw(){ return reason.c_str();}
};

class MessageQueEmpty: public std::exception{
  std::string reason;
 public:
  MessageQueEmpty():reason("MessageQueEmpty exception!"){
  }
  ~MessageQueEmpty()throw(){}
  
  const char *what()const throw(){ return reason.c_str();}
}; 



/**
 * MessageQue implementerer en meldingskø for bruk i flertråd programmer.
 * Ved initialisering av køen angir man maksimal størrelse på køen. Maksimal
 * størrelse kan være ubegrenset. Hvis man prøver å legge inn et element i 
 * køen eller å  hente ut et element når køen er tom vil køen blokkere tråden. 
 */

template <class MessageType>
class MessageQue
{
    MessageQue(const MessageQue &);
    MessageQue &operator=(const MessageQue &);

    typedef          std::deque<MessageType>                 Que;
    typedef typename std::deque<MessageType>::iterator       IQue;
    typedef typename std::deque<MessageType>::const_iterator CIQue;

    typedef boost::mutex::scoped_lock lock;

    mutable boost::mutex     mutex;
    boost::condition emptyCond;
    boost::condition fullCond;

    Que   que_;
    int   maxSize_;
    bool  cancel_;

 public:

    /**
     * MessageQue, ctor. 
     * 
     * \param maxSize, angir maksimalt antall meldinger som kan være i køen
     *                 på en gang. funksjonene post og postAndBrodcast vil
     *                 blokkere dersom køen er full. Hvis maxSize er 0 
     *                 størrelsen på køene ubegrenset.
     */
    MessageQue(int maxSize=0);
    ~MessageQue();

    /**
     *
     * /exception MessageQueCancelException.
     */
    bool post(MessageType message);
   
    MessageType get(int timeoutInSeconds=0);
    bool      isEmpty()const;
    int       size()const;
    int       maxSize()const { return maxSize_;}
    bool      cancel()const;
    void      cancel(bool flag);
};


#include "MessageQue.tcc"

#endif
