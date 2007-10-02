HTOP:=$(shell pwd)
KVDIR:=$(subst /src/service-libs/java,,$(HTOP))

override IDLJAVA:=$(KVDIR)/share/kvalobs/java/bin/idl
override JACORBJAR:=$(KVDIR)/share/kvalobs/java/lib/jacorb/jacorb.jar
override JUNITJAR:=$(KVDIR)/share/kvalobs/java/lib/junit.jar
override HSQLJAR:=$(KVDIR)/share/kvalobs/java/lib/hsqldb.jar

export KVDIR IDLJAVA JACORBJAR

#debug:
#	@echo "KVDIR: $(KVDIR)"
#	@echo "IDLJAVA: $(IDLJAVA)"
#	./myecho



include $(HTOP)/../../../conf/make.$(OSTYPE)

IDLDIR:=$(HTOP)/../../../share/kvalobs/idl
IDLFILES:=$(IDLDIR)/kvService.idl              \
          $(IDLDIR)/managerInput.idl           \
          $(IDLDIR)/kvDataNotifySubscriber.idl \
          $(IDLDIR)/kvDataSubscriber.idl       \
          $(IDLDIR)/kvHintSubscriber.idl       \
          $(IDLDIR)/kvServiceCommon.idl        \
          $(IDLDIR)/commonStationInfo.idl      \
	      $(IDLDIR)/datasource.idl             \
	      $(IDLDIR)/admin.idl	

KVFILES=$(HTOP)/src/kvalobs/priv/kvDataNotifySubscriberImpl.java \
        $(HTOP)/src/kvalobs/priv/kvDataSubscriberImpl.java       \
        $(HTOP)/src/kvalobs/priv/kvHintSubscriberImpl.java	     \
        $(HTOP)/src/kvalobs/priv/CorbaThread.java                \
        $(HTOP)/src/kvalobs/priv/KvEventThread.java              \
        $(HTOP)/src/kvalobs/priv/MtQue.java                      \
        $(HTOP)/src/kvalobs/priv/KvEventQue.java                 \
        $(HTOP)/src/kvalobs/priv/SubscriberInfo.java             \
        $(HTOP)/src/kvalobs/priv/ShutdownHook.java               \
        $(HTOP)/src/kvalobs/priv/GetDataThread.java              \
        $(HTOP)/src/kvalobs/priv/GetDataThreadManager.java       \
        $(HTOP)/src/kvalobs/KvDataSubscribeInfo.java             \
        $(HTOP)/src/kvalobs/KvDataNotifyEvent.java               \
        $(HTOP)/src/kvalobs/KvDataEvent.java                     \
        $(HTOP)/src/kvalobs/KvGetDataEvent.java                  \
        $(HTOP)/src/kvalobs/KvHintEvent.java                     \
        $(HTOP)/src/kvalobs/KvApp.java                           \
        $(HTOP)/src/kvalobs/KvDataNotifyEventListener.java       \
        $(HTOP)/src/kvalobs/KvDataEventListener.java             \
        $(HTOP)/src/kvalobs/KvHintEventListener.java             \
        $(HTOP)/src/kvalobs/KvEvent.java	                     \
        $(HTOP)/src/kvalobs/SwingEvent.java	                     \
        $(HTOP)/src/kvalobs/KvNoConnection.java                  \
        $(HTOP)/src/kvalobs/KvDataIterator.java


BUILD_DIR=$(HTOP)/build

METNO_FILES=$(HTOP)/src/metno/util/MiTime.java            \
            $(HTOP)/src/metno/util/MiGMTTime.java         \
            $(HTOP)/src/metno/util/StringHolder.java      \
            $(HTOP)/src/metno/util/LongHolder.java        \
            $(HTOP)/src/metno/util/IntHolder.java         \
            $(HTOP)/src/metno/dbutil/DbConnection.java    \
            $(HTOP)/src/metno/dbutil/DbConnectionMgr.java 

TESTDIR=$(HTOP)/test

TEST_FILES=$(TESTDIR)/metno/dbutil/UtilTest.java

CKVALOBS=$(HTOP)/src/generated/CKvalObs/CService
MICUTIL=$(HTOP)/micutil

