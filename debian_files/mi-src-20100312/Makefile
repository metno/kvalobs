# includefile contains Compiler definitions etc.
include ./make.${OSTYPE}

DESTDIR= ../tmp/usr/lib/kvalobs
DESTNAME= bin

.SUFFIXES: .f .F .cc .o

# From .F to .o
.F.o:
	$(FC) -c $(FCFLAGS) $(FCDEFINES) $<

# From .f to .o
.f.o:
	$(FC) -c $(FCFLAGS) $<

# From .cc to .o
.cc.o:
	$(CXX) -c $(CXXFLAGS) $<

BINFILES= $(BINDIR)/metdat  $(BINDIR)/posdat 

#BINFILES= $(BINDIR)/binfdif2  $(BINDIR)/cressm $(BINDIR)/cvaflag \
#          $(BINDIR)/dig2felt $(BINDIR)/digpar $(BINDIR)/flt2flt $(BINDIR)/flt2fltmega $(BINDIR)/fltdif \
#          $(BINDIR)/flttst $(BINDIR)/obsread $(BINDIR)/fmfelt $(BINDIR)/fmfeltmega $(BINDIR)/fstrip \
#          $(BINDIR)/funpack $(BINDIR)/getftime $(BINDIR)/mergeclim $(BINDIR)/metdat \
#          $(BINDIR)/nyfelt $(BINDIR)/obslist $(BINDIR)/posdat $(BINDIR)/rfinh $(BINDIR)/rmcomment \
#          $(BINDIR)/sefelt $(BINDIR)/setftime $(BINDIR)/sinter1 $(BINDIR)/sondat \
#          $(BINDIR)/sondpos $(BINDIR)/tetapv $(BINDIR)/tmfelt $(BINDIR)/tmfeltmega $(BINDIR)/vcdata \
#	   $(BINDIR)/vcinfo $(BINDIR)/wspecinfo \
#          $(BINDIR)/ddseqi2file $(BINDIR)/ddseqi4file \
#          $(BINDIR)/host_bigendian $(BINDIR)/do_seqfelt_swap $(BINDIR)/do_seqfile_swap \
#          $(BINDIR)/ukrelhum $(BINDIR)/uk1000 $(BINDIR)/ggtoxy1 \
#          $(BINDIR)/ftnsplit
 
#BINFILES= $(BINDIR)/binfdif2 $(BINDIR)/copflt $(BINDIR)/copfltmega $(BINDIR)/cressm $(BINDIR)/cvaflag \
#          $(BINDIR)/dig2felt $(BINDIR)/digpar $(BINDIR)/flt2flt $(BINDIR)/flt2fltmega $(BINDIR)/fltdif \
#          $(BINDIR)/flttst $(BINDIR)/obsread $(BINDIR)/fmfelt $(BINDIR)/fmfeltmega $(BINDIR)/fstrip \
#          $(BINDIR)/funpack $(BINDIR)/getftime $(BINDIR)/mergeclim $(BINDIR)/metdat \
#          $(BINDIR)/nyfelt $(BINDIR)/obslist $(BINDIR)/posdat $(BINDIR)/rfinh $(BINDIR)/rmcomment \
#          $(BINDIR)/sefelt $(BINDIR)/setftime $(BINDIR)/sinter1 $(BINDIR)/sondat \
#          $(BINDIR)/sondpos $(BINDIR)/tetapv $(BINDIR)/tmfelt $(BINDIR)/tmfeltmega $(BINDIR)/vcdata \
#	  $(BINDIR)/vcinfo $(BINDIR)/wspecinfo \
#          $(BINDIR)/ddseqi2file $(BINDIR)/ddseqi4file \
#          $(BINDIR)/host_bigendian $(BINDIR)/do_seqfelt_swap $(BINDIR)/do_seqfile_swap \
#          $(BINDIR)/ukrelhum $(BINDIR)/uk1000 $(BINDIR)/ggtoxy1 \
#          $(BINDIR)/ftnsplit $(BINDIR)/flt2v5d
     
all: prepare $(BINFILES)

#----------------------------------------------------------------------------------------------

$(BINDIR)/binfdif2: binfdif2.o
	$(FC) $(LDFLAGS) binfdif2.o $(LIBS) -o $@

$(BINDIR)/copflt: copflt.o copflts.o
	$(FC) $(LDFLAGS) copflt.o copflts.o $(LIBS) -o $@

$(BINDIR)/copfltmega: copflt.F copflts.F copflt.inc-mega
	sed -e s/copflt.inc/copflt.inc-mega/g copflt.F > copflt_mega_copy.F
	sed -e s/copflt.inc/copflt.inc-mega/g copflts.F > copflts_mega_copy.F
	$(FC) $(FCFLAGS) $(LDFLAGS) copflt_mega_copy.F copflts_mega_copy.F $(LIBS) -o $@
