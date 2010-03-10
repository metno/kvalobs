/*
  Kvalobs - Free Quality Control Software for Meteorological Observations

  $Id: kvDbBase.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#ifndef _kvDbBase_h
#define _kvDbBase_h

#include <kvdb/kvdb.h>
#include <puTools/miString.h>
#include <iostream>
#include <fstream>
#include <puTools/miTime.h>
#include <dnmithread/mtcout.h>

/* Created  by JS at Mon Aug 26 13:25:00 2002 */

namespace kvalobs {

  /**
   * \addtogroup dbutility
   *
   * @{
   */

  /**
   * \brief The base class to all kvalobs database interfaces classes.
   *
   * All kvalobs database interface classes inherit from this class
   * and must implement the methods:
   *
   * \li uniqueKey()
   * \li tableName()
   * \li toSend()
   *
   * They must also implements getter and setter functions for every field
   * in the table and a constructors that accepts all fields in the table
   * as arguments.
   *
   * For an example look at \ref kvDbBaseExample.doc "kvDbBaseExample".
   */
  class kvDbBase {

  protected:
    miutil::miString sortBy_;
    miutil::miDate julianDayThatYear(int addOn,int year=-1) const;

  public:


    /**Used to specifie an NULL value for a FLOAT field*/
    static const float       FLT_NULL;

    /**Used to specifie an NULL value for a INT field*/
    static const int         INT_NULL;

    /**Used to specifie an NULL value for a TEXT field*/
    static const std::string TEXT_NULL;

    kvDbBase();
    virtual ~kvDbBase();

    /**
     * \brief creates a SQL insert statment.
     *
     * toSend assume a SQL inser statemen on the form: \n
     * \verbatim
        INSERT INTO table VALUES( ... )
	                        ^^^^^^^
				  (*)
       \endverbatim
     *
     * It is the (*) part that is toSend return.
     *
     * It is importent to remeber to quote string, data, time and datetime
     * types. Use the quoted methods to do this.
     *
     * Take a  look at the example \ref kvDbBaseExample.doc "kvDbBaseExample"
     * to see how this is done.
     *
     * \return The value part of a SQL insert statment.
     *
     */
    virtual miutil::miString toSend()const=0;

    /**
     * \brief make quoted string of the input parameter.
     *
     * \param toQuote the string to be quoted.
     * \return a quoted version of the toQuote string.
     */
    miutil::miString quoted(const miutil::miString &toQuote) const;

    /**
     * /brief make a quoted ISO time.
     *
     * \param timeToQuote the time we want a quoted ISO time.
     * \return a quoted ISO time formatted version of timetoQuote.
     */
    miutil::miString quoted(const miutil::miTime& timeToQuote) const;

    /**
     * \brief create a quoted string versjon of a integer.
     *
     * \param intToQuote the integer to create a quoted string version of.
     * \return a quoted string version of intToQuote.
     */
    miutil::miString quoted(const int &intToQuote) const;

    /**
     * /brief is this instance initialized with walues.
     *
     * \return true if initialized and false otherwise.
     */
    bool exists() const {return sortBy_.exists();}

    /**
     * \brief generate a string that can be used to update the table from
     *        the data in this instance.
     *
     *  \return A string that can be used to update a row in the table.
     */
    virtual miutil::miString toUpdate()const;

    /**
     * \brief create a string that uniqly idenitify this instance
     *        in the table.
     *
     * \return a string that can be used in a query to uniqly indentify
     *         the data in this instance.
     */
    virtual miutil::miString uniqueKey()const =0;


    miutil::miString insertQuery(const bool replace) const;
    miutil::miString selectAllQuery() const;
    miutil::miString selectAllQuery(const miutil::miString &tblName) const;

    /**
     * \brief the table name/view class represent.
     *
     * \return the table name.
     */
    virtual const char* tableName()const=0;

    friend bool operator>(const kvDbBase& lhs,const kvDbBase& rhs);
    friend bool operator<(const kvDbBase& lhs, const kvDbBase& rhs);
    friend bool operator==(const kvDbBase& lhs, const kvDbBase& rhs);
    friend bool operator!=(const kvDbBase& lhs, const kvDbBase& rhs);

    friend std::ostream& operator<<(std::ostream&, const kvDbBase&);

  };


  bool  operator>(const kvDbBase& lhs,const kvDbBase& rhs);
  bool  operator<(const kvDbBase& lhs, const kvDbBase& rhs);
  bool  operator==(const kvDbBase& lhs, const kvDbBase& rhs);
  bool  operator!=(const kvDbBase& lhs, const kvDbBase& rhs);


  std::ostream& operator<<(std::ostream&, const kvDbBase&);

  /**
   * \example kvDbBaseExample.doc
   * Here is an example how to use kvalobs::kvDbBase. We hava a table
   * \em   types in the kvalobs database which is created with the SQL create
   * statement as:
   *
   * \verbatim
     CREATE TABLE types (
        typeid   INTEGER NOT NULL,
        format   TEXT DEFAULT NULL,
        earlyobs INTEGER DEFAULT NULL,
        lateobs  INTEGER DEFAULT NULL,
        read     TEXT DEFAULT NULL,
        obspgm   TEXT DEFAULT NULL,
        comment  TEXT DEFAULT NULL,
        UNIQUE ( typeid )
     );
   * \endverbatim
   *
   * We implement this as: \n
   * The headerfile \em kvTypes.h
   *
   * \include  kvTypes.h
   *
   * And the implementation file \em kvTypes.cc.
   *
   * \include kvTypes.cc
   */

  /** @} */
}





#endif


