
CXXFLAGS=$(shell pkg-config --cflags libkvcpp)
LIBS=$(shell pkg-config --libs libkvcpp)
KVLIBDIR=$(shell kvconfig --pkglibdir)
LIBDIRS=$(shell pkg-config --libs-only-L libkvcpp)
LDRPATH=-Wl,-rpath=
RPATH=$(subst -L, $(LDRPATH),$(LIBDIRS)) 

DBDRIVERDIR=$(KVLIBDIR)/db
DECODEDIR=$(KVLIBDIR)/decode


KVDEFINES=-DDBDRIVERDIR="\""$(DBDRIVERDIR)"\"" \
          -DDECODEDIR="\""$(DECODEDIR)"\""

#INCLUDE=-I$(PUTOOLS)/include         \
#        -I$(PUTOOLS)/include/puTools \
#        -I$(TOP)/include -I../include\
#       $(BOOSTINCLUDE)
        

#LIBS += -lkvalobs_kvalobs -lkvalobs_kvdb -lkvalobs_fileutil -lkvalobs_miutil -lpuTools -ldl -lkvalobs_dnmithread -lboost_thread -lpthread

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(KVDEFINES) -c $< 

all: kvdatatest kvgatetest datainsertTPG

kvdatatest: kvdatatest.o
	$(CXX) $(CXXLINK) -o $@ $< $(LIBS) $(RPATH)

kvgatetest: kvDbGateTest.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBS) $(RPATH)
	
datainsertTPG: datainsertTPG.o 
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBS) $(RPATH)
	
datainsertTSqlite3: datainsertTSqlite3.o 
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBS) $(RPATH)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f kvdatatest kvgatetest
	rm -f core
	rm -f core.*