DOCFILES=$(HTOP)/src/kvalobs/KvDataSubscribeInfo.java             \
         $(HTOP)/src/kvalobs/KvDataNotifyEvent.java               \
         $(HTOP)/src/kvalobs/KvDataEvent.java                     \
         $(HTOP)/src/kvalobs/KvHintEvent.java                     \
         $(HTOP)/src/kvalobs/KvApp.java                           \
         $(HTOP)/src/kvalobs/KvDataNotifyEventListener.java       \
         $(HTOP)/src/kvalobs/KvDataEventListener.java             \
         $(HTOP)/src/kvalobs/KvHintEventListener.java             \
         $(HTOP)/src/kvalobs/KvEvent.java	                      \
         $(CKVALOBS)/kvDataNotifySubscriberPackage/What*.java \
         $(CKVALOBS)/ObsData*.java                            \
         $(CKVALOBS)/Param*.java                              \
         $(CKVALOBS)/QcId*.java                               \
         $(CKVALOBS)/StatusId*.java                           \
         $(CKVALOBS)/WhichData*.java                          \
         $(CKVALOBS)/DataElem*.java                           \
         $(CKVALOBS)/Obs_pgm*.java                            \
         $(CKVALOBS)/Station*.java                            \
         $(CKVALOBS)/Types*.java                              \
         $(MICUTIL)/*.java

export IDLDIR IDLFILES 

CKVPATH_=$(BUILD_DIR)/CKvalObs/CService
METNOPATH=$(BUILD_DIR)/metno/util:$(BUILD_DIR)/metno/dbutil
KVPATH=$(BUILD_DIR)/kvalobs:$(BUILD_DIR)/kvalobs/priv
CKVPATH=$(CKVPATH_):$(CKVPATH_)/kvDataNotifySubscriberPackage:$(CKVPATH_)/CDataSource:$(CKVPATH_)/CManager:$(BUILD_DIR)/micutil

MYCLASSPATH:=$(BUILD_DIR):$(METNOPATH):$(KVPATH):$(CKVPATH):$(JACORBJAR):$(CLASSPATH)

JFLAGS:=-d build -classpath $(MYCLASSPATH) 

TESTCLASSPATH=$(MYCLASSPATH):$(JUNITJAR):$(BUILD_DIR)/test:$(HSQLJAR)
JFLAGS_TEST:=-d build/test -classpath $(TESTCLASSPATH) 



%.class: %.java
	 $(JAVAC) $(JFLAGS)   $< 


.PHONY: idl doc clean veryclean userdoc jar

all: clean idl metno kvalobsclt jar


jar:
	(mkdir -p build;                                            \
	 cd build;                                                   \
	 jar cf kvalobsclt.jar                                       \
        CKvalObs/*.class                                        \
        CKvalObs/CService/*.class                               \
        CKvalObs/CDataSource/*.class                            \
        CKvalObs/CManager/*.class                               \
        CKvalObs/CService/kvDataNotifySubscriberPackage/*.class \
        kvalobs/*.class                                         \
        kvalobs/priv/*.class                                    \
        metno/util/*.class                                      \
        micutil/*.class ;                                       \
        cp *.jar ..;                                            \
     rm -f *.jar)
	cp kvalobsclt.jar  $(KVDIR)/share/kvalobs/java/lib


idl:
	@echo "IDLJAVA: $(IDLJAVA)" 
	mkdir -p build;
	rm -rf $(HTOP)/build/CKvalObs
	for i in $$IDLFILES; do \
       $(IDLJAVA) -d src/generated  $$i; done
	$(JAVAC) $(JFLAGS) \
	     src/generated/CKvalObs/*.java \
	     src/generated/CKvalObs/CService/*.java \
         src/generated/CKvalObs/CService/kvDataNotifySubscriberPackage/*.java\
         src/generated/CKvalObs/CDataSource/*.java  \
         src/generated/CKvalObs/CManager/*.java \
         src/generated/micutil/*.java 

kvalobsclt:
	mkdir -p build	
	$(JAVAC) $(JFLAGS) $(KVFILES)

metno:
	mkdir -p build
	$(JAVAC) $(JFLAGS) $(METNO_FILES)

kvalobs: kvalobsclt jar


doc:
	mkdir -p html
	$(JAVADOC) -d html $(KVFILES) $(MIUTIL_FILES) \
	       src/generated/CKvalObs/CService/*.java \
	       src/generated/CKvalObs/CService/kvDataNotifySubscriberPackage/*.java \
	       src/generated/CKvalObs/CDataSource/*.java \
	       src/generated/CKvalObs/CManager/*.java

userdoc:
	mkdir -p user-html
#	$(JAVADOC) -use -windowtitle 'kvalobs - bruker manual'  -d user-html $(DOCFILES)
	$(JAVADOC) -encoding 'iso-8859-1' -charset 'iso-8859-1' -use -windowtitle 'kvalobs - bruker manual'  -d user-html \
	    src/kvalobs/*.java \
	    src/generated/CKvalObs/CService/*.java \
	    src/generated/CKvalObs/CService/kvDataNotifySubscriberPackage/*.java \
	    src/generated/CKvalObs/CDataSource/*.java $(MIUTIL_FILES)
	    
jtest:
	mkdir -p build/test	
	$(JAVAC) $(JFLAGS_TEST) $(TEST_FILES)
	$(JAVA_HOME)/bin/java -cp $(TESTCLASSPATH) junit.textui.TestRunner metno.dbutil.UtilTest

clean:
	rm -rf build;
	rm -rf $(HTOP)/src/generated
	rm -rf  user-html
	rm -f html.tar user-html.tar
	find . -name '*~' -exec rm -f {} \;
	find . -name '*.class' -exec rm -f {} \;	

veryclean: clean
	rm -f kvalobsclt.jar
	rm -rf html
