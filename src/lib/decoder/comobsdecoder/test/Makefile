# includefile contains Compiler definition
TOP:=../../../..

export PROJECT=alltest

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk

LIBXML_INCLUDE:=$(shell xml2-config --cflags)
LIBXML_LIB_TMP:=$(shell xml2-config --libs)
LIBXML_LIB:=$(filter-out -lpthread, $(LIBXML_LIB_TMP))

export TOBIN=

override DEFINES += -D_REENTRANT

INCLUDE=-I$(TOP)/include             \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(LIBXML_INCLUDE)            \
	    $(BOOSTINCLUDE)             



LIBS=-L$(TOP)/lib $(BOOSTLIB) $(LIBXML_LIB) -lkvalobs -lkvdb -ldnmithread  \
     -lboost_thread  -lboost_regex -lfileutil  -lmiutil -lmilog -lpthread \
     -lm -ldl 

#LIBS=-L$(TOP)/lib $(BOOSTLIB) -lkvdb -lfileutil -ldl 

#MYLIBS=-L$(TOP)/lib 

.PHONY: all veryclean clean depends

.SUFFIXES: .c .o

# From .cc to .o
.cc.o: 
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -c $< 
#	$(CXX) -c $(CCFLAGS) $< $(INCLUDE)

alltest: cleanbin testsmsbasetimes testsms2


testsmsbasetimes: testsmsbasetimes.o ReadParamsFromFile.o
	$(CXX) $(CXXLDFLAGS) -o $@ testsmsbasetimes.o ReadParamsFromFile.o \
	     ../smsbase.o $(LIBS) -lmilog


testsms2: cleanbin testsms2.o ReadParamsFromFile.o 
	$(CXX)  $(CXXLINKSO)  -o $@  testsms2.o ../sms2.o    \
	   ../smsbase.o  ../smsdata.o ReadParamsFromFile.o   \
	   ../smsmelding.o ../smsmeldingparser.o $(LIBS) 

cleanbin:
	cd ..; $(MAKE) 
	rm -f testsms2
#	rm -f testsmsbasetimes

clean:
	rm -f core core.* 
	rm -f *.o
	rm -rf lib/
	rm -f *.so
	rm -f *~
	rm -f testsmsbasetimes testsms2
	rm -f make.depends

veryclean: clean





