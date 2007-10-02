TOP:=../../..

include $(TOP)/conf/make.$(OSTYPE)

INCLUDE=-I$(TOP)/include        \
	    -I$(TOP)/include/kvskel \
	    -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(OMNIINCLUDE) $(BOOSTINCLUDE)

LIBNAME=kvcpp

INCTO:=$(TOP)/include/kvservice/kvcpp

OBJFILES:=KvCorbaApp.o KvApp.o kvCorbaThread.o WhichDataHelper.o\
	  kvWhat.o kvDataSubscribeInfoHelper.o kvDataNotifySubscriberImpl.o \
	  kvDataSubscriberImpl.o  kvHintSubscriberImpl.o  KvAppSimple.o \
	  timer.o  kveventinterface.o kvevents.o RejectdecodeIterator.o KvObsData.o

DEPENDSFILE=make.depends

LIBFILE:=lib$(LIBNAME).a($(OBJFILES))
LIBSRC:=$(OBJFILES:.o=.cc)
ALLSRC:=$(LIBSRC)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

.PHONY: mark pretty clean veryclean install depends

all:   mark $(LIBFILE) install


mark:
	@echo "[0;34m============================================= [0m"



depends:
	rm -f $(DEPENDSFILE)
	echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CFLAGS) $(INCLUDE) -M $(ALLSRC) $(ALLSRC) >> $(DEPENDSFILE)

pretty:
	find . \( -name 'core' -o -name '*~' \) -exec rm -f {} \;

clean:
	@make pretty
	rm -f *.o
	rm -f *~
	rm -f lib*.a
	(cd test; $(MAKE) clean)
	rm -f make.depends

veryclean: clean
	@make pretty
	rm -f $(DEPENDSFILE)
	touch $(DEPENDSFILE)


install:
	cp lib$(LIBNAME).a $(TOP)/lib
	mkdir -p $(TOP)/include/kvservice/kvcpp
	@for FILE in `ls -A1 -ICVS -I*~ *.h` ; do             \
	  if [ -f $(INCTO)/$$FILE ]; then                     \
	    if diff $$FILE $(INCTO)/$$FILE > /dev/null ; then \
		continue ;                                    \
	    fi ;                                              \
          fi ;                                                \
	  echo '[1m updating: ..... '$$FILE'[0m';         \
	  cp $$FILE $(INCTO) ;                                \
	done

include $(wildcard $(DEPENDSFILE))
