#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif


OBJFILES:=db.o dbdrivermgr.o transactionhelper.o
SRCFILES:=$(OBJFILES:.o=.cc)

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 


LIBFILE=lib$(LIBNAME).a($(OBJFILES))


all: libs 

libs: $(LIBFILE)

depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(SRCFILES) >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))
