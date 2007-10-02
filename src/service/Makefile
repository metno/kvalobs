# includefile contains Compiler definition
TOP=../..

PROJECT=kvServiced 

TEST_BIN=

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk


ifeq ($(wildcard $(DEPENDSFILE)),$(DEPENDSFILE))
include $(wildcard $(DEPENDSFILE))
endif

DEFINES +=  -D_REENTRANT


ALLOBJ= kvServiced.o  ServiceApp.o DataReadyInputImpl.o ObjReaper.o \
	ReaperBase.o ServiceSubscriber.o InitLogger.o \
	kvSubscriberCollection.o kvDataSubscriber.o kvDataSubscriberInfo.o \
	kvServiceImpl.o kvModelDataIteratorImpl.o kvDataIteratorImpl.o \
	kvRejectedIteratorImpl.o kvSubscriberBase.o kvDataNotifySubscriber.o \
	DataReadyCommand.o SubscriberThread.o
ALLSRC= $(ALLOBJ:.o=.cc) 

INCLUDE=-I$(TOP)/include/            \
	    -I$(TOP)/include/kvskel      \
	    -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(OMNIINCLUDE)               \
	    $(BOOSTINCLUDE)              \

LINKS= -L$(TOP)/lib $(OMNILIB) $(BOOSTLIB) -L$(PUTOOLS)/lib -lkvalobs  \
	-lmiutil -ldb -lfileutil -lmiconfparser -lcorbahelper -ldnmithread \
	-lcorba_skel -lpuTools \
	-lboost_thread 	-lmilog -lomniORB4 -lomnithread  -lboost_regex \
	-lpthread -lm  -ldl

# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $(DEFINES) $< $(INCLUDE)


kvServiced: $(ALLOBJ)
	$(CXX)  $(CXXLINKSO) -o $@ -static-libgcc $(ALLOBJ) $(LINKS)

clean: 	
	rm -f *.o
	rm -f kvServiced
	rm -f $(DEPENDSFILE)
	rm -f core core.*
	rm -rf bin
	cd test; $(MAKE) clean

veryclean: clean
	cd test; $(MAKE) veryclean
