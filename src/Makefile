TOP=..

LIBLIST:=skel lib decode service-libs 
LIBLIST_DRIFT:=skel lib decode

#DIRLIST:=data_source_server manager kvServiced2 \
	     norcom2kv kvsynopd agregator tools kvQabased2

#DIRLIST_DRIFT:=data_source_server manager kvServiced2 \
	     norcom2kv kvsynopd agregator tools

DIRLIST_DRIFT:=data_source_server manager service \
	     norcom2kv kvsynopd agregator tools


#DIRLIST:=data_source_server manager qabase service \
	 norcom2kv kvsynopd agregator kvQabased2


DIRLIST:=data_source_server manager kvQabased2 service \
	 norcom2kv kvsynopd agregator


export DIRLIST DIRLIST_DRIFT LIBLIST LIBLIST_DRIFT

.PHONY: all libs veryclean clean depends

all: libs
	for DIR in $$DIRLIST ; do \
	   (cd $$DIR; $(MAKE) ) \
	done

all-drift: drift-libs
	for DIR in $$DIRLIST ; do \
	   (cd $$DIR; $(MAKE) ) \
	done ; \
	cd kvQabased2; $(MAKE) install

drift-libs:	
	for DIR in $$LIBLIST_DRIFT ; do \
	   (cd $$DIR; $(MAKE) ) \
	done ; \
	cd service-libs; $(MAKE) drift

libs:
	for DIR in $$LIBLIST ; do \
	   (cd $$DIR; $(MAKE) ) \
	done


veryclean:
	for FILE in `ls -1` ; do \
	   if [ -d $$FILE -a $$FILE != "CVS" ] ; then \
              (cd $$FILE ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) veryclean ; \
	      fi) \
	   fi \
	done

clean:
	for FILE in `ls -1` ; do \
	   if [ -d $$FILE -a $$FILE != "CVS" ] ; then \
              (cd $$FILE ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) clean ; \
	      fi) \
	   fi \
	done

depends:
	for FILE in `ls -1` ; do \
	   if [ -d $$FILE -a $$FILE != "CVS" ] ; then \
              (cd $$FILE ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) depends ; \
	      fi) \
	   fi \
	done

