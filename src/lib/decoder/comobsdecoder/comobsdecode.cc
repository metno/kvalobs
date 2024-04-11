/*
 Kvalobs - Free Quality Control Software for Meteorological Observations 

 $Id: comobsdecode.cc,v 1.7.2.6 2007/09/27 09:02:24 paule Exp $                                                       

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
#include <memory>
#include <stdlib.h>
#include <sstream>
#include <milog/milog.h>
#include <puTools/miTime.h>
#include <miutil/trimstr.h>
#include <miutil/timeconvert.h>
#include <kvalobs/kvDbGate.h>
#include <decoder/decoderbase/DataUpdateTransaction.h>
#include "smsmeldingparser.h"
#include "comobsdecode.h"
#include "sms2.h"
#include "sms12.h"
#include "sms13.h"

using namespace std;
using namespace kvalobs;
using namespace miutil;
using namespace kvalobs::decodeutil;

namespace pt = boost::posix_time;

namespace {

bool decodeKeyVal( const string &keyval, string &key, string &val ) {
    string::size_type i = keyval.find_first_of( "=" );

    if( i == string::npos ) {
        key = keyval;
        val.erase();
    } else {
        key = keyval.substr( 0, i );
        val = keyval.substr( i + 1 );
    }

    trimstr( key );
    trimstr( val );

    return !key.empty();
}

void getObsLines( SmsMelding *sms, const string &obs ) {
    std::list<std::string> allObses;
    istringstream in_( obs );
    std::string line;
    while( getline( in_, line ) ) {
        boost::trim( line );
        if( line.empty() )
            continue;
        sms->addMelding( line );
    }
}
}

namespace kvalobs {
namespace decoder {
namespace comobsdecoder {

ComObsDecoder::ComObsDecoder( dnmi::db::Connection &con,
        const ParamList &params, const std::list<kvalobs::kvTypes> &typeList,
        const std::string &obsType, const std::string &obs, int decoderId )
        : DecoderBase( con, params, typeList, obsType, obs, decoderId ) {
}

ComObsDecoder::~ComObsDecoder() {
}

std::string ComObsDecoder::name() const {
    return "ComObsDecoder";
}

kvalobs::decoder::comobsdecoder::SmsBase*
ComObsDecoder::smsfactory( int smscode ) {
    try {
        if( smscode == 2 )
            return new Sms2( paramList, *this );
        else if( smscode == 13 ) {
            return new Sms13( paramList, *this );
        }
#if 0
        if(smscode==12)
        return new Sms12(paramList, *this);
#endif

    }
    catch( ... ) {
        LOGERROR(
                "NOMEM: smsfactory, cant allocate an sms decoder for smscode: " << smscode << endl );
        return 0;
    }

    return 0;
}

bool ComObsDecoder::decodeObsType() {
    string keyval;
    string key;
    string val;
    string::size_type i;
    string::size_type iKey;
    CommaString cstr( obsType, '/' );
    long id;
    const char *keys[] = { "stationid", "code", "received_time", "redirected", 0 };

    LOGDEBUG( "decodeObsType: '" << obsType << "'" );
    header.clear();
    header.stationid = INT_MAX;

    if( cstr.size() < 1 ) {
        LOGERROR( "decodeObsType: To few keys!" );
//        msg="obsType: Invalid Format!";
        header.error << "ObsType: To few keys!";
        return false;
    }

    for( int index = 1; index < cstr.size(); ++index ) {
        if( !cstr.get( index, keyval ) ) {
            LOGERROR( "ObsType: INTERNALERROR: InvalidFormat!" );
            header.error << "ObsType: InvalidFormat! obstype: '" << obsType << "'.";
            return false;
        }

        if( !decodeKeyVal( keyval, key, val ) )  //keyval empty
            continue;

        for( iKey = 0; keys[iKey]; ++iKey ) {
            if( key == keys[iKey] )
                break;
        }

        if( !keys[iKey] ) {
            LOGWARN( "decodeObsType: unknown key '" << key << "'" );
            continue;
        }

        if( key == "received_time" ) {
            header.receivedTime = pt::to_miTime(
                    pt::time_from_string_nothrow( val ) );
        } else if( key == "redirected" ) {
            header.redirectedFrom = val;
        } else if( key == "stationid" ) {
            header.stationid =
                    const_cast<ComObsDecoder*>( this )->DecoderBase::getStationId(
                            key, val );

            if( header.stationid < 0 ) {
                LOGERROR(
                        "No station with stationid '" << key << "=" << val << "'" );
                header.error << "No station with stationid '" << key << "=" << val << "'.";
                return false;
            }
        } else if( key == "code" ) {
            try {
                header.code = boost::lexical_cast<int>( val );

                if( header.code != 2 && header.code != 13 ) {
                    header.error << "Unknown 'code' (" << header.code << "), valid codes is 2 and 13.";
                    return false;
                }
            }
            catch( const std::exception &ex ) {
                LOGERROR(
                        "'code' is not a number '" << key << "=" << val << "'" );
                header.error << "'code' is not a number '" <<  val << "'.";
                return false;
            }
        } else {
            LOGERROR(
                    "decodeObsType: INTERNAL: unhandled key '" << key << "', this is a error and must be fixed." );
        }
    }

    LOGDEBUG(
            "ecodeObsType: stationID: " << header.stationid << " code: " << header.code << " redirectedFrom: '" << header.redirectedFrom <<"'" << " received_time: " << (header.receivedTime.undef()?"(none)":header.receivedTime.isoTime()) );

    if( header.stationid < 0  ) {
        header.error << "Missing required parameter(s) stationid or code.";
        return false;
    }

    return true;
}

long ComObsDecoder::getStationid( long obsid, bool isWmono ) {
    ostringstream ost;
    string key;
    string val;

    if( isWmono )
        key = "wmonr";
    else
        key = "nationalnr";

    ost << obsid;
    val = ost.str();

    return DecoderBase::getStationId( key, val );
}

std::string ComObsDecoder::getMetaSaSdEm( int stationid, int typeid_,
        const miutil::miTime &obstime ) {
    string sasdem = "000";

    if( obsPgm.obstime.undef() || obsPgm.obstime != obstime ) {
        if( !loadObsPgmParamInfo( stationid, typeid_, obstime, obsPgm ) ) {
            return "000";
            LOGDEBUG( "DBERROR: SaSdEm:  000" );
        }
    }

    kvalobs::decoder::Active state;

    if( obsPgm.isActive( stationid, typeid_, 112, 0, 0, obstime, state ) ) {
        if( state == kvalobs::decoder::YES )
            sasdem[0] = '1';
    }

    if( obsPgm.isActive( stationid, typeid_, 18, 0, 0, obstime, state ) ) {
        if( state == kvalobs::decoder::YES )
            sasdem[1] = '1';
    }

    if( obsPgm.isActive( stationid, typeid_, 7, 0, 0, obstime, state ) ) {
        if( state == kvalobs::decoder::YES )
            sasdem[2] = '1';
    }

    LOGDEBUG( "SaSdEm: " << sasdem );

    return sasdem;
}

SmsMelding *ComObsDecoder::getSmsMelding( std::string &error ) {
    ostringstream err;
    SmsMelding *smsMelding;

    if( ! decodeObsType() ) {
        error = header.error.str();
        return 0;
    }

    if( header.code > 0 ) {
        smsMelding = new SmsMelding( header.stationid, -1, header.code );
        smsMelding->setRawDoc( obsType + obs );
        getObsLines( smsMelding, obs );
        return smsMelding;
    } else {
        SmsMeldingParser mParser;
        error = mParser.getErrMsg();
        smsMelding = mParser.parse( obs );
        return smsMelding;
    }
}

kvalobs::decoder::DecoderBase::DecodeResult ComObsDecoder::execute(
        std::string &msg ) {
    std::auto_ptr<SmsMelding> smsMelding;
    SmsBase *decoder;
    long obsid;
    bool isWmono;
    long stationid;
    int typeId;
    bool hasRejected = false;
    DecodedData *data;
    string logid;
    int code;
    string sError;

    list<kvRejectdecode> rejected;
    ostringstream s;
    s << name() << " (" << serialNumber << ")";
    milog::LogContext lcontext( s.str());

    LOGINFO( "ComObsDecoder:  " << miutil::miTime::nowTime() );

    smsMelding.reset( getSmsMelding( sError ) );

    if( !smsMelding.get() ) {
        kvRejectdecode myrejected = kvRejectdecode( obs,
                boost::posix_time::microsec_clock::universal_time(),
                "comobs/typeid=<UNKNOWN>", sError );

        putRejectdecodeInDb( myrejected );

        LOGWARN(
                "Decoder: " << name() <<". Rejected!" << endl << sError << endl );
        msg="Rejected: " + sError;
        return Rejected;
    }

    if( smsMelding->getClimano() > 0 ) {
        stationid = getStationid( smsMelding->getClimano(), false );
    } else if( smsMelding->getSynopno() > 0 ) {
        stationid = getStationid( smsMelding->getSynopno(), true );
    } else {
        ostringstream ost;

        ost << "comobs/typeid=" << smsMelding->getCode() + 300;

        kvRejectdecode myrejected = kvRejectdecode( obs,
                boost::posix_time::microsec_clock::universal_time(), ost.str(),
                "No identification!" );

        putRejectdecodeInDb( myrejected );

        LOGWARN(
                "Decoder: " << name() << ". Rejected! No identification!" << endl );
        msg = "Rejected: Unknown stationid.";
        return Rejected;
    }

    if( stationid <= 0 ) {
        ostringstream ost;

        ost << "comobs/typeid=" << smsMelding->getCode() + 300;

        kvRejectdecode myrejected = kvRejectdecode( obs,
                boost::posix_time::microsec_clock::universal_time(), ost.str(),
                "Uknown station!" );

        putRejectdecodeInDb( myrejected );

        LOGWARN(
                "Decoder: " << name() << "Rejected!" << endl <<"Unknown station!" << endl );
        msg= "Rejected: Unknown stationid.";
        return Rejected;
    }

    typeId = smsMelding->getCode() + 300;
    decoder = smsfactory( smsMelding->getCode() );

    IdlogHelper idlogHelper( stationid, smsMelding->getCode() + 300, this );
    logid = idlogHelper.logid();

    if( !decoder ) {
        ostringstream ost;
        ostringstream ost1;

        ost1 << "comobs/typeid=" << smsMelding->getCode() + 300;
        ost << "No decoder for SMS code <" << smsMelding->getCode() << ">!";

        kvRejectdecode myrejected = kvRejectdecode( obs,
                boost::posix_time::microsec_clock::universal_time(), ost1.str(),
                ost.str() );

        putRejectdecodeInDb( myrejected );

        LOGWARN(
                "Decoder: " << name() << "Rejected! stationid: "<< stationid << " typeid: " << typeId << endl << ost.str() << endl );
        IDLOGERROR( logid, "Rejected!" << endl << ost.str() << endl );
        msg = "Rejected: " + ost.str();
        return Rejected;
    }
    decoder->logid = logid;

    data = decoder->decode( stationid, smsMelding->getCode(),
            smsMelding->getMeldingList(), msg, rejected, hasRejected );

    if( !data ) {
        msg = "No mem!";
        return Error;
    }

    if( hasRejected ) {
        list<kvRejectdecode>::iterator it = rejected.begin();

        for( ; it != rejected.end(); it++ )
            putRejectdecodeInDb( *it );

        LOGWARN(
                "Decoder: " << name() << ". Rejected! statioid: "<< stationid << " typeid: " << typeId << endl << msg << endl );

        msg="Rejected: " + msg;
        return Rejected;
    }

    int nData = 0;
    TDecodedDataElem *theData = data->data();
    bool error = false;

    for( CITDecodedDataElem it = theData->begin(); it != theData->end();
            it++ ) {
        list<kvData> d = it->data();
        list<kvTextData> td = it->textData();
        //int priority;

        // Publish the data if the filter is set up to do that.
        dataToPublish(d, td);

        // Filter the data to see if it is to be saved to the database.
        std::tuple<std::list<kvalobs::kvData>, std::list<kvalobs::kvTextData>> fd=filterSaveDataToDb(d,td);
        d=std::get<0>(fd);
        td=std::get<1>(fd);

/*
        if( smsMelding->getCode() == 2 || smsMelding->getCode() == 13
                || smsMelding->getCode() == 3 )
            priority = 8;
        else
            priority = 4;
*/
        IDLOGDEBUG3( logid, *it );

        if( d.size() > 0 || td.size() > 0 ) {
            try {
                kvalobs::decoder::DataUpdateTransaction work(
                        to_ptime( it->getDate() ), it->stationID(),
                        it->typeID(), &d, &td, logid, DbInsert, true, false, kvalobs::decoder::DataUpdateTransaction::Partial, useQaId(it->typeID()));

                con.perform( work );
                //updateStationInfo( work.stationInfoList() );
                nData += it->dataSize() + it->textDataSize();
            }
            catch( const dnmi::db::SQLException &ex ) {
                IDLOGERROR( logid,
                        "Failed to save data: Reason: " << ex.what() );
            }
            catch( const std::exception &ex ) {
                IDLOGERROR( logid,
                        "Failed to save data: Reason: " << ex.what() << "(*)" );
            }
            catch( ... ) {
                IDLOGERROR( logid, "Failed to save data. Reason: Unknown." );
            }
        }
    }

    if( error ) {
        if( nData > 0 ) {
            IDLOGERROR( logid,
                    "Not all data was saved to the data base!" << endl << "#saved: " << nData );
            LOGWARN(
                    "Decoder: " << name() << ". Data saved with warnings. stationid: " << stationid << " typeid: " << typeId );
            msg = "Not all data could be saved to the database!";
        } else {
            IDLOGERROR( logid, "Can't save data." );
            LOGERROR(
                    "Decoder: " << name() << ". Can't save data. stationid: " << stationid << " typeid: " << typeId );

            msg = "Can't save data to the database!";
        }
        msg="NotSaved: " + msg;
        return NotSaved;
    }

    LOGINFO(
            "Decoder: " << name() <<". Saved data. stationid: " << stationid << " typeid: " << typeId );

    msg = "OK";
    return Ok;

}

}
}
}
