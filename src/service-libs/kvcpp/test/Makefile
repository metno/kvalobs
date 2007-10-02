TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

DEPENDSFILE=make.depends

#CORBA stuff
CORBA_INCLUDE=$(OMNIINCLUDE) 
CORBA_LIB=$(OMNILIB) -lomniORB4 -lomnithread 


#BOOST stuff
BOOST_INCLUDE=$(BOOSTINCLUDE)
BOOST_LIB=$(BOOSTLIB) -lboost_thread


#KVALOBS stuff
KVLIB:=-L$(TOP)/lib -lkvcpp -lmiutil -lkvalobs -ldb -lcorbahelper \
	-ldnmithread -lcorba_skel -lmilog -lmiconfparser

KVINCLUDE:=-I$(TOP)/include/kvskel      \
	   -I$(TOP)/include/miutil      \
	   -I$(TOP)/include/kvalobs     \
	   -I$(TOP)/include/corbahelper \
	   -I$(TOP)/include/db          \
	   -I$(TOP)/include/dnmithread  \
	   -I$(TOP)/include/kvservice/kvcpp


GUIOBJFILES:=kvDataView.o  ParamView.o kvQtMainWindow.o  guitestapp.o guitest.o
CONOBJFILES:=kvConTest.o DataReceiver.o



GUI_SRC:=$(GUIOBJFILES:.o=.cc)
CON_SRC:=$(CONOBJFILES:.o=.cc)

testKvGetParams_OBJ:=testKvGetParams.o
testKvGetParams_SRC:=$(testKvGetParams_OBJ:.o=.cc)

testKvGetTypes_OBJ:=testKvGetTypes.o
testKvGetTypes_SRC:=$(testKvGetTypes_OBJ:.o=.cc)

testKvGetStations_OBJ:=testKvGetStations.o
testKvGetStations_SRC:=$(testKvGetStations_OBJ:.o=.cc)

testKvGetData_OBJ:=testKvGetData.o
testKvGetData_SRC:=$(testKvGetData_OBJ:.o=.cc)

testKvGetDataReceiver_OBJ:=testKvGetDataReceiver.o
testKvGetDataReceiver_SRC:=$(testKvGetDataReceiver_OBJ:.o=.cc)

testKvGetModelData_OBJ:=testKvGetModelData.o
testKvGetModelData_SRC:=$(testKvGetModelData_OBJ:.o=.cc)

kv2kvTest_OBJ:=kv2kvTest.o
kv2kvTest_SRC:=$(kv2kvTest_OBJ:.o=.cc)


ALLSRC:=$(testKvGetParams_SRC) $(testKvGetTypes_SRC) $(testKvGetStations_SRC) \
        $(testKvGetData_SRC) $(testKvGetModelData_SRC) $(CON_SRC) \
	$(testKvGetDataReceiver_SRC) $(kv2kvTest_SRC)

INCLUDE:=-I. -I$(TOP)/include $(BOOST_INCLUDE) $(KVINCLUDE) $(CORBA_INCLUDE)

CXXFLAGS += -g 

LIBS:= $(KVLIB)  $(BOOST_LIB)  $(CORBA_LIB) -L$(TOP)/lib -lkvcpp -ldecodeutility -lmiutil
# /kvalobs/include/decodeutility/

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

%: %.o
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIBS)

tests: testKvGetStations testKvGetParams testKvGetTypes testKvGetData \
      testKvGetModelData kv2kvTest

testKvGetStations: $(testKvGetStations_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetStations_OBJ) $(LIBS)

testKvGetParams: $(testKvGetParams_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetParams_OBJ) $(LIBS)

testKvGetTypes: $(testKvGetTypes_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetTypes_OBJ) $(LIBS)

testKvGetData: $(testKvGetData_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetData_OBJ) $(LIBS)

testKvGetDataReceiver: $(testKvGetDataReceiver_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetDataReceiver_OBJ) $(LIBS)

testKvGetModelData: $(testKvGetModelData_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetModelData_OBJ) $(LIBS)

guitest: $(GUIQT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(GUIQT_OBJ) $(LIBS)   		

contest: $(CONOBJFILES)
	$(CXX) $(CXXFLAGS) -o $@ $(CONOBJFILES) $(LIBS)   		

DataReceiverSimple: DataReceiverSimple.o
	$(CXX) $(CXXFLAGS) -o $@ DataReceiverSimple.o  $(LIBS)


kv2kvTest: kv2kvTest.o
	$(CXX) $(CXXFLAGS) -o $@ kv2kvTest.o  $(LIBS)

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(GUI_SRC) $(CON_SRC) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))

clean:
	rm -rf moc_*.*
	rm -rf *.o *~
	rm -f *.a
	rm -f core
	rm -f core.*
	rm -f contest guitest testKvGetData testKvGetParams testKvGetStations 
	rm -f testKvGetTypes testKvGetModelData DataReceiverSimple
	rm -f DataReceiverSimple testKvGetData testKvGetDataReceiver 
	rm -f testKvGetModelData testKvGetParams testKvGetStations
	rm -f testKvGetTypes getKvRejectdecode
	rm -f kv2kvTest
	rm -f $(DEPENDSFILE)

veryclean: clean

