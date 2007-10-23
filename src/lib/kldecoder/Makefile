# includefile contains Compiler definition
TOP:=../../..

export PROJECT:=kldecoder.so

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk


export TOBIN:=$(TOP)/lib/decode

#CC=gcc
#CXX=g++

override DEFINES += -D_REENTRANT

INCLUDE=-I$(TOP)/include \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(BOOSTINCLUDE)  



KLSRC= kldecoder.cc klentry.cc paramdef.cc
ALLSRC= $(KLSRC)


KLOBJ=  $(KLSRC:.cc=.o) 

SOOBJ  := $(KLOBJ) 


LINKS= $(BOOSTLIB) -L${TOP}/lib -lkvalobs -lkvdb -ldnmithread  \
	-lboost_thread  -lboost_regex -lfileutil -ldl -lmiutil -lpthread -lm 

SOLINK:=

.PHONY: all veryclean clean depends

.SUFFIXES: .c .o

# From .cc to .o
.cc.o:
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXSOFLAGS) $(INCLUDE) -c $< 

#	$(CXX) -c $(CCFLAGS) $< $(INCLUDE)

DEPENDSFILE=make.depends

CCFLAGS= -LANG:std



$(PROJECT): $(SOOBJ)
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -fPIC -o $@ \
	       $(OBJFILES) $(LIB_DIR) $(LIBS) $(SOOBJ)
	mkdir -p lib/decode
	cp $@ lib/decode

clean:
	rm -f core core.* 
	rm -f *.o
	rm -rf lib/
	rm -f *.so
	rm -f *~
	rm -f make.depends

veryclean: clean

include $(wildcard $(DEPENDSFILE))
#include $(DEPENDSFILE)



