/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: dir.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __DNMI_FILE_DIR_H_
#define __DNMI_FILE_DIR_H_

#include <sys/types.h>
#include <dirent.h>
#include <string>
#include <time.h>

namespace dnmi {
namespace file {

/**
 * \addtogroup  fileutil
 *
 * @{
 */

class File;

/**
 * \brief The base exception used by the Dir class.
 */
class DirException : public std::exception {
  std::string reason;

 public:
  DirException(const std::string &reason_)
      : reason(reason_) {
  }
  ~DirException() throw () {
  }

  const char *what() const throw () {
    return reason.c_str();
  }
};

/**
 * \brief A class that can be used to traverse a directory.
 */
class Dir {
  Dir(const Dir &);
  const Dir& operator=(const Dir &);

  DIR *dir;
  dirent *entry;
  std::string pattern;
  std::string err;
  bool casesensitive;
  std::string dirname;

 public:
  /**
   * \brief default contructor.
   * 
   * It does nothing except to initialize a Dir instance.
   */
  Dir()
      : dir(0),
        entry(0),
        casesensitive(true) {
  }

  /**
   * \brief Construct a instance that is ready to traverse the
   *        directory \em dirname.
   *
   * \param dirname The directory to traverse.
   */
  Dir(const std::string &dirname);
  ~Dir();

  /**
   * \brief  Open the directory dirname for reading.
   *
   * You can give a pattern to use when the directory is traversed. Only
   * files matching the pattern is returned by the \em next() function.
   *
   * Allowed metacharacters in the pattern is * and ?. This has the same 
   * meaning as the shell file pattern.
   *
   * \code
   Example 
   
   Search for all files matching the pattern mi*.dat in the directory
   my/path. my/path is a reletive directory to the current working 
   directory.

   Dir dir;
   
   if(!dir.open("my/path", "mi*.dat")){
   cerr << "ERROR: Cant open <my/path> for reading." << endl
   << " reason: " << dir.getErr() << endl;
   exit(1);
   }
   
   while(dir.hasNext()){
   cout << dir.next() << endl;
   } 
   \endcode
   * 
   * \param dirname The directory to read.
   * \param pattern A file pattern to use while traversing the directry.
   * \param casesensitive Should case matter.
   *
   * \return true if dirname is a directory, and we have permission
   *         to read it.
   *
   */
  bool open(const std::string &dirname = ".", const std::string &pattern = "",
            bool casesensitive = true);

  std::string getErr() const {
    return err;
  }

  /**
   * \brief operator is this a valid directory.
   *
   * \return true if this is a valid directory..
   */
  bool operator!() const {
    return !dir;
  }

  /**
   * \brief Is there a next directory entry.
   *
   * \return true if there is a next dierctory entry, 
   * false otherwise.
   *
   * \see open(const std::string &dirname,const std::string &pattern,bool  casesensitive)
   * \exception DirException on io error.
   */
  bool hasNext();

  /**
   * \brief Returns the next entry in the directory.
   *
   * The function will throw an exception on error. It is
   * an error to call next after hasNext has returned false.
   *
   * \return the name of the next entry in the directory.
   *
   * \see open(const std::string &dirname,const std::string &pattern,bool  casesensitive)
   * \exception DirException on error.
   */
  std::string next();

  /**
   * \brief Returns the next entry in the directory.
   *
   * The function will throw an exception on error. It is
   * an error to call next after hasNext has returned false.
   *
   * \param file[out] Sets the file on return. 
   * \return nothing, but the file output variable is set.
   *
   * \see open(const std::string &dirname,const std::string &pattern,bool  casesensitive)
   * \exception DirException on error.
   */
  void next(dnmi::file::File &file);

  /**
   * \brief rewind to start the directory searh from the
   * begining again..
   *
   * \return true if we succeed with the rewind, false otherwise.
   */
  bool rewind();
};

/**
 * \brief Test if \em name is a directory.
 */
bool isDir(const std::string &name);

/**
 * \brief Test if \em name is a regular file.
 */
bool isFile(const std::string &name);

/**
 * \brief return the file size in byte to the file \em name.
 */
long fileSize(const std::string &name);

/**
 * \brief Has we permission to read the file \em name.
 */
bool canRead(const std::string &name);

/**
 * \brief Has we permission to write to the file \em name.
 */
bool canWrite(const std::string &name);

/**
 * \brief When was the file last modifyed.
 */
time_t modTime(const std::string &name);

/**
 * \brief fileFactory sets the 'file' parameter and return true if 'filename' exist.
 * false is returned if the file do not exist.
 *
 * @param filename The name and path to the file to return a dnmi::file::File instance to.
 * @param[out] file On success the file parameter is filled with data. The file parameter
 * is not touched if the file do not exist.
 * @return true on success and false otherwise.
 */
bool fileFactory(const std::string &filename, dnmi::file::File &file);

/**
 * \brief fileFactory returns a pointer a File if 'filename' exist.
 * 0 is returned if the file do not exist.
 *
 * The pointer must be deleted by the caller.
 *
 * @param filename The name and path to the file to return a dnmi::file::File instance to.
 * @return A pointer to dnmi::file::File on success and 0 otherwise.
 */
File* fileFactory(const std::string &filename);

/** @} */
}
}

#endif
