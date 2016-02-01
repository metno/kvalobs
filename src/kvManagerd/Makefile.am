bin_PROGRAMS = kvManagerd

kvManagerd_SOURCES = \
	main.cpp \
	ManagerApp.cpp \
	ManagerApp.h \
	MissingRunner.cpp \
	MissingRunner.h \
	ObservationComplete.cpp \
	ObservationComplete.h \
	KvalobsDatabaseAccess.cpp \
	KvalobsDatabaseAccess.h \
	DataIdentifier.cpp \
	DataIdentifier.h 

#	transaction/AddMissingData.cpp \
#	transaction/AddMissingData.h \
#	transaction/CompleteObservation.cpp \
#	transaction/CompleteObservation.h \
#	transaction/FindAllMissing.cpp \
#	transaction/FindAllMissing.h


AM_LDFLAGS = \
	$(top_builddir)/src/lib/milog/libkvalobs_milog.la \
	$(top_builddir)/src/lib/kvdb/libkvalobs_kvdb.la \
	$(top_builddir)/src/lib/kvalobs/libkvalobs_kvalobs.la \
	$(BOOST_DATE_TIME_LIB)

AM_CPPFLAGS = \
	-I$(top_srcdir)/src/ \
	-I$(top_srcdir)/src/lib \
	-D_REENTRANT
