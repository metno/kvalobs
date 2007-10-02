# includefile contains Compiler definition
TOP:=../..

export PROJECT:=decodemetar

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk



override DEFINES += -D_REENTRANT


INCLUDE=-I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    -I$(TOP)/include             \
	    -I$(TOP)/include/tips        \
	    $(BOOSTINCLUDE)   


ALLOBJ= kvMetar.o kvMetarDecoder.o decode.o

ALLSRC= $(ALLOBJ:.o=.cc) 
LINKS=-L${TOP}/lib -L$(PUTOOLS)/lib -lkvalobs -ldb -ldnmithread\
	  -ltips 	-lmiutil -lfileutil -lpuTools -lpthread -ldl        \
	   $(BOOSTLIB) 	-lboost_thread  -lboost_regex -lm 

.PHONY: all veryclean clean depends

.SUFFIXES: .c .o

# From .cc to .o
.cc.o:
#	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXSOFLAGS) $(INCLUDE) -c $< 

	$(CXX) -c $(CCFLAGS) $< $(INCLUDE)

DEPENDSFILE=make.depends



#$(PROJECT): $(SOOBJ)
#	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -fPIC -o $@ \
#	       $(OBJFILES) $(LIB_DIR) $(LIBS) $(SOOBJ)

decodemetar: $(ALLOBJ) 
	$(CXX)  $(CXXFLAGS)  -o $@ $(ALLOBJ) $(LINKS) 


include $(DEPENDSFILE)



