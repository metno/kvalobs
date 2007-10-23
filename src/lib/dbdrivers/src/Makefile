#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif


#Driver for postgresql and ODBC
PGDRIVEROBJ=pgdriver.o pgsqldb.o
SQLITEOBJ=
SQLITE3OBJ=
ODBCOBJ=


#Which drivers shall we install
DRIVERS:=pgdriver dummydriver #odbcdriver


SQLITE3INCLUDE=$(shell pkg-config --cflags sqlite3 2>/dev/null)
SQLITE3LIB=$(shell pkg-config --libs sqlite3 2>/dev/null)

ifdef SQLITEINCLUDE 
  TMPSQLITEINCLUD:=$(SQLITEINCLUDE)
  ifneq ($(strip $(TMPSQLITEINCLUDE)),"")
     DRIVERS+=sqlitedriver
     SQLITEOBJ+=sqlitedriver.o sqlitedb.o	
  endif 
endif

ifneq ($(SQLITE3INCLUDE),)
     DRIVERS+=sqlite3driver
     SQLITE3OBJ+=sqlite3driver.o sqlite3db.o	
endif


%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


OBJFILES:=$(PGDRIVEROBJ) $(ODBSOBJ) $(SQLITEOBJ) 
SRCFILES:=$(OBJFILES:.o=.cc)

#debug: 	
#	@echo "DRIVERS: $(DRIVERS)"
#	@echo "SQLITEINCLUDE: $(SQLITEINCLUDE)"

all: setup $(DRIVERS)
	@echo "DRIVERS: $(DRIVERS)"
	@echo "SQLITEINCLUDE: $(SQLITEINCLUDE)"

setup:
	@echo "DRIVERS: $(DRIVERS)"
	@echo "SQLITEINCLUDE:  $(SQLITEINCLUDE)"
	@echo "SQLITE3INCLUDE: $(SQLITE3INCLUDE)"
	@echo "SQLITE3LIB:     $(SQLITE3LIB)"

#Compiles the postgresql driver.
pgdriver: pgdriver.o pgsqldb.o 
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -o $@.so -fPIC $< \
	    pgsqldb.o $(POSTGRESLIB) -L$(TOP)/lib -lpq -lkvdb -lfileutil
	mkdir -p ../lib/db
	cp $@.so ../lib/db


#Compiles the dummy driver
dummydriver.o: dummydriver.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 

dummysqldb.o: dummysqldb.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 

dummydriver: dummydriver.o dummysqldb.o 
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -o $@.so -fPIC $< \
	    dummysqldb.o -L$(TOP)/lib -lkvdb 
	mkdir -p ../lib/db
	cp $@.so ../lib/db

pgdriver.o: pgdriver.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 

pgsqldb.o: pgsqldb.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 


#Compiles the sqlite driver.
sqlitedriver: $(SQLITEOBJ)
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -o $@.so -fPIC \
	    $(SQLITEOBJ) $(SQLITELIB) -L$(TOP)/lib  -lkvdb -lfileutil
	mkdir -p ../lib/db
	cp $@.so ../lib/db

sqlitedriver.o: sqlitedriver.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 

sqlitedb.o: sqlitedb.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 



#Compiles the sqlite3 driver.
sqlite3driver: $(SQLITE3OBJ)
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -o $@.so -fPIC \
	    $(SQLITE3OBJ) $(SQLITE3LIB)  \
		-L$(TOP)/lib  -lkvdb -lfileutil
	mkdir -p ../lib/db
	cp $@.so ../lib/db

sqlite3driver.o: sqlite3driver.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 

sqlite3db.o: sqlite3db.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(SQLITE3INCLUDE) $(INCLUDE) -c $< 



#Compiles the odbc driver. (Not implemented)
odbcdriver: odbcdriver.o odbcdb.o
	$(CXX) $(CXXLDFLAGS) $(CXXLDSOFLAGS) -o $@.so $< \
	    odbcdb.o db.o -L$(ODBCLIB) -lodbc -lkvdb 

odbcdriver.o: odbcdriver.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 

odbcdb.o: odbcdb.cc
	$(CXX) $(CXXFLAGS) $(CXXSOFLAGS) $(INCLUDE) -c $< 



depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(SRCFILES) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))
