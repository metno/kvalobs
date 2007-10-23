TOP=../../..

include $(TOP)/conf/make.$(OSTYPE)

DEPENDSFILE=make.depends
LIBNAME=kvQt

#QT stuff
include $(QTDIR)/lib/libqt-mt.prl
MOC:=$(QTDIR)/bin/moc
QTLIB:=-L$(QTDIR)/lib -lqt-mt $(QMAKE_PRL_LIBS)
QTINCLUDE:=-I$(QTDIR)/include
QTDEFINES:=$(patsubst %,-D%,$(QMAKE_PRL_DEFINES))
ALL_Q_OBJECT_H:=$(shell grep -H -l Q_OBJECT *.h)	

#CORBA stuff
CORBA_INCLUDE=$(OMNIINCLUDE) 
CORBA_LIB=$(OMNILIB) -lomniORB4 -lomnithread 


#BOOST stuff
BOOST_INCLUDE=$(BOOSTINCLUDE)
BOOST_LIB=$(BOOSTLIB) -lboost_thread


#KVALOBS stuff
KVLIB:=-L$(TOP)/lib -lmiutil -lkvalobs -lkvdb -lcorbahelper \
	-ldnmithread  -lcorba_skel 

KVINCLUDE:=-I$(TOP)/include    \
           -I$(TOP)/include/kvskel \

Q_OBJECT_H:=$(shell grep -H -l Q_OBJECT *.h)	

OBJFILES:=kvWhat.o  kvDataNotifySubscriberImpl.o        \
	  kvDataSubscriberImpl.o kvHintSubscriberImpl.o \
	  kvQtApp.o kvCorbaThread.o kvQtCorbaApp.o      \
	  kvDataSubscribeInfoHelper.o WhichDataHelper.o
#kvNewDataThread.o

INSTALL_HEADERFILES=kvWhat.h kvQtApp.h kvservicetypes.h \
	    kvDataSubscribeInfoHelper.h WhichDataHelper.h	

#QT: some magic to generate moc targets.
KVQT_SRC:=$(OBJFILES:.o=.cc)
KVQT_Q_OBJECT_H:=$(filter $(OBJFILES:.o=.h), $(Q_OBJECT_H))
KVQT_OBJ:=$(OBJFILES) $(addprefix moc_, $(KVQT_Q_OBJECT_H:.h=.o))



INCLUDE:= $(QTINCLUDE) $(BOOST_INCLUDE)  $(KVINCLUDE) \
         -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	     $(CORBA_INCLUDE)

DEFINES=$(QTDEFINES)

CXXFLAGS += -g $(DEFINES) 


LIBS:=$(KVLIB) -L$(PUTOOLS)/lib -lpuTools $(BOOST_LIB) $(QTLIB) $(CORBA_LIB) \
      -ldl -lpthread

export TOP

moc_%.cc: %.h
	$(MOC) $< -o $@


%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

LIBFILE=lib$(LIBNAME).a($(KVQT_OBJ))
LIBSRC:=$(OBJFILES:.o=.cc)

#kvQtTest: $(KVQTTEST_OBJ)
#	$(CXX) $(CXXFLAGS) -o $@ $(KVQTTEST_OBJ) $(LIBS)   	

.PHONY: depends clean veryclean


all: $(LIBFILE)
	cp lib$(LIBNAME).a $(TOP)/lib
	mkdir -p $(TOP)/include/kvservice/qt
	cp $(INSTALL_HEADERFILES) $(TOP)/include/kvservice/qt

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)


clean:
	rm -rf moc_*.*
	rm -rf *.o *~
	rm -f *.a
	rm -f $(DEPENDSFILE)

veryclean: clean
	(cd test; $(MAKE) clean)


-include $(DEPENDSFILE)
