/**********************************************************************
 *
 * Swap sequential fortran file containing integer*2 numbers
 *
 * DNMI/FoU  18.03.2004  Anstein Foss
 **********************************************************************/


#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

//inline static void twoByteSwap(int ndata, short int *data) {
//  short int iswap;
//  for (int i=0; i<ndata; ++i) {
//    iswap= data[i];
//    data[i]= (iswap << 8) | ((iswap >> 8) & 0xFF);
//  }
//}

inline static void fourByteSwap(int ndata, int *data) {
  int iswap;
  for (int i=0; i<ndata; ++i) {
    iswap= data[i];
    data[i]= ((iswap << 24) | ((iswap << 8) & 0xFF0000)) |
             (((iswap >> 8) & 0xFF00) | ((iswap >> 24) & 0xFF));
  }
}

int main(int argc, char **argv){

  string ifileName, ofileName;
  string option="-s";
  bool swap= true;
  bool error= false;

  if (argc==3) {
    ifileName= argv[1];
    ofileName= argv[2];
  } else if (argc==4) {
    option=    argv[1];
    if (option!="-s" && option!="-u")
      error= true;
    if (option=="-u")
      swap= false;
    ifileName= argv[2];
    ofileName= argv[3];
  } else {
    error= true;
  }
  if (error) {
    cerr<<"  usage: "<<argv[0]<<" infile outfile"<<endl;
    cerr<<"     options:  -s : swap (default)"<<endl;
    cerr<<"               -u : unswap"<<endl;
    return 1;
  }

  FILE* ifile;
  FILE* ofile;

  if ((ifile= fopen(ifileName.c_str(),"rb"))==0) {
    cerr<<"  ERROR open input file "<<ifileName<<endl;
    return 2;
  }

  if ((ofile= fopen(ofileName.c_str(),"wb"))==0) {
    cerr<<"  ERROR open output file "<<ofileName<<endl;
    return 3;
  }

  int nbytes,nbytes1,nbytes2,length;
  int buflength=0;
  int *buf= 0;

  while (true) {

    error= false;
    fread(&nbytes1,4,1,ifile);
    if (feof(ifile))
      break;
    error= true;
    if (ferror(ifile))
      break;
    nbytes= nbytes1;
    if (!swap) fourByteSwap(1,&nbytes);
    if (nbytes1%4 != 0 || nbytes<4)
      break;
    length= nbytes/4;
    if (length>buflength) {
      if (buf) delete[] buf;
      buflength= length;
      buf= new int[buflength];
    }
    fread(buf,4,length,ifile);
    if (feof(ifile) || ferror(ifile))
      break;
    fread(&nbytes2,4,1,ifile);
    if (feof(ifile) || ferror(ifile))
      break;
    if (nbytes2!=nbytes1)
      break;

    if (swap) fourByteSwap(1,&nbytes);

    fourByteSwap(length,buf);

    fwrite(&nbytes,4,1,ofile);
    if (ferror(ofile))
      break;
    fwrite(buf,4,length,ofile);
    if (ferror(ofile))
      break;
    fwrite(&nbytes,4,1,ofile);
    if (ferror(ofile))
      break;
  }

  fclose(ifile);
  fclose(ofile);

  if (error) {
    cerr<<"ERROR exit.  Something wrong..."<<endl;
    return 4;
  }

  return 0;
}

