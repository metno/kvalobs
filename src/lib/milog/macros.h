/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: macros.h,v 1.1.2.2 2007/09/27 09:02:31 paule Exp $                                                       

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
#ifndef __milog_macros_h__
#define __milog_macros_h__

#include <milog/miloghelper.h>

/**
 * \addtogroup milog
 * @{
 */

#define LOGFATAL(msg) {\
                        milog::LogFatal __milog_fatal_xxx2334__; \
                       __milog_fatal_xxx2334__ << msg;   \
                     }

#define LOGERROR(msg) {\
                        milog::LogError __milog_error_xxx2334__; \
                       __milog_error_xxx2334__ << msg;   \
                     }


#define LOGWARN(msg) {\
                        milog::LogWarn __milog_warn_xxx2334__; \
                       __milog_warn_xxx2334__ << msg;   \
                     }


#define LOGINFO(msg) {\
                        milog::LogInfo __milog_info_xxx2334__; \
                       __milog_info_xxx2334__ << msg;   \
                     }

#define LOGDEBUG(msg) {\
                        milog::LogDebug __milog_debug_xxx2334__; \
                       __milog_debug_xxx2334__ << msg;   \
                     }

#define LOGDEBUG1(msg) {\
                        milog::LogDebug1 __milog_debug1_xxx2334__; \
                       __milog_debug1_xxx2334__ << msg;   \
                     }

#define LOGDEBUG2(msg) {\
                        milog::LogDebug2 __milog_debug2_xxx2334__; \
                       __milog_debug2_xxx2334__ << msg;   \
                     }

#define LOGDEBUG3(msg) {\
                        milog::LogDebug3 __milog_debug3_xxx2334__; \
                       __milog_debug3_xxx2334__ << msg;   \
                     }

#define LOGDEBUG4(msg) {\
                        milog::LogDebug4 __milog_debug4_xxx2334__; \
                       __milog_debug4_xxx2334__ << msg;   \
                     }

#define LOGDEBUG5(msg) {\
                        milog::LogDebug5 __milog_debug5_xxx2334__; \
                       __milog_debug5_xxx2334__ << msg;   \
                     }

#define LOGDEBUG6(msg) {\
                        milog::LogDebug6 __milog_debug6_xxx2334__; \
                       __milog_debug6_xxx2334__ << msg;   \
                     }



#define IDLOGFATAL(id, msg) {\
                        milog::LogFatal __milog_fatal_xxx2334__(id, ""); \
                       __milog_fatal_xxx2334__ << msg;   \
                     }

#define IDLOGERROR(id, msg) {\
                        milog::LogError __milog_error_xxx2334__(id, ""); \
                       __milog_error_xxx2334__ << msg;   \
                     }


#define IDLOGWARN(id, msg) {\
                        milog::LogWarn __milog_warn_xxx2334__(id, ""); \
                       __milog_warn_xxx2334__ << msg;   \
                     }


#define IDLOGINFO(id, msg) {\
                        milog::LogInfo __milog_info_xxx2334__(id, ""); \
                       __milog_info_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG(id, msg) {\
                        milog::LogDebug __milog_debug_xxx2334__(id, ""); \
                       __milog_debug_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG1(id, msg) {\
                        milog::LogDebug1 __milog_debug1_xxx2334__(id, ""); \
                       __milog_debug1_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG2(id, msg) {\
                        milog::LogDebug2 __milog_debug2_xxx2334__(id, ""); \
                       __milog_debug2_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG3(id, msg) {\
                        milog::LogDebug3 __milog_debug3_xxx2334__(id, ""); \
                       __milog_debug3_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG4(id, msg) {\
                        milog::LogDebug4 __milog_debug4_xxx2334__(id, ""); \
                       __milog_debug4_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG5(id, msg) {\
                        milog::LogDebug5 __milog_debug5_xxx2334__(id, ""); \
                       __milog_debug5_xxx2334__ << msg;   \
                     }

#define IDLOGDEBUG6(id, msg) {\
                        milog::LogDebug6 __milog_debug6_xxx2334__(id, ""); \
                       __milog_debug6_xxx2334__ << msg;   \
                     }




#define PUSH_LOG_CONTEXT(ctxt) {\
                      milog::Logger::logger().push(ctxt); \
                      }

#define POP_LOG_CONTEXT {milog::Logger::logger().pop();}

#define PUSH_IDLOG_CONTEXT(id, ctxt) {\
                      milog::Logger::logger(id).push(ctxt); \
                      }

#define POP_IDLOG_CONTEXT(id) {milog::Logger::logger(id).pop();}

/** @} */

#endif
