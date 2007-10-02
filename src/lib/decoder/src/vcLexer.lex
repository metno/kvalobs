
%{ 
  /* Routine to parse the verification control structure
     from input file/string 
     DNMI/PU - JS 9/2000                                 */

#include <stringmanip.h>
#include <vcLexer.h>
#include <tipsLexTools.h> 
#include <mathExtra.h>
#include <bool.h>
#include <filetools.h>

#define LEAD        	     101 
#define ISSUED    	     102 
#define OBSNAME   	     103 
#define FCANAME   	     104 
#define DDTHOLD              105               
#define DDFFMIN              106 
#define FFTHOLD              107 
#define FFFFMIN              108 
#define FFNUMTH              109

#undef  YY_INPUT
#define YY_INPUT(b,r,ms) (r=vc_yyinput(b,ms))

  static int vc_yyinput(char*, int); 	      
  
  static char* vcToParse;       /* input string for lexer */
  static char* vcToParsePtr;    /* pointer to current position */
  static char* vcToParseLim;    /* pointer to end of vcToParse */
  
#define YY_SKIP_YYWRAP				      
  int yyVcwrap() { return 1; }	      

  vControl *vc;
  void setVal(const int, char*);
  void setPar(char*);
  void setTime(bool, char*);

%}

/* General rules */

word   [^ \t\n]+  
ws     [ \t]+
eol    \n
eq     {ws}?={ws}?
end    {ws}?{eol}
str    [A-Z]+
num    [0-9]+
ymd    [0-9]{4}\-[0-9]{2}\-[0-9]{2}{end}
list   \{{ws}?{str}({ws}?,{ws}?{str})*{ws}?\}{end}

%%
{eol}   |
{end}   |
{ws}    |
{eq}    |
{str}   |
{num}   | 
{ymd}   |
{list}  |
{word}  { break;}

START{eq}{ymd}             {setTime(true,yyVctext);}
STOP{eq}{ymd}              {setTime(false,yyVctext);}
ISSUED{eq}{num}{end}       {setVal(ISSUED,yyVctext);}
LEAD{eq}{num}{end}         {setVal(LEAD,yyVctext);}
OBSERVATION{eq}{str}{end}  {setVal(OBSNAME,yyVctext);}
FORECAST{eq}{str}{end}     {setVal(FCANAME,yyVctext);}

PARAMETERS{eq}{list}       {setPar(yyVctext);}

DD_THOLD{eq}{num}{end}     {setVal(DDTHOLD,yyVctext);}
DD_FF_MIN{eq}{num}{end}    {setVal(DDFFMIN,yyVctext);}
FF_THOLD{eq}{num}{end}     {setVal(FFTHOLD,yyVctext);}
FF_FF_MIN{eq}{num}{end}    {setVal(FFFFMIN,yyVctext);}
FF_NUM_THRES{eq}{num}{end} {setVal(FFNUMTH,yyVctext);}


%%

static int vc_yyinput(char* buf, int max_size)
{
  int n=Min(max_size, vcToParseLim-vcToParsePtr);
  if (n>0) {
    memcpy(buf, vcToParsePtr, n);
    vcToParsePtr+=n;
  }
  return n;
}



void setVal(const int choice, char* text)
{
  char* token = NULL;
  int itok=0;

  if(!copyToken(&token,text)) return;
  

  if( choice != FCANAME && choice != OBSNAME)
    itok = atoi(token);
  
  switch (choice) {
  case LEAD:
    vc->lead    = itok;
    break;
  case ISSUED:
    vc->issued  = itok;
    break;
  case OBSNAME:
    vc->obsName = strdup(token);
    break;
  case FCANAME:  
    vc->fcaName = strdup(token);
    break;
  case  DDTHOLD:  
    vc->ddThold = (itok ? itok : 30 ); 
    break;
  case  DDFFMIN:  
    vc->ddffMin = (itok ? itok : 5  );
    break;
  case  FFTHOLD:  
    vc->ffThold = (itok ? itok : 5  );
    break;
  case  FFFFMIN:  
    vc->ffffMin = (itok ? itok : 10 );
    break;
  case  FFNUMTH:  
    vc->numFFthres = (itok ? itok : 7);
    break;
  }

  free(token);

}

void setPar(char* text)
{
  char* token = NULL;
  ptrChar* tmp;
  int i,s=0;
  pType p;
  
  if(!copyToken(&token,text)) return;
  tmp=getStrList(token);

  for(i=0;i<tmp->size;i++)
    if(str2pType(tmp->ptr[i]))
      s++;

  vc->list = ptrPtypeCreate(s);
  s=0;

  for(i=0;i<tmp->size;i++){
    p = str2pType(tmp->ptr[i]);
    if(p) {
      vc->list.ptr[s] = p;
      s++;
    }
  }
  ptrCharKill(&tmp);

  free(token);

}

void setTime(bool start, char* text)
{
  cDate tmp;
  char* token=NULL;

  if(!copyToken(&token,text)) return;

  tmp = str2cDate(token);
  
  if(start)
    vc->start = tmp;
  else
    vc->stop = tmp;

  free(token);

}



vControl* lexVC(const char* vcStr)
{
  vc=NULL;
  
  if(!vcStr)
    return vc;

  vcToParse= strdup(vcStr);
 
  vcToParsePtr=vcToParse;
  vcToParseLim=vcToParsePtr+strlen(vcToParse);
 
  vc=vControlCreate(0);
    
  yyVclex();
  
  return vc;
}


vControl* lexVCfile(const char* fname)
{
  char *vcIn=file2str(fname);

  lexVC(vcIn);

  free(vcIn);

  return vc;
}







