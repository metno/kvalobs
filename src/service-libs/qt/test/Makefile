TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

DEPENDSFILE=make.depends

#QT stuff
include $(QTDIR)/lib/libqt-mt.prl
MOC:=$(QTDIR)/bin/moc
QTLIB:=-L$(QTDIR)/lib -lqt-mt $(QMAKE_PRL_LIBS)
QTINCLUDE:=-I$(QTDIR)/include
QTDEFINES:=$(patsubst %,-D%,$(QMAKE_PRL_DEFINES))
ALL_Q_OBJECT_H:=$(shell grep -H -l Q_OBJECT *.h)	

#CORBA stuff
CORBA_INCLUDE=$(OMNIINCLUDE) 
CORBA_LIB=$(OMNILIB) -lomniORB4 -lomnithread 


#BOOST stuff
BOOST_INCLUDE=$(BOOSTINCLUDE)
BOOST_LIB=$(BOOSTLIB) -lboost_thread


#KVALOBS stuff
KVLIB:=-L$(TOP)/lib -lkvQt -lmiutil -lkvalobs -ldb -lcorbahelper \
	-ldnmithread -lcorba_skel -lmilog -lmiconfparser

KVINCLUDE:=-I$(TOP)/include/kvskel      \
	   -I$(TOP)/include/miutil      \
	   -I$(TOP)/include/kvalobs     \
	   -I$(TOP)/include/corbahelper \
	   -I$(TOP)/include/db          \
	   -I$(TOP)/include/dnmithread  \
	   -I$(TOP)include/kvservice

Q_OBJECT_H:=$(shell grep -H -l Q_OBJECT *.h)	

GUIOBJFILES:=kvDataView.o  ParamView.o kvQtMainWindow.o  guitestapp.o guitest.o
CONOBJFILES:=kvQtConTest.o DataReceiver.o



#QT: some magic to generate moc targets.
GUI_SRC:=$(GUIOBJFILES:.o=.cc)
GUIQT_Q_OBJECT_H:=$(filter $(GUIOBJFILES:.o=.h), $(Q_OBJECT_H))
GUIQT_OBJ:=$(GUIOBJFILES) $(addprefix moc_, $(GUIQT_Q_OBJECT_H:.h=.o))



CON_SRC:=$(CONOBJFILES:.o=.cc)
CONQT_Q_OBJECT_H:=$(filter $(CONOBJFILES:.o=.h), $(Q_OBJECT_H))
CONQT_OBJ:=$(CONOBJFILES) $(addprefix moc_, $(CONQT_Q_OBJECT_H:.h=.o))


testKvGetParamsOBJFiles:=testKvGetParams.o
testKvGetParams_SRC:=$(testKvGetParamsOBJFiles:.o=.cc)
testKvGetParams_Q_OBJECT_H:=$(filter $(testKvGetParamsOBJFiles:.o=.h), $(Q_OBJECT_H))
testKvGetParams_OBJ:=$(testKvGetParamsOBJFiles) $(addprefix moc_, \
	$(testKvGetParams_Q_OBJECT_H:.h=.o))

testKvGetTypesOBJFiles:=testKvGetTypes.o
testKvGetTypes_SRC:=$(testKvGetTypesOBJFiles:.o=.cc)
testKvGetTypes_Q_OBJECT_H:=$(filter $(testKvGetTypesOBJFiles:.o=.h), $(Q_OBJECT_H))
testKvGetTypes_OBJ:=$(testKvGetTypesOBJFiles) $(addprefix moc_, \
	$(testKvGetTypes_Q_OBJECT_H:.h=.o))


testKvGetOperatorsOBJFiles:=testKvGetOperators.o
testKvGetOperators_SRC:=$(testKvGetOperatorsOBJFiles:.o=.cc)
testKvGetOperators_Q_OBJECT_H:=$(filter $(testKvGetOperatorsBJFiles:.o=.h), $(Q_OBJECT_H))
testKvGetOperators_OBJ:=$(testKvGetOperatorsOBJFiles) $(addprefix moc_, \
	$(testKvGetOperators_Q_OBJECT_H:.h=.o))




testKvGetStationsOBJFiles:=testKvGetStations.o
testKvGetStations_SRC:=$(testKvGetStationsOBJFiles:.o=.cc)
testKvGetStations_Q_OBJECT_H:=$(filter $(testKvGetStationsOBJFiles:.o=.h), $(Q_OBJECT_H))
testKvGetStations_OBJ:=$(testKvGetStationsOBJFiles) $(addprefix moc_, \
	$(testKvGetStations_Q_OBJECT_H:.h=.o))



testKvGetDataOBJFiles:=testKvGetData.o
testKvGetData_SRC:=$(testKvGetDataOBJFiles:.o=.cc)
testKvGetData_Q_OBJECT_H:=$(filter $(testKvGetDataOBJFiles:.o=.h), $(Q_OBJECT_H))
testKvGetData_OBJ:=$(testKvGetDataOBJFiles) $(addprefix moc_, \
	$(testKvGetData_Q_OBJECT_H:.h=.o))


testKvGetModelDataOBJFiles:=testKvGetModelData.o
testKvGetModelData_SRC:=$(testKvGetModelDataOBJFiles:.o=.cc)
testKvGetModelData_Q_OBJECT_H:=$(filter $(testKvGetModelDataOBJFiles:.o=.h), $(Q_OBJECT_H))
testKvGetModelData_OBJ:=$(testKvGetModelDataOBJFiles) $(addprefix moc_, \
	$(testKvGetModelData_Q_OBJECT_H:.h=.o))



INCLUDE:=-I. -I$(TOP)/include -I$(TOP)/include/kvservice/qt  \
	$(QTINCLUDE) $(BOOST_INCLUDE)  $(KVINCLUDE) $(CORBA_INCLUDE)

DEFINES=$(QTDEFINES)

CXXFLAGS += -g $(DEFINES) 


LIBS:= $(KVLIB)  $(BOOST_LIB)  $(CORBA_LIB) $(QTLIB) 


moc_%.cc: %.h
	$(MOC) $< -o $@


%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

guitest: $(GUIQT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(GUIQT_OBJ) $(LIBS)   		


contest: $(CONQT_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(CONQT_OBJ) $(LIBS)   		

testKvGetStations: $(testKvGetStations_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetStations_OBJ) $(LIBS)

testKvGetParams: $(testKvGetParams_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetParams_OBJ) $(LIBS)

testKvGetTypes: $(testKvGetTypes_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetTypes_OBJ) $(LIBS)

testKvGetOperators: $(testKvGetOperators_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetOperators_OBJ) $(LIBS)


testKvGetData: $(testKvGetData_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetData_OBJ) $(LIBS)

testKvGetModelData: $(testKvGetModelData_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetModelData_OBJ) $(LIBS)

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
	rm -f testKvGetTypes
	rm -f $(DEPENDSFILE)

veryclean: clean

