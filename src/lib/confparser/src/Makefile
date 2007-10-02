#This Makefile must be called from ../Makefile

ifeq ($(MAKELEVEL), 0)
dummy:
	@echo -e '\n   This makefile must be called from ../Makefile\n\n'
endif

#override INCLUDE+=-I../include 

%.o: %.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< 

OBJFILES:= valelement.o confsection.o trimstr.o confparser.o \
	   ConfParserLexer.o 
HEADERFILES=valelement.h confexception.h confsection.h \
	    confparser.h 

LIBFILE=lib$(LIBNAME).a($(OBJFILES))
LIBSRC:=$(OBJFILES:.o=.cc)
LIBSRC:=$(subst ConfParserLexer.cc,,$(LIBSRC))



all: $(LIBFILE) miconfparse

ConfParserLexer.o: ConfParserLexer.l 
	$(LEX) -PyyConfParser $< 
	mv -f lex.yyConfParser.c ConfParserLexer.cc
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c ConfParserLexer.cc  

miconfparse: miconfparse.o lib$(LIBNAME).a
	$(CXX) $(CXXLINK) -o $@ $< -L. -lmiconfparser
	@echo '[4;32m Install bin: ..... '$@'[0;0m';	
	@cp $@ $(TOP)/bin


depends:
	rm -f $(DEPENDSFILE)
	@echo '# Automatically-generated dependencies list:' > $(DEPENDSFILE)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -M $(LIBSRC) >> $(DEPENDSFILE)


include $(wildcard $(DEPENDSFILE))
