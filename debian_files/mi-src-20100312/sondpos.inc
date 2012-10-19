c..include file for sondpos.f
c
c  maxpos - max antall input        posisjoner/stasjoner
c  maxpar - max antall input/output parametre
c  maxniv - max antall input/output nivaa
c  maxtid - max antall input/output tidspunkt
c  maxdat - max total  input        datamengde (npos*npar*ntid)
c  mtekst - max antall input/output tekst-linjer
c  mposid - max antall input/output ekstra parametre (ikke tidsavhengig,
c                                   posisjon og/eller 'parameter'-felt)
c  mcont  - max antall input/output spesifikasjoner i header
c  midnum - max antall input/output ord i foerste header spesifikasjon
c  maxout - max antall output       posisjoner/stasjoner, en file
c  maxall - max antall output       posisjoner/stasjoner, alle filer
c  maxfil - max antall output       filer
c  mparprt- max antall "print_param=..."
c
c..input
      parameter (maxpos=1000,maxpar=15,maxniv=75,maxtid=75)
      parameter (maxdat=3000000)
      parameter (mtekst=10,mposid=8)
      parameter (mcont=40,midnum=20)
c
c..en output-file
      parameter (maxout=1000)
c
c..alle output-filer
      parameter (maxall=1000,maxfil=100)
c
      parameter (mparprt=50)
