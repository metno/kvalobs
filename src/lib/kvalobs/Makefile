#Major Makefile for file Library

# includefile contains Compiler definitions
TOP=../../..
include $(TOP)/conf/make.$(OSTYPE)


export LIBNAME:=kvalobs


LOCALIDIR:=$(LOCALDIR)/include/$(LIBNAME)

INCLUDE=-I../include                     \
        -I$(PUTOOLS)/include             \
		-I$(PUTOOLS)/include/puTools     \
	    -I$(TOP)/../include              \
	    $(BOOSTINCLUDE) $(OMNIINCLUDE)




include $(TOP)/conf/common.mk
