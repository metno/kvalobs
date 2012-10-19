c  include file for digpar.f
c
c  maxsiz: max field size (incl. extra geometry identification)
c  maxtst: max no. test specifications
c  maxlin: max no. lines     from line file
c  maxpos: max no. positions from line file
c  maxfil: max no. files
c
      parameter (maxsiz=300000+50)
      parameter (maxtst=100)
      parameter (maxlin=5000,maxpos=100000)
      parameter (maxfil=32)
