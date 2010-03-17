c..include file for posdat.f
c
c  maxpos - max antall input        posisjoner/stasjoner
c  maxpar - max antall input/output parametre
c  maxtid - max antall input/output tidspunkt
c  maxdat - max total  input        datamengde (npos*npar*ntid)
c  mtekst - max antall input/output tekst-linjer
c  mxpar  - max antall input/output ekstra parametre (ikke tidsavhengig,
c                                   posisjon og/eller 'parameter'-felt)
c  maxout - max antall output       posisjoner/stasjoner, en file
c  maxdt1 - max total  output       datamengde (npos*npar*ntid), en file
c  maxall - max antall output       posisjoner/stasjoner, alle filer
c  maxfil - max antall output       filer
c  madfil - max antall input        filer for 'add.data.from.file'
c  madpar - max antall input        parametre for 'add.parameter'
c  mparprt- max antall "print_param=..."
c
c..input
      parameter (maxpos=10000,maxpar=200,maxtid=90)
      parameter (maxdat=5000000)
      parameter (mtekst=10,mxpar=8)
      parameter (madfil=8,madpar=16)
c
c..en output-file
      parameter (maxout=10000)
      parameter (maxdt1=5000000)
c
c..alle output-filer
      parameter (maxall=10000,maxfil=1000)
c
      parameter (mparprt=50)
