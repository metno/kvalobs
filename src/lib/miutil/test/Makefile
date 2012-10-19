TOP=../../../..
include $(TOP)/conf/make.linux-gnu

INCLUDE=-I../include -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools
LIBDIR=-L../obj
LIBS=-lmiutil -L$(PUTOOLS)/lib -lpuTools

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

all: testcommastring testTimeParser

testcommastring: testcommastring.o
	$(CXX) $(CXXLDFLAGS) -o $@ $< $(LIBDIR) $(LIBS)
	
testTimeParser: testTimeParser.o
	$(CXX) $(CXXLDFLAGS) -o $@ $< $(LIBDIR) $(LIBS)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f testisnumber testcommastring

#depends:
#	rm -f $(DEPENDSFILE)
#	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)
#
#include $(DEPENDSFILE)
