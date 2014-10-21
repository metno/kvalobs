/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: dummydecoder.cc,v 1.1.2.2 2007/09/27 09:02:28 paule Exp $                                                       

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
#include <vector>
#include <fstream>
#include <stdexcept>
#include <unistd.h>
#include <milog/milog.h>
#include <puTools/miTime.h>
#include "kvalobs/kvPath.h"
#include "fileutil/fileutil.h"
#include "fileutil/readfile.h"
#include "miutil/aexecclient.h"
#include "miutil/base64.h"

#include "execdecoder.h"


using namespace std;

namespace kvalobs {
namespace decoder {
namespace execdecoder{

class aexecd_error : public exception {
public:
    typedef enum  { FormatError, Timeout, Error }ErrorType;
    ErrorType error;
    string msg;
    aexecd_error( const std::string &m, ErrorType e ) : error( e ) {
        ostringstream o;
        switch( e ) {
        case FormatError: o << "FormatError: "; break;
        case Timeout: o << "Timeout: "; break;
        case Error:o << "Error: "; break;
        }
        o << m;
        msg = o.str();
    }
    virtual ~aexecd_error() throw() {}
    const char *what() const throw() { return msg.c_str(); }
};


using namespace miutil::conf;

ExecDecoder::
ExecDecoder(
      dnmi::db::Connection   &con,
      const ParamList        &params,
      const std::list<kvalobs::kvTypes> &typeList,
      const std::string &obsType,
      const std::string &obs,
      int   decoderId)
:DecoderBase(con, params, typeList, obsType, obs, decoderId)
{
}

ExecDecoder::
~ExecDecoder()
{
}

std::string
ExecDecoder::
name() const
{
   return string("ExecDecoder");
}

std::string
ExecDecoder::
getBindir()
{
    string bindir;
    ConfSection *conf = myConfSection();

    if( !conf )
        return "";

    ValElementList val=conf->getValue("bindir" );

    if( val.empty() ) {
        bindir = kvalobs::kvPath( kvalobs::bindir );
        if( ! miutil::file::isdir( bindir ) ) {
            LOGERROR( "The bindir variable is NOT set. Tryning to use '" << bindir << "', but it must be a directory or we have not permission to use it.");
            return "";
        }
        LOGINFO("The bindir variable is NOT set. Using '" << bindir << "' as default directory to look up decoder programs.");
        return bindir;
    }

    try {
        bindir = val[0].valAsString();
        if( bindir.empty() )
            return "";
        if( bindir[0] != '/') {
            LOGERROR( "The bindir: '" << bindir << "' must be an absolute path.");
            return "";
        }

        if( ! miutil::file::isdir( bindir ) ) {
            LOGERROR( "The bindir: '" << bindir << "' must be a directory.");
            return "";
        }
        return bindir;
    }catch( const std::exception &ex ) {
        LOGERROR("The bindir '" << bindir << " does NOT exist or we do not have permission to access it. (" << ex.what()<<".)" );
        return "";
    }

}
std::string
ExecDecoder::
getDecoderName()
{
    string decoder( obsType );
    string::size_type i = decoder.find("/");

    if( i != string::npos )
        decoder.erase( i );

    ConfSection *conf = myConfSection();

    if( !conf ) {
        string myName = name();
        LOGERROR("No decoder section defined for '" << myName << "'.");
        return "";
    }

    string decoderkey = "decoders."+decoder;
    ConfSection *decoderSection=conf->getSection( decoderkey );

    if( ! decoderSection ) {
        string myName = name();
        LOGERROR("No section defined for obstype '" << decoder << "' in section 'kvDataInpud." << myName << ".decoders' in the configuration file.");
        return "";
    }

    return decoder;
}

std::string
ExecDecoder::
getDecoderProg()
{
   ConfSection *conf = myConfSection();

   if( !conf )
       return "";

   string decoder = getDecoderName();

   if( decoder.empty() )
       return "";
   string decoderkey="decoders."+ decoder +".decoder";
   string decoderargs="decoders."+ decoder +".decoderarg";
   ValElementList val=conf->getValue( decoderkey );

   if( val.empty() || val[0].valAsString().empty() ) {
       LOGERROR("No decoder program defined '" << decoderkey <<"'.");
       return "";
   }

   string prog = val[0].valAsString();

   try {
       if( prog[0] != '/') {
           string bindir = getBindir();

           if( bindir.empty() ) {
               LOGERROR( "The '" << decoderkey << "' must be an absolute path or relative to bindir, but bindir is undefined or invalid.");
               return "";
           }

           prog = bindir +"/" +prog;
       }
       if( ! miutil::file::isRunable( prog ) ){
           LOGERROR( "The decoder: '" << prog << "' do NOT exist or is NOT executable.");
           return "";
       }

       val=conf->getValue( decoderargs );

       LOGDEBUG("ARGS KEY: '" << decoderargs << "'  " << val.size() );
       if( ! val.empty() ) {
            string args =val[0].valAsString();
            LOGDEBUG("ARGS: " << decoderargs << ": '"  << args << "'.");

            if( ! args.empty() )
                prog += " " + val[0].valAsString();
       }

       return prog + " ";
   }catch( const std::exception &ex ) {
       LOGERROR("The decoder '" << prog << " does NOT exist or we do not have permission to access it. (" << ex.what() <<".)");
       return "";
   }
}


std::string
ExecDecoder::
loglevel()
{
    milog::LogLevel ll = getConfLoglevel();

    switch( ll ) {
    case milog::WARN: return "warn";
    case milog::INFO: return "info";
    case milog::DEBUG:
    case milog::DEBUG1:
    case milog::DEBUG2:
    case milog::DEBUG3:
    case milog::DEBUG4:
    case milog::DEBUG5:
    case milog::DEBUG6: return "debug";
    default:
        return "error";
    }
}

bool
ExecDecoder::
createInputFile( const std::string &filename )
{
    string val = getObsTypeKey( "encoding" );
    bool base64 = (val == "base64");
    ofstream f( filename.c_str(), ofstream::out | ofstream::trunc | ofstream::binary );

    if( ! f.is_open() ) {
        LOGERROR("Failed to create the input data file '"<<filename << "'.");
        unlink( filename.c_str() );
        return false;
    }

    if( base64 ) {
        string theData;
        miutil::decode64( obs, theData );
        f.write( theData.data(), theData.size() );
    } else {
        f.write( obs.data(), obs.size() );
    }

    f.close();

    if( f.fail() ) {
        LOGERROR("Failed to write the input data file '"<<filename << "'.");
        unlink( filename.c_str() );
        return false;
    }

    return true;
}

bool
ExecDecoder::
getAexecd( std::string &host, int &port )
{
    ConfSection *conf = myConfSection();

    if( !conf )
        return false;

    ValElementList val=conf->getValue( "aexecd" );

    if( val.size() != 2 ) {
        LOGERROR("Format error: aexecd=(\"hostname\", port).");
        return false;
    }

    host = val[0].valAsString();
    port = val[1].valAsInt( -1 );

    if( port < 0 || host.empty() ) {
        LOGERROR("Format error: aexecd=(\"hostname\", port). Values: host '" << host << "' port '" <<val[1].valAsString() <<"'.");
        return false;
    }

    return true;
}

int
ExecDecoder::
getProgTimeout( int defaultTimeout )
{
   ConfSection *conf = myConfSection();

   if( !conf )
       return defaultTimeout;

   string decoder = getDecoderName();

   if( decoder.empty() )
       return defaultTimeout;

   string timeoutkey="decoders."+ decoder +".timeout";
   ValElementList val=conf->getValue( timeoutkey );

   if( val.empty() || val[0].valAsString().empty() ) {
       LOGERROR("No no timeout defined '" << timeoutkey <<"'.");
       return defaultTimeout;
   }

   return val[0].valAsInt( defaultTimeout );
}


int
ExecDecoder::
runProg( const std::string &cmd, const std::string &logfile )
{
    string host;
    int port;
    int timeout = getProgTimeout();

    if( ! getAexecd( host, port ) )
        throw aexecd_error( "Expecting aexecd=(\"hostname\", port)", aexecd_error::FormatError );

    LOGINFO("Connecting to aexecd at '" << host << ":" << port<<"'. Timeout: " << timeout << ".");
    miutil::AExecClient aclient( host, port );

    pid_t pid = aclient.exec( cmd, logfile, timeout );

    if( pid <= 0 )
        throw aexecd_error( aclient.getErrMsg(), aexecd_error::Error );

    int ret = aclient.wait( timeout );

    if( ret >= 0  )
        return ret;

    if( aclient.timeout() )
        aexecd_error( "The command timed out, the process is killed.", aexecd_error::Timeout );
    else
        throw aexecd_error( aclient.getErrMsg(), aexecd_error::Error );

}

DecoderBase::DecodeResult
ExecDecoder::
doRedirect( const std::string &kvdata, std::string &msg  )
{
    string buf;
    string obstype;

    if( !dnmi::file::ReadFile( kvdata, buf ) ) {
        LOGERROR("Could not read file '"<< kvdata <<"'.");
        unlink( kvdata.c_str() );
        msg = "DECODE ERROR";
        return Error;
    }

    //The first line is the decoder header and the rest is the data.

    string::size_type i;

    i = buf.find_first_not_of("\r\n ");

    if( i != string::npos )
        buf.erase( 0, i ); //Remove empty space at the beginning.

    i = buf.find("\n");

    if( i == string::npos ) {
        LOGERROR( "Format error in file '" << kvdata << "'.\nExpecting at least 2 lines. Where the first line is the decoder header.");
        msg = "DECODE ERROR";
        return Error;
    }

    obstype = buf.substr( 0, i );
    buf.erase( 0, i+1 );
    setRedirectInfo( obstype, buf );
    unlink( kvdata.c_str() );
    return Redirect;
}


DecoderBase::DecodeResult
ExecDecoder::
execute(std::string &msg)
{
   milog::LogContext lcontext(name());
   ostringstream ost;
   ostringstream cmd;
   LOGINFO("New observation!  " << miutil::miTime::nowTime());

//   std::cerr << name() << ": " << obsType <<  endl;
//   std::cerr << "[" << obs << "]" << endl << endl;

   string decoder = getDecoderName();
   string prog = getDecoderProg();
   string logdir = logdirForLogger( decoder );
   string tmpdir = datdirForLogger("tmp");
   string tmpName = semiuniqueName(decoder, "");
   string inputFile=tmpdir+tmpName+".bufr";
   string kvdataFile=tmpdir+tmpName+".kvdata";
   string logfile=logdir+tmpName+".log";
   string ll = loglevel();

   if( prog.empty() ) {
       LOGERROR("No decoder defined for '"<< obsType << "'.");
       msg = "NO DECODER";
       return Error;
   }


   if( ! createInputFile( inputFile ) ) {
       msg = "Internal error in the decoder.";
       return Error;
   }

   cmd << prog << "--" << decoder << " " << inputFile << " --kvdata " << kvdataFile << " --loglevel " << ll;
   LOGDEBUG("Running decoder prog '" << cmd.str() << "'\nlogfile '" << logfile << "'\n"<< decoder << " '" <<  inputFile << "'\nkvdata '" << kvdataFile <<"'.");

   try {
       int exitcode = runProg( cmd.str(), logfile );
       LOGDEBUG("exitcode: "<< exitcode << ".");

       unlink( inputFile.c_str() );

       if( exitcode == 0 && ll != "debug") {
           unlink( logfile.c_str() );
       }

       if( exitcode == 0 || exitcode == 1 ) {
           return doRedirect( kvdataFile, msg );
       } else {
           LOGERROR("The '" << decoder << "' failed exitcode '" << exitcode << "'. See logfile '" << logfile << "'.");
           msg = "DECODE ERROR.";
           return Error;
       }
   }
   catch( const aexecd_error &ex ) {
       LOGERROR( "aexecd error: " << ex.what() );
   }
   catch( const std::exception &ex) {
       LOGERROR( "aexecd: " << ex.what() )
   }
   unlink( inputFile.c_str() );
   unlink( kvdataFile.c_str() );
   return Error;
}


}
}
}
