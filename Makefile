TOP=.

.PHONY: libs veryclean prepare tag-cvs dist


all: prepare
	cd src; $(MAKE) all


drift: prepare
	cd src; $(MAKE) all-drift

libs: prepare  
	cd src; $(MAKE) libs

prepare:
	mkdir -p lib
	mkdir -p lib/db
	mkdir -p lib/decode
	mkdir -p bin
	mkdir -p include

qtlib: libs
	cd src/service-libs/qt; $(MAKE)

veryclean:
	if [ -d bin ]; then                 \
	   cd bin; for FILE in `ls -1` ; do \
	      if [ -f $$FILE ] ; then       \
	         rm -f $$FILE ;             \
	      fi                            \
	  done                              \
	fi	
	if [ -d lib ]; then                    \
	   cd lib/;                            \
	   for FILE in `ls -1` ; do            \
	      if [ -f $$FILE ]; then           \
	         rm -f $$FILE;                 \
	      elif [ -d $$FILE ]; then         \
	         if [ $$FILE != 'CVS' ]; then  \
	            rm -Rf $$FILE ;            \
	         fi                            \
	      fi                               \
	   done                                \
	fi
	if [ -d include ]; then                \
	   cd include;                         \
	   for FILE in `ls -1` ; do            \
	      if [ -f $$FILE ]; then           \
	         rm -f $$FILE;                 \
	      elif [ -d $$FILE ]; then         \
	         if [ $$FILE != 'CVS' ]; then  \
	            rm -Rf $$FILE ;            \
	         fi                            \
	      fi                               \
	   done                                \
	fi
	cd doc; $(MAKE) veryclean
	cd share; $(MAKE) veryclean
	find . -name '*~' -type f -exec rm -f  {} \;
	find . -name 'tca.map' -type f -exec rm -f  {} \;
	find . -name 'tca.log' -type f -exec rm -f  {} \;
	find . -name '.inslog2' -type f -exec rm -f  {} \;
	cd src; $(MAKE) veryclean


tag-cvs:
	now=`date '+%Y%m%dT%H%M'` ;                 \
	echo "$$now" > VERSION ;                    \
	cvs commit -m "Ny version: $$now" VERSION ; \
	cvs tag rel-$$now 

dist:
	cd dist/kvalobs;                                               \
	rel=`cat ../../VERSION`;                                       \
	rm -rf tmp;                                                    \
	mkdir -p tmp/kvalobs-drift-$$rel;                              \
	where=`pwd`/tmp/kvalobs-drift-$$rel;                           \
    (cd ../..; ./INSTALL.main -f -d $$where);                      \
	mkdir -p tmp/kvalobs-drift-$$rel/mipkg;		                   \
	cp -f ../../CHANGELOG tmp/kvalobs-drift-$$rel;                 \
	cp -f ../../VERSION  tmp/kvalobs-drift-$$rel;                  \
	cp -f ../mipkg/PKG-INSTALL   tmp/kvalobs-drift-$$rel/mipkg;    \
	cp -f ../mipkg/PKG-UNINSTALL tmp/kvalobs-drift-$$rel/mipkg;    \
	cp -f ../mipkg/PKG-ROLLBACK  tmp/kvalobs-drift-$$rel/mipkg;    \
	cp -f NAME tmp/kvalobs-drift-$$rel;                            \
	cp -f files  tmp/kvalobs-drift-$$rel/mipkg;                    \
	(cd tmp; tar cpf kvalobs-drift-$$rel.tar kvalobs-drift-$$rel); \
	gzip tmp/kvalobs-drift-$$rel.tar;                              \
	mv tmp/kvalobs-drift-$$rel.tar.gz ../..
