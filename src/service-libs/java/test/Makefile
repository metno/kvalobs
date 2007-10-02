HTOP:=$(shell pwd)
KVDIR:=$(subst /src/service-libs/java/test,,$(HTOP))
TOP:=$(HTOP)/../../../..
include $(KVDIR)/conf/make.$(OSTYPE)


override IDLJAVA:=$(KVDIR)/share/kvalobs/java/bin/idl
override JACORBJAR:=$(KVDIR)/share/kvalobs/java/lib/jacorb/jacorb.jar
export KVDIR IDLJAVA JACORBJAR





debug:
	@echo "IDLJAVA: $(IDLJAVA)"
	@echo

PBASE:=$(HTOP)/lib/CKvalObs/CService

MYCLASSPATH:=-classpath $(HTOP)/../kvalobsclt.jar:$(JACORBJAR):$(CLASSPATH)

JFLAGS:= $(MYCLASSPATH) 


%.class: %.java
	$(JAVAC) $(JFLAGS)   $< 

testTime:
	$(JAVAC) $(JFLAGS) testTime.java	

getparams:
	$(JAVAC) $(JFLAGS) getparams.java	

senddata:
	$(JAVAC) $(JFLAGS) senddata.java	

senddataf:
	$(JAVAC) $(JFLAGS) senddataf.java	

getstations:
	$(JAVAC) $(JFLAGS) getstations.java	

gettypes:
	$(JAVAC) $(JFLAGS) gettypes.java	

getobspgm:
	$(JAVAC) $(JFLAGS) getobspgm.java	

getdata:
	$(JAVAC) $(JFLAGS) getdata.java	

datasubscriber:
	$(JAVAC) $(JFLAGS) datasubscriber.java DataReceiver.java	

getAndSubscribeDataWithListener:
	$(JAVAC) $(JFLAGS) getAndSubscribeDataWithListener.java DataReceiver.java
kv2kl:
	$(JAVAC) $(JFLAGS) kv2klMain.java KlDataReceiver.java  KlApp.java \
	        DbConnection.java
	jar cmf kv2kl.manifest kv2kl.jar kv2klMain.class \
		KlDataReceiver.class  KlApp.class \
	        DbConnection.class KlApp$$DbCleanup.class

getdataWithListener:
	$(JAVAC) $(JFLAGS) getdataWithListener.java DataReceiver.java

.PHONY: clean kv2kl

clean:
	rm *.class
	rm -f kv2kl.jar
	find . -name '*~' -exec rm -f {} \;
