%{
#include "miconfparser/confparser.h"
#include <fstream>
#include <iostream>

namespace{
  miutil::conf::ConfParser    *theParser;
  bool                        stringContinue;
  
  /*struct TIstStack{
  	 YY_BUFFER_STATE state;
  	 std::istream    *ist;
  	 int             lineno;
  	 string          file;
  };*/
}

#define YY_INPUT(buf, result, bufLen) (result=theParser->yyInput(buf, bufLen)) 

using namespace std;

%}


%pointer
%option noyywrap
%x START_STRING
%x START_COMMENT
%x START_INCLUDE
%x FILENAME

DIGIT  [0-9]
ID     [_a-zA-Z]+[a-zA-Z0-9_\-]*
STRING [^"]*
FILENAME [^"]*
FLOAT  [-+]?{DIGIT}+\.{DIGIT}*
INT  [-+]?{DIGIT}+
   
%%
[ \r\t]+      /*Eat whitespace*/
\n             { theParser->charToken(yyConfParsertext[0]);
					  theParser->curIst->lineno++;
               }

@include          {  BEGIN(START_INCLUDE); }

<START_INCLUDE>[ \t]+ //Eat whitespace 
<START_INCLUDE>[^ \r\t\n]+ { theParser->newStream(yyConfParsertext); 
									  BEGIN(INITIAL); 
								   }

#                 {  BEGIN(START_COMMENT); }
                  
<START_COMMENT>[^\n]*  /*Eat comment*/
<START_COMMENT>\n { 
                    theParser->curIst->lineno++;
                    BEGIN(INITIAL);
                  } 
   /* {DIGIT}+    { theParser->intToken(yyConfParsertext);} */
{INT}    { theParser->intToken(yyConfParsertext);}
              

{FLOAT}     { theParser->floatToken(yyConfParsertext); }

{ID}        {  theParser->idToken(yyConfParsertext);}

!{ID}        {  theParser->ignoreIdToken(yyConfParsertext);}


${ID}       {  theParser->aliasToken(yyConfParsertext);}


\"	     { BEGIN(START_STRING); }

<START_STRING>{STRING}  {  stringContinue=false;
                           theParser->stringToken(yyConfParsertext);
                           
                           if(yyConfParserleng>=1){
                             if(yyConfParsertext[yyConfParserleng-1]=='\\'){
                               stringContinue=true;
                             }
                           }                   
                        }

<START_STRING>\" { 
                  if(!stringContinue)
                     BEGIN(INITIAL);  
                 }
                      
.         {  theParser->charToken(yyConfParsertext[0]);}

<<EOF>>   { 
            //cerr << "LEX:  EOF" << endl;
				if(!theParser->deleteStream()){
					//cerr << "LEX: terminate!" << endl;
					yyterminate();
				}
			 }

%%

int 
miutil::conf::priv::
ConfParserLexer(miutil::conf::ConfParser *parser)
{
   int ret;

   theParser=(miutil::conf::ConfParser*) parser;
   ret=yyConfParserlex();
   
   
   return ret;
}


int   
miutil::conf::ConfParser::
yyInput(char *buf, int bufLen)
{
  std::streamsize count;

  if(error_)
    return YY_NULL;

  if(!(*(curIst->ist)))
    return YY_NULL;
  
  curIst->ist->read(buf, bufLen); 

  count=curIst->ist->gcount();
  
  if(count>0)
    return count;
  else
    return YY_NULL;
  
}

bool
miutil::conf::ConfParser::
newStream(const std::string &file_)
{
	using namespace std;
	
	string file(file_);
	string::size_type i;
	string::size_type ii;
	string val;
	TIstStack *stackElem;
	
	//Serch for values on the form ${VAL}, VAL is the name of an
	//environment variable. It is expected that this value contains part of
	//a filepath.
	
	while((i=file.find("${"))!=string::npos){
		val.erase();
	
		if(i!=string::npos){
			file.erase(i, 2);
			ii=file.find("}", i);
		}else 
			ii=string::npos;	
		
		if(ii!=string::npos){
			string var=file.substr(i, ii-i);
			file.erase(ii,1);
		
			if(!var.empty()){
				char *e=getenv(var.c_str());
			
				if(e)
					val=e;
				else
					errs_ << "File: " << curIst->file << " Line: " << curIst->lineno << " No environment variable '"<<var<<"'!" 
							<< endl;
			}
		}
	
		if(!val.empty())
			file.replace(i, ii-i, val);
	}
	
	if(file.empty())
		return false;
	
	ifstream *f;
	
	try{
		f=new ifstream();
	}
	catch(...){
		errs_ << "NOMEM: cant open file: " << file << endl; 
		return false;
	}
	
	f->open(file.c_str());
	
	if(!f->is_open()){
		delete f;
		return false;
	}

	try{
	   stackElem= new TIstStack;
	}catch(...){
		f->close();
		delete f;
		errs_ << "NOMEM (Stack): cant open file: " << file << endl; 
		return false;
	}

	if(debugLevel_>0)
		cout << "\nReading file: " << file << endl;

	stackElem->state=0;
	stackElem->ist=f;
	stackElem->lineno=1;
	stackElem->file=file;
	
	curIst->state=YY_CURRENT_BUFFER;
	istStack.push(curIst);
	curIst=stackElem;
	yyConfParser_switch_to_buffer(yyConfParser_create_buffer(NULL, YY_BUF_SIZE));
	
	return true;
}

bool
miutil::conf::ConfParser::
deleteStream()
{
	using namespace std;
	//istStack empty: curIst is pointing to the stream that represent the 
	//main stream. ie. Not opened with an @include statement.

	if(istStack.empty())
		return false;
	
	//This is a ifstream opned with a call to newStream.	
	static_cast<ifstream*>(curIst->ist)->close();
	delete curIst->ist;
	yyConfParser_delete_buffer(YY_CURRENT_BUFFER);
	delete curIst;
	curIst=istStack.top();
	istStack.pop();
	
	yyConfParser_switch_to_buffer(static_cast<YY_BUFFER_STATE>(curIst->state));
	
	if(debugLevel_>0)
		cerr << "\nReading from prev stream: "<<curIst->file << endl;
		
	return true;
}
	

