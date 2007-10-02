#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override INCLUDE += -I../include  
 

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


OBJFILES:=CommandQue.o ThrPoolQue.o thread_private.o
SRCFILES:=$(OBJFILES:.o=.cc)


LIBFILE:=lib$(LIBNAME).a($(OBJFILES))


all: $(LIBFILE) 



depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(SRCFILES) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))
