c  tetapvtab.inc ... include file for tetapv.f
c
      integer mpitab,mpptab
      real    dptab,dpinv,dpitab,dpiinv
c
      parameter (mpitab=400)
      parameter (dptab=5.0)
      parameter (dpinv=1.0/dptab)
c
      parameter (mpptab=800)
      parameter (dpitab=1.5)
      parameter (dpiinv=1.0/dpitab)
c
      real pitab,pktab,pptab
c
      common/tables/pitab(0:mpitab),pktab(0:mpitab)
     +             ,pptab(0:mpptab)
c
      real    xt
      integer lt
