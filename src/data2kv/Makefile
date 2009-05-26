# includefile contains Compiler definition
HTOP:=$(shell pwd)
TOP:=$(HTOP)/../..

include $(TOP)/config.mk

LIBS=-L$(TOP)/lib

DEPENDSFILE=make.depends

EXECUTEABLE=autoobs2kv signalnewfile2kv
DEFINES +=  -D_REENTRANT


OBJ:=autoobs2kv.o App.o mtcout.o corbaApp.o CollectData.o newfileImpl.o \
     kvDataSrcList.o NewData.o AdminImpl.o File.o

SIGNALNEWFILE_OBJ:= signalNewFileApp.o signalnewfile.o corbaApp.o mtcout.o

ALL_OBJ:=$(OBJ) $(SIGNALNEWFILE_OBJ)
SRC:=$(ALL_OBJ:.o=.cc) 



# From .cc to .o
#.cc.o:
#	$(CXX) -c $(CXXFLAGS) $(DEFINES) $< $(INCLUDE)

all: autoobs2kv signalnewfile2kv


autoobs2kv: $(OBJ)
	$(CXXLD) $(LDFLAGS) -o $@ $(OBJ)  -lautoobs -laotool  \
	-Wl,-Bstatic -lmiconfparser -lmilog -Wl,-Bdynamic \
	 -lskelPoa $(CORBA_LIBS) $(STDLIB) $(ENDLIB)
	mkdir -p $(TOP)/bin
	cp $@ $(TOP)/bin

signalnewfile2kv: $(SIGNALNEWFILE_OBJ)
	$(CXXLD) $(LDFLAGS) -o $@ $(SIGNALNEWFILE_OBJ)  \
	-Wl,-Bstatic -lmiconfparser -lmilog -Wl,-Bdynamic \
	-laotool -lskelPoa $(CORBA_LIBS) $(STDLIB) $(ENDLIB)
	mkdir -p $(TOP)/bin
	cp $@ $(TOP)/bin


depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE)  -M $(SRC) \
	  >> $(DEPENDSFILE)


clean: 	
	rm -f *.o
	rm -f *~
	rm -f autoobs2kv signalnewfile2kv
	rm -f $(DEPENDSFILE)
	rm -f core core.*

veryclean: clean

include $(wildcard $(DEPENDSFILE))