##	rm -f copflt_mega_copy.F copflts_mega_copy.F copflt_mega_copy.o copflts_mega_copy.o

$(BINDIR)/cressm: cressm.o
	$(FC) $(LDFLAGS) cressm.o $(LIBS) -o $@

$(BINDIR)/cvaflag: cvaflag.o
	$(FC) $(LDFLAGS) cvaflag.o $(LIBS) -o $@

$(BINDIR)/dig2felt: dig2felt.o
	$(FC) $(LDFLAGS) dig2felt.o $(LIBS) -o $@

$(BINDIR)/digpar: digpar.o
	$(FC) $(LDFLAGS) digpar.o $(LIBS) -o $@

$(BINDIR)/flt2flt: flt2flt.o
	$(FC) $(LDFLAGS) flt2flt.o $(LIBS) -o $@

$(BINDIR)/flt2fltmega: flt2flt.f flt2flt.inc-mega
	sed -e s/flt2flt.inc/flt2flt.inc-mega/g flt2flt.f > flt2flt_mega_copy.f
	$(FC) $(FCFLAGS) $(LDFLAGS) flt2flt_mega_copy.f $(LIBS) -o $@
##	rm -f flt2flt_mega_copy.f flt2flt_mega_copy.o

$(BINDIR)/fltdif: fltdif.o
	$(FC) $(LDFLAGS) fltdif.o $(LIBS) -o $@

$(BINDIR)/flttst: flttst.o
	$(FC) $(LDFLAGS) flttst.o $(LIBS) -o $@

$(BINDIR)/obsread: obsread.o
	$(FC) $(LDFLAGS) obsread.o $(LIBS) -o $@

$(BINDIR)/fmfelt: fmfelt.o
	$(FC) $(LDFLAGS) fmfelt.o $(LIBS) -o $@

$(BINDIR)/fmfeltmega: fmfelt.f fmfelt.inc-mega
	sed -e s/fmfelt.inc/fmfelt.inc-mega/g fmfelt.f > fmfelt_mega_copy.f
	$(FC) $(FCFLAGS) $(LDFLAGS) fmfelt_mega_copy.f $(LIBS) -o $@
##	rm -f fmfelt_mega_copy.f fmfelt_mega_copy.o

$(BINDIR)/fstrip: fstrip.o
	$(FC) $(LDFLAGS) fstrip.o $(LIBS) -o $@

$(BINDIR)/funpack: funpack.o
	$(FC) $(LDFLAGS) funpack.o $(LIBS) -o $@

$(BINDIR)/getftime: getftime.o
	$(FC) $(LDFLAGS) getftime.o $(LIBS) -o $@

$(BINDIR)/mergeclim: mergeclim.o
	$(FC) $(LDFLAGS) mergeclim.o $(LIBS) -o $@

$(BINDIR)/metdat: metdat.o
	$(FC) $(LDFLAGS) metdat.o $(LIBS) -o $@

$(BINDIR)/nyfelt: nyfelt.o
	$(FC) $(LDFLAGS) nyfelt.o $(LIBS) -o $@

$(BINDIR)/obslist: obslist.o
	$(FC) $(LDFLAGS) obslist.o $(LIBS) -o $@

$(BINDIR)/posdat: posdat.o
	$(FC) $(LDFLAGS) posdat.o $(LIBS) -o $@

$(BINDIR)/rfinh: rfinh.o
	$(FC) $(LDFLAGS) rfinh.o $(LIBS) -o $@

$(BINDIR)/rmcomment: rmcomment.o
	$(FC) $(LDFLAGS) rmcomment.o $(LIBS) -o $@

$(BINDIR)/sefelt: sefelt.o
	$(FC) $(LDFLAGS) sefelt.o $(LIBS) -o $@

$(BINDIR)/setftime: setftime.o
	$(FC) $(LDFLAGS) setftime.o $(LIBS) -o $@

$(BINDIR)/sinter1: sinter1.o siarnt.o sirut1.o sirut2.o simati.o sirobs.o
	$(FC) $(LDFLAGS) sinter1.o siarnt.o sirut1.o sirut2.o simati.o sirobs.o $(LIBS) -o $@

$(BINDIR)/sondat: sondat.o
	$(FC) $(LDFLAGS) sondat.o $(LIBS) -o $@

$(BINDIR)/sondpos: sondpos.o
	$(FC) $(LDFLAGS) sondpos.o $(LIBS) -o $@

$(BINDIR)/tetapv: tetapv.o
	$(FC) $(LDFLAGS) tetapv.o $(LIBS) -o $@

$(BINDIR)/tmfelt: tmfelt.o
	$(FC) $(LDFLAGS) tmfelt.o $(LIBS) -o $@

