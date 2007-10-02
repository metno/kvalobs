TOP=../..
include $(TOP)/conf/make.$(OSTYPE)

DEPENDSFILE=make.depends

#CORBA stuff
CORBA_INCLUDE=$(OMNIINCLUDE) 
CORBA_LIB=$(OMNILIB) -lomniORB4 -lomnithread 


#BOOST stuff
BOOST_INCLUDE=$(BOOSTINCLUDE)
BOOST_LIB=$(BOOSTLIB) -lboost_thread


#KVALOBS stuff
KVLIB:=-L$(TOP)/lib -lkvcpp -lmiutil -lkvalobs -ldb -lfileutil -ldecodeutility\
	-lcorbahelper -ldnmithread -lcorba_skel -lmilog -lmiconfparser 

KVINCLUDE:=-I$(TOP)/include           \
	       -I$(TOP)/include/kvservice \
    	   -I$(TOP)/include/kvskel    \
		   -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools


KVSYNOPD_OBJ:=kvsynopd.o App.o DataReceiver.o Data.o SynopWorker.o SynopData.o\
	      DataList.o StationInfo.o StationInfoParse.o tblSynop.o\
	      kvsynopdImpl.o Replay.o synop.o ValidData.o tblWaiting.o \
	      Waiting.o delaycontrol.o tblKeyVal.o InitLogger.o \
	      getDataReceiver.o GetDataThread.o

KVSYNOPD_CLT_OBJ:=kvsynopclt.o kvsynopCltSynopcbImp.o kvsynopCltApp.o \
	          kvsynopCorbaThread.o

KVSYNOPD_SRC:=$(KVSYNOPD_OBJ:.o=.cc) 
KVSYNOPD_CLT_SRC:=$(KVSYNOPD_CLT_OBJ:.o=.cc)

KVSYNOPDBCLT_OBJ:=kvsynopdbclt.o tblSynop.o
KVSYNOPDBCLT_SRC:=$(KVSYNOPDBCLT_OBJ:.o=.cc)

ALLSRC:=$(KVSYNOPD_SRC) $(KVSYNOPD_CLT_SRC) $(KVSYNOPDBCLT_SRC) kvsynopdparse.cc

INCLUDE:=-I. -I$(TOP)/include $(BOOST_INCLUDE) $(KVINCLUDE) $(CORBA_INCLUDE)

CXXFLAGS += -g 

LIBS:= $(KVLIB)  -L$(PUTOOLS)/lib -lpuTools $(BOOST_LIB)  $(CORBA_LIB)  -ldl


%.o: %.cc
	$(CXX) $(CXXFLAGS) -Wall $(INCLUDE) -c $< 

all: kvsynopd kvsynopclt kvsynopdparse kvsynopdbclt

kvsynopd: $(KVSYNOPD_OBJ)
	$(CXX) $(CXXLINKSO) -o $@ $(KVSYNOPD_OBJ) $(LIBS)	
	cp $@ $(TOP)/bin

testarrayop: testarrayop.o SynopData.o
	$(CXX) $(CXXLINKSO) -o $@ testarrayop.o SynopData.o $(LIBS)

kvsynopdparse: kvsynopdparse.o StationInfo.o StationInfoParse.o
	$(CXX) $(CXXLINK) -o $@ kvsynopdparse.o StationInfo.o \
	       StationInfoParse.o $(LIBS)	
	cp $@ $(TOP)/bin

kvsynopdbclt: $(KVSYNOPDBCLT_OBJ)
	$(CXX) $(CXXLINKSO) -o $@ $(KVSYNOPDBCLT_OBJ)  $(LIBS)	
	cp $@ $(TOP)/bin

kvsynopclt: $(KVSYNOPD_CLT_OBJ)
	$(CXX) $(CXXLINK) -o $@ $(KVSYNOPD_CLT_OBJ)  $(LIBS)	
	cp $@ $(TOP)/bin


depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(ALLSRC) >> $(DEPENDSFILE)

-include $(DEPENDSFILE)

clean:
	rm -rf *.o *~
	rm -f *.a
	rm -f core
	rm -f core.*
	rm -f kvsynopd testarrayop kvsynopdparse kvsynopclt kvsynopdbclt
	(cd doc; rm -rf html latex *~)
	rm -f $(DEPENDSFILE)
	rm -rf bin
	rm -rf share

veryclean: clean

