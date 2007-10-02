#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override DEFINES+= -D_REENTRANT

LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB)
LIBS=-ldnmithread -lkvalobs -ldb -lfileutil -lmiutil -lcorbahelper \
     -lmilog -lmiconfparser\
     -lboost_thread -lcorba_skel -lomniORB4 -lomnithread \
     -lomniDynamic4 -ldl -lpthread



#Objfiles for kvManagerd  
KVMANAGER:= mgrApp.o kvManagerd.o managerInputImpl.o NewDataCommand.o \
	    checkedInputImpl.o PreProcessJob.o NewDataJob.o OneTimeJob.o\
	    PreProcessWorker.o InitLogger.o PreProcessMissingData.o \
	    kvCheckedDataThread.o CheckForMissingObsMessages.o \
	    AdminImpl.o

#WorkQue1.o

OBJFILES:=$(KVMANAGER)

SRCFILES:=$(OBJFILES:.o=.cc)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -c $< 



all: kvManagerd
	cp kvManagerd $(TOP)/bin


kvManagerd: $(KVMANAGER)
	$(CXX) $(CXXLINKSO) -o $@ $(KVMANAGER) $(LIB_DIR) $(LIBS)
	mkdir -p ../bin
	cp ../obj/kvManagerd ../bin/kvManagerd


.PHONY: depends clean veryclean

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -M $(SRCFILES) >> $(DEPENDSFILE)

clean:
	rm -rf ../bin
	rm -rf ../obj
	rm -rf ../lib
	cd ../test; $(MAKE) clean

veryclean: clean

include $(wildcard $(DEPENDSFILE))
