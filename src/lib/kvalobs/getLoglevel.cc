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

#include "getLoglevel.h"

using namespace std;

milog::LogLevel
getLoglevel( const miutil::conf::ConfSection *conf, const std::string &section )
{
    if( ! conf )
        return milog::NOTSET;

    string key;

    if( section.empty() )
        key = "loglevel";
    else
        key = section + ".loglevel";

    miutil::conf::ValElementList vals = conf->getValue( key );

    if( vals.size() == 0 )
        return milog::NOTSET;

    string val = vals[0].valAsString();

    if(strcasecmp("FATAL", val.c_str() )==0){
        return milog::FATAL;
    }else if(strcasecmp("ERROR", val.c_str())==0){
        return milog::ERROR;
    }else if(strcasecmp("WARN", val.c_str())==0){
        return milog::WARN;
    }else if(strcasecmp("DEBUG", val.c_str())==0){
        return milog::DEBUG;
    }else if(strcasecmp("INFO", val.c_str())==0){
        return milog::INFO;
    }else if(strcmp("0", val.c_str())==0){
        return milog::FATAL;
    }else if(strcmp("1", val.c_str())==0){
        return milog::ERROR;
    }else if(strcmp("2", val.c_str())==0){
        return milog::WARN;
    }else if(strcmp("3", val.c_str())==0){
        return milog::INFO;
    }else if(strcmp("4", val.c_str() )==0){
        return milog::DEBUG;
    }else{
        if( section.empty() )
            key = "globale";
        else
            key = section;
        LOGERROR("Invalid loglevel value: '" << val << "' in section '"<<key<< "'. Valid values fatal, error, warn, info or debug.");
        return milog::NOTSET;
    }
}



