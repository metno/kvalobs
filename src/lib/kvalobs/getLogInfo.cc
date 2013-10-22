/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: kvapp.h,v 1.1.2.2 2007/09/27 09:02:30 paule Exp $

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
#include <algorithm>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "getLogInfo.h"

using namespace std;

namespace {
milog::LogLevel
getLevelFromVal( const miutil::conf::ValElementList &vals_, const string &what, const string &section_, const milog::LogLevel defLoglevel )
{
    string section( section_);
    miutil::conf::ValElementList &vals = const_cast<miutil::conf::ValElementList&>( vals_);

    string sVal = vals[0].valAsString();

    milog::LogLevel ll=loglevelFromString(  sVal );

    if( ll != milog::NOTSET )
        return ll;

    if( section.empty() )
        section = "__globale__";

     LOGERROR("Invalid " << what << " value: '" << sVal << "' in section '"<<section<< "'. Valid values fatal, error, warn, info or debug.");

     return defLoglevel;

}

int
getNumber( const string &numberStr, bool allowPrefix )
{
    string number( numberStr );
    string::size_type i;
    int n;
    int factor=1;

    boost::trim( number );
    i = number.find_first_not_of("0123456789");

    if( i != string::npos ) {
        string sf=number.substr( i );
        number.erase(i);
        boost::trim( sf );

        if( sf.size() > 0 ) {
            switch( sf[0] ) {
            case 'k' : factor=1000; break;
            case 'M' : factor=1000000; break;
            case 'G' : factor=1000000000; break;
            default:
                factor=1;
            }
        }
    }

    n = boost::lexical_cast<int>( number );
    return n*factor;
}


milog::LogLevel
getXLevel( const miutil::conf::ConfSection *conf,
             const std::string &section,
             const std::string &what,
             const milog::LogLevel defLoglevel  )
{
    if( ! conf )
        return defLoglevel;

    string key;

    if( section.empty() )
        key = what;
    else
        key = section + "." + what;

    miutil::conf::ValElementList vals = conf->getValue( key );

    if( vals.size() == 0 )
        return defLoglevel;

    return getLevelFromVal( vals, what, section, defLoglevel );
}


milog::LogLevel
getXLevelRecursivt( const miutil::conf::ConfSection *conf,
                    const std::string &startAtSection,
                    const std::string &what,
                    const milog::LogLevel defLoglevel )
{
    miutil::conf::ValElementList val;
    string section;

    val = conf->getValueRecursivt( startAtSection, what, section );

    if( val.empty() )
        return defLoglevel;

    return getLevelFromVal( val, what, section, defLoglevel );
}

}


milog::LogLevel
loglevelFromString( const std::string &level)
{
    if(strcasecmp("FATAL", level.c_str() )==0){
        return milog::FATAL;
    }else if(strcasecmp("ERROR", level.c_str())==0){
        return milog::ERROR;
    }else if(strcasecmp("WARN", level.c_str())==0){
        return milog::WARN;
    }else if(strcasecmp("DEBUG", level.c_str())==0){
        return milog::DEBUG;
    }else if(strcasecmp("INFO", level.c_str())==0){
        return milog::INFO;
    }else if(strcmp("0", level.c_str())==0){
        return milog::FATAL;
    }else if(strcmp("1", level.c_str())==0){
        return milog::ERROR;
    }else if(strcmp("2", level.c_str())==0){
        return milog::WARN;
    }else if(strcmp("3", level.c_str())==0){
        return milog::INFO;
    }else if(strcmp("4", level.c_str() )==0){
        return milog::DEBUG;
    }else{
        return milog::NOTSET;
    }
}


milog::LogLevel
getTracelevel( const miutil::conf::ConfSection *conf, const std::string &section,
               const milog::LogLevel defLoglevel )
{
    return getXLevel( conf, section, "tracelevel", defLoglevel );
}


milog::LogLevel
getTracelevelRecursivt( const miutil::conf::ConfSection *conf, const std::string &startAtSection,
                        const milog::LogLevel defLoglevel )
{
    return getXLevelRecursivt( conf, startAtSection, "tracelevel", defLoglevel );
}

milog::LogLevel
getLoglevel( const miutil::conf::ConfSection *conf, const std::string &section,
             const milog::LogLevel defLoglevel )
{
    return getXLevel( conf, section, "loglevel", defLoglevel );
}


milog::LogLevel
getLoglevelRecursivt( const miutil::conf::ConfSection *conf, const std::string &startAtSection,
                        const milog::LogLevel defLoglevel )
{
    return getXLevelRecursivt( conf, startAtSection, "loglevel", defLoglevel );
}


void
getLogfileInfo( const miutil::conf::ConfSection *conf,
                const std::string &startAtSection,
                int &nRotation, int &fileSize )
{
    using namespace miutil::conf;
    string section;
    string key("logfile_info");
    ValElementList defValues;

    nRotation = -1;
    fileSize = -1;

    defValues.push_back(ValElement("2"));
    defValues.push_back(ValElement("102400"));

    try {
        ValElementList vals;
        vals=conf->getValueRecursivt( startAtSection, "logfile_info", section, defValues);

        if( vals.size()>0 )
            nRotation = vals[0].valAsInt();

        if( vals.size()>1 )
            fileSize = getNumber( vals[1].valAsString(), true );
    }
    catch( const std::exception &ex ) {
        if( section.empty() )
            section = "__globale__";
        LOGERROR( "Invalid value for '" << key << "' in section '"<<section<<"'.");
    }

    if( nRotation < 1 )
        nRotation = 2;

    if( fileSize < 102400)
        fileSize = 102400;
}
