%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "puCtools/bool.h"
#include "libpose/pose.h"
#include "setup.h"
#include "puCtools/safety.h"

static int fcount=0;

%}

%union {
  int integer;
  enum filetype ftype;
  char* string;
  struct posfile pfile;
  struct bistringlist* bstrl;
  struct bistring* bstr;
}

%token DATAIN FILETYPE OUTPUT POSNCHANGE PARCHANGE PARDEF MODNAME
%token SUFFTAG
%token<string> QSTRING
%token<integer> CMDNO
%token<ftype> FTYPE

%type<string> name
%type<pfile> datafile
%type<bstr> bistr
%type<bstrl> bistrlist

%%
items:		item
	|	items item
	;

item:		datafile   { inputlist.datafile=true; }
	|	outputfile { inputlist.outputfile=true; }
        |       posnamelist {}
	|	pardef {}
	|	sufftag {}
	;

name:		QSTRING {
  		  $$=$1;
}
	|	CMDNO {
		  $$=strdup_check(yysuargv[$1]);
}
	;

datafile:	DATAIN name FILETYPE FTYPE {
		  $$.name=$2;
		  $$.type=$4;
		  $$.modname=0;
		  setup.posfilelist[fcount++]=$$;
}
	|	datafile MODNAME name {
		  setup.posfilelist[fcount-1].modname=$3;
}
	|	datafile PARCHANGE bistrlist '.' {
		  setup.posfilelist[fcount-1].parchanges=$3;
}
	;

outputfile:	OUTPUT name {
		  setup.outfile=$2;
}
	;

bistrlist:      bistr {
		  $$=malloc_check(sizeof(struct bistringlist));
		  (*$$).list[0]=$1;
		  (*$$).entries=1;
}
        |       bistrlist bistr {
		  if ((*$$).entries<MAXPOS)
		    (*$$).list[(*$$).entries++]=$2;
		  else {
		    fprintf(stderr,"Limit of changes: %d",MAXPOS);
		    yysuerror("Too many name changes!");
		  }
}
        ;

bistr:          QSTRING ':' QSTRING {
                  $$=malloc_check(sizeof(struct bistring));
                  (*$$).changefrom=$1;
		  (*$$).changeto=$3;
}
        ;

posnamelist:    POSNCHANGE bistrlist '.' {
		  setup.sub.posnamealias=$2;
}
        ;

pardef:		PARDEF name {
		  setup.pardeffile=$2;
}
	;

sufftag:	SUFFTAG name {
		  setup.sub.suffixtag=*$2;
		  free($2);
}

%%
