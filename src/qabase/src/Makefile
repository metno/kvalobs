#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

#LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB) $(PERLLIB)
#LIBS=-ldnmithread -lkvalobs -lkvdb -lfileutil -lmiutil -ldl -lcorbahelper \
#     -lboost_thread -lcorba_skel -lomniORB4 -lomnithread \
#     -lomniDynamic4 -lpthread \
#     -rdynamic -lperl -lnsl -ldl -lm -lc -lcrypt -lutil

LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB)  -L$(PUTOOLS)/lib

LIBS=-lmilog -ldnmithread -lkvalobs -lkvdb -lfileutil -lmiutil -ldl \
     -lmiconfparser -lcorbahelper -lpuTools -lboost_thread -lcorba_skel \
     -lomniORB4 -lomnithread -lomniDynamic4 -lpthread \
     $(PERLLIBS)



#Objfiles for kvQabased
KVQABASE:= QaWorkCommand.o QaWorkThread.o qabaseApp.o kvQabased.o qabaseInputImpl.o kvQABaseDBConnection.o kvQABaseMetadata.o kvQABaseMeteodata.o kvQABaseScriptManager.o kvQABaseControl.o kvMetadataTable.o kvPerlParser.o InitLogger.o kvCronString.o AdminImpl.o

OBJFILES:=$(KVQABASE)

SRCFILES:=$(OBJFILES:.o=.cc)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -DMILOG_REENTRANT -c $< 

all: kvQabased 


kvQabased: $(KVQABASE)
	$(CXX) $(CXXLINKSO) -o $@ $(KVQABASE) $(LIB_DIR) $(LIBS)
	cp $@ ../bin

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -DMILOG_REENTRANT -M $(SRCFILES) \
	  >> $(DEPENDSFILE)


include $(DEPENDSFILE)
