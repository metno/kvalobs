TOP=../../..
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


BIN_FILES=DataReceiver

DataReceiver_OBJ:=DataReceiver.o DataReceiverApp.o


DataReceiver_SRC:=$(DataReceiver_OBJ:.o=.cc)

ALLSRC:=$(DataReceiver_SRC)

INCLUDE:=-I. -I$(TOP)/include $(BOOST_INCLUDE) $(KVINCLUDE) $(CORBA_INCLUDE)

CXXFLAGS += -g 

LIBS:= $(KVLIB)  $(BOOST_LIB)  $(CORBA_LIB) -L$(TOP)/lib -lkvcpp 


%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


DataReceiver: $(DataReceiver_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(DataReceiver_OBJ)  $(LIBS)
	mkdir -p data

getdata: getdata.o
	$(CXX) $(CXXFLAGS) -o $@ $<  $(LIBS)

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(ALLSRC) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))

clean:
	rm -rf *.o *~
	rm -f *.a
	rm -f core
	rm -f core.*
	rm -f $(BIN_FILES)
	rm -f $(DEPENDSFILE)

veryclean: clean
	rm -rf data/	