$(BINDIR)/tmfeltmega: tmfelt.f tmfelt.inc-mega
	sed -e s/tmfelt.inc/tmfelt.inc-mega/g tmfelt.f > tmfelt_mega_copy.f
	$(FC) $(FCFLAGS) $(LDFLAGS) tmfelt_mega_copy.f $(LIBS) -o $@
##	rm -f tmfelt_mega_copy.f tmfelt_mega_copy.o

$(BINDIR)/vcdata: vcdata.o vcdsub.o
	$(FC) $(LDFLAGS) vcdata.o vcdsub.o $(LIBS) -o $@

$(BINDIR)/vcinfo: vcinfo.o
	$(FC) $(LDFLAGS) vcinfo.o $(LIBS) -o $@

$(BINDIR)/wspecinfo: wspecinfo.o
	$(FC) $(LDFLAGS) wspecinfo.o $(LIBS) -o $@

$(BINDIR)/ddseqi2file: ddseqi2file.o
	$(CXX) $(LDFLAGS) ddseqi2file.o $(LIBS) -o $@

$(BINDIR)/ddseqi4file: ddseqi4file.o
	$(CXX) $(LDFLAGS) ddseqi4file.o $(LIBS) -o $@

$(BINDIR)/host_bigendian: host_bigendian.o
	$(FC) $(LDFLAGS) host_bigendian.o $(LIBS) -o $@

$(BINDIR)/do_seqfelt_swap: do_seqfelt_swap.o
	$(FC) $(LDFLAGS) do_seqfelt_swap.o $(LIBS) -o $@

$(BINDIR)/do_seqfile_swap: do_seqfile_swap.o
	$(FC) $(LDFLAGS) do_seqfile_swap.o $(LIBS) -o $@

$(BINDIR)/ukrelhum: ukrelhum.o
	$(FC) $(LDFLAGS) ukrelhum.o $(LIBS) -o $@

$(BINDIR)/uk1000: uk1000.o
	$(FC) $(LDFLAGS) uk1000.o $(LIBS) -o $@

$(BINDIR)/ggtoxy1: ggtoxy1.o
	$(FC) $(LDFLAGS) ggtoxy1.o $(LIBS) -o $@

$(BINDIR)/flt2v5d: flt2v5d.o putv5d.o
	$(FC) $(LDFLAGS) flt2v5d.o putv5d.o $(LIBS) $(V5DLIB) -o $@

$(BINDIR)/ftnsplit: ftnsplit.o
	$(FC) $(LDFLAGS) ftnsplit.o $(LIBS) -o $@

#----------------------------------------------------------------------------------------------

copflt.o: copflt.inc
copflts.o: copflt.inc
cressm.o: cressm.inc
dig2felt.o: dig2felt.inc
digpar.o: digpar.inc
flt2flt.o: flt2flt.inc
fltdif.o: fltdif.inc
flttst.o: flttst.inc
fmfelt.o: fmfelt.inc
metdat.o: metdat.inc
nyfelt.o: nyfelt.inc
posdat.o: posdat.inc
sefelt.o: sefelt.inc
sondat.o: sondat.inc
sondpos.o: sondpos.inc
tetapv.o: tetapv.inc tetapvtab.inc
tmfelt.o: tmfelt.inc
vcdata.o: vcdata.inc
vcdsub.o: vcdata.inc
vcinfo.o: vcdata.inc
ukrelhum.o: ukrelhum.inc
uk1000.o: uk1000.inc
ggtoxy1.o: ggtoxy1.inc
flt2v5d.o: flt2v5d.inc
putv5d.o: putv5d.f $(V5DINC)/v5df.h
	$(FC) -c $(FCFLAGS) -I$(V5DINC) putv5d.f

#----------------------------------------------------------------------------------------------

prepare:
	if [ ! -d $(BINDIR) ] ; then \
	  mkdir -pv $(BINDIR) ; \
	fi ; \

install:
	if [ ! -d $(DESTDIR)/$(DESTNAME) ] ; then \
	  mkdir -pv $(DESTDIR)/$(DESTNAME) ; \
	fi ; \
	for FILE in $(BINFILES) ; do \
	  cp -upv $$FILE $(DESTDIR)/$(DESTNAME) ; \
	done

install.all:
	@echo "[1;31m INSTALLING TO OSLO [0m"
	@make install DESTDIR=/oslo/metno/local
	@echo "[1;31m INSTALLING TO TROMS [0m"
	@make install DESTDIR=/troms/metno/local
	@echo "[1;31m INSTALLING TO BERGEN [0m"
	@make install DESTDIR=/bergen/metno/local

install.server:
	@echo "[1;31m INSTALLING TO TROMS [0m"
	@make install DESTDIR=/troms/metno/local
	@echo "[1;31m INSTALLING TO BERGEN [0m"
	@make install DESTDIR=/bergen/metno/local

clean:
	rm -f *.o
	rm -f *~
	rm -f *_mega_copy.*
