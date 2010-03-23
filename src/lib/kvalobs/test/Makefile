TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(PUTOOLS)/include         \
        -I$(PUTOOLS)/include/puTools \
        -I$(TOP)/include -I../include\
        $(BOOSTINCLUDE)
        
LIBDIR=-L$(TOP)/lib -L$(PUTOOLS)/lib
LIBS= -lkvalobs_kvalobs -lkvalobs_kvdb -lkvalobs_fileutil -lkvalobs_miutil -lpuTools -ldl -lkvalobs_dnmithread -lboost_thread -lpthread

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


kvdatatest: kvdatatest.o
	$(CXX) $(CXXLINK) -o $@ $< $(LIBDIR) $(LIBS)

kvgatetest: kvDbGateTest.o
	$(CXX) $(CXXLINKSO) -o $@ $< ../obj/kvDbBase.o $(LIBDIR) $(LIBS)	

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f kvdatatest kvgatetest
	rm -f core
	rm -f core.*


#depends:
#	rm -f $(DEPENDSFILE)
#	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)
#
#include $(DEPENDSFILE)
