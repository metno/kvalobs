TOP=../../..
include $(TOP)/conf/make.$(OSTYPE)

	
INC:=$(TOP)/include
	
INCLUDE=-I$(INC)/miutil \
	    -I$(INC)/dnmithread\
	    -I$(INC)/kvskel\
	    -I$(INC)/kvalobs\
	    -I$(INC)/db\
	    -I$(INC)/corbahelper\
	    $(BOOSTINCLUDE) $(OMNIINCLUDE)	
	
	
LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB)
LIBS=-lkvalobs -lkvdb  -lmilog -lmiconfparser -lmiutil  -lcorbahelper \
     -ldnmithread  -lboost_thread -lcorba_skel  -lomniORB4 -lomnithread \
     -lomniDynamic4  -lpthread
	
%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

kvDataInput_clt:  kvinputd_clt.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIB_DIR) $(LIBS)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f kvinputd_clt
	rm -f *.so

