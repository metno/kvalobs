TOP=../..
include $(TOP)/conf/make.$(OSTYPE)

override INCLUDE+=$(OMNIINCLUDE) -I.


INCTO=$(TOP)/include/kvskel
INCFROM=./
LIBFROM=.
LIBTO=$(TOP)/lib

LIBNAME:=libcorba_skel.a

IDLDIR=$(TOP)/share/kvalobs/idl

IDLFILES=admin.idl KeyVal.idl datasource.idl \
         commonStationInfo.idl managerInput.idl \
         qabase.idl kvServiceCommon.idl \
	 kvDataNotifySubscriber.idl  kvDataSubscriber.idl \
	 kvHintSubscriber.idl kvService.idl 	\
	 newfile.idl kvsynopd.idl 



SKEL=$(subst .idl,SK.o,$(IDLFILES))
LIB=$(LIBNAME)($(SKEL))

export LIBNAME INCLUDE

%SK.cc : $(IDLDIR)/%.idl
	$(IDLCXX) $< 

%SK.o %.hh: %SK.cc
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(INCLUDE) -c $< -o $@

lib: $(LIB) otherlib install


otherlib:
	cd src; $(MAKE) 

clean: veryclean

veryclean:
	rm -f *.o *SK.cc *.hh *_i.cc libcorba_skel.a 
	rm -f *~
	rm -f *.o
	rm -f core
	rm -f core.*
	cd src; $(MAKE) clean
	cd include; rm -f *~

depends: 
	echo "No make.depends for skel"



install:	
	cd $(INCFROM) ;                                           \
	mkdir -p $(INCTO);                                        \
	for FILE in `ls -A1 *.hh ` ;                 do           \
	     if [ -f $(INCTO)/$$FILE ]; then                      \
	        if diff $$FILE $(INCTO)/$$FILE > /dev/null ; then \
			continue ;                                \
	        fi ;                                              \
             fi ;                                                 \
	 echo '[1m updating: ..... '$$FILE'[0m';              \
	 cp $$FILE $(INCTO) ;                                     \
	done
	cp -f include/*.h $(INCTO)
	cp -f $(LIBFROM)/libcorba_skel.a $(LIBTO)


