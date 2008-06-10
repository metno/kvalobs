# includefile contains Compiler definition

PROJECT=kv2metdat
TOP=../..

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk
include $(DEPENDSFILE)


ALLOBJ= kv2metdat.o 
ALLSRC= $(ALLOBJ:.o=.cc) 

INCLUDE=-I$(TOP)/include \
        -I$(PUTOOLS)/include  -I$(PUTOOLS)/include/puTools  \
	     $(BOOSTINCLUDE)


LINKS=   -L$(TOP)/lib $(BOOSTLIB) -L$(PUTOOLS)/lib -lkvalobs -ldnmithread -lboost_thread  -lpthread -lkvdb -lfileutil -ldl -lpuTools -lm 


# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $< $(INCLUDE)


$(PROJECT): $(ALLOBJ)
	$(CXX)  $(CXXFLAGS) -o $@ $(ALLOBJ) $(LINKS)







