#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif


override DEFINES+= -D_REENTRANT
override INCLUDE+=-I../include 

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -c $< 


OBJFILES:= kvDbGate.o kvStationInfo.o kvAlgorithms.o kvChecks.o kvParam.o \
	   kvModelData.o kvData.o paramlist.o kvapp.o \
	   bitmanip.o kvDbBase.o kvTypes.o kvRejectdecode.o \
	   kvStation.o kvStationParam.o kvQcxInfo.o kvDataFlag.o  \
	   kvTimecontrol.o kvQueries.o kvStationInfoCommand.o kvPath.o \
	   kvObsPgm.o kvTextData.o kvReferenceStation.o kvParamset.o \
	   kvKeyVal.o kvWorkelement.o kvServiceElement.o kvGeneratedTypes.o \
	   kvOperator.o kvDataOperations.o kvTextDataOperations.o kvPsSubscribers.o



LIBFILE=lib$(LIBNAME).a($(OBJFILES))
LIBSRC:=$(OBJFILES:.o=.cc)

all: $(LIBFILE)


depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))









