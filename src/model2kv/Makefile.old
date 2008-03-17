# Makefile to generate a interface between POSE and QUBA

PROJECT=model2kv
TOP=../..

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk
-include $(DEPENDSFILE)


LOCALDIR=/metno/local

LINCDIR=$(LOCALDIR)/include
LIBDIR=$(LOCALDIR)/lib


C_INCLUDE = -I./ -I$(LINCDIR)/puCtools\
	    -I$(LINCDIR)/libpose\
	    -I$(LINCDIR)/parameter

INCLUDE = $(C_INCLUDE) -I$(LINCDIR)/puTools

LINKS = -L$(LIBDIR) -lpose -lpuCtools -lparameter  -lpuTools  -lm 


LEX=flex

YACC=bison
YFLAGS= -d


POSEOBJ = $(PROJECT).o
PARSEOBJ= suParse.o suLex.o

ALLOBJ=   $(PARSEOBJ) $(POSEOBJ) 


ALLSRC= $(POSEOBJ:.o=.cc)


.SUFFIXES: .cc .c .o

# From .c to .o
.c.o:
	$(CC) -c $(CFLAGS) $< $(C_INCLUDE)

# from .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $< $(INCLUDE)



$(PROJECT): $(ALLOBJ)
	$(CXX)  $(CXXFLAGS) -o $@ $(ALLOBJ) $(LINKS)


#-----------------------------------------------------------------

suParse.o: suParse.y
	$(YACC) $(YFLAGS) -p yysu -o suParse.c suParse.y; \
	$(CC) $(CFLAGS) -c suParse.c $(C_INCLUDE)

suLex.o: suLex.lex
	$(LEX) -Pyysu suLex.lex; \
	mv -f lex.yysu.c suLex.c
	$(CC) $(CFLAGS) -c suLex.c $(C_INCLUDE)




