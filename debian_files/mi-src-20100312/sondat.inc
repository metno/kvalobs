c..include file for sondat.f
c
c        maxpos - posisjoner/stasjoner
c        maxpar - parametre
c        maxniv - niv$
c        maxtid - tidspunkt
c        maxdat - total datamengde (npos*npar*nniv*ntid)
c        maxflt - st@rrelse av felt fra felt-file (nx*ny)
c        maxgrd - antall grid/sektorer input
c
      parameter (maxpos=1000,maxpar=15,maxniv=75,maxtid=75)
c
      parameter (maxdat=3000000)
c
      parameter (maxflt=300000,maxgrd=1)
