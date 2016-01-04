/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: corbamacros.h,v 1.1.2.2 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <milog/milog.h>
#include <sstream>

/**
 * \defgroup corbahelper CORBA helper classes and macros
 *
 * @{
 */

#define CORBA_STD_EXCEPTION catch(CORBA::UNKNOWN &)\
                            {\
				LOGDEBUG( "\nCORBA exception UNKNOWN\n");\
			    }\
                            catch(CORBA::BAD_PARAM &)\
                            {\
				LOGDEBUG( "\nCORBA exception BAD_PARAM\n");\
			    }\
                            catch(CORBA::NO_MEMORY &)\
                            {\
				LOGDEBUG( "\nCORBA exception NO_MAMORY\n");\
			    }\
                            catch(CORBA::IMP_LIMIT &)\
                            {\
				LOGDEBUG( "\nCORBA exception IMP_LIMIT\n");\
			    }\
                            catch(CORBA::COMM_FAILURE &)\
                            {\
				LOGDEBUG( "\nCORBA exception COMM_FAILURE\n");\
			    }\
                            catch(CORBA::INV_OBJREF &)\
                            {\
				LOGDEBUG( "\nCORBA exception INV_OBJREF\n");\
			    }\
                            catch(CORBA::NO_PERMISSION &)\
                            {\
				LOGDEBUG( "\nCORBA exception NO_PERMISSION\n");\
			    }\
                            catch(CORBA::INTERNAL &)\
                            {\
				LOGDEBUG( "\nCORBA exception INTERNAL\n");\
			    }\
                            catch(CORBA::MARSHAL &)\
                            {\
				LOGDEBUG( "\nCORBA exception MARSHAL\n");\
			    }\
                            catch(CORBA::INITIALIZE &)\
                            {\
				LOGDEBUG( "\nCORBA exception INITIALIZE\n");\
			    }\
                            catch(CORBA::NO_IMPLEMENT &)\
                            {\
				LOGDEBUG( "\nCORBA exception NO_IMPLEMENT\n");\
			    }\
                            catch(CORBA::BAD_TYPECODE &)\
                            {\
				LOGDEBUG( "\nCORBA exception BAD_TYPECODE\n");\
			    }\
                            catch(CORBA::BAD_OPERATION &)\
                            {\
				LOGDEBUG( "\nCORBA exception BAD_OPERATION\n");\
			    }\
                            catch(CORBA::NO_RESOURCES &)\
                            {\
				LOGDEBUG( "\nCORBA exception NO_RESOURCES\n");\
			    }\
                            catch(CORBA::NO_RESPONSE &)\
                            {\
				LOGDEBUG( "\nCORBA exception NO_RESPONSE\n");\
			    }\
                            catch(CORBA::PERSIST_STORE &)\
                            {\
				LOGDEBUG( "\nCORBA exception PERSIST_STORE\n");\
			    }\
                            catch(CORBA::BAD_INV_ORDER &)\
                            {\
				LOGDEBUG( "\nCORBA exception BAD_INVORDER\n");\
			    }\
                            catch(CORBA::TRANSIENT &)\
                            {\
				LOGDEBUG( "\nCORBA exception TRANSIENT\n");\
			    }\
                            catch(CORBA::FREE_MEM &)\
                            {\
				LOGDEBUG( "\nCORBA exception FREE_MEM\n");\
			    }\
                            catch(CORBA::INV_IDENT &)\
                            {\
				LOGDEBUG( "\nCORBA exception INV_IDENT\n");\
			    }\
                            catch(CORBA::INV_FLAG &)\
                            {\
				LOGDEBUG( "\nCORBA exception INV_FLAG\n");\
			    }\
                            catch(CORBA::INTF_REPOS &)\
                            {\
				LOGDEBUG( "\nCORBA exception INTF_REPOS\n");\
			    }\
                            catch(CORBA::BAD_CONTEXT &)\
                            {\
				LOGDEBUG( "\nCORBA exception BAD_CONTEXT\n");\
			    }\
                            catch(CORBA::OBJ_ADAPTER &)\
                            {\
				LOGDEBUG( "\nCORBA exception OBJ_ADAPTER\n");\
			    }\
                            catch(CORBA::DATA_CONVERSION &)\
                            {\
				LOGDEBUG( "\nCORBA exception DATA_CONVERSION\n");\
			    }\
                            catch(CORBA::OBJECT_NOT_EXIST &)\
                            {\
				LOGDEBUG( "\nCORBA exception OBJECT_NOT_EXIST\n");\
			    }\
                            catch(CORBA::TRANSACTION_REQUIRED &)\
                            {\
				LOGDEBUG( "\nCORBA exception TRANSACTION_REQURIERED\n");\
			    }\
                            catch(CORBA::TRANSACTION_ROLLEDBACK &)\
                            {\
				LOGDEBUG( "\nCORBA exception TRANSACTION_ROLLEDBACK\n");\
			    }\
                            catch(CORBA::INVALID_TRANSACTION &)\
                            {\
				LOGDEBUG( "\nCORBA exception INVALID_TRANSACTION\n");\
			    }

