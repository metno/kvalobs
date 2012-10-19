TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(TOP)/include
LIBDIR=-L$(TOP)/lib
LIBS=-lkvalobs_fileutil 

PHONY: clean veryclean

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

all: testdir testmkdir

testdir: testdir.o
	$(CXX) $(CXXLDFLAGS) -o $@ $< $(LIBDIR) $(LIBS)

testmkdir: clean testmkdir.o
	$(CXX) $(CXXLDFLAGS) -o $@ testmkdir.o $(LIBDIR) $(LIBS)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f testdir testmkdir
	rm -rf my
	rm -f core
	rm -f core.*
