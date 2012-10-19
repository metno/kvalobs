c..include file for copflt.f
c
c  maxij:  max field size
c
      parameter (maxij=1200000)
c
      parameter (maxset=50,mstore=30000,maxgrd=24,mloops=16)
      parameter (mchang=80,mchnga=20,mintpm=80,mxyvec=40,mcompu=16)
      parameter (maddpa=20,maddpt=100,macspa=12)
      parameter (mreadf=40)
c
      parameter (mpkscl=40,mpkdir=16,mpknot=16)
      parameter (maxncp=32,mpsmooth=100,mpasmpsc=32)
c
      parameter (mdefloop=100,mloopdef=2000)
c
c  don't change the following ......................................
c
      parameter (limit=20+maxij)
c
      parameter (mwrec=1024,minrec=64)
      parameter (mwrech=mwrec/2)
