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
#include <unistd.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <stdexcept>
#include <utility>
#include <boost/algorithm/string.hpp>
#include <milog/milog.h>
#include "kvalobs/kvPath.h"
#include "fileutil/fileutil.h"
#include "fileutil/readfile.h"
#include "miutil/aexecclient.h"
#include "miutil/base64.h"
#include "miutil/splitstr.h"
#include "fileutil/mkdir.h"


#include "execdecoder.h"


using namespace std;
using namespace miutil::conf;

namespace kvalobs {
namespace decoder {
namespace execdecoder{

namespace {
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


pair<string, string>
getKey( const std::string &keyval  )
{
    string key;
    string val;
    string::size_type i=keyval.find("=");

    if( i == string::npos )
        return make_pair( boost::trim_copy( keyval ), "" );

    key = keyval.substr( 0, i );
    val = keyval.substr( i+1 );
    boost::trim( key );
    boost::trim( val );
    return make_pair( key, val );
}

}
using namespace miutil::conf;

boost::mutex   ExecDecoder::mutex;
boost::posix_time::ptime ExecDecoder::logCleanUpTime;

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
    decodeObstype();
}

ExecDecoder::
~ExecDecoder()
{
}


void
ExecDecoder::
writeProgLog(const std::string &logfileToWrite, const std::string &someId )
{
    using namespace boost::posix_time;
    string buf;
    string path( kvPath(kvalobs::logdir) );
    string logpath("decoders/"+name());

    while(!path.empty() && path[path.length()-1]=='/')
        path.erase(path.length()-1);

    if(path.empty())
        return;

    if(!dnmi::file::mkdir(logpath, path))
        return;

    if( ! dnmi::file::ReadFile( logfileToWrite, buf ) ) {
        LOGERROR("Cant read the logfile: " << logfileToWrite << ".");
        return;
    }

    ptime now( second_clock::universal_time() );
    ofstream of;
    char tb[32];

    sprintf(tb, "%04d%02d%02d",
            int(now.date().year()), now.date().month().as_number(), now.date().day().as_number());

    string logfile=path+"/"+logpath+"/"+decoderName_ +"_decoder-" +tb + ".log";

    Lock lock( mutex );
    of.open(logfile.c_str(), ios::out|ios::app);

    if(!of.is_open())
        return;

    of << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    of << ">>> BEGIN: " << now << "   " << someId << endl;
    of << ">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>" << endl;
    of << buf << endl;
    of << "<<< END <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;

    of.close();
    unlink( logfileToWrite.c_str() );
}

void
ExecDecoder::
decodeObstype()
{
    //decodeObsType, decodes the obsType.
    //obsType is on the form: decoder/keyval1/keyval2/.../keyvalN
    //Where keyval is either a key or key=value.
    //It creates a new obsType_, where the decoder is removed. It also
    //removes the keyval encoding.
    //It sets the variables decoderName_ and encoding_.

    list<string> obsTypeList;
    vector<string> elems;
    obsTypePart_ = obsType;
    string tmp;

    string::size_type i = obsTypePart_.find("/");

    if( i != string::npos ) {
        decoderName_ = obsTypePart_.substr( 0, i );
        obsTypePart_.erase( 0, i+1 );
        boost::trim( decoderName_ );
    }

    pair<string,string> keyval;
    elems = miutil::splitstr( obsTypePart_, '/', '"');
    vector<string>::iterator it = elems.begin();

    while( it != elems.end() ) {
        keyval = getKey( *it );
        if( keyval.first == "encoding" ) {
            encoding_ = keyval.second;
            it = elems.erase( it );
        } else {
            tmp = *it;
            boost::trim( tmp );
            if( !tmp.empty() )
                obsTypeList.push_back( *it );
            ++it;
        }
    }

    ostringstream ost;
    for( list<string>::iterator it=obsTypeList.begin(); it != obsTypeList.end(); ++it ) {
        if( it != obsTypeList.begin() )
            ost << "/";
        ost << *it;
    }

    obsTypePart_ = ost.str();
    if( ! obsTypePart_.empty() )
        obsTypePart_ += "/";
    obsTypePart_ += "redirected=" + name() + "." + decoderName_;
}

bool
ExecDecoder::
hasDecoderConfSection()
{
    ConfSection *conf = myConfSection();

    if( !conf ) {
        string myName = name();
        LOGERROR("No decoder section defined for '" << myName << "'.");
        return false;
    }

    string decoderkey = createDecoderConfKey("");

    ConfSection *decoderSection=conf->getSection( decoderkey );

    if( ! decoderSection ) {
        string myName = name();
        LOGERROR("No section defined for obstype '" << decoderName_ << "' in section 'kvDataInpud." << myName << ".decoders' in the configuration file.");
        decoderName_.erase();
        return false;
    }

    return true;
}

std::string
ExecDecoder::
createDecoderConfKey( const std::string &key )const
{
    if( key.empty() )
        return "decoders."+decoderName_;
    else
        return "decoders."+decoderName_ + "." + key;
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

    string val=conf->getValue("bindir" ).valAsString("");

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
        bindir = val;
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
    if( decoderName_.empty() )
        return "";

    return decoderName_;
}

std::string
ExecDecoder::
getDecoderProg()
{
    string decoder = getDecoderName();
    if( decoder.empty() ) {
        LOGERROR("No decoder section defined for '" << decoderNamePart_ << "'.");
        return "";
    }

    string progkey = createDecoderConfKey( "decoder" );
    string argkey = createDecoderConfKey( "decoderarg" );
    string prog = getConfKey( progkey ).valAsString("");
    string args = getConfKey( argkey ).valAsString("");

    if(  prog.empty() ) {
        LOGERROR("No decoder program defined '" << progkey <<"'.");
        return "";
    }

    try {
        if( prog[0] != '/') {
            string bindir = getBindir();

            if( bindir.empty() ) {
                LOGERROR( "The '" << progkey << "' must be an absolute path or relative to bindir, but bindir is undefined or invalid.");
                return "";
            }

            prog = bindir +"/" +prog;
        }
        if( ! miutil::file::isRunable( prog ) ){
            LOGERROR( "The decoder: '" << prog << "' do NOT exist or is NOT executable.");
            return "";
        }

        if( ! args.empty() ) {
            if( ! args.empty() )
                prog += " " + args;
        }

        return prog + " ";
    }catch( const std::exception &ex ) {
        LOGERROR("The decoder '" << prog << " does NOT exist or we do not have permission to access it. (" << ex.what() <<".)");
        return "";
    }
}




miutil::conf::ValElementList
ExecDecoder::
getConfKey( const std::string &key, const miutil::conf::ValElementList &defaultVal )
{
    return getKeyInMyConfSection( key, defaultVal );
}

miutil::conf::ValElementList
ExecDecoder::
getDecoderConfKey( const std::string &key, const miutil::conf::ValElementList &defaultVal )
{
    string decoder = getDecoderName();

    if( decoder.empty() )
        return defaultVal;

    string theKey=createDecoderConfKey( key );

    return getConfKey( theKey, defaultVal );
}


bool
ExecDecoder::
removeProgLogs()
{
    return ! getDecoderConfKey( "keep_decoder_logs" ).valAsBool( false );
}

bool
ExecDecoder::
removeFileToDecode()
{
    return ! getDecoderConfKey( "keep_file_to_decode" ).valAsBool( false );
}

bool
ExecDecoder::
removeKvData()
{
    return ! getDecoderConfKey( "keep_kvdata_file" ).valAsBool( false );
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
    string val = getEncoding();
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
    ValElementList val=getKeyInMyConfSection("aexecd" );

    if( val.size() != 2 ) {
        LOGERROR("Format error: aexecd=(\"hostname\", port).");
        return false;
    }

    host = val.valAsString("", 0);
    port = val.valAsInt( -1, 1 );

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
    return getDecoderConfKey( "timeout" ).valAsInt( defaultTimeout );
}


int
ExecDecoder::
runProg( const std::string &cmd, const std::string &logfile, const std::string &someId )
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

