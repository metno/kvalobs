# includefile contains Compiler definitions


LIBOBJ=test.o

PARSEOBJ = tstLexer.o 

LIBSRC   = $(LIBOBJ:.o=.c)
PARSESRC = $(PARSEOBJ:.o=.c)

ALLOBJ   = $(TIPSOBJ)  $(PARSEOBJ) 

ALLSRC   = $(TIPSSRC) $(TOOLSRC) $(PARSESRC)

VPATH = ../$(LIBDIR):../include

.SUFFIXES: .c .o .cc .o

# From .c to .o
.c.o:
	$(CC) -c $(CFLAGS) $< $(INCLUDE)

# From .cc to .o

	$(CXX) $(CCFLAGS) -c $< $(INCLUDE)


all: mark $(LIBDIR)/libdecoder.a

$(LIBDIR)/libdecoder.a: $(ALLOBJ)
	$(AR)  $(ARFLAGS) $@ $(ALLOBJ) 


include $(DEPENDSFILE)


tstLexer.o: tstLexer.lex
	$(LEX) $(LFLAGS) -PyyTst tstLexer.lex; \
	mv -f lex.yyTst.c tstLexer.c
	$(CC) $(CFLAGS) -c tstLexer.c $(INCLUDE) 

parseclean:
	rm -f $(PARSESRC)
