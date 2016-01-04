/*
 Kvalobs - Free Quality Control Software for Meteorological Observations

 $Id: comobsentry.cc,v 1.1.2.1 2007/09/27 09:02:24 paule Exp $

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

#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <map>
#include "bufrdefs.h"
#include "BufrMessage.h"
#include "BufrDecodeSynoptic.h"
#include "BufrDecodeKvResult.h"
#include "kvParamdefs.h"

using namespace std;
using namespace kvalobs::decoder::bufr;

bool readFile(const std::string &name, std::string &buf) {
  ostringstream ost;
  char ch;
  int i = 0;
  ifstream inf(name.c_str());

  if (!inf) {
    cerr << "grmf .... \n";
    return false;
  }

  while (inf.good()) {
    ch = inf.get();

    if (inf.good()) {
      i++;
      //cerr << "("<<(isprint( ch )?ch:'.' )<<")";
      ost << ch;
    }
  }
  //cerr << endl << "i: " << i << endl;
  buf = ost.str();
  return inf.eof();
}

void splitBufr(const std::string &bufr, list<string> &bufrs) {

  string::size_type prev = string::npos;
  string::size_type i = bufr.find("BUFR");

  bufrs.clear();

  while (i != string::npos) {
    if (prev == string::npos) {
      prev = i;
      i = bufr.find("BUFR", i + 1);
      continue;
    }

    bufrs.push_back(bufr.substr(prev, i - prev));

    prev = i;
    i = bufr.find("BUFR", i + 1);
  }

  if (prev != string::npos)
    bufrs.push_back(bufr.substr(prev));
}

int main(int argn, char **argv) {
  string bufrname("1492-20100406.bufr");
  //string bufrname("syno_2010092106.bufr");
  string sBufr;
  kvalobs::decoder::bufr::DescriptorFloatValue descriptorVal;
  kvalobs::decoder::bufr::BufrMessage bufrMsg;
  map<int, int> bufrTbls;

  if (!readFile(bufrname, sBufr)) {
    cerr << "Failed to read file: " << bufrname << endl;
    return 1;
  }

  list<string> bufrList;

  splitBufr(sBufr, bufrList);

  if (bufrList.empty()) {
    cerr << "No BUFR messages in <" << bufrname << ">" << endl;
    return 0;
  }

  cerr << "#BUFR messages: " << bufrList.size() << endl;
  kvalobs::decoder::bufr::BufrDecodeKvResult decodeResult;
  //kvalobs::decoder::bufr::BufrDecodeSequences decoder( unitConverter );
  kvalobs::decoder::bufr::BufrDecodeSynoptic decoder;

  for (list<string>::iterator it = bufrList.begin(); it != bufrList.end();
      ++it) {
    sBufr = *it;

    if (!bufrMsg.bufrExpand(sBufr, bufrMsg)) {
      cerr << "Failed to expand BUFR." << endl;
      return 1;
    }

    pair<map<int, int>::iterator, bool> res = bufrTbls.insert(
        pair<int, int>(bufrMsg.descriptorTbl(), 0));

    cerr << bufrMsg << endl;
    decoder.decodeBufrMessage(&bufrMsg, &decodeResult);
    if (!res.second)
      res.first->second++;

    //cerr << bufrMsg.descriptorTbl() << endl;
    //cerr << bufrMsg << endl;
  }

  cerr << endl << "BufrTables: " << endl;
  for (map<int, int>::iterator it = bufrTbls.begin(); it != bufrTbls.end();
      ++it) {
    cerr << "  " << it->first << "(" << paramid::paramName(it->first) << "): "
         << it->second << endl;
  }

#if 0
  unsigned long int *kbuff;
  long int ksup[9];
  long int ksec0[3];
  long int ksec1[40];
  long int ksec2[4096];
  long int ksec3[4];
  long int ksec4[2];
  long int key[46];
  long int kerr;

  if( ! readFile( bufrname, sBufr ) ) {
    cerr << "Failed to read file: " << bufrname << endl;
    return 1;
  }

  list<string> bufrList;

  splitBufr( sBufr, bufrList );

  if( bufrList.empty() ) {
    cerr << "No BUFR messages in <" << bufrname << ">" << endl;
    return 0;
  }

  cerr << "#BUFR messages: " << bufrList.size() << endl;

  for( list<string>::iterator it = bufrList.begin(); it != bufrList.end(); ++it ) {
    sBufr = *it;

    length = sBufr.length();
    length /= 4;
    long int kelem=KELEM;
    long int kvals=KELEM;
    char cnames[KELEM][64];
    char cunits[KELEM][24];
    double vals[KVALS];
    char cvals[KVALS][80];

    //bus0123_( &length , reinterpret_cast<unsigned long int*>( const_cast<char*>(sBufr.data())), ksup, ksec0, ksec1, ksec2, ksec3, &kerr );
    bufrex_( &length , reinterpret_cast<unsigned long int*>( const_cast<char*>(sBufr.data())), ksup, ksec0, ksec1, ksec2, ksec3, ksec4,
        &kelem, (char **)cnames, (char **)cunits, &kvals, vals, (char **)cvals, &kerr );

    if( kerr != 0 ) {
      cerr << "Failed to decode bufr: " << bufrname << endl;
      return 1;
    }

    cerr << "sBuf length: " << sBufr.length() << endl;
    cerr << "Ret length:  " << length << endl;
    cerr << "kelem:       " << kelem << endl;
    cerr << "kvals:       " << kvals << endl;
    cerr << "Dim of Array KSEC0: " << ksup[8] << endl;
    cerr << "Dim of Array KSEC1: " << ksup[0] << endl;
    cerr << "Dim of Array KSEC2: " << ksup[1] << endl;
    cerr << "Dim of Array KSEC3: " << ksup[2] << endl;
    cerr << "Dim of Array KSEC4: " << ksup[3] << endl;
    cerr << "Number of expanded elements: " << ksup[4] << endl;
    cerr << "Number of subsets: " << ksup[5] << endl;
    cerr << "Number of elements in CVALS: " << ksup[6] << endl;
    cerr << "Bufr message size: " << ksup[7] << endl;

    long int ktdlen=KELEM;
    long int kdtlist[KELEM];
    long int kdtexplen = KELEM;
    long int kdtexplist[KELEM];
    busel_( &ktdlen, kdtlist, &kdtexplen, kdtexplist, &kerr );

    if( kerr != 0 ) {
      cerr << "Failed to decode descriptors: " << bufrname << endl;
      return 1;
    }

    cerr << "Des      #: " << ktdlen << endl;
    cerr << "Des exp: #: " << kdtexplen << endl;

    cerr << "Unexpanded table. " << endl;
    for( int i=0; i<ktdlen; ++i ) {
      cerr << setw(3) << i << ":" << kdtlist[i] << endl;
    }
    cerr << endl;
    cerr << "Expnaded table." << endl;

    int prev=0;
    for( int i=0; i<kdtexplen; ++i ) {
      //F XX YYY
      int N = kdtexplist[i];
      int F=int(N/100000);
      int X=int((N-F*100000)/1000);
      int Y=int((N-X*1000));
      cerr << setw(3) << i << ": " << setfill('0') << setw(6) << N
      << " (" << setw(1) << setfill('0') << F << " "
      << setw(2) << setfill('0') << X << " "
      << setw(3) << setfill('0') << Y << ")" <<(N==prev?'*':' ')<< endl;
      prev = N;
    }

    cerr << endl;
    cerr << "Values: " << endl;
    for( int i=0; i<ksup[4]; ++i ) {
      cerr << setw(4)<<setfill('0')<< i << ": " << vals[i] << endl;  //"("<<cnames[i]<<" ["<<cunits[i]<<"])" << endl;
    }
  }
#endif
}
