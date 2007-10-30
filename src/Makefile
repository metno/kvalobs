TOP=..

LIBLIST:=skel lib decode service-libs 
LIBLIST_DRIFT:=skel lib decode

#DIRLIST:=kvDataInputd kvManagerd kvServiced \
	     norcom2kv kvsynopd kvAgregated tools kvQabased

#DIRLIST_DRIFT:=kvDataInputd kvManagerd kvServiced \
	     norcom2kv kvsynopd kvAgregated tools

DIRLIST_DRIFT:=kvDataInputd kvManagerd kvServiced \
	     norcom2kv kvsynopd kvAgregated tools


#DIRLIST:=kvDataInputd kvManagerd kvQabased kvServiced \
	 norcom2kv kvsynopd kvAgregated kvQabased


DIRLIST:=kvDataInputd kvManagerd kvQabased kvServiced \
	 norcom2kv kvsynopd kvAgregated


export DIRLIST DIRLIST_DRIFT LIBLIST LIBLIST_DRIFT

.PHONY: all libs veryclean clean depends

all: libs
	for DIR in $$DIRLIST ; do \
           (cd $$DIR && $(MAKE) ) \
        done


all-drift: drift-libs
	for DIR in $$DIRLIST_DRIFT ; do \
	   (cd $$DIR && $(MAKE) ) \
	done ; \
	cd kvQabased; $(MAKE) install

drift-libs:	
	for DIR in $$LIBLIST_DRIFT ; do \
	   (cd $$DIR && $(MAKE) ) \
	done ; \
	cd service-libs; $(MAKE) drift

libs:
	for DIR in $$LIBLIST ; do \
	   (cd $$DIR && $(MAKE) ) \
	done


veryclean:
	for FILE in `ls -1` ; do \
	   if [ -d $$FILE -a $$FILE != ".svn" ] ; then \
              (cd $$FILE ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) veryclean ; \
	      fi) \
	   fi \
	done

clean:
	for FILE in `ls -1` ; do \
	   if [ -d $$FILE -a $$FILE != ".svn" ] ; then \
              (cd $$FILE ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) clean ; \
	      fi) \
	   fi \
	done

depends:
	for FILE in `ls -1` ; do \
	   if [ -d $$FILE -a $$FILE != ".svn" ] ; then \
              (cd $$FILE ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) depends ; \
	      fi) \
	   fi \
	done

