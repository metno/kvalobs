/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: kvalobsdataparser.cc,v 1.1.2.2 2007/09/27 09:02:27 paule Exp $                                                       

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
#include "kvalobsdataparser.h"
#include <algorithm>
#include <functional>
#include <iostream>
#include <miutil/trimstr.h>
#include <boost/lexical_cast.hpp>
#include <boost/assign.hpp>
#include <boost/shared_ptr.hpp>
#include <map>

using namespace std;
using namespace xmlpp;
using namespace boost;

namespace kvalobs
{

namespace serialize
{

/**
 * Handler function objects for tags in the XML document
 */
namespace
{
  typedef SaxParser::AttributeList AttributeList;

  struct has_key : public std::unary_function<SaxParser::Attribute, bool>
  {
    string key;
    has_key( string k ) : key( k ) {}
    bool operator()( const SaxParser::Attribute & a ) const { return a.name == key; }
  };

  bool has_attr( const AttributeList & attributes, const string & key )
  {
    AttributeList::const_iterator find = find_if( attributes.begin(), attributes.end(), has_key( key ) );
    return find != attributes.end();
  }
  string get_attr( const AttributeList & attributes, const string & key )
  {
    AttributeList::const_iterator find = find_if( attributes.begin(), attributes.end(), has_key( key ) );
    if ( find == attributes.end() )
      return "";
    return find->value;
  }

  struct HandlerFunction
  {
    virtual ~HandlerFunction() {}
    virtual void operator()( const AttributeList & attr, KvalobsData & data_, map<string, string> & context_ ) const =0;
  };
  struct KvalobsDataHandler : public HandlerFunction
  {
    virtual void operator()( const AttributeList & attr, KvalobsData & data_, map<string, string> & context_ ) const
    {
      data_.overwrite( has_attr( attr, "overwrite" ) );
    }
  };
  class ValHandler : public HandlerFunction
  {
    string tag_;
    public:
      explicit ValHandler( string tag ) : tag_( tag ) {}
      void operator()( const AttributeList & attr, KvalobsData & data_, map<string, string> & context_ ) const
      {
        context_[ tag_ ] = get_attr( attr, "val" );
      }
  };

  struct ObstimeHandler : public HandlerFunction
  {
    void operator()( const AttributeList & attr, KvalobsData & data_, map<string, string> & context_ ) const
    {
      string obstime = get_attr( attr, "val" );
      context_[ "obstime" ] = obstime;
      if ( has_attr( attr, "invalidate" ) ) {
        try {
          data_.invalidate( true, lexical_cast<int>(context_[ "station" ] ),
                            lexical_cast<int>(context_["typeid"] ),
                            obstime.empty() ? boost::posix_time::ptime() : boost::posix_time::time_from_string( obstime ) );
        }
        catch ( bad_lexical_cast & ) {
          throw parse_error( "Invalid parameter values" );
        }
      }
    }
  };

  class CorrectedRejectionHandler : public HandlerFunction
  {
  public:
	  void operator()( const AttributeList & attr, KvalobsData & data_, map<string, string> & context_ ) const
	  {
		  std::string tbtime = get_attr(attr, "tbtime");
		  context_["message/tbtime"] = tbtime;
	  }
  };

  typedef shared_ptr<HandlerFunction> hfp;
  typedef map<string, hfp> HandlerMap;
  const HandlerMap handlers_ = assign::map_list_of
      ("KvalobsData", hfp( new KvalobsDataHandler ) )
      ("station", hfp( new ValHandler( "station" ) ) )
      ("typeid", hfp( new ValHandler( "typeid" ) ) )
      ("obstime", hfp( new ObstimeHandler() ) )
      ("sensor", hfp( new ValHandler( "sensor" ) ) )
      ("level", hfp( new ValHandler( "level" ) ) )

