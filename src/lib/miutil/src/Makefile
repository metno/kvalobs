#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override INCLUDE+=-I../include 

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

#OBJFILES:=miString.o miClock.o miDate.o miTime.o commastring.o trimstr.o \
#	  cmprspace.o miTimeParse.o

OBJFILES:=commastring.o trimstr.o cmprspace.o miTimeParse.o
	  
#HEADERFILES=miString miString.h miClock miClock.h miDate miDate.h miTime\
#	   miTime.h puAlgo.h commastring.h trimstr.h cmprspace.h miTimeParse.h

HEADERFILES=commastring.h trimstr.h cmprspace.h miTimeParse.h
       
LIBFILE=lib$(LIBNAME).a($(OBJFILES))
LIBSRC:=$(OBJFILES:.o=.cc)

all: $(LIBFILE)

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)

-include $(DEPENDSFILE)
