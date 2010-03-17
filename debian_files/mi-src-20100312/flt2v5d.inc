c..flt2v5d.inc .... include file for flt2v5d.f
c
c..max2d  : max size of 2d fields
c..max3d  : max size of 3d fields
c..maxpar : max no. of parameters
c..maxlev : max no. of levels
c..maxtim : max no. of timesteps
c
      parameter (max2d=468*378)
      parameter (max3d=max2d*40)
      parameter (maxpar=25)
      parameter (maxlev=50)
      parameter (maxtim=50)
c
      parameter (ldata=20+max2d+50)
      parameter (maxinh=512)
