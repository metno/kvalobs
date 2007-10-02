TOP=../../..
include $(TOP)/conf/make.$(OSTYPE)

INC:=$(TOP)/include

INCLUDE=-I$(INC) \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    -I$(INC)/kvskel\
	     $(BOOSTINCLUDE) $(OMNIINCLUDE)


LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB) -L$(PUTOOLS)/lib
LIBS=-lkvalobs -ldb  -lmilog -lmiconfparser -lmiutil  -lcorbahelper \
     -ldnmithread  -lboost_thread -lcorba_skel -lpuTools -lomniORB4 -lomnithread \
     -lomniDynamic4  -lpthread -lfileutil -ldl

QABASESIM=qabasesim.o QaWorkThread.o qabaseApp.o qabaseInputImpl.o \
	  QaWorkCommand.o

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


all: kvManagerd_clt qabasesim


MissingObsCheckTest: clean MissingObsCheckTest.o
	$(CXX) $(CXXLINKSO) -o $@ MissingObsCheckTest.o  \
	        ../GenCacheElem.o ../MissingObsCheck.o \
	        $(LIB_DIR) $(LIBS)

kvManagerd_clt:  kvManager_clt.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIB_DIR) $(LIBS)

kvDataInput_clt:  kvDataInput_clt.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIB_DIR) $(LIBS)

qabasesim: $(QABASESIM)
	$(CXX) $(CXXLDFLAGS) -o $@ $(QABASESIM) \
	       $(LIB_DIR) $(LIBS)

testfdate: testfdate.o
	$(CXX) $(CXXLDFLAGS) -o $@  $<\
	       $(LIB_DIR) $(LIBS)

updatefdate: updatefdate.o
	$(CXX) $(CXXLDFLAGS) -o $@  $<\
	       $(LIB_DIR) $(LIBS)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f  qabasesim kvManagerd_clt testDataNotifySubscriber 
	rm -f kvDataInput_clt
	rm -f *.so

