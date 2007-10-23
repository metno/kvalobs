# includefile contains Compiler definition
TOP=../../..

PROJECT=kvpush 

TEST_BIN=

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk

-include $(DEPENDSFILE)

override DEFINES +=  -D_REENTRANT

KVPUSH_OBJ=kvpushApp.o kvpush.o DataTblView.o
KVPUSH_SRC=$(KVPUSH_OBJ:.o=.cc) 
ALLSRC= $(KVPUSH_SRC)

INCLUDE=-I$(TOP)/include              \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    -I$(TOP)/include/kvskel       \
	    $(BOOSTINCLUDE)\
	    $(OMNIINCLUDE) 

LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB) -L$(PUTOOLS)/lib

LIBS=-lkvalobs -lkvdb -lfileutil -lmiutil -lcorbahelper \
     -lmilog -lmiconfparser\
     -ldnmithread -lpuTools -lboost_thread -lcorba_skel -lomniORB4 -lomnithread \
     -lomniDynamic4  -ldl -lpthread



# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $(DEFINES) $< $(INCLUDE)

kvpush: $(KVPUSH_OBJ)
	$(CXX)  $(CXXLINKSO) -o $@  $(KVPUSH_OBJ) $(LIB_DIR) $(LIBS)
	cp -f $@ $(TOP)/bin	

clean: 	
	rm -f *.o
	rm -f kvpush
	rm -f $(DEPENDSFILE)
	rm -f core core.*
	rm -rf bin

veryclean: clean

