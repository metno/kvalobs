c..include file for fltcop.f
c
c  maxij:  max field size
c
      parameter (maxij=200000)
c
      parameter (maxset=50,mstore=30000,maxgrd=24,mloops=16)
      parameter (mchang=80,mchnga=20,mintpm=80,mxyvec=40,mcompu=16)
      parameter (maddpa=20,maddpt=100)
      parameter (mreadf=40)
c
c  don't change the following ......................................
c
      parameter (limit=20+maxij)
c
      parameter (mwrec=1024,minrec=64)
      parameter (mwrech=mwrec/2)