      ("decoder", hfp( new ValHandler("decoder") ) )
      ("message", hfp( new CorrectedRejectionHandler ) )
      ;
}

void KvalobsDataParser::parse( const string & xml, KvalobsData & d )
{
  KvalobsDataParser parser( d );
  parser.parse_memory( xml );
}


KvalobsDataParser::KvalobsDataParser( KvalobsData & d )
    : xmlpp::SaxParser()
    , data_( d )
{
}

KvalobsDataParser::~KvalobsDataParser()
{}

void KvalobsDataParser::on_start_element( const Glib::ustring & name, const AttributeList & attributes )
{
  currentContext_.push( name );

  HandlerMap::const_iterator handler = handlers_.find( name );
  if ( handler != handlers_.end() ) {
    (*handler->second)( attributes, data_, context_ );
    return;
  }

  if ( name == "kvdata" or name == "kvtextdata" ) {
    AttributeList::const_iterator find = std::find_if( attributes.begin(), attributes.end(), has_key( "paramid" ) );
    if ( find == attributes.end() )
      throw parse_error( "Tag misses a required key" );
    context_[ "paramid" ] = find->value;
  }
}

void KvalobsDataParser::on_end_element( const Glib::ustring & name )
{
  currentContext_.pop();

  if ( name == "kvdata" or name == "kvtextdata" ) {
    if ( name == "kvdata" )
      insertData();
    else
      insertTextData();

    context_.erase( "paramid" );
    context_.erase( "original" );
    context_.erase( "corrected" );
    context_.erase( "controlinfo" );
    context_.erase( "useinfo" );
    context_.erase( "cfailed" );
  }
  else if ( name == "message")
  {
	  std::string message = context_["message"];
	  boost::posix_time::ptime tbtime(boost::posix_time::time_from_string(context_["message/tbtime"]));
	  std::string decoder = context_["decoder"];
	  kvalobs::kvRejectdecode reject(message, tbtime, decoder, "");
	  std::cout << reject.toSend() << std::endl;
	  data_.setMessageCorrectsThisRejection(reject);
	  context_.erase("message");
	  context_.erase("message/tbtime");
  }
  else if ( name == "level" )
    context_.erase( "level" );
  else if ( name == "sensor" )
    context_.erase( "sensor" );
  else if ( name == "decoder" )
	  context_.erase("decoder");
  else if ( name == "fixed_rejected_message" )
	  context_.erase("fixed_rejected_message");
}

void KvalobsDataParser::on_characters( const Glib::ustring & characters )
{
  string s( characters );
  miutil::trimstr( s );
  if ( not ( s.empty() or currentContext_.empty() ) )
    context_[ currentContext_.top() ] = s;
}

namespace
{
  using namespace boost;
  int get_station( std::map<std::string, std::string> & context_ )
  {
    return lexical_cast<int>( context_[ "station" ] );
  }
  int get_typeid( std::map<std::string, std::string> & context_ )
  {
    return lexical_cast<int>( context_[ "typeid" ] );
  }
  boost::posix_time::ptime get_obstime( std::map<std::string, std::string> & context_ )
  {
    string ot = context_[ "obstime" ];
    return ot.empty() ? boost::posix_time::ptime() : boost::posix_time::time_from_string( ot );
    //return miutil::miTime( context_[ "obstime" ] );
  }
  int get_sensor( std::map<std::string, std::string> & context_ )
  {
    if ( context_[ "sensor" ].empty() )
      return 0;
    return lexical_cast<int>( context_[ "sensor" ] );
  }
  int get_level( std::map<std::string, std::string> & context_ )
  {
    if ( context_[ "level" ].empty() )
      return 0;
    return lexical_cast<int>( context_[ "level" ] );
  }
  int get_paramid( std::map<std::string, std::string> & context_ )
  {
    return lexical_cast<int>( context_[ "paramid" ] );
  }
  template<typename T>
  T get_original( std::map<std::string, std::string> & context_ )
  {
    return lexical_cast<T>( context_[ "original" ] );
  }
  float get_corrected( std::map<std::string, std::string> & context_ )
  {
    if ( context_[ "corrected" ].empty() )
      return get_original<float>( context_ );
    return lexical_cast<float>( context_[ "corrected" ] );
  }
  kvControlInfo get_controlinfo( std::map<std::string, std::string> & context_ )
  {
    return kvControlInfo( context_[ "controlinfo" ] );
  }
  kvUseInfo get_useinfo( std::map<std::string, std::string> & context_ )
  {
    return kvUseInfo( context_[ "useinfo" ] );
  }
  string get_cfailed( std::map<std::string, std::string> & context_ )
  {
    return context_[ "cfailed" ];
  }
};

void KvalobsDataParser::insertData()
{
  try {
    int station_ = get_station( context_ );
    int typeid_ = get_typeid( context_ );
    boost::posix_time::ptime obstime_ = get_obstime( context_ );
    int sensor_ = get_sensor( context_ );
    int level_ = get_level( context_ );
    int paramid_ = get_paramid( context_ );
    float original_ = get_original<float>( context_ );
    float corrected_ = get_corrected( context_ );
    kvControlInfo controlinfo_ = get_controlinfo( context_ );
    kvUseInfo useinfo_ = get_useinfo( context_ );
    string cfailed_ = get_cfailed( context_ );

    data_.insert( kvData( station_, obstime_, original_, paramid_, boost::posix_time::ptime(),
                  typeid_, sensor_, level_, corrected_, controlinfo_, useinfo_, cfailed_ ) );
  }
  catch( ... ) {
    throw parse_error( "Invalid parameter values" );
  }
}

void KvalobsDataParser::insertTextData()
{
  try {
    int station_ = get_station( context_ );
    int typeid_ = get_typeid( context_ );
    boost::posix_time::ptime obstime_ = get_obstime( context_ );
    int paramid_ = get_paramid( context_ );
    string original_ = get_original<string>( context_ );

    data_.insert( kvTextData( station_, obstime_, original_, paramid_,
                  boost::posix_time::ptime(), typeid_ ) );
  }
  catch( ... ) {
    throw parse_error( "Invalid parameter values" );
  }
}


};

};

