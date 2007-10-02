# includefile contains Compiler definition
TOP=../..

PROJECT=kvServiced 

TEST_BIN=

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/single.mk

-include $(DEPENDSFILE)


DEFINES +=  -D_REENTRANT


ALLOBJ= kvServiced.o  ServiceApp.o DataReadyInputImpl.o ObjReaper.o \
	ReaperBase.o ServiceSubscriber.o InitLogger.o \
	SubscriberData.o  kvDataSubscriberInfo.o DataCommand.o \
	DataCommandExt.o DataNotifyCommandExt.o \
	DataNotifyCommand.o kvServiceImpl.o kvModelDataIteratorImpl.o \
	kvDataIteratorImpl.o kvRejectedIteratorImpl.o kvSubscriberBase.o \
	DataReadyCommand.o kvPsSubscriber.o PsSubscriberDbHelper.o

#	kvHintSubscriber.o kvDataNotifySubscriber.o kvDataSubscriber.o \

ALLSRC= $(ALLOBJ:.o=.cc) SubscriberThread.tcc

INCLUDE=-I$(TOP)/include/            \
	    -I$(TOP)/include/kvskel      \
	    -I$(PUTOOLS)/include/ -I$(PUTOOLS)/include/puTools \
	    $(OMNIINCLUDE)               \
	    $(BOOSTINCLUDE)              

LINKS= -L$(TOP)/lib $(OMNILIB) $(BOOSTLIB) -L$(PUTOOLS)/lib -lkvalobs  \
	-lmiutil -ldb -lfileutil -lmiconfparser -lcorbahelper -ldnmithread \
	-lcorba_skel -lpuTools\
	-lboost_thread 	-lmilog -lomniORB4 -lomnithread  -lboost_regex \
	-lpthread -lm  -ldl

# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $(DEFINES) $< $(INCLUDE)


kvServiced: $(ALLOBJ)
	$(CXX)  $(CXXLINKSO) -o $@  $(ALLOBJ) $(LINKS)


sbtest:  sbtest.o SubscriberThread.o
	$(CXX)  $(CXXLINKSO) -o $@ sbtest.o  SubscriberThread.o  $(LINKS)

clean: 	
	rm -f *.o
	rm -f kvServiced
	rm -f *~
	rm -f $(DEPENDSFILE)
	rm -f core core.*
	rm -rf bin
	if cd test > /dev/null 2>&1 ; then $(MAKE) clean ; fi
	rm -f sbtest

veryclean: clean
	if cd test > /dev/null 2>&1 ; then $(MAKE) veryclean ; fi
