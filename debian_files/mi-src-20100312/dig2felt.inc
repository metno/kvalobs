c  dig2felt.inc : include file for dig2felt.f
c
c  maxsiz: max field size
c  mfwork: max no. of work fields
c  maxlin: max no. of lines     from line file
c  maxpos: max no. of positions from line file
c  maxone: max no. of positions in one line
c  maxint: max no. of inetersections between lines and
c          a straight line through the grid (plus coast points)
c  maxlinice: for "transfer" from ice to sst
c  maxposice: for "transfer" from ice to sst
c
      parameter (maxsiz=300000)
      parameter (mfwork=6)
      parameter (maxlin=5000,maxpos=100000,maxone=10000)
      parameter (maxint=1000)
      parameter (maxlinice=200,maxposice=maxone*2)
c
      parameter (undef=+1.e+35)
