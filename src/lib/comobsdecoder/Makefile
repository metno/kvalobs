# includefile contains Compiler definition
TOP:=../../..

export PROJECT:=comobsdecoder.so

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk


export TOBIN:=$(TOP)/lib/decode

override DEFINES += -D_REENTRANT

INCLUDE=-I$(TOP)/include             \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(BOOSTINCLUDE)              \
	    $(LIBXML_INCLUDE)



KLSRC= comobsdecode.cc comobsentry.cc smsmelding.cc smsmeldingparser.cc \
       decodeutil.o decodeddata.cc smsbase.cc sms2.cc sms12.cc \
       ConfInfo12.o	

ALLSRC= $(KLSRC)


KLOBJ=  $(KLSRC:.cc=.o) 

SOOBJ  := $(KLOBJ) 


LINKS=  -L${TOP}/lib  $(BOOSTLIB)  -ldb -ldnmithread  \
	-lboost_thread  -lboost_regex -lfileutil $(LIBXML_LIB) \
	-ldl -lmiutil -lpthread -lm 

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
	       $(SOOBJ) $(LIB_DIR) $(LINKS) 
	mkdir -p lib/decode
	cp $@ lib/decode

testsmsbasetimes: testsmsbasetimes.o smsbase.o
	$(CXX) $(CXXLDFLAGS) -o $@ testsmsbasetimes.o smsbase.o $(LINKS) \
	                           -lmilog


clean:
	rm -f core core.* 
	rm -f *.o
	rm -rf lib/
	rm -f *.so
	rm -f *~
	rm -f testsmsbasetimes
	rm -f make.depends

veryclean: clean

include $(wildcard $(DEPENDSFILE))
#include $(DEPENDSFILE)



