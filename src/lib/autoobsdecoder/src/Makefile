#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override DEFINES= -D_REENTRANT

LIB_DIR=-L$(TOP)/lib $(BOOSTLIB)  
LIBS= -lmiutil -ldecodeutility  

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(CXXSOFLAGS) $(INCLUDE) -c $< 

OBJFILES=convert.o autoobs.o autoobsdecoder.o SplitDef.o NameConvertDef.o \
	 DontDecodeParams.o 

SRCFILES:=$(OBJFILES:.o=.cc)  


all: autoobs

autoobs: $(OBJFILES)   
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -fPIC -o $@.so \
	   $(OBJFILES) $(LIB_DIR) $(LIBS)
	mkdir -p ../lib/decode
	cp $@.so ../lib/decode

clean: veryclean

veryclean:
	rm -f *~
	rm -f *.o
	rm -f testdecodermgr
	rm -f *.so
	rm -rf html latex
	rm -rf lib/decode

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES) -M $(SRCFILES) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))
