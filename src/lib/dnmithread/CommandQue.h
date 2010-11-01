/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: CommandQue.h,v 1.1.2.2 2007/09/27 09:02:33 paule Exp $                                                       

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
#ifndef __CommandQue_bm314_h__
#define __CommandQue_bm314_h__

#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/condition.hpp>
#include <string>
#include <deque>

namespace dnmi {
  namespace thread{
    /**
     * \addtogroup threadutil
     * @{
     */

    /**
     * \brief Exception calss for the que.
     *
     * The que is suspened an dont accepts new messages.
     */
    class QueSuspended{
    };

    /**
     * \brief A base class to be used for all messages to be posted
     * on a CommandQue.
     */
    class CommandBase
      {
      protected: 
	std::string comment;
 
      public:
	
	CommandBase(){}
	CommandBase(const std::string &comment_):comment(comment_){}
	virtual ~CommandBase(){}
	
	/**
	 * \brief This command calls executeImpl and can be used as an 
	 * interceptor.
	 *
	 * It is nice to have if we need to do pre/post prossesing before
	 * executeImpl is called.
	 */
	virtual bool       execute();

	/**
	 * \brief This function implements what the command has to do.
	 */
	virtual bool       executeImpl()=0;
	virtual void       debugInfo(std::ostream &info)const;
	const std::string& getComment()const{ return comment;}
	void               setComment(const std::string &comment_)
	  {
	    comment=comment_;
	  }

	friend std::ostream& operator<<(std::ostream& os, 
					const CommandBase &c)
	  {
	    c.debugInfo(os);
	    return os;
	  }

      };



    /**
     * \brief A que to comunicate between threads.
     */
    class CommandQue: private boost::noncopyable 
      {

      protected:
	typedef boost::mutex::scoped_lock                Lock;
	typedef std::deque<CommandBase*>                 Que;
	typedef std::deque<CommandBase*>::iterator       QueIterator;
	typedef std::deque<CommandBase*>::const_iterator QueCIterator;

	boost::mutex     m;
	boost::condition cond;
	Que              que;
	bool             suspended;

      public:
	explicit  CommandQue(bool suspended=false);
	~CommandQue();
	
	void         post(CommandBase *command);
	void         postAndBrodcast(CommandBase *command);
	CommandBase  *get(int timeoutInSeconds=0);

	CommandBase  *peek(int timeoutInSeconds=0);
	
	/**
	 * \brief remove, removes the command com from the que.
	 */
	CommandBase  *remove(CommandBase *com);

	/**
	 * \brief removeAll, removes all commands currently in
	 * the que. 
	 *
	 * The commands is returned in a list so the
	 * caller can decide what shall be done with them.
	 */
	std::list<CommandBase*> *removeAll();

	/**
	 * \brief clear, remove all the command in the que.
	 */
	void         clear();

	bool         empty();
	int          size();
	void         brodcast();
	
	void         suspend();
	void         resume();
	bool         isSuspended(){ Lock lk(m); return suspended;}

	/**
	 * \brief signal, make the get function return imidetly for all 
	 * threads that is blocked on  get().
	 */
	void         signal();
      }; 

    /** @} */
  };
};

#endif
