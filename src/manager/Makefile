# includefile contains Compiler definition
TOP=../..

PROJECT=kvManagerd

TEST_BIN=

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk

include $(wildcard $(DEPENDSFILE))


override DEFINES +=  -D_REENTRANT



KVMANAGER_OBJ= mgrApp.o kvManagerd.o managerInputImpl.o NewDataCommand.o \
	       checkedInputImpl.o PreProcessJob.o NewDataJob.o OneTimeJob.o\
	       PreProcessWorker.o InitLogger.o PreProcessMissingData.o \
	       kvCheckedDataThread.o CheckForMissingObsMessages.o MissingObsCheck.o \
	       AdminImpl.o SelectDataToProcess.o SendDataToQa.o \
	       CheckedDataHelper.o CheckedDataCommand.o GenCacheElem.o \
	       ServiceCheckedInputImpl.o ServiceDataCommand.o

KVMANAGER_SRC=$(KVMANAGER_OBJ:.o=.cc) 

ALLSRC= $(KVMANAGER_SRC)


INCLUDE=-I$(TOP)/include              \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    -I$(TOP)/include/kvskel       \
	    $(BOOSTINCLUDE)\
	    $(OMNIINCLUDE) 


LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB) -L$(PUTOOLS)/lib

LIBS=-lkvalobs -ldb -lfileutil -lmiutil -lcorbahelper \
     -lmilog -lmiconfparser\
     -ldnmithread -lpuTools -lboost_thread -lcorba_skel -lomniORB4 -lomnithread \
     -lomniDynamic4 -ldl -lpthread



# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $(DEFINES) $< $(INCLUDE)


kvManagerd: $(KVMANAGER_OBJ)
	$(CXX)  $(CXXLINKSO) -o $@ $(KVMANAGER_OBJ) $(LIB_DIR) $(LIBS)

kvmgrclt: kvmgrclt.o kvmgrcltApp.o
	$(CXX)  $(CXXLINKSO) -o $@  kvmgrclt.o kvmgrcltApp.o $(LIB_DIR) $(LIBS)
	
earlytest: earlytest.o
	$(CXX)  $(CXXLINKSO) -o $@  $<  $(LIB_DIR) $(LIBS)

clean: 	
	rm -f *.o
	rm -f kvManagerd
	rm -f $(DEPENDSFILE)
	rm -f core core.*
	rm -rf bin

veryclean: clean

