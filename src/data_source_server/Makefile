#Major Makefile for file Library

# includefile contains Compiler definitions
TOP=../..
include $(TOP)/conf/make.$(OSTYPE)
include $(TOP)/conf/common.mk

export LIBNAME:=kvDataInputd
export BINS:=kvDataInputd kvDataInputd_clt

LOCALIDIR:=$(LOCALDIR)/include/$(LIBNAME)


install:
	echo "[1mMAKE INSTALL ...................[0m";        
	mkdir -p $(TOP)/bin
	if cd obj ; then                                             \
		cp $(BINS) ../$(TOP)/bin;                                \
		cd ..                ;                                   \
		echo "[4;34m Install bin(s): ..... '$$BINS'[0m";      \
		echo "[1mINSTALL FINISHED................[0m";        \
	else                                                          \
		echo "[1;31mERROR: Cant change directory to obj.[0m";  \
		echo "[1;31mINSTALL FAILED..................[0m";      \
	fi

INCLUDE=-I$(TOP)/../include \
        -I$(TOP)/../include/kvskel     \
        -I$(PUTOOLS)/include -I$(PUTOOLS)/include/puTools \
	    $(BOOSTINCLUDE) \
	    $(OMNIINCLUDE) \
	    $(LIBXMLPPINCLUDE)


