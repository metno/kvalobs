
%{ 
  /* 
     Routine to parse a synop from a string
     DNMI/PU - JS 12/2001                                 
  */

#include "synopLexer.h"


#undef  YY_INPUT
#define YY_INPUT(b,r,ms) (r=synop_yyinput(b,ms))

  static int synop_yyinput(char*, int); 	      
  
  static char* synopToParse;       /* input string for lexer */
  static char* synopToParsePtr;    /* pointer to current position */
  static char* synopToParseLim;    /* pointer to end of synopToParse */
  
#define YY_SKIP_YYWRAP				      
  int yySynopwrap() { return 1; }	      
  synop *Synop;
%}

/* General rules */

word    [^ \t\n\r]+  
ws      [ \t\n\r]+
end     [= ]+
trash   (\/|X|x)
value   ([0-9]|\/|X|x)
section (222|333|444|555|999)
ice     (ICE|ice)
icing   (ICING|icing)



%%
{icing} |
{ice}   |
{value} |
{ws}    |
{end}   { break;}

{trash}{5}({ws}|{end})          { Synop->sortToken(yySynoptext, true);               }
222{value}{2}({ws}|{end})       { Synop->fetchSection(yySynoptext);                  }
{section}({ws}|{end})           { Synop->fetchSection(yySynoptext);                  }
{value}{5}({ws}|{end})          { Synop->sortToken(yySynoptext);                     }
(OOXX|AAXX|BBXX){ws}            { Synop->setType(yySynoptext);                       }
{word}                          { Synop->setText(yySynoptext);                       }
{ice}{ws}{value}{5}({ws}|{end}) { Synop->setIceToken(yySynoptext);                   }
{ice}{ws}{word}({ws}|{end})     { Synop->setIceText(syn::ICETEXTTOKEN,yySynoptext);  }
{icing}{ws}{word}({ws}|{end})   { Synop->setIceText(syn::ICINGTEXTTOKEN,yySynoptext);}



%%

static int synop_yyinput(char* buf, int max_size)
{

  int A = max_size;
  int B = synopToParseLim-synopToParsePtr;

  int n= ( A < B ? A : B );

  if (n>0) {
    memcpy(buf, synopToParsePtr, n);
    synopToParsePtr+=n;
  }
  return n;
}


/*
 * 2003.11.25 Bxrge
 * Fixed a memmory leak. synopToParse was never deleted!
 */
synop lexSynop(const std::string& synopStr)
{
  synop newSynop;
  Synop = &newSynop;

  //synopToParse =  (char*)malloc((synopStr.size()+1)*sizeof(char));
  try{
    synopToParse =  new char[(synopStr.size()+1)*sizeof(char)];
  }
  catch(...){
    //NOMEM
    return newSynop;
  }

  strcpy(synopToParse,synopStr.c_str());
 
  synopToParsePtr=synopToParse;
  synopToParseLim=synopToParsePtr+strlen(synopToParse);
     
  yySynoplex();

  delete synopToParse;
  
  Synop=NULL;
  return newSynop;
}












