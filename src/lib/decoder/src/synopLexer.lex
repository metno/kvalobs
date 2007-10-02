
%{ 
  /* 
     Routine to parse a synop from a string
     DNMI/PU - JS 12/2001                                 
  */

#include <synopLexer.h>


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

word   [^ \t\n\r]+  
ws     [ \t\n\r]+
end    [= ]+
value  ([0-9]|\/)
section (111|222|333|444|555)

%%

{value} |
{ws}    |
{end}   { break;}

222{value}{2}{ws}      { Synop->fetchSection(yySynoptext);                  }
{section}{ws}          { Synop->fetchSection(yySynoptext);                  }
{value}{5}{ws}         { Synop->sortToken(false,yySynoptext);               }
{value}{5}{end}        { Synop->sortToken(true,yySynoptext);                }
(OOXX|AAXX|BBXX){ws}   { Synop->setType(yySynoptext);                       }
{word}                 { Synop->setText(yySynoptext);                       }
ICE{ws}{value}{5}{ws}  { Synop->setIceToken(yySynoptext);                   }
(ICE){ws}{word}{ws}    { Synop->setIceText(syn::ICETEXTTOKEN,yySynoptext);  }
(ICING){ws}{word}{ws}  { Synop->setIceText(syn::ICINGTEXTTOKEN,yySynoptext);}



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


synop lexSynop(const std::string& synopStr)
{
  synop newSynop;
  Synop = &newSynop;

  synopToParse =  (char*)malloc((synopStr.size()+1)*sizeof(char));
  strcpy(synopToParse,synopStr.c_str());
 
  synopToParsePtr=synopToParse;
  synopToParseLim=synopToParsePtr+strlen(synopToParse);
 
     
  yySynoplex();
  
  Synop=NULL;
  return newSynop;
}












