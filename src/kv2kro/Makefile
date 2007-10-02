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
KVLIB:=-L$(TOP)/lib -lkvcpp -lmiutil -lkvalobs -ldb -lcorbahelper \
	-ldnmithread -lcorba_skel -L$(PUTOOLS)/lib -lpuTools -lmilog -lmiconfparser

KVINCLUDE:=-I$(TOP)/include           \
           -I$(TOP)/include/kvskel    \
           -I$(TOP)/include/kvservice \
		   -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools 

#CONOBJFILES:= DataReceiver.o
#CON_SRC:=$(CONOBJFILES:.o=.cc)
#testKvGetDataReceiver_OBJ:=testKvGetDataReceiver.o
#testKvGetDataReceiver_SRC:=$(testKvGetDataReceiver_OBJ:.o=.cc)
#ALLSRC:= $(CON_SRC) $(testKvGetDataReceiver_SRC)

INCLUDE:=-I. -I$(TOP)/include $(BOOST_INCLUDE) $(KVINCLUDE) $(CORBA_INCLUDE)

CXXFLAGS += -g 

LIBS:= $(KVLIB)  $(BOOST_LIB)  $(CORBA_LIB) -L$(TOP)/lib -lkvcpp 


%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


#testKvGetData: $(testKvGetData_OBJ)
#	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetData_OBJ) $(LIBS)

#testKvGetDataReceiver: $(testKvGetDataReceiver_OBJ)
#	$(CXX) $(CXXFLAGS) -o $@ $(testKvGetDataReceiver_OBJ) $(LIBS)


DataReceiverSimpleKro: DataReceiverSimpleKro.o
	$(CXX) $(CXXFLAGS) -o $@ DataReceiverSimpleKro.o  $(LIBS)

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M  $(CON_SRC) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))

clean:
	rm -rf moc_*.*
	rm -rf *.o *~
	rm -f *.a
	rm -f core
	rm -f core.*
	rm -f DataReceiverSimpleKro #testKvGetData testKvGetDataReceiver 
	rm -f $(DEPENDSFILE)

veryclean: clean


