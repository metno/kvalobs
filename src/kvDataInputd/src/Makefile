#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override DEFINES+= -D_REENTRANT

LIBXML_INCLUDE:=$(shell xml2-config --cflags)
LIBXML_LIB_TMP:=$(shell xml2-config --libs)
LIBXML_LIB:=$(filter-out -lpthread, $(LIBXML_LIB_TMP))

LIB_DIR=-L$(TOP)/lib $(BOOSTLIB) $(OMNILIB) 
LIBS=-ldecoderbase -ldecodeutility $(LIBXML_LIB) -lmiutil  -lkvalobs -ldecodeutility \
	 $(LIBXMLPPLIB) -lkvdb -lfileutil -lmiconfparser 	-ldnmithread  \
	-lcorbahelper -lmilog -L$(PUTOOLS)/lib -lpuTools -lboost_thread \
	-lcorba_skel -lomniORB4 -lomnithread -lomniDynamic4 -ldl -lpthread $(LIBXMLPPLIB)

LIBS_CLT= -Wl,-Bstatic $(LIB_DIR) -lmiutil -lkvalobs -lmiconfparser -ldnmithread \
           -lcorbahelper -lmilog -L$(PUTOOLS)/lib -lpuTools -lboost_thread \
           -lcorba_skel -lomniORB4 -lomnithread -lomniDynamic4 \
	       -Wl,-Bdynamic -ldl -lpthread

#Objfiles for kvDataInputd 
KVDATA:=InitLogger.o DataSrcImpl.o kvDataInputd.o DecodeCommand.o \
	    DataSrcApp.o ConnectionCache.o  force.o

OBJFILES:=$(KVDATA)

SRCFILES:=$(OBJFILES:.o=.cc)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -c $< 



all: prepare kvDataInputd kvDataInputd_clt

prepare:
	mkdir -p ../bin

kvDataInputd: $(KVDATA)
	$(CXX) $(CXXLINKSO) -o $@ $(KVDATA) $(LIB_DIR) $(LIBS)
	cp ../obj/kvDataInputd ../bin/kvDataInputd

kvDataInputd_clt:  kvinputd_clt.o
	$(CXX) $(CXXLDFLAGS) -o $@ $<  $(LIBS_CLT)
	cp ../obj/kvDataInputd_clt ../bin


depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(DEFINES)  -M $(SRCFILES) >> $(DEPENDSFILE)


include $(wildcard $(DEPENDSFILE))
