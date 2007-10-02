#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

override DEFINES+=-D_REENTRANT

OBJFILES:=decodermgr.o decoder.o metadata.o

LIBSRC:=$(OBJFILES:.o=.cc)

SRCFILES:=$(OBJFILES:.o=.cc)

LIBFILE:=lib$(LIBNAME).a($(OBJFILES))


%.o: %.cc
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) $(LIBXML_INCLUDE) -c $< 



all: $(LIBFILE)


#prepare:
#	if [ ! -d ../decoderbase ]; then               \
#	   mkdir -p ../decoderbase;                    \
#	   (cd ../decoderbase; ln -s ../include/*.h .) \
#	fi

depends: 
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDE) $(LIBXML_INCLUDE) -M $(SRCFILES) \
	  >> $(DEPENDSFILE)

include $(wildcard $(DEPENDSFILE))
