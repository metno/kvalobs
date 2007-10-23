TOP=../../..

LIBLIST:=kvcpp kvcpp2
LIBLIST_DRIFT:=kvcpp 
DIRLIST:=kvcpp java  


export DIRLIST LIBLIST LIBLIST_DRIFT

.PHONY: all libs veryclean clean depends

all: libs

drift: libs-drift

libs:
	for DIR in $$LIBLIST ; do \
	   (cd $$DIR; $(MAKE) ) \
	done

libs-drift:
	for DIR in $$LIBLIST_DRIFT ; do \
	   (cd $$DIR; $(MAKE) ) \
	done ;\
	cd kvcpp2; $(MAKE) install


veryclean:
	for DIR in $(DIRLIST) ; do \
	   if [ -d $$DIR  ] ; then \
              (cd $$DIR ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) veryclean ; \
	      fi) \
	   fi \
	done

clean:
	for DIR in $(DIRLIST) ; do \
	   if [ -d $$DIR  ] ; then \
              (cd $$DIR ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) clean ; \
	      fi) \
	   fi \
	done

depends:
	for DIR in $(DIRLIST) ; do \
	   if [ -d $$DIR  ] ; then \
              (cd $$DIR ; \
	      if [ -f Makefile ]; then \
		    $(MAKE) depends ; \
	      fi) \
	   fi \
	done

