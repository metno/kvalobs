
DEFINES=

BINDIR=bin
SRCDIR=src
LIBDIR=lib
OBJDIR=obj
TESTDIR=test

DEPENDSFILE=$(OBJDIR)/make.depends

OPTIONS=$(LOCALOPTIONS) "CXX=${CXX}" "CXXFLAGS=${CXXFLAGS} ${DEFINES}" "CXXSOFLAGS=${CXXSOFLAGS}" "CXXLDSOFLAGS=${CXXLDSOFLAGS}" "CXXLINKSO=${CXXLINKSO}" "CC=${CC}" "CFLAGS=${CFLAGS} ${DEFINES}" "LDFLAGS=${CXXLDFLAGS} ${DEFINES}" "AR=${AR}" "ARFLAGS=${ARFLAGS}" "INCLUDE=${INCLUDE}" "LIBDIR=../${LIBDIR}" "DEPENDSFILE=../${DEPENDSFILE}" "LEX=${LEX}" "TOP=../${TOP}"

all: mark directories libs install

mark:
	@echo '[1;31mMaking library "${LIBNAME}"[0m'

depends:
	cd $(SRCDIR); make $(OPTIONS) depends

directories:
	if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	if [ ! -d $(BINDIR) ] ; then mkdir $(BINDIR) ; fi
	if [ ! -f $(DEPENDSFILE) ] ; \
	then touch $(DEPENDSFILE) ; make depends ; fi
	cd $(OBJDIR) ; ln -sf ../$(SRCDIR)/* .

libs:
	cd $(OBJDIR); make $(OPTIONS)

pretty:
	echo "[1;31m MAKE PRETTY .... !!!![0m"; 
	find . \( -name 'core' -o -name 'core.o' -o -name '*~' \) \
               -exec rm -f {} \;

clean:
	echo "[1;31m MAKE CLEAN ........ !!!![0m"; 	
	@make pretty
	if [ -d $(OBJDIR) ]; then cd $(OBJDIR); rm -f *.o; fi
	if [ -d $(BINDIR) ]; then cd $(BINDIR); rm -rf * ; fi

veryclean:
	echo "[1;31m MAKE VERYCLEAN ....... !!!![0m"; 
	@make pretty
	if [ -d $(OBJDIR) ]; then rm -rf $(OBJDIR); fi
	if [ -d $(BINDIR) ]; then rm -rf $(BINDIR); fi
	rm -f $(DEPENDSFILE)
	if [ -d $(LIBDIR) ]; then          \
	   cd $(LIBDIR); rm -f *.a *.so;   \
	   cd ..;                          \
	   if [ ! -d $(LIBDIR)/CVS ]; then \
	      rm -rf $(LIBDIR) ;           \
	   fi                              \
	fi
	if [ -d $(TESTDIR) ] ; then\
	   cd  $(TESTDIR) ;  $(MAKE) veryclean ; \
        fi


.SILENT: install veryclean clean pretty mark


.PHONY: install

install:
	echo "[1mMAKE INSTALL ...................[0m"
	@make pretty
	MYINCFROM=$(INCFROM);                                     \
	if [ -d $$INCFROM ]; then                               \
	   if [ -d $$MYINCFROM/$(LIBNAME) ]; then                 \
	   	  MYINCFROM=$(INCFROM)/$(LIBNAME);                      \
	   fi;                                                    \
	   if [ $(INCTO) ]; then                                  \
	      mkdir -p $(INCTO);                                  \
	      if [ -d $(INCTO) ]; then                            \
	         for FILE in `ls -A1 -ICVS -I*~ $(IIGNORE) $$MYINCFROM` ; do  \
	            if [ -f $(INCTO)/$$FILE ]; then               \
	               if diff $$MYINCFROM/$$FILE $(INCTO)/$$FILE > /dev/null ; then \
			          continue ;                              \
	               fi ;                                       \
                fi ;                                          \
	            echo '[1m updating: ..... '$$FILE'[0m';   \
	            cp $$MYINCFROM/$$FILE $(INCTO) ;                          \
	         done                                               \
	      fi                                                 \
	   fi                                                     \
     fi                                                       
	 if [ -d $(LIBFROM) ]; then                               \
	   cd $(LIBFROM);                                         \
	   FILES=`ls -1d lib*.a lib*.so *.so 2>/dev/null` ;       \
	   if [ -z $FILES ]; then                                 \
	      echo "[1;31mNO library "${LIBNAME}" found !!!![0m"; \
	   else                                                   \
	      if [ ! -d ../$(LIBTO) ]; then                       \
	         mkdir -p ../$(LIBTO) ;                           \
	      fi ;                                                \
	      for FILE in $$FILES ; do                            \
	         if [ -f $$FILE ]; then                           \
	 	    cp  $$FILE ../$(LIBTO);                       \
	            echo '[4;32m Install lib: ..... '$$FILE'[0;0m';\
	         fi                                               \
              done                                                \
	   fi                                                     \
	 else                                                      \
         echo "[1;31mInvalid LIBFROM in library "${LIBNAME}" found !!!![0m"; \
	 fi                                                                
	 echo "[1mINSTALL FINISHED................[0m"; 	








