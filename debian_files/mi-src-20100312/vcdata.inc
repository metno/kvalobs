c..vcdata.inc ..... include file for vcdata.f
c
c  maxij   : max input field size
c  maxd    : max data length (multi  level parameters), one timestep
c  maxd1   : max data length (single level parameters), one timestep
c  maxlev  : max no. of levels
c  maxpar  : max no. of multi  level parameters
c  maxpr1  : max no. of single level parameters
c  maxcrs  : max no. of crossections
c  maxtim  : max no. of timesteps
c  maxinp  : max no. of input positions, all crossections
c  maxpos  : max no. of positions along all crossections
c  maxflt  : max no. of fields for misc. horizontal computations
c  mindat  : max length of read field buffer
c  mparlim : max no. of parameter with min/max limits (after interpolation) 
c
      parameter (maxij=610000)
      parameter (maxd=10000000,maxd1=1000000)
      parameter (maxlev=90,maxpar=40,maxpr1=15)
      parameter (maxcrs=80,maxtim=250)
      parameter (maxinp=500)
      parameter (maxpos=30000)
      parameter (maxflt=14)
      parameter (mparlim=50)
c
      parameter (mindat=20+maxij+50)
