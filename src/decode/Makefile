# includefile contains Compiler definition
TOP:=../..

export PROJECT:=synop.so

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk


export TOBIN:=$(TOP)/lib/decode




#CC=gcc
#CXX=g++



override DEFINES += -D_REENTRANT


INCLUDE= -I$(TOP)/include             \
         -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
		 $(BOOSTINCLUDE)  



COMSRC= synop.cc kvSynopDecoder.cc
ALLSRC= synopdecoder.cc synopentry.cc decode.cc $(COMSRC)


COMOBJ=  $(COMSRC:.cc=.o) synopLexer.o

SOOBJ  := $(COMOBJ) synopdecoder.o synopentry.o
DECOBJ  = $(COMOBJ) decode.o




LINKS= $(BOOSTLIB) -L${TOP}/lib -L$(PUTOOLS)/lib -lkvalobs -lkvdb -lpuTools -ldnmithread  \
	-lboost_thread  -lboost_regex -lfileutil -ldl -lmiutil -lpthread -lm 

SOLINK:=

.PHONY: all veryclean clean depends

.SUFFIXES: .c .o

# From .cc to .o
.cc.o:
	$(CXX)  $(CXXFLAGS) $(DEFINES) $(CXXSOFLAGS) $(INCLUDE) -c $< 

#	$(CXX) -c $(CCFLAGS) $< $(INCLUDE)

DEPENDSFILE=make.depends

CCFLAGS= -LANG:std



$(PROJECT): $(SOOBJ)
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -fPIC -o $@ \
	       $(OBJFILES) $(LIB_DIR) $(LIBS) $(SOOBJ)
	mkdir -p lib/decode
	cp $@ lib/decode

decode: $(DECOBJ) 
	$(CXX)  $(CXXFLAGS) $(CXXLINKSO) -o $@ $(DECOBJ) $(LINKS) $(LIBS)


synopLexer.o: synopLexer.lex
	$(LEX) $(LFLAGS) -PyySynop synopLexer.lex; \
	mv -f lex.yySynop.c synopLexer.cc
	$(CXX) $(CCFLAGS) -c synopLexer.cc $(INCLUDE) 


clean:
	rm -f core core.* 
	rm -f synopLexer.cc
	rm -f decode synop.so
	rm -f *.o
	rm -rf cronjob
	rm -rf lib/decode
	rm -f make.depends

veryclean: clean

include $(wildcard $(DEPENDSFILE))
#include $(DEPENDSFILE)



