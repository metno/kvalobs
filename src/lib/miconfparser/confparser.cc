/*
  Kvalobs - Free Quality Control Software for Meteorological Observations 

  $Id: confparser.cc,v 1.1.6.4 2007/09/27 09:02:25 paule Exp $                                                       

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
#include <iostream>
#include <fstream>
#include <boost/thread/mutex.hpp>
#include <miconfparser/confparser.h>

using namespace std;

namespace miutil {
   namespace conf {
      typedef std::map<const ConfParser*, void*> PimpelType;
      PimpelType ConfParser::pimpel;
   }
}

namespace {

boost::mutex mutex;

struct MyPimpel {
   bool allowMultipleSections;

   MyPimpel( bool allowMultipleSections_ )
      : allowMultipleSections( allowMultipleSections_ ) {}
};
}

miutil::conf::
ConfParser::
ConfParser()
:curIst(0), debugLevel_(0)
{
   boost::mutex::scoped_lock lock( mutex );
   pimpel[this] = new MyPimpel( false );
}

miutil::conf::
ConfParser::
ConfParser( bool allowMultipleSections_ )
:curIst(0), debugLevel_(0)
{
   boost::mutex::scoped_lock lock( mutex );
   pimpel[this] = new MyPimpel( allowMultipleSections_ );
}

miutil::conf::
ConfParser::
ConfParser(std::istream &ist )
:debugLevel_(0)
{
   boost::mutex::scoped_lock lock( mutex );
   pimpel[this] = new MyPimpel( false );

   try{
      curIst=new TIstStack;
   }
   catch(...){
      curIst=0;
      return;
   }

   curIst->state=0;
   curIst->ist=&ist;
   curIst->lineno=0;
   curIst->file="<STREAM>";
}

miutil::conf::
ConfParser::
ConfParser( std::istream &ist,  bool allowMultipleSections_)
:debugLevel_(0)
{
   boost::mutex::scoped_lock lock( mutex );
   pimpel[this] = new MyPimpel( allowMultipleSections_ );

   try{
      curIst=new TIstStack;
   }
   catch(...){
      curIst=0;
      return;
   }

   curIst->state=0;
   curIst->ist=&ist;
   curIst->lineno=0;
   curIst->file="<STREAM>";
}



miutil::conf::ConfParser::
~ConfParser()
{
   boost::mutex::scoped_lock lock( mutex );
   PimpelType::iterator it = pimpel.find( this );

   if( it != pimpel.end() ) {
      delete static_cast<MyPimpel*>(it->second);
      pimpel.erase( it );
   }

   if(curIst)
      delete curIst;
}

bool
miutil::conf::
ConfParser::
allowMultipleSections()const
{
   boost::mutex::scoped_lock lock( mutex );
   PimpelType::const_iterator it = pimpel.find( this );

   if( it != pimpel.end() )
      return static_cast<MyPimpel*>( it->second )->allowMultipleSections;

   return false;
}


void 
miutil::conf::ConfParser::
deleteTokenStack()
{
   while(!tokenStack_.empty()){
      tokenStack_.pop_front();
   }
}

std::ostream&
miutil::conf::ConfParser::
printTokenStack(std::ostream &ost)const
{
   CITokenStack it=tokenStack_.begin();

   ost << "***TokenStack***\n";

   if(it==tokenStack_.end()){
      ost << "--- <<EMPTY>> ---\n";
      return ost;
   }

   for(;it!=tokenStack_.end(); it++){
      ost << " --- " << it->print() << endl;
   }

   return ost;
}

miutil::conf::ConfSection*
miutil::conf::ConfParser::
parse(std::istream &ist)
{
   if(!curIst){
      try{
         curIst=new TIstStack;
      }
      catch(...){
         curIst=0;
         return 0;
      }
   }

   curIst->state=0;
   curIst->ist=&ist;
   curIst->lineno=0;
   curIst->file="<STREAM>";

   return parse();
}

miutil::conf::ConfSection*
miutil::conf::ConfParser::
parse( const std::string &filename )
{
   return parse( filename, false );
} 

miutil::conf::ConfSection*
miutil::conf::ConfParser::
parse( const std::string &filename,
       bool allowMultipleSections )
{
   miutil::conf::ConfParser  parser( allowMultipleSections );
   miutil::conf::ConfSection *conf;
   ifstream    fis;

   fis.open(filename.c_str());

   if(!fis){
      throw std::logic_error("Reading configuration from file <"+filename + ">!");
   }else{
      conf=parser.parse(fis);

      if(!conf){
         throw std::logic_error("Error while reading configuration file: <"
               + filename +
               "> Reason: " + parser.getError() );
      }

      return conf;
   }
}

miutil::conf::ConfSection* 
miutil::conf::ConfParser::
parse()
{
   error_=false;

   listContext_=false;
   errs_.str("");

   if(!curIst){
      errs_ << "No STREAM!";
      return 0;
   }

   curIst->lineno=1;
   deleteStack();
   deleteTokenStack();

   try{
//      cerr << "parse: allowMultipleSections: "
//                       << (allowMultipleSections?"true":"false")<< endl;
      stack_.push_back(new ConfSection( allowMultipleSections(), filename(), lineno() ) );
   }
   catch(...){
      errs_ << "NO MEM";
      delete curIst;
      curIst=0;
      return 0;
   }

   priv::ConfParserLexer(this);

   if(error_){
      deleteStack();
      delete curIst;
      curIst=0;
      return 0;
   }


   if(!tokenStack_.empty()){
      errs_ << "Unexpected tokens at the token stack.\n";

      printTokenStack(errs_);
      deleteTokenStack();
      deleteStack();
      delete curIst;
      curIst=0;
      return 0;
   }


   //The size of the stack shall be 1 and
   //the token stack shall be empty.

   if(stack_.size() > 1){
      errs_ << "Unbalanced section somewhere?";
      deleteStack();
      delete curIst;
      curIst=0;
      return 0;
   }

   ConfSection *result=stack_.front();
   stack_.pop_front();

   delete curIst;
   curIst=0;

   return result;
}

void  
miutil::conf::ConfParser::
deleteStack()
{
   while(!stack_.empty()){
      delete stack_.front();
      stack_.pop_front();
   }
}


void  
miutil::conf::ConfParser::
intToken(const char *i)
{
   if(debugLevel_>0)
      cout << "intToken: " << i << endl;

   checkToken(Token(MiTT_INT, i, curIst->lineno));
}

void  
miutil::conf::ConfParser::
floatToken(const char *d)
{
   if(debugLevel_>0)
      cout << "floatToken: " << d << endl;

   checkToken(Token(MiTT_FLOAT, d, curIst->lineno));
}

void  
miutil::conf::ConfParser::
idToken(const char *id)
{
   if(debugLevel_>0)
      cout << "idToken: " << id << endl;

   checkToken(Token(MiTT_ID, id, curIst->lineno));
}

void  
miutil::conf::ConfParser::
aliasToken(const char *id)
{
   if(debugLevel_>0)
      cout << "aliasToken: " << id << endl;
}

void  
miutil::conf::ConfParser::
charToken(char ch)
{
   if(debugLevel_>0){
      char buf[2];
      buf[0]=ch;
      buf[1]='\0';
      cout << "charToken: " << (ch=='\n'?"\\n":buf) << endl;
   }

   switch(ch){
      case '=': checkToken(Token(MiTT_EQUAL, "=", curIst->lineno));
      break;
      case ')': checkToken(Token(MiTT_CP, ")", curIst->lineno));
      break;
      case '(': checkToken(Token(MiTT_OP, "(", curIst->lineno));
      break;
      case '}': checkToken(Token(MiTT_CB, "}", curIst->lineno));
      break;
      case '{': checkToken(Token(MiTT_OB, "{", curIst->lineno));
      break;
      case ',': checkToken(Token(MiTT_COMMA, ",", curIst->lineno));
      break;
      case '\n': checkToken(Token(MiTT_NL, "\\n", curIst->lineno));
      break;
      default:
         errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
         << ": Invalid token, (" << ch << ")";
         error_=true;
   }
}

void  
miutil::conf::ConfParser::
stringToken(const char *s)
{
   if(debugLevel_>0)
      cout << "stringToken: " << s << endl;

   checkToken(Token(MiTT_STRING, s, curIst->lineno));
}


bool
miutil::conf::ConfParser::
checkToken(const Token &t)
{
   if(error_){
      return false;
   }

   if(tokenStack_.empty()){
      if(debugLevel_>1)
         cout << "PUSH token: " << t.val << (t.tt==MiTT_NL?" ignored":"") << endl;

      if(t.tt==MiTT_NL)
         return true;

      tokenStack_.push_back(t);
      return true;
   }

   Token &prevToken=tokenStack_.front();

   if(listContext_){
      switch(t.tt){
         case MiTT_NL: //Just ignore newline.
            if(debugLevel_>1)
               cout << "Token: \\n ignored!" << endl;

            return true;
         case MiTT_STRING:
         case MiTT_ID:
         case MiTT_INT:
         case MiTT_FLOAT:
         case MiTT_COMMA:
            tokenStack_.push_front(t);
            return true;
         case MiTT_CP:
            listContext_=false;
            return colapseList();
         default:
            error_=true;
            errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Invalid token in list context!";
            return false;
      }
   }

   if(t.tt==MiTT_OP){  //Start of a list context
      if(debugLevel_>1)
         cout << "Start of listContext!" << endl;

      if(prevToken.tt==MiTT_EQUAL){
         listContext_=true;
         tokenStack_.push_front(Token(MiTT_OP,"", curIst->lineno));
         return true;
      }else{
         errs_ << "File: " << curIst->file << " Line: " << curIst->lineno<< ": Cant start a list context here!";
         error_=true;
         return false;
      }
   }

   if(prevToken.tt==MiTT_EQUAL){
      switch(t.tt){
         case MiTT_ID:
         case MiTT_STRING:
         case MiTT_INT:
         case MiTT_FLOAT:
         case MiTT_NL:
            tokenStack_.pop_front();
            return colapseKeyVal(t);
         default:
            errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": Invalid variable specification, expecting STRING, ID, INT"
            << " or FLOAT";
            error_=true;
            return false;
      }
   }

   switch(t.tt){
      case MiTT_NL:  //Just ignore newline here.
         if(debugLevel_>1)
            cout << "Token: \\n ignored!" << endl;

         return true;
      case MiTT_EQUAL:
         tokenStack_.push_front(Token(MiTT_EQUAL, "", curIst->lineno));
         return true;
      case MiTT_OB:
         if(!tokenStack_.empty() &&
               tokenStack_.front().tt!=MiTT_ID){
            errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
                  << ": Cant start a new section here!";
            error_=true;
            return false;
         }

         try{
//            cerr << "checkToken: allowMultipleSections: "
//                 << (allowMultipleSections?"true":"false")<< endl;
            stack_.push_front(new ConfSection( allowMultipleSections(), filename(), lineno() ) );
         }
         catch(...){
            errs_ << "NO MEM";
            error_=true;
            return false;
         }

         tokenStack_.push_front(t);
         return true;
      case MiTT_CB:
         return colapseSection();
      default:
         tokenStack_.push_front(t);
         return true;
   }

   return true;
}

bool
miutil::conf::ConfParser::
colapseList()
{
   ValElementList vl;
   TokenType prevToken=MiTT_CP;

   if(debugLevel_>1){
      if(debugLevel_>2){
         cout << "Tokenstack before colapseList ....\n";
         printTokenStack(cout);
      }else
         cout << "colapseList......\n";
   }


   if(stack_.empty()){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": Internal, unexpected stack is empty!";
      error_=true;
      return false;
   }

   while(!tokenStack_.empty()){
      Token t=tokenStack_.front();
      tokenStack_.pop_front();

      switch(t.tt){
         case MiTT_STRING:
            vl.push_front(ValElement(t.val, miutil::conf::STRING));
            break;
         case MiTT_ID:
            if(prevToken==MiTT_EQUAL){
               if(!stack_.front()->addValue(t.val, vl, false)){
                  errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
                        << ": overwritning variable <" << t.val << ">!";
                  error_=true;
                  return false;
               }

               if(debugLevel_>1){
                  if(debugLevel_>2){
                     cout << "Tokenstack after colapseList ....\n";
                     printTokenStack(cout);
                  }else
                     cout << "colapseList (colapsed, leaving listContext)\n";
               }


               //After we have colapsed the list the token stack should
               //be empty or the top token must be OB ie. {

               if(tokenStack_.empty() || tokenStack_.front().tt==MiTT_OB)
                  return true;
               else{
                  errs_ << "File: " << curIst->file <<" Line: " << curIst->lineno
                        << ": Unexpected token <" << tokenStack_.front().val << ">!";
                  error_=true;
                  return false;
               }
            }else{
               vl.push_front(ValElement(t.val, miutil::conf::STRING));
            }
            break;
         case MiTT_INT:
            vl.push_front(ValElement(t.val, miutil::conf::INT));
            break;
         case MiTT_FLOAT:
            vl.push_front(ValElement(t.val, miutil::conf::FLOAT));
            break;
         case MiTT_EQUAL:
            if(prevToken!=MiTT_OP){
               errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
                     << ": Invalid start of list!";
               error_=true;
               return false;
            }
            break;
         case MiTT_COMMA:
            if(prevToken==MiTT_COMMA || prevToken==MiTT_CP)
               vl.push_front(ValElement("", miutil::conf::STRING));
            break;
         case MiTT_OP:
            if(prevToken==MiTT_COMMA || prevToken==MiTT_CP)
               vl.push_front(ValElement("", miutil::conf::STRING));
            break;
         default:
            break;
      }

      prevToken=t.tt;
   }

   errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Invalid variable specification!";
   error_=true;
   return false;
}

bool
miutil::conf::ConfParser::
colapseKeyVal(const Token &t)
{  
   //The top of stack must be a ID
   bool ignore=false;

   if(debugLevel_>1){
      if(debugLevel_>2){
         cout << "Tokenstack before colapseKeyVal ....\n";
         printTokenStack(cout);
      }else
         cout << "colapseKeyVal ... \n";
   }


   if(tokenStack_.empty()){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Missing variable name!";
      error_=true;
      return false;
   }

   if(stack_.empty()){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Internal, unexpected end of stack!";
      error_=true;
      tokenStack_.pop_front();
      return false;
   }

   Token &tt=tokenStack_.front();

   if(tt.tt!=MiTT_ID){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Expecting a varible name (ID)!";
      error_=false;
      return false;
   }


   ValElement v;

   switch(t.tt){
      case MiTT_STRING: v=ValElement(t.val, miutil::conf::STRING);
      break;
      case MiTT_ID:     v=ValElement(t.val, miutil::conf::STRING);
      break;
      case MiTT_FLOAT: v=ValElement(t.val, miutil::conf::FLOAT);
      break;
      case MiTT_INT:  v=ValElement(t.val, miutil::conf::INT);
      break;
      case MiTT_NL:  //New line, ignore the key.
         ignore=true;
         if(debugLevel_>1)
            cout << "Key: " << tt.val << ". Missing value, ignored! File: " << curIst->file << " Line: " << curIst->lineno  << endl;
         break;
      default:
         errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Internal, unexpected token!";
         error_=true;
         tokenStack_.pop_front();
         return false;
   }

   if(!ignore && !stack_.front()->addValue(tt.val, v, false)){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": ERROR, overwriting existing value for key <" << t.val << ">!";
      error_=true;
      tokenStack_.pop_front();
      return false;
   }

   tokenStack_.pop_front();

   if(debugLevel_>1){
      if(debugLevel_>2){
         cout << "Tokenstack after colapseKeyVal ....\n";
         printTokenStack(cout);
      }else
         cout << "colapseKeyVal (colapsed)\n";
   }

   //After we have colapsed the key=val the token stack should
   //be empty or the top token must be OB ie. {

   if(tokenStack_.empty() || tokenStack_.front().tt==MiTT_OB)
      return true;
   else{
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": Unexpected token <" << tokenStack_.front().val << ">!";
      error_=true;
      return false;
   }

}


bool
miutil::conf::ConfParser::
colapseSection()
{
   //The token stack must have the elements OB and ID on the top

   if(debugLevel_>1){
      if(debugLevel_>2){
         cout << "Tokenstack before colapseSection ....\n";
         printTokenStack(cout);
      }else
         cout << "colapseSection ... \n";
   }


   if(stack_.empty()){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Unexpected empty stack!";
      error_=true;
   }

   if(tokenStack_.empty()){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << ": Unexpected empty token stack!";
      error_=true;
      return false;
   }

   if(tokenStack_.front().tt!=MiTT_OB){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": Unexpected token <" << tokenStack_.front().val << ">!";
      error_=true;
      return false;
   }

   tokenStack_.pop_front();

   if(tokenStack_.front().tt!=MiTT_ID){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": Unexpected token <" << tokenStack_.front().val << ">!";
      error_=true;
      return false;
   }


   if(stack_.size()<2){
      errs_ << "File: " <<curIst->file << " Line: " << curIst->lineno
            << ": There is unbalanced sections, ie. there is missing { or }.";
      error_=true;
      return false;
   }

   ConfSection *sect=stack_.front();
   stack_.pop_front();

   if(!stack_.front()->addSection(tokenStack_.front().val, sect, false)){
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": overwriting section variable <" << tokenStack_.front().val
            << ">!";
      error_=true;
      delete sect;
      return false;
   }

   //get rid of ID
   tokenStack_.pop_front();


   if(debugLevel_>1){
      if(debugLevel_>2){
         cout << "Tokenstack after colapseSection ....\n";
         printTokenStack(cout);
      }else
         cout << "colapseSection (colapsed) ... \n";
   }


   //After we have colapsed the section the token stack should
   //be empty or the top token must be OB ie. {

   if(tokenStack_.empty() || tokenStack_.front().tt==MiTT_OB)
      return true;
   else{
      errs_ << "File: " << curIst->file << " Line: " << curIst->lineno
            << ": Unexpected token <" << tokenStack_.front().val << ">!";
      error_=true;
      return false;
   }

}




std::string
miutil::conf::ConfParser::Token::
print()const
{
   ostringstream ost;

   ost << "Line: " << line << " <" << val << "> Tokentype: ";

   switch(tt){
      case MiTT_ID:     ost << "ID";
      break;
      case MiTT_STRING: ost << "STRING";
      break;
      case MiTT_INT:    ost << "INT";
      break;
      case MiTT_FLOAT:  ost << "FLOAT";
      break;
      case MiTT_ALIAS:  ost << "ALIAS";
      break;
      case MiTT_EQUAL:  ost << "EQUAL"; //=
      break;
      case MiTT_OB:     ost << "OB";    //{
      break;
      case MiTT_CB:     ost << "CB";    //}
      break;
      case MiTT_OP:     ost << "OP";    //(
      break;
      case MiTT_CP:     ost << "CP";    //)
      break;
      case MiTT_COMMA:  ost << "COMMA"; //,
      break;
   }

   return ost.str();
}
