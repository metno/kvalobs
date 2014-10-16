/*
 * ExecHelper.h
 *
 *  Created on: Oct 16, 2014
 *      Author: borgem
 */

#ifndef __EXECHELPER_H__
#define __EXECHELPER_H__

#include "miutil/simplesocket.h"

namespace exechelper {
void
exec( miutil::SimpleSocket *pSocket,
      const std::string &cmdstring,
      int timeout );

}


#endif
