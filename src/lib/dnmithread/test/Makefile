TOP=../../../..
include $(TOP)/conf/make.linux-gnu

INCLUDE=-I../include 
LIBDIR=-L../lib
LIBS=-ldnmithread

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f testisnumber

#depends:
#	rm -f $(DEPENDSFILE)
#	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)
#
#include $(DEPENDSFILE)