#define CORBAMACRO_LOGDEBUG(exception, name) {                                    \
                                                std::ostringstream ost;           \
                                                                                  \
                                                ost << "\nCORBA exception "       \
						    <<name << " \n"               \
                                                    << "  completed: ";           \
										  \
					        switch(exception.completed()){  \
					          case CORBA::COMPLETED_YES:      \
		                                       ost << "YES";              \
				                  break;                          \
				                  case CORBA::COMPLETED_NO:       \
				                       ost << "NO";               \
				                  break;                          \
				                  case CORBA::COMPLETED_MAYBE:    \
				                       ost << "MAYBE";            \
				                       break;                     \
                                                }                                 \
						LOGDEBUG(ost.str());              \
					     }

#define CORBA_STD_EXCEPTION_COMPLETED catch(CORBA::UNKNOWN &ex)                 \
                                        CORBAMACRO_LOGDEBUG(ex, "UNKNOWN" )        \
                                      catch(CORBA::BAD_PARAM &ex)                  \
                                        CORBAMACRO_LOGDEBUG(ex, "BAD_PARAM" )     \
				      catch(CORBA::NO_MEMORY &ex)                  \
                                        CORBAMACRO_LOGDEBUG(ex, "NO_MEMORY" )    \
                                      catch(CORBA::IMP_LIMIT &ex)                  \
                                         CORBAMACRO_LOGDEBUG(ex, "IMP_LIMIT" )    \
                                      catch(CORBA::COMM_FAILURE &ex)               \
                                         CORBAMACRO_LOGDEBUG(ex, "COMM_FAILURE" ) \
                                      catch(CORBA::INV_OBJREF &ex)                 \
                                         CORBAMACRO_LOGDEBUG(ex, "INV_OBJREF" )   \
                                      catch(CORBA::NO_PERMISSION &ex)              \
                                         CORBAMACRO_LOGDEBUG(ex, "NO_PERMISSION" )\
                                      catch(CORBA::INTERNAL &ex)                   \
                                         CORBAMACRO_LOGDEBUG(ex, "INTERNAL" )     \
                                      catch(CORBA::MARSHAL &ex)                    \
                                         CORBAMACRO_LOGDEBUG(ex, "MARSHAL" )      \
                                      catch(CORBA::INITIALIZE &ex)                 \
                                         CORBAMACRO_LOGDEBUG(ex, "INITIALIZE" )   \
                                      catch(CORBA::NO_IMPLEMENT &ex)               \
                                         CORBAMACRO_LOGDEBUG(ex, "NO_IMPLEMENT" ) \
                                      catch(CORBA::BAD_TYPECODE &ex)               \
                                         CORBAMACRO_LOGDEBUG(ex, "BAD_TYPECODE" ) \
                                      catch(CORBA::BAD_OPERATION &ex)              \
                                        CORBAMACRO_LOGDEBUG(ex, "BAD_OPERATION" )\
                                  catch(CORBA::NO_RESOURCES &ex)               \
                                     CORBAMACRO_LOGDEBUG(ex, "NO_RESOURCES" ) \
                                  catch(CORBA::NO_RESPONSE &ex)                \
                                     CORBAMACRO_LOGDEBUG(ex, "NO_RESPONSE" )  \
                                  catch(CORBA::PERSIST_STORE &ex)              \
                                     CORBAMACRO_LOGDEBUG(ex, "PERSIST_STORE" )\
                                  catch(CORBA::BAD_INV_ORDER &ex)              \
                                     CORBAMACRO_LOGDEBUG(ex, "BAD_INV_ORDER" )\
                                  catch(CORBA::TRANSIENT &ex)                  \
                                     CORBAMACRO_LOGDEBUG(ex, "TRANSIENT" )    \
                                  catch(CORBA::FREE_MEM &ex)                   \
                                     CORBAMACRO_LOGDEBUG(ex, "FREE_MEM" )     \
                                  catch(CORBA::INV_IDENT &ex)                  \
                                     CORBAMACRO_LOGDEBUG(ex, "INV_IDENT" )    \
                                  catch(CORBA::INV_FLAG &ex)                   \
                                     CORBAMACRO_LOGDEBUG(ex, "INV_FLAG" )     \
                                  catch(CORBA::INTF_REPOS &ex)                 \
                                     CORBAMACRO_LOGDEBUG(ex, "INTF_REPOS" )   \
                                  catch(CORBA::BAD_CONTEXT &ex)                \
                                     CORBAMACRO_LOGDEBUG(ex, "BAD_CONTEXT" )  \
                                  catch(CORBA::OBJ_ADAPTER &ex)                \
                                     CORBAMACRO_LOGDEBUG(ex, "OBJ_ADAPTER" )  \
                                  catch(CORBA::DATA_CONVERSION &ex)                    \
                                     CORBAMACRO_LOGDEBUG(ex, "DATA_CONVERSION" )      \
                                  catch(CORBA::OBJECT_NOT_EXIST &ex)                   \
                                     CORBAMACRO_LOGDEBUG(ex, "OBJECT_NOT_EXIST" )     \
                                  catch(CORBA::TRANSACTION_REQUIRED &ex)               \
                                     CORBAMACRO_LOGDEBUG(ex, "TRANSACTION_REQUIRED" ) \
                                  catch(CORBA::TRANSACTION_ROLLEDBACK &ex)             \
                                     CORBAMACRO_LOGDEBUG(ex, "TRANSACTION_ROLLEDBACK")\
                                  catch(CORBA::INVALID_TRANSACTION &ex)                \
                                     CORBAMACRO_LOGDEBUG(ex, "INVALID_TRANSACTION" )  
//#endif

#define OMNI_FATAL

/** @} */

