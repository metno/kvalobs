qabase_src_SOURCES += \
	src/db/KvalobsDatabaseAccess.cpp \
	src/db/FilteredDatabaseAccess.h \
	src/db/DelayedSaveDatabaseAccess.cpp \
	src/db/databaseResultFilter.h \
	src/db/DatabaseAccess.h \
	src/db/DatabaseAccess.cpp \
	src/db/CachedDatabaseAccess.cpp \
	src/db/CachedDatabaseAccess.h \
	src/db/KvalobsDatabaseAccess.h \
	src/db/DelayedSaveDatabaseAccess.h \
	src/db/databaseResultFilter.cpp

include src/db/cache/qabase.mk
include src/db/returntypes/qabase.mk
