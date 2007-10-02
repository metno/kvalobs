
LIBNAME=kvglue

#CXX=/disk1/bin/g++
#CC=/disk1/bin/gcc
CXX=g++
CC=gcc

OBJ:=kvglueParam.o


SRC:=$(OBJ:.o=.cc)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) -c $< 

%.o: %.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDE) -c $< 


LIBFILE=lib$(LIBNAME).a($(OBJ))

all: $(LIBFILE)


paramtest: paramtest.o
	$(CC) -o $@ $< -L. -lkvglue -lstdc++
