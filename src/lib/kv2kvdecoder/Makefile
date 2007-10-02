# includefile contains Compiler definition
TOP:=../../..

export PROJECT:=kv2kvDecoder.so

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk


export TOBIN:=$(TOP)/lib/decode

#CC=gcc
#CXX=g++

override DEFINES += -D_REENTRANT

INCLUDE=-I$(TOP)/include             \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(BOOSTINCLUDE)              \
	   $(LIBXMLPPINCLUDE) $(CPPUNIT_INCLUDE)



KLSRC= kv2kvDecoder.cc kv2kvEntry.cc
ALLSRC= $(KLSRC)


KLOBJ=  $(KLSRC:.cc=.o)

SOOBJ  := $(KLOBJ)


LINKS= $(BOOSTLIB) -L${TOP}/lib -lkvalobs -ldb -ldnmithread  \
	-lboost_thread  -lboost_regex -lfileutil -ldl -lmiutil -lpthread -lm $(LIBXMLPPLIB) $(CPPUNIT_LIB)

SOLINK:=

.PHONY: all veryclean clean depends

.SUFFIXES: .c .o

# From .cc to .o
.cc.o:
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXSOFLAGS) $(INCLUDE) -o $@ -c $<

#	$(CXX) -c $(CCFLAGS) $< $(INCLUDE)

DEPENDSFILE=make.depends

CCFLAGS= -LANG:std

$(PROJECT): $(SOOBJ)
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -fPIC -o $@ \
	       $(OBJFILES) $(LIB_DIR) $(LIBS) $(SOOBJ)
	mkdir -p lib/decode
	cp $@ lib/decode
	rm -f core core.*

LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB)
LIBS=-ldecodeutility -lmiutil -ldecoderbase -lkvalobs -ldb -lfileutil -lmiconfparser \
	-ldnmithread  \
	-lcorbahelper -lmilog -lboost_thread -lcorba_skel -lomniORB4 \
	-lomnithread -lomniDynamic4 -ldl -lpthread \
  $(LIBXMLPPLIB)


-include test/make.mk
TESTOBJ=$(TESTSRC:.cc=.o)

testKv2kvDecoder: $(KLOBJ) $(TESTOBJ)
	g++ -o testKv2kvDecoder $(CCFLAGS) $(KLOBJ) $(TESTOBJ) $(LIB_DIR) $(LIBS) -lcppunit

test: testKv2kvDecoder
	./testKv2kvDecoder

clean:
	rm -f core core.*
	rm -f $(KLOBJ) $(TESTOBJ)
	rm -rf lib/
	rm -f *.so
	rm -f *~
	rm -f make.depends

veryclean: clean

include $(wildcard $(DEPENDSFILE))
#include $(DEPENDSFILE)



