#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif


override DEFINE+= -D_REENTRANT

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINE) $(INCLUDE) -c $< 


OBJFILES:= Logger.o LogStream.o Layout.o StdLayout.o FLogStream.o \
	   LoggerImpl.o TraceLayout.o LogManager.o ThreadLogManager.o\
	   HtmlStream.o HtmlLayout.o StdErrStream.o PThread.o miloghelper.o



LIBFILE=lib$(LIBNAME).a($(OBJFILES))
LIBSRC:=$(OBJFILES:.o=.cc)

.PHONY: depends

all: $(LIBFILE)
	mkdir -p $(TOP)/include/milog/private 
	mkdir -p $(TOP)/include/milog/thread 	
	@make installsub "SUBINC=thread" "INCFROM=${INCFROM}" "INCTO=${INCTO}"
	@make installsub "SUBINC=private" "INCFROM=${INCFROM}" "INCTO=${INCTO}"

depends:
	rm -f $(DEPENDSFILE)
	mkdir -p ../obj
	echo '# Automatically-generated dependencies list:'  >  $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(DEFINE) $(INCLUDE) -M $(LIBSRC) > $(DEPENDSFILE)



installsub:
	echo "[1mINSTALL SUB $(SUBINC) ...................[0m"
	cd ../include/milog/$(SUBINC); \
	mkdir -p $(TOP)/../../include/milog/$(SUBINC) ; \
	  for FILE in `ls -A1 -ICVS -I*~` ; do     \
	    if [ -f $(TOP)/../../include/milog/$(SUBINC)//$$FILE ]; then    \
	       if  diff $$FILE $(TOP)/../../include/milog/$(SUBINC)/$$FILE  > /dev/null ; then \
		   continue ;                                \
	       fi ;                                          \
             fi ;      \
	   echo '[1m updating: ..... '$$FILE'[0m';     \
	   cp $$FILE  $(TOP)/../../include/milog/$(SUBINC)/$$FILE;    \
	done  
	echo "[1mSUBINSTALL :  $(SUBINC) FINISHED................[0m"; 	



include $(wildcard $(DEPENDSFILE))

