/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: pidfileutil.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __dnmi_file_pidfileutil_h__
#define __dnmi_file_pidfileutil_h__

#include <string>

namespace dnmi{
  namespace file {

    /**
     * \addtogroup fileutil
     *
     * @{
     */

  
   /**
    * Create a name for the pidfile. The name is on the form:
    * 
    * 'localstatedir'/kvalobs/run/progname-nodename.pid
    * 
    * Where nodename is the name of the machine we are running on.
    * 
    * @param progname the progname part of the pidfile name.
    * @return The name to use as the pidfile.
    */  
    std::string createPidFileName( const std::string &progname );
  
    /**
     * \brief Create a pidfile.
     * 
     * Unconditonaly write the pidfile even if a file with
     * the name 'em pidfile allready exist.
     *
     * \param pidfile the name and path to the pidfile to create.
     * \return true on success and false otherwise.
     */
    bool createPidFile(const std::string &pidfile);

    /**
     * \brief Remove a pidfile.
     * 
     * Unconditonaly remove the pidfile.
     *
     * \param pidfile the name and path to the pidfile to remove.
     * \return true on success and false otherwise.
     */
    bool deletePidFile(const std::string &pidfile);

    /**
     * \brief read a pid from a pidfile.
     *
     * Returns true if the file exist and the pid in the file is in fact
     * represent a running process. It return also true if the file exist
     * but we cant read it. The param \a error is then set to true.
     *
     * \param pidfile the pidfile we shall look up the pid in.
     * \param error   an out parameter that indicate an error while
     *        reading the pidfile.
     * \return If it returns true and error=false, the pid in the pidfile 
     *         represent a running process. If \a error is false we cant 
     *         decide and return true.
     *         False there is no pidfile or the pid in the pidfile dos'nt
     *         represent a running process.
     *
     */
    bool isRunningPidFile(const std::string &pidfile, bool &error);


    /**
     * \brief read a pid from an existing pidfile. 
     *
     * \param pidfile The full path to the pidfile.
     * \param pid     The pid in the pidfile.
     * \param fileExist true if the pidfile exist an false if it 
     *        dont exist.
     * \return true if the file exixt and we could read the pid from the file.
     *         Return false if the file dont exist or if we could NOT read the
     *         pid from the file. 
     *
     * \note If the function returns false the pidfile my or may not exist.
     *       Use the \a fileExist parameter to decide if the file exist.
     */
    bool readPidFromPidFile(const std::string &pidfile, 
			    pid_t             &pid, 
			    bool              &fileExist);


    /**
     * \brief A helper class to use to manage \em pidfiles in your
     * application.
     *
     * A pidfile is created either with the constructor or with the
     * function createPidFile. The \em pidfile is automaticaly removed when 
     * the destructor is executed. If an object of this class is instantiated 
     * in the \em main function the \em pidfile will be removed on exit
     * from main, ie when the application is about to terminate.
     *
     * \code
       Example
       
       int
       main(int argn, cgar **argv)
       {
          //Create a pidfile 
          PidFileHelper pid(string(argv[0])+".pid");

	  .....
	  .....
	  ....
	  
	  cout << "About to terminate!" << endl;
       }  <---- The pidfile is removed by the destructor of \em pid (PidFileHelper).
       \endcode
     */
    class PidFileHelper{
      PidFileHelper(const PidFileHelper &);
      PidFileHelper& operator=(const PidFileHelper &);
      
      std::string pidfile;

    public:
      PidFileHelper(){};
      
      /**
       * \brief Constructor that createa pidfile.
       */
      PidFileHelper(const std::string &pidfile);
      
      /**
       * \brief The destructor removes the pidfile.
       */
      ~PidFileHelper();

      /**
       * \brief create a pidfile.
       */
      bool createPidFile(const std::string &pidfile);
    };
    
    /** @} */
  }
}

#endif
