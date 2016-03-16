qabase_src_SOURCES += \
	src/Exception.h \
	src/CheckRunner.h \
	src/LogFileCreator.cpp \
	src/CheckRunner.cpp \
	src/Configuration.cpp \
	src/Configuration.h \
	src/LogFileCreator.h \
	src/CheckRequestConsumer.cpp \
	src/CheckRequestConsumer.h \
	src/DataProcessor.cpp \
	src/DataProcessor.h \
	src/QaBaseApp.cpp \
	src/QaBaseApp.h \
	src/NewDataListener.cpp \
	src/NewDataListener.h \
	src/TransactionLogger.cpp \
	src/TransactionLogger.h
	
	
kvQabased_SOURCES += src/main.cpp

include src/scriptrunner/qabase.mk
include src/scriptcreate/qabase.mk
include src/db/qabase.mk
