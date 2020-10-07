
CPPFLAGS := $(shell pkg-config --cflags libkvcpp)
LIBS := $(shell pkg-config --libs libkvcpp)

CXX=g++


kvpull_SRC= kvpull.cc Option.cc Option.h
kvpull_OBJ=kvpull.o Option.o

all: kvpull


$(kvpull_OBJ): %.o: %.cc
	$(CXX) $(CPPFLAGS) -c $< -o $@ 
	

kvpull:  $(kvpull_OBJ)
	$(CXX) -o $@ $(kvpull_OBJ) $(LIBS) 


.PHONY: clean

clean:
	rm -f $(kvpull_OBJ) kvpull
