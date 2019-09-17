#include <iostream>
#include <list>
#include <vector>
#include <string>
#include "utils.h"

using namespace std;
namespace utils {

namespace {

//At start the index points to the start of the separator.
bool findKeyStart(const string &conf, string::size_type *index) {
  if (*index==0)
    return false;

  //Skip whitespace before sep
  for( (*index)--;*index>0 && (conf[*index]==' ' || conf[*index]=='\t'); --(*index));
  
  if ( *index==0 && (conf[*index]==' ' || conf[*index]=='\t') ) {
    return false;
  }

  for( ;*index>0 && (conf[*index]!=' ' && conf[*index]!='\t'); --(*index));
  
  if(conf[*index]==' ' || conf[*index]=='\t'){
    (*index)++;
  }
  
  return true;
}


bool findKeyVal(const string &conf, string::size_type *iStart_, string &keyval, const string &sep="=") 
{
  string::size_type i;
  string::size_type iStart=*iStart_;

  string::size_type iSep=conf.find(sep, static_cast<string::size_type>(iStart));
  keyval.erase();
  if( iSep == string::npos) {
    *iStart_= string::npos;
    return true;
  }

  iStart = iSep;
  if( !findKeyStart(conf, &iStart) ) {
    return false;
  }

  string::size_type iEnd=conf.find(sep, iSep+sep.length());

  if( iEnd == string::npos) {
    keyval=rtrim(conf.substr(iStart));
    *iStart_=conf.length();
    return true;
  } 
  
  
  if( ! findKeyStart(conf, &iEnd) ) {
    return false;
  } 

  if( iEnd < iStart) {
    return false;
  }

  keyval=trim(conf.substr(iStart, iEnd-iStart));
  *iStart_=iEnd+1;
  return true;
}
}  // namespace

string
rtrim(const string& v, const std::string& trimset)
{
  string::size_type i = v.find_last_not_of(trimset);
  if (i != string::npos) {
    i = i + 1;
  }
  return v.substr(0, i);
}

string
ltrim(const string& v, const std::string& trimset)
{
  string::size_type i = v.find_first_not_of(trimset);
  if (i == string::npos) {
    i = 0;
  }
  return v.substr(i);
}

string
trim(const string& v, const std::string& trimset)
{
  return rtrim(ltrim(v, trimset), trimset);
}

vector<string>
splitstr(const string& val, char ch, bool compress)
{
  vector<string> ll;
  string::size_type iStart = 0;
  string::size_type i = 0;
  string::size_type len = val.length();
  
  if( len==0) {
    return ll;
  }

  while (iStart != string::npos && iStart<=len) {
    i = val.find_first_of(ch, iStart);
    //Handle first iteration
    if (iStart==0) {
      if (i == string::npos) {
        ll.push_back(val.substr(0));
        return ll;
      } 
        
      ll.push_back(val.substr(0, i));
      if( compress ) {
        iStart=val.find_first_not_of(ch, i);
        if ( iStart == string::npos) {
          ll.push_back("");
          return ll;
        }
      } else{
        iStart=i+1;
      }
      continue;
    }

    if (i == string::npos)  {
      ll.push_back(val.substr(iStart));
      iStart = i;
    } else  {
      ll.push_back(val.substr(iStart, i - iStart));
      if (compress) {
        iStart = val.find_first_not_of(ch, i);
        if (iStart==string::npos){
          ll.push_back("");
          return ll;
        }
      } else {
        iStart=i+1;
      }
    } 
  }
  return ll;
}



bool
findKeyVals(const std::string &confstr, list<string> *keyvals, const string &sep){
  string::size_type i;
  string::size_type iStart=0;
  string keyval;

  if( keyvals)
    keyvals->clear();

  while (iStart>=0 && iStart != string::npos) {
    if( !findKeyVal(confstr, &iStart, keyval) ) {
      return false;
    }
    if( keyvals)
      keyvals->push_back(keyval);
  }

  return true;
}


}
