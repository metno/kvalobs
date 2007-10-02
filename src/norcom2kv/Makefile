# includefile contains Compiler definition
TOP=../..

PROJECT=norcom2kv 

TEST_BIN=testWMORaport  

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk

include $(wildcard $(DEPENDSFILE))


DEFINES +=  -D_REENTRANT


ALLOBJ= norcom2kv.o  CollectSynop.o App.o WMORaport.o crc_ccitt.o \
	CorbaThread.o AdminImpl.o File.o InitLogger.o

 
ALLSRC= $(ALLOBJ:.o=.cc) 

INCLUDE= -I$(TOP)/include        \
         -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	     -I$(TOP)/include/kvskel      \
	     $(OMNIINCLUDE)               \
	     $(BOOSTINCLUDE)              

LINKS= -L$(TOP)/lib $(OMNILIB) $(BOOSTLIB) -L$(PUTOOLS)/lib -lkvalobs -lfileutil \
	-lmiutil -lmiconfparser -lcorbahelper -ldnmithread -lpuTools -lcorba_skel \
	-lboost_thread  -lmilog -lomniORB4 -lomnithread  -lboost_regex \
	-lpthread -lm 

# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $(DEFINES) $< $(INCLUDE)


norcom2kv: $(ALLOBJ)
	$(CXX)  $(CXXFLAGS) -o $@  $(ALLOBJ) $(LINKS)


testWMORaport: testWMORaport.o WMORaport.o
	$(CXX) $(CXXLDFLAGS)   -o $@ \
	testWMORaport.o WMORaport.o\
	       $(OBJFILES) $(BOOSTLIB) -lboost_regex

wmo2sdecode: wmo2sdecode.o WMORaport.o
	$(CXX) $(CXXLDFLAGS)   -o $@ \
	wmo2sdecode.o WMORaport.o\
	       $(OBJFILES) $(BOOSTLIB) -lboost_regex

clean: 	
	rm -f *.o
	rm -f norcom2kv testWMORaport wmo2sdecode
	rm -f $(DEPENDSFILE)
	rm -f core core.*
	rm -rf bin/
	rm -f make.depends

veryclean: clean

