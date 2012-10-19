TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(TOP)/include/milog $(BOOSTINCLUDE)
LIBDIR=-L$(TOP)/lib
LIBS= -lkvalobs_kvalobs -lkvalobs_kvdb -lkvalobs_fileutil -lkvalobs_miutil  -ldl -lkvalobs_dnmithread -lboost_thread -lpthread

override DEFINE+= -D_REENTRANT

.PHONY: clean veryclean

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINE) $(INCLUDE) -c $< 


threadtest: threadtest.o
	$(CXX) $(CXXLINK) -o $@ $< $(BOOSTLIB) -L../obj \
	-lmilog -lboost_thread -lpthread


simple: simple.o
	$(CXX) $(CXXLINK) -o $@ $< -L../obj -lmilog -lpthread	


simple1: simple1.o
	$(CXX) $(CXXLINK) -o $@ $< -L../obj -lmilog -lpthread	

simple2: simple2.o
	$(CXX) $(CXXLINK) -o $@ $< -L../obj -lmilog -lpthread	

manager: manager.o
	$(CXX) $(CXXLINK) -o $@ $< -L../obj -lmilog -lpthread

miloghelper: miloghelper.o
	$(CXX) $(CXXLINK) $(INCLUDE) -o $@ $< -L../obj -lmilog -lpthread


clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f simple manager miloghelper threadtest
	rm -f core
	rm -f core.*


#depends:
#	rm -f $(DEPENDSFILE)
#	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)
#
#include $(DEPENDSFILE)

