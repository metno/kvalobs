TOP=../../../..
include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(TOP)/include/db\
	-I$(TOP)/include/fileutil

LIBDIR=-L$(TOP)/lib
LIBS=-lkvdb -lfileutil -ldl

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


drivermgr_test: drivermgr_test.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBDIR) $(LIBS)

pgtest: pgtest.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBDIR) $(LIBS)

sqlitetest: sqlitetest.o
	$(CXX) $(CXXLINKSO) -o $@ $< $(LIBDIR) $(LIBS)

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f drivermgr_test pgtest sqlitetest
	rm -f core
	rm -f core.*


#depends:
#	rm -f $(DEPENDSFILE)
#	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
#	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)
#
#include $(DEPENDSFILE)
