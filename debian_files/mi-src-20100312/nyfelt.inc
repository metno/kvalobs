c..include file for nyfelt.f
c
c..maxij - max dimension of fields (for copy and update)
c
      parameter (maxij=1200000)
c
      parameter (maxset=50,mstore=30000,maxgrd=24,mloops=24)
c
      parameter (mdefloop=100,mloopdef=2000)
c
c..don't change the following:
c
      parameter (limit=20+maxij+100)
c
      parameter (nwrec=1024,ninrec=64)
c
      common/a/ntim(2,maxset),nlev(2,maxset),npar(2,maxset),
     *         itimef(5),ntstep,itstep(5),ngrid,igrid(3,maxgrd),
     *         imode(mloops),
     *         ioverw(mloops),ictime(mloops),info(mloops),
     *         numgrd(2,mloops),numset(2,mloops),
     *         idrec1(1024),idrec2(2,512),idata(limit),
     *         istore(2,mstore),
     *         fileot,filein(mloops),filnam
      integer*2 idrec1,idrec2,idata,istore
      character*256 fileot,filein,filnam
