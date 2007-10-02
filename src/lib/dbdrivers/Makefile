# includefile contains Compiler definitions
TOP=../../..
include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/common.mk

export LIBNAME:=

export LIBTO=$(TOP)/lib/db

LOCALIDIR:=$(LOCALDIR)/include/$(LIBNAME)
INCLUDE=-I. -I$(TOP)/../include $(POSTGRESINCLUDE) $(BOOSTINCLUDE) \
        $(SQLITEINCLUDE)

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
	rm -rf lib/db
