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
KVLIB:=-L$(TOP)/lib -lkvcpp2 -lmiutil -lkvalobs -lkvdb -lcorbahelper \
	   -ldnmithread -lcorba_skel -L$(PUTOOLS)/lib -lpuTools -lmilog -lmiconfparser

KVINCLUDE:=-I$(TOP)/include           \
           -I$(TOP)/include/kvskel    \
           -I$(TOP)/include/kvservice \
		   -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools 


INCLUDE:=-I. -I$(TOP)/include $(BOOST_INCLUDE) $(KVINCLUDE) $(CORBA_INCLUDE)

CXXFLAGS += -g 

LIBS:= $(KVLIB)  $(BOOST_LIB)  $(CORBA_LIB) -L$(TOP)/lib -lkvcpp2 -lfileutil 


kv2kro_OBJ = GetData.o App.o DataReceiver.o kv2kro.o
kv2kro_SRC:=$(kv2kro_OBJ:.o=.cc)
ALLSRC:= $(kv2kro_SRC) 

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

kv2kro: $(kv2kro_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(kv2kro_OBJ) $(LIBS)

DataReceiverSimpleKro: DataReceiverSimpleKro.o
	$(CXX) $(CXXFLAGS) -o $@ DataReceiverSimpleKro.o  $(LIBS)

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M  $(CON_SRC) >> $(DEPENDSFILE)

-include $(DEPENDSFILE)

clean:
	rm -rf moc_*.*
	rm -rf *.o *~
	rm -f *.a
	rm -f core
	rm -f core.*
	rm -f DataReceiverSimpleKro #testKvGetData testKvGetDataReceiver 
	rm -f $(DEPENDSFILE)

veryclean: clean
