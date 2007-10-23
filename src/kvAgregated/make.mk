MAIN:= main.cc

SRC+= AbstractAgregator.cc \
  AgregatorHandler.cc \
  KvDataFunctors.cc \
  minmax.cc \
  ra2rr_12.cc \
  rr.cc \
  GenerateZero.cc

include proxy/makefile