    if( ret >= 0  ) {
        using namespace boost::posix_time;
        time_duration t = aclient.getExecutionTime();
        if( ! t.is_special() ) {
            LOGINFO("Time: " << (double(t.total_microseconds())/1.0e6) << " second. Id: " << someId << ".");
        } else {
            LOGWARN("Time: <missing>. Id: " << someId <<".");
        }
        return ret;
    }

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
    string part=getObsTypeParts();

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

    boost::trim_if( obstype, boost::is_any_of("\r\n /"));

    if( ! part.empty() )
        obstype += "/" + part;

    setRedirectInfo( obstype, buf );
    removeFile( kvdata, removeKvData() );
    return Redirect;
}

std::string
ExecDecoder::
removeFile( const std::string &filename, bool doRemove )
{
    if( ! doRemove )
        return filename;

    if( filename.empty() )
        return filename;

    unlink( filename.c_str() );
    return "";
}


namespace {
class LogLevel {
    milog::Logger &log;
    milog::LogLevel llsaved;
public:
    LogLevel( milog::LogLevel ll )
: log( milog::Logger::logger() ) {
        llsaved = log.logLevel();
        log.logLevel( ll );
    }
    ~LogLevel( ) {
        log.logLevel( llsaved );
    }
};
}

DecoderBase::DecodeResult
ExecDecoder::
execute(std::string &msg)
{
    milog::LogContext lcontext(name());
    ostringstream ost;
    ostringstream cmd;
    LOGINFO("New observation!  " << miutil::miTime::nowTime());

    if( ! hasDecoderConfSection() ) {
        LOGERROR("No decoder defined for '" << decoderName_ << "'.")
        msg = "NODECODER: No decoder defined for '" + decoderName_ +"'.";
        return Error;
    }

    LogLevel changeLogLevel( getConfLoglevel() );
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
        int exitcode = runProg( cmd.str(), logfile, tmpName );
        LOGDEBUG("exitcode: "<< exitcode << ".");

        inputFile = removeFile( inputFile, removeFileToDecode() );

        if( exitcode > 0 || ! removeProgLogs() || ll == "debug" ) {
            writeProgLog( logfile,  tmpName  );
        }

        logfile = removeFile( logfile );

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

    logfile = removeFile( logfile );
    inputFile = removeFile( inputFile, removeFileToDecode() );
    kvdataFile = removeFile( kvdataFile, removeKvData() );
    return Error;
}


}
}
}
