c  tetapv.inc ... include file for tetapv.f
c
c  max2d  : max size of 2d-fields (nx*ny)
c  max3d  : max size of 3d-fields (nx*ny*kk) in sigma/eta levels
c  kkmax  : max no. of input sigma/eta levels
c  mpvlev : max no. of output pv levels (p(pv=constant) fields)
c  mtclev : max no. of output t  levels (p,z(tt=constant) fields)
c
      integer max2d,max3d,kkmax,mpvlev,mtclev
c
      parameter (max2d=200000,max3d=200000*60)
      parameter (kkmax=100,mpvlev=6,mtclev=20)
