#Major Makefile for file Library

# includefile contains Compiler definitions
TOP=../../..

include $(TOP)/conf/make.$(OSTYPE)

#INCTO=$(TOP)/../../include/decoderbase
#LIBTO=$(TOP)/lib/

include $(TOP)/conf/common.mk

export LIBNAME:=decoderbase



LOCALIDIR:=$(LOCALDIR)/include/$(LIBNAME)

INCLUDE=-I../include                     \
        -I$(PUTOOLS)/include             \
	    -I$(PUTOOLS)/include/puTools     \
	    -I$(TOP)/../include              \
	    $(BOOSTINCLUDE)  

clean:
	echo "[1;31m MAKE CLEAN ........ !!!![0m"; 	
	@make pretty
	if [ -d obj ]; then cd obj; rm -f *.o; fi
	if [ -d bin ]; then cd bin; rm -rf * ; fi

veryclean:
	echo "[1;31m MAKE VERYCLEAN ....... !!!![0m"; 
	@make pretty
	if [ -d obj ]; then rm -rf obj; fi
	if [ -d bin ]; then rm -rf bin; fi
	if [ -d decoderbase ]; then rm -rf decoderbase; fi 
	rm -f $(DEPENDSFILE)
	if [ -d lib ]; then          \
	   cd lib; rm -f *.a *.so;   \
	   cd ..;                          \
	   if [ ! -d lib/CVS ]; then \
	      rm -rf lib ;           \
	   fi                              \
	fi
	if [ -d test ] ; then\
	   cd  test ;  $(MAKE) veryclean ; \
        fi
