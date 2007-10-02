TOP=../../..
include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(TOP)/include \
	    -I$(TOP)/include/kvskel \
	    -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
    	 $(BOOSTINCLUDE) $(OMNIINCLUDE)
    	 
LIBDIR=-L$(TOP)/lib $(BOOSTLIB) 
LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB) -L$(PUTOOLS)/lib 
LIBS=-lmiconfparser -lmilog -lkvalobs -lmiutil -ldb -ldnmithread \
     -lcorbahelper -lpuTools -lboost_thread \
     -lcorba_skel -lomniORB4 -lomnithread -lomniDynamic4  -lpthread

%.o: %.cc
	$(CXX) $(CXXFLAGS) -D_REENTRANT $(INCLUDE) -c $< 


forceCheckObj=CheckedDataCommand.o CheckedInputImpl.o SendDataToQaBase.o \
	      forceCheck.o

all: forceCheck

forceCheck:  $(forceCheckObj)
	$(CXX) $(CXXLINKSO) -o $@ $(forceCheckObj) $(LIB_DIR) $(LIBS)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f forceCheck
	rm -f *.so

