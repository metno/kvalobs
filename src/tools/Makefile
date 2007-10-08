TOP=../..

TOOLS:= kvpush 


export TOOLS

.PHONY: all veryclean clean depends

all:
	for DIR in $$TOOLS ; do \
	   (cd $$DIR; $(MAKE) ) \
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

