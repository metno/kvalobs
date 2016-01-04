/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: file.h,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#ifndef __File_Bm314_h__
#define __File_Bm314_h__

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

namespace dnmi {
namespace file {

/**
 * \addtogroup fileutil
 *
 * @{
 */

/**
 * \brief A helper class to access metadata for a file.
 */
class File {
  std::string name_;
  struct stat stat_;

 public:
  /**
   * \brief default constructor that set the instance to invalid.
   */
  File() {
  }
  ;

  /**
   * \brief a constructor that try to stat the file given with name.
   *
   * The construction can fail if the file given with \a name dont
   * exist. Use \a ok() th check the status after construction.
   * 
   * \param name the file to stat.
   */
  File(const std::string &name)
      : name_(name) {
    if (!reStat())
      name_.erase();
  }

  /**
   * \brief a constructor that initialize the instance with a 
   * filename and a stat structure.
   *
   * \param name the file to stat.
   * \param stat the stat structure that represent the file.
   */

  File(const std::string &name, struct stat &stat)
      : name_(name),
        stat_(stat) {
  }

  /**
   * \brief copy constructor.
   */
  File(const File &f)
      : name_(f.name_),
        stat_(f.stat_) {
  }

  /**
   * \brief assign a the rhs to this instance.
   */
  File& operator=(const File &rhs) {
    if (this != &rhs) {
      name_ = rhs.name_;
      stat_ = rhs.stat_;
    }

    return *this;
  }

  /**
   * \brief Check the status of this instance.
   * \return true if this instance represen a file.
   */
  bool ok() const {
    return !name_.empty();
  }

  /**
   * \brief Check the stat for the file this instance represent 
   * again.
   *
   * \return true if the stat check succeeded.
   */
  bool reStat() {
    if (name_.empty())
      return false;

    if (stat(name_.c_str(), &stat_) < 0)
      return false;
    else
      return true;
  }

  /**
   * \brief return the path part of the name.
   *
   * \return the path to the file.
   */
  std::string path() const;

  /**
   * \brief return the filename part of the name.
   *
   * \return the filename.
   */
  std::string file() const;

  /**
   * \brief The name of the file on the form path/name. 
   * 
   * a empty string is returned if the instance dont represent any file.
   *
   * \return The name of the file or an empty string.
   */
  std::string name() const {
    return name_;
  }

  /**
   * \brief last modification time.
   * \return last modificatio time.
   */
  time_t mtime() const {
    return (name_.empty() ? 0 : stat_.st_mtime);
  }

  /**
   * \brief last access time.
   * \return last access time.
   */
  time_t atime() const {
    return (name_.empty() ? 0 : stat_.st_atime);
  }

  /**
   * \brief last change of inode time.
   * \return last change of inode time.
   */
  time_t ctime() const {
    return (name_.empty() ? 0 : stat_.st_ctime);
  }

  /**
   * \brief The size of the file.
   * \return The size of the file in byte.
   */
  off_t size() const {
    return (name_.empty() ? 0 : stat_.st_size);
  }

  /**
   * \brief userid to the owner of the file.
   * \return userid to the owner of the file.
   */
  uid_t uid() const {
    return (name_.empty() ? 0 : stat_.st_uid);
  }

  /**
   * \brief groupid to the owner of the file.
   * \return group to the owner of the file.
   */
  gid_t gid() const {
    return (name_.empty() ? 0 : stat_.st_gid);
  }

  /**
   * \brief The protection mode to the file.
   * \return Protection mode to the file.
   */
  mode_t mode() const {
    return (name_.empty() ? 0 : stat_.st_mode);
  }

  /**
   * \brief Number of hardlinks to the file.
   */
  nlink_t nlink() const {
    return (name_.empty() ? 0 : stat_.st_nlink);
  }

  /**
   * \brief Is the file a regular file.
   * \return true if the file is a regular file.
   */
  bool isFile() const {
    return S_ISREG(mode());
  }

  /**
   * \brief Is the file a directory.
   * \return true if the file is a directory.
   */
  bool isDir() const {
    return S_ISDIR(mode());
  }

  /**
   * \brief Is the file a FIFO.
   * \return true if the file is a FIFO.
   */
  bool isFifo() const {
    return S_ISFIFO(mode());
  }

  /**
   * \brief Is the file a symlink.
   * \return true if the file is a symlink.
   */
  bool isSymLink() const {
    return S_ISLNK(mode());
  }

  /**
   * \brief Is the file a socket.
   * \return true if the file is a socket.
   */
  bool isSocket() const {
    return S_ISSOCK(mode());
  }

  /**
   * \brief Is the file is a block device.
   * \return true if the file is a block device.
   */
  bool isBlock() const {
    return S_ISBLK(mode());
  }

  /**
   * \brief Owner can write to the file.
   * \return true if the owner can write to the file.
   */
  bool ownerCanWrite() const {
    return mode() & S_IWUSR;
  }

  /**
   * \brief Owner can read the file.
   * \return true if the owner can read the file.
   */
  bool ownerCanRead() const {
    return mode() & S_IRUSR;
  }

  /**
   * \brief Owner can execute the file.
   * \return true if the owner can execute the file.
   */
  bool ownerCanExecute() const {
    return mode() & S_IXUSR;
  }

  /**
   * \brief Group can write to the file.
   * \return true if the group can write to the file.
   */
  bool groupCanWrite() const {
    return mode() & S_IWGRP;
  }

  /**
   * \brief Group can read the file.
   * \return true if the group can read the file.
   */
  bool groupCanRead() const {
    return mode() & S_IRGRP;
  }

  /**
   * \brief Group  can execute the file.
   * \return true if the group can execute the file.
   */

  bool groupCanExecute() const {
    return mode() & S_IXGRP;
  }

  /**
   * \brief Other can write to the file.
   * \return true if the other can write to the file.
   */
  bool otherCanWrite() const {
    return mode() & S_IWOTH;
  }

  /**
   * \brief Other can read the file.
   * \return true if other can read the file.
   */
  bool otherCanRead() const {
    return mode() & S_IROTH;
  }

  /**
   * \brief Other can execute the file.
   * \return true if other can execute the file.
   */
  bool otherCanExecute() const {
    return mode() & S_IXOTH;
  }

  /**
   * \brief is the set UID bit set.
   * \return true if the set UID bit is set.
   */
  bool isSetUID() const {
    return mode() & S_ISUID;
  }

  /**
   * \brief is the set GID bit set.
   *
   * The set GID bit (S_ISGID) has several special uses: For a directory 
   * it indicates  that  BSD  semantics is to be used for that directory: 
   * files created there inherit their group ID from the directory, not  
   * from  the effective  gid  of  the creating process, and directories 
   * created there will also get the S_ISGID bit set.  For a file that 
   * does not  have  the group  execution  bit (S_IXGRP) set, it indicates
   * mandatory file/record locking.
   *
   * \return true if the GID bit is set.
   */
  bool isSetGID() const {
    return mode() & S_ISGID;
  }

};
}
}

#endif
