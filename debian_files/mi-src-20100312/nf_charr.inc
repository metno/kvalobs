      integer maxarrs,maxchars
      parameter (maxarrs = 500)
      parameter (maxchars = 200000)
      common /charr/ arrcount,freeptr,arrs(2,maxarrs),chars
      integer arrcount,freeptr,arrs
      character*200000 chars
