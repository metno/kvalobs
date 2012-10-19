c..include file for metdat.f
c
c        maxpos - posisjoner/stasjoner
c        maxpar - parametre
c        maxtid - tidspunkt
c        maxdat - total datamengde (npos*npar*ntid)
c        maxflt - st@rrelse av felt fra felt-file (nx*ny)
c        maxgrd - antall grid/sektorer input
c        mxpar  - ekstra parametre (ikke tids-avhengig,
c                                   posisjon og/eller 'parameter'-felt)
c
      parameter (maxpos=10000,maxtid=90,maxpar=200)
c
      parameter (mxpar=8)
c
c..mi_meteogram, +0 - +48  dt=2:    25 tidspunkt  10 parametre
c..ec_meteogram, +0 - +240 dt=6/12: 31 tidspunkt   6 parametre
c
      parameter (maxdat=5000000)
c
      parameter (maxflt=1200000,maxgrd=1)
