qabase_src_SOURCES += \
	src/Exception.h \
	src/CheckRunner.h \
	src/LogFileCreator.cpp \
	src/CheckRunner.cpp \
	src/Configuration.cpp \
	src/Configuration.h \
	src/LogFileCreator.h
	
qabase_SOURCES += src/main.cpp

include src/scriptrunner/qabase.mk
include src/corbalistener/qabase.mk
include src/scriptcreate/qabase.mk
include src/db/qabase.mk
