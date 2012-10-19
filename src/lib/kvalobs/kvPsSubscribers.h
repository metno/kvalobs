/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvPsSubscribers.h,v 1.1.2.3 2007/09/27 09:02:30 paule Exp $

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
#ifndef __kvPsSubscribers_h__
#define __kvPsSubscribers_h__

#include <kvalobs/kvDbBase.h>
#include <miutil/commastring.h>

/* Created by DNMI/IT: borge.moe@met.no Oct 13 2006
 */


namespace kvalobs{

  /**
   * \addtogroup  dbinterface
   *
   * @{
   */


  /**
   * \brief Interface to the table ps_subscribers in the kvalobs database.
   */
class kvPsSubscribers : public kvDbBase {
private:

	miutil::miString name_;
   int              subscribertype_;
   miutil::miString comment_;
   int              delete_after_hours_;
   miutil::miString sior_;
   miutil::miTime   created_;

public:
  	kvPsSubscribers() {}
  	kvPsSubscribers(const dnmi::db::DRow &r){ set(r);}
  	kvPsSubscribers(const  miutil::miString &name,
   	             int                     subscribertype,
                   const  miutil::miString &comment,
                   int                     &delete_after_hours,
                   const  miutil::miString &sior,
                   const  miutil::miTime   &created){
			set(name, subscribertype, comment, delete_after_hours, sior, created);

    	}

  	bool set(const  miutil::miString &name,
            int                     subscribertype,
            const  miutil::miString &comment,
            int                     &delete_after_hours,
            const  miutil::miString &sior,
            const  miutil::miTime   &created);

  	bool set(const dnmi::db::DRow&);

  	const char* tableName() const {return "ps_subscribers";}
  	miutil::miString toSend() const;
  	miutil::miString toUpdate() const;
  	miutil::miString uniqueKey()const;

   /**
    * Return a subscriberid agregated from name and subscribertype.
    *
    * At the momment two subscribertypes is defined:
    *  - data subscribers       (subscribertype=0)
    *  - notify subscribers     (subscribertype=1)
    *
    * The returned subscriberid is on the form:
    *   ps_subscribertype_name. Where subscribertype is data or notify.
    *
    * Ex.
    *   If we have defined a data subscriber with name dvh the subscriberid
    *   is: ps_data_dvh.
    */
   miutil::miString subscriberid()const;

	/**
	 * Find the subscribername from an subscriberid.
	 *
	 * The name is the last part of the subscriberid.
	 *
	 * Ex
	 * 	- The name for the subscriberid ps_data_dvh is dvh.
	 *    - The name for the subscriberid ps_data_dvh_test is
	 *      dvh_test.
	 */
	static miutil::miString nameFromSubscriberid(const miutil::miString &subscriberid);

	/**
	 * Find the subscriber type from an subscriberid.
	 *
	 * The subscriber type is the second part of the subscriberid.
	 *
	 * Ex
	 * 	- The subscriber type for the subscriberid ps_data_dvh is 0.
	 *    - The subscriber type for the subscriberid ps_notify_dvh_test is
	 *      1.
	 */
	static int typeFromSubscriberid(const miutil::miString &subscriberid);


  	miutil::miString name()const { return name_; }
   int              subscribertype()const{ return subscribertype_;};
   miutil::miString comment()const { return comment_; }
   int              deleteAfterHours()const { return delete_after_hours_; }
   miutil::miString sior()const { return sior_; }
   miutil::miTime   created()const { return created_; }

   void subscribertype(int subtype) { subscribertype_=subtype;}
   void comment(const miutil::miString &comment) { comment_=comment; }
   void deleteAfterHours(int hours) { delete_after_hours_=hours; }
   void sior(const miutil::miString& sior__) { sior_=sior__; }
   void created(const miutil::miTime& created__) { created_=created__; }
};

/** @} */
};
#endif
