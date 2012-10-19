%{
  /* suLex.lex -- lexer for pose input.
     NOTE: This lexer uses some of the special features of GNU flex.
     AUTHOR: Lars Brusletto Sveen, August 1997, continously improved until Feb. 1998
  */

#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "libpose/pose.h"
#include "libpose/psSort.h"
#include "puCtools/safety.h"
#include "libpose/psSubstitute.h"
#include "setup.h"
#include "suParse.h"

  /* We don't want YY_BREAK */
#undef  YY_BREAK
#define YY_BREAK

  /* A tiny hack to fix wrapping 
#undef yywrap
#define yywrap yywrap */

int yywrap() { return 1; }

static int countfiles();

static int lineno=1;
static int nfiles;

struct globalinput setup;
struct nescessaryinput inputlist;

int    yysuargc;
char** yysuargv;

%}

ws [ \t]+
comment \*.*
string \"[^\"\n]*[\"\n]
eq {ws}?={ws}?
cmdline \#[0-9]+

%x FTYPENAME
%x COUNT_FILES

%%

<COUNT_FILES>DATA\.IN   { nfiles++; break; }
<COUNT_FILES>.		|
<COUNT_FILES>\n		{ break; }

{ws}			{ break; }
<*>{comment}		{ break; }

{string}		{ yysulval.string=strdup_check(yysutext+1);
			  if (yysulval.string[yysuleng-2]!='"') { 
			    yysuwarning("Unterminated character string");
			    lineno++;
			  }
			  yysulval.string[yysuleng-2]='\0';
			  return QSTRING; }

{cmdline}		{ yysulval.integer=atoi(yysutext+1);
			  if (yysulval.integer>yysuargc-1)
			    yysuerror("No such command line argument");
			  return CMDNO; } 


  /* Keywords follow */
DATA\.IN{eq}		{ return DATAIN; }
FILE\.TYPE{eq}		{ BEGIN(FTYPENAME);
			  return FILETYPE; }
OUTPUT\.FILE{eq}        { return OUTPUT; }
POSITION\.NAMECHANGES{eq} {
                          return POSNCHANGE; }
PARAMETER.CHANGES{eq} 	{ return PARCHANGE; }
PARAMETER\.DEFINITION\.FILE{eq} {
			  return PARDEF; }
MODEL\.NAME{eq}		{ return MODNAME; }
MAKE\.OUTPUT\.BACKUP    { setup.makeoutputbackup=true; break; }
NORWEGIAN\.CHARACTERS   { setup.sub.norwegianchars=true; break; }
LOWERCASE\.NAMES        { setup.sub.lowercasenames=true; break; }
REMOVE\.SUFFIXES 	{ setup.sub.removesuffixes=true; break; }
SUFFIX\.TAG{eq} 	{ return SUFFTAG; }

  /* Reserved words for input file types */
<FTYPENAME>[a-z]+ 	{ if (strcmp(yysutext,"metdat")==0)
  			    yysulval.ftype=metdat;
			  else if (strcmp(yysutext,"sondat")==0)
			    yysulval.ftype=sondat;
			  else if (strcmp(yysutext,"sea")==0)
			    yysulval.ftype=sea;
			  else if (strcmp(yysutext,"wave")==0)
			    yysulval.ftype=wave;
			  else 
			    yysuerror("Illegal file type");
			  BEGIN(INITIAL);
			  return FTYPE; }

  /* Return any other character and count lines */
.			{ return yysutext[0]; }
\n			{ lineno++; break; }

%%

void yysuwarning(const char* s)
{
  printf("\nWarning (%s line %d): %s\n",yysuargv[1],lineno,s);
}

void yysuerror(const char* s)
{
  fprintf(stderr,"\nError parsing file %s (at line %d): %s\n",
	  yysuargv[1],lineno,s);
  exit(1);
}

/* countfiles counts the occurrences of DATA.IN statements in the setup file */

int countfiles()
{
  BEGIN(COUNT_FILES);
  yysulex();

  /* reset lexer for subsequent parsing */
  rewind(yysuin);
  yysurestart(yysuin);
  BEGIN(INITIAL);

  return nfiles;
}

/* parseinput: read, check and fix input from setup file */

int parseinput()
{
  int i,ret;
  char message[80];
  char* oldoutput;
  FILE* sufile;

  /* setup file: */
  printf("Reading setup file %s. . . ",yysuargv[1]);
  fflush(stdout);
  sufile=fopen_check(yysuargv[1],"r");
  yysuin=sufile;

  /* Default values: */
  setup.sub.norwegianchars=false;
  setup.sub.lowercasenames=false;
  setup.sub.removesuffixes=false;
  setup.makeoutputbackup=false;
  setup.sub.suffixtag='%';
  setup.sub.posnamealias=0;
  setup.nfiles=countfiles();

  /* Allocate place */
  setup.posfilelist=malloc_check(setup.nfiles*sizeof(struct posfile));
  for (i=0; i<nfiles; i++) {
    setup.posfilelist[i].parchanges=0;
    setup.posfilelist[i].modname=0;
  }

  /* inputlist is used to check whether sufficient input is given: */
  inputlist.datafile=false;
  inputlist.outputfile=false;

  ret=yysuparse();

  printf("%s OK.\n",yysuargv[1]);
  fflush(stdout);
  fclose(sufile);

  /* let's see what we've found... */
  message[0]='\0';
  if (!inputlist.datafile)
    strcat(message,"No data file specified.\n");
  /* I really should check if a file name is listed twice. . . */
  if (!inputlist.outputfile) { /* We can live with that, I suppose */
    printf("\nWarning: Anomaly in input: No output file specified.\n"
	   "Output will be given default name %s.\n",_defaultoutfile);
    setup.outfile=strdup_check(_defaultoutfile);
  }

  /* If any serious misses were detected, print error message and exit */
  if (*message) {
    fprintf(stderr,"\nError(s) discovered when processing input:\n%s",
	    message);
    exit(1);
  }

  /* Finally, some simple refinements if the input is a bit dodgy: */
  if (setup.makeoutputbackup) {
    /* Make backup if a file called outfile already exists */
    oldoutput=malloc_check(strlen(setup.outfile)+strlen(_backupextension)+1);
    strcat(strcpy(oldoutput,setup.outfile),_backupextension);
    if (rename(setup.outfile,oldoutput)==0)
      printf("Warning: Existing output file %s renamed %s.\n",
	     setup.outfile,oldoutput);
    free(oldoutput);
  }
  else
    /* If there is a file setup.outfile, remove it */
    if (remove(setup.outfile)==0)
      printf("Existing file %s removed.\n",setup.outfile);

  /* The posnamealias list of station name substitutes must be sorted: */
  if (setup.sub.posnamealias)
    qsort(setup.sub.posnamealias->list,setup.sub.posnamealias->entries,
	  sizeof(struct bistring*),
	  (int (*)(const void*, const void*))bistrcmp);
  return ret;
}
