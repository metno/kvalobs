#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override INCLUDE+=-I../include 

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

OBJFILES:=dir.o dso.o pidfileutil.o readfile.o file.o copyfile.o mkdir.o


LIBFILE=lib$(LIBNAME).a($(OBJFILES))
LIBSRC:=$(OBJFILES:.o=.cc)

all: $(LIBFILE)


depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))
