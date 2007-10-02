#Major Makefile for file Library

# includefile contains Compiler definitions
TOP=../..

export LIBNAME:=kvQabase

include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/common.mk

TOINC=

export LIBNAME:=kvQabase
export BINS:=kvQabased


LOCALIDIR:=$(LOCALDIR)/include/$(LIBNAME)


LINC=-I$(TOP)/../include

INCLUDE= $(LINC)             \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	     $(LINC)/kvskel      \
	     $(BOOSTINCLUDE)     \
	     $(OMNIINCLUDE)      \
	     $(PERLCCOPTS)

install:
	echo "[1mMAKE INSTALL ...................[0m";        
	mkdir -p $(TOP)/bin
	cp  obj/$(BINS) $(TOP)/bin
	echo '[4;34m Install bin(s): ..... '$$BINS'[0;0m'	
	echo "[1mINSTALL FINISHED................[0m";

