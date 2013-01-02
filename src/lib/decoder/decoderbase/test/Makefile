TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(TOP)/include $(BOOSTINCLUDE)
LIBDIR=-L$(TOP)/lib $(BOOSTLIB) 
LIBS=-ldecoderbase -lmiutil -lkvdb -lfileutil -ldnmithread -lboost_thread -lboost_date_time\
	-lpthread  -ldl  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


testdecodermgr: testdecodermgr.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBDIR) $(LIBS)



clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f testdecodermgr
	rm -f *.so


#depends:
#	rm -f $(DEPENDSFILE)
#	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)
#
#include $(DEPENDSFILE)
