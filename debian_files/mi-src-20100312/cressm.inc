c  include file for cressm.f
c
c  maxsiz: max field size
c  maxobs: max no. of observations or line positions
c  mxscan: max no. scans in each analysis
c  maxfil: max no. files
c  maxtab: table size for weights
c  ldata:  max size of buffer for field i/o
c
      parameter (maxsiz=300000)
      parameter (maxobs=50000)
      parameter (mxscan=24)
      parameter (maxfil=16)
      parameter (maxtab=2500)
      parameter (ldata=20+maxsiz+50)
