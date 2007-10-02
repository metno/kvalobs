HTOP:=$(shell pwd)

KVDIR=$(shell (cd ../..;pwd))

include $(KVDIR)/conf/make.$(OSTYPE)

JAVALIB:=$(KVDIR)/share/kvalobs/java/lib

override IDLJAVA:=$(KVDIR)/share/kvalobs/java/bin/idl
override JACORBJAR:=$(JAVALIB)/jacorb/jacorb.jar

export KVDIR IDLJAVA JACORBJAR

MYCLASSPATH:=$(JAVALIB)/kvalobsclt.jar:$(JAVALIB)/log4j-1.2.13.jar:$(JACORBJAR):$(CLASSPATH)

JFLAGS:= -d build -classpath $(MYCLASSPATH) 

LOG4J:=$(KVDIR)/share/kvalobs/java/lib/log4j-1.2.13.jar

kv2kl_FILES=src/metno/kvalobs/kv2kl/Kv2KlMain.java      \
            src/metno/kvalobs/kv2kl/KlDataReceiver.java \
            src/metno/kvalobs/kv2kl/Kv2KlApp.java       \
	        src/metno/kvalobs/kv2kl/Filter.java         \
	        src/metno/kvalobs/kv2kl/Kv2KlimaFilter.java 
	        
kl2kv_FILES=src/metno/kvalobs/kl2kv/Kl2KvMain.java        \
            src/metno/kvalobs/kl2kv/Kl2KvApp.java         \
	        src/metno/kvalobs/kl2kv/DbConnection.java     \
	        src/metno/kvalobs/kl2kv/TableInfoType.java    \
	        src/metno/kvalobs/kl2kv/SendData.java         \
            src/metno/kvalobs/kl2kv/DataToKv.java         \
            src/metno/kvalobs/kl2kv/TableInfoType302.java \
            src/metno/kvalobs/kl2kv/TableInfoType402.java \
            src/metno/kvalobs/kl2kv/TableInfoType404.java \
            src/metno/kvalobs/kl2kv/TableInfoType412.java \
            src/metno/kvalobs/kl2kv/TableInfoType433.java
            
SqlHelper_FILES=src/metno/kvalobs/SqlHelper/SqlHelper.java \
                src/metno/kvalobs/SqlHelper/DbConnection.java

%.class: %.java
	$(JAVAC) $(JFLAGS)   $< 

all: kv2kl kl2kv SqlHelper

kv2kl:
	mkdir -p build
	$(JAVAC) $(JFLAGS)  $(kv2kl_FILES)
	( cd build;                                     \
	  jar cmf ../kv2kl.manifest kv2kl.jar           \
	      metno/kvalobs/kv2kl;                      \
      cp kv2kl.jar ..;                              \
      cp kv2kl.jar $(KVDIR)/share/kvalobs/java/lib; \
      rm -f kv2kl.jar                               \
	)

kl2kv:
	$(JAVAC) $(JFLAGS) $(kl2kv_FILES)  
	( cd build;                                                \
	  jar cmf ../kl2kv.manifest kl2kv.jar metno/kvalobs/kl2kv; \
	  cp kl2kv.jar $(KVDIR)/share/kvalobs/java/lib;            \
	  cp kl2kv.jar ..;                                         \
	  rm -f kl2kv.jar                                          \
	)

SqlHelper :
	$(JAVAC) $(JFLAGS) $(SqlHelper_FILES)
	( cd build;                                                         \
	  jar cmf ../SqlHelper.manifest SqlHelper.jar metno/kvalobs/SqlHelper; \
	  cp SqlHelper.jar $(KVDIR)/share/kvalobs/java/lib;                 \
	  cp SqlHelper.jar ..;                                              \
	  rm SqlHelper.jar                                                  \
	)


.PHONY: clean kv2kl veryclean prepare kl2kv-install make-kl2kv-dist

kl2kv-install: kl2kv-dist
	cp kl2kv.tar $(KVALOBS)
	cd $(KVALOBS); tar xpf kl2kv.tar; rm -f kl2kv.tar

kl2kv-dist:
	rm -rf dist
	mkdir -p dist/share/kvalobs/java/lib
	mkdir -p dist/share/kvalobs/java/lib/jacorb	
	mkdir -p dist/share/kvalobs/java/bin
	mkdir -p dist/share/kvalobs/java/etc
	mkdir -p dist/etc	
	mkdir -p dist/bin
	cp -f src/etc/kl2kv.conf.template dist/etc
	cp -f $(LOG4J) $(KVALOBS)/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/kvalobsclt.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/ojdbc14.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/pgjdbc3.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/kl2kv.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/jacorb/*.jar \
	      dist/share/kvalobs/java/lib/jacorb
	cp -f $(KVDIR)/share/kvalobs/java/bin/jaco \
	      dist/share/kvalobs/java/bin/jaco
	cp -f $(KVDIR)/share/kvalobs/java/etc/jacorb.properties \
	      dist/share/kvalobs/java/etc
	cp -f src/bin/kl2kv dist/bin
	(cd dist; tar cpf kl2kv.tar *; cp  kl2kv.tar ..)


kv2kl-install: kv2kl-dist
	cp kv2kl.tar $(KVALOBS)
	cd $(KVALOBS); tar xpf kv2kl.tar; rm -f kv2kl.tar

kv2kl-dist:
	rm -rf dist
	mkdir -p dist/share/kvalobs/java/lib
	mkdir -p dist/share/kvalobs/java/lib/jacorb	
	mkdir -p dist/share/kvalobs/java/bin
	mkdir -p dist/share/kvalobs/java/etc
	mkdir -p dist/etc	
	mkdir -p dist/bin
	cp -f src/etc/kv2kl_log.conf dist/etc
	cp -f src/etc/kv2kl.conf.template dist/etc
	cp -f $(KVDIR)/share/kvalobs/java/lib/kvalobsclt.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(LOG4J) dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/ojdbc14.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/pgjdbc3.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/kv2kl.jar \
	      dist/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/jacorb/*.jar \
	      dist/share/kvalobs/java/lib/jacorb
	cp -f $(KVDIR)/share/kvalobs/java/bin/jaco \
	      dist/share/kvalobs/java/bin/jaco
	cp -f $(KVDIR)/share/kvalobs/java/etc/jacorb.properties \
	      dist/share/kvalobs/java/etc
	cp -f src/bin/kv2kl dist/bin
	(cd dist; tar cpf kv2kl.tar  bin/kv2kl etc/  share/; cp  kv2kl.tar ..)


SqlHelper-install:
	mkdir -p $(KVALOBS)/share/kvalobs/java/lib
	mkdir -p $(KVALOBS)/share/kvalobs/java/bin
	mkdir -p $(KVALOBS)/share/kvalobs/java/etc
	mkdir -p $(KVALOBS)/etc
	cp -f src/etc/SqlHelper.conf.template $(KVALOBS)/etc
	cp -f src/etc/SqlHelper_log.conf $(KVALOBS)/etc
	cp -f $(LOG4J) $(KVALOBS)/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/ojdbc14.jar \
	      $(KVALOBS)/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/pgjdbc3.jar \
	      $(KVALOBS)/share/kvalobs/java/lib
	cp -f $(KVDIR)/share/kvalobs/java/lib/SqlHelper.jar \
	      $(KVALOBS)/share/kvalobs/java/lib
	cp -f src/bin/sqlhelper $(KVALOBS)/bin



clean:
	rm -rf build
	rm -f *.class
	find . -name '*~' -exec rm -f {} \;

veryclean: clean
	rm -f *.jar
	rm -f *.tar
	rm -rf dist


