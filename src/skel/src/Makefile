TOP=../../..
include $(TOP)/conf/make.$(OSTYPE)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -I.. -c $< 

MYOBJ=adminInterface.o

MYLIB=../$(LIBNAME)($(MYOBJ))

libs: $(MYLIB) 

clean:
	rm -f *.o *~
