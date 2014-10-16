#include <cstring>
#include "ArgvClass.h"

using namespace std;

namespace miutil {
//
//ArgvClass::
//ArgvClass(const conf::ConfSection &collection )
//{
//    unsigned int i;
//    std::string  strVal;
//    std::string  key;
//    int          len;
//    int          nTmp;
//    int          nSize;
//
//    nSize = collection.getKeys().size();
//
//    if(nSize <= 0)
//        return;
//
//
//    try {
//        argv=new char* [nSize+1];
//
//        for(i=0; i<=nSize; i++)
//            argv[i]=0;
//    }
//    catch(...) {
//        n=0;
//        argv=0;
//    }
//
//    try
//    {
//        list<string> keys = collection.getKeys();
//        int i=0;
//        for( list<string>::const_iterator it=keys.begin(); i<keys.end(); ++it, ++i )
//        {
//            conf::ValElementList val = collection.getValue( *it );
//
//            if( val.empty() )
//                continue;
//
//            key = *it;
//            strVal=val.getStrVal();
//
//            len=key.length()+strVal.length()+2; //setter av plass til = og 0 terminering
//
//            argv[i]=new char[len];
//            n++;
//            argv[i][0]='\0';
//            strcat(&(argv[i][0]), key.c_str());
//            strcat(&(argv[i][0]), "=");
//            strcat(&(argv[i][0]), strVal.c_str());
//        }
//    }
//    catch(...)
//    {
//        for(i=0; i<n; i++)
//            delete[] argv[i];
//
//        delete[] argv;
//
//        n=0;
//        argv=0;
//        return;
//    }
//}

ArgvClass::~ArgvClass()
{
    if(n>0)
    {
        for(unsigned int i=0; i<n; i++)
            delete[] argv[i];

        delete[] argv;
    }
}

ArgvClass::ArgvClass(const std::string &str)
{
    list<string> strList;
    list<string>::iterator it;
    unsigned int  i;

    argv=0;
    n=0;

    split(strList, str);

    if(strList.size()==0)
        return;

    try
    {
        argv=new char*[strList.size()+1];

        for(i=0; i<=strList.size(); i++)
            argv[i]=0;
    }
    catch(...)
    {
        n=0;
        argv=0;
    }

    try
    {
        n=0;
        it=strList.begin();

        for(;it!=strList.end(); it++)
        {
            argv[n]=new char[(*it).length()+1];
            strcpy(&(argv[n][0]),(*it).c_str());
            n++;
        }
    }
    catch(...)
    {
        for(i=0; i<n; i++)
            delete[] argv[i];

        delete[] argv;

        n=0;
        argv=0;
        return;
    }
}

char* const *
ArgvClass::getArgv()
{
    return argv;
}

unsigned int    
ArgvClass::getArgn()
{
    return n;
}

void 
ArgvClass::split( list<string> &strList, const std::string &buf)
{
    std::string::const_iterator it=buf.begin();
    std::string   tmp;

    strList.erase(strList.begin(), strList.end());

    while(it!=buf.end())
    {
        while(it!=buf.end() && *it==' ')
            it++;

        tmp="";

        while(it!=buf.end() && *it!=' ')
        {
            tmp+=*it;
            it++;
        }

        strList.push_back(tmp);
    }
}


}
