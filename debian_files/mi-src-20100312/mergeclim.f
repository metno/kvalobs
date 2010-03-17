*deck mergeclim
      program mergeclim
c
c******************************************************************
c
c mergeclim - merge climate fields
c
c purpose:
c
c merge hirlam climate fields transfered from ecmwf with
c dnmi topography and anlysed sst, ice and snow.
c
c method:
c
c - read from dnmi parameter file the following fields:
c   > topography
c   > sst in degrees celcius
c   > matrix for surface types (0 = sea, 1 = sea ice, >1 = land)
c   > snowcover.
c - read from hirlam climate file the following fields:
c   > fraction of land
c   > fraction of ice
c   > albedo
c   > snow depth
c - compute new hirlam climate+analysed fields:
c   > dnmi topography
c   > dnmi sst from celcius to kelvin
c   > add dnmi analysed ice over hirlam seapoints
c   > add analysed snow over hirlam landpoints
c   > dnmi surface classes
c - overwrite the hirlam climate file with updates fields.
c
c externals:
c
c mrfelt  - read from felt-file
c mwfelt  - write to felt-file (incl. scaling)
c minmax  - compute mean, min and max values
c
c history:
c
c j.e. haugen   dnmi   june 95
c   d. bjoerge  dnmi   april 97
c   a. foss     dnmi   april 97
c j.e. haugen   dnmi   jan 98
c   v. oedegard dnmi   jan 2001
c   a. foss     dnmi   june 2005 ... float() -> real()
c
c******************************************************************
c
      implicit none
c
c declarations
c
      integer maxhor,mdata
c
      parameter (maxhor=75000)
      parameter (mdata=20+maxhor)
c
      real topo(maxhor),znull(maxhor),sst(maxhor),
     +     snowc(maxhor),snowd(maxhor),frl(maxhor),
     +     fri(maxhor),alb(maxhor)
c
      integer*2 in(16),idata(mdata),ident(20),ifelt(maxhor)
      equivalence (idata( 1),ident(1)),
     +            (idata(21),ifelt(1))
c
      integer ipack,luarg,lures,igrid,imonth,ierror,nx,ny,i,j,k,
     +        iprod
      integer narg,ios
      integer iargc
      real udef
      character*256 filarg,filres,cinput
c
ccc   data ipack /2/
      data ipack /1/
      data udef /1.e+35/
c
      luarg=10
      lures=20
c
c get input parameters:
c filarg - input dnmi parameter feltfile
c filres - input/output hirlam climate feltfile
c igrid  - grid number
c imonth - month (00-12)
c iprod  - producer number of input hirlam climate file
c
      narg=iargc()
      ierror=0
c
      if(narg.eq.5) then
	call getarg(1,filarg)
	call getarg(2,filres)
	call getarg(3,cinput)
	i=index(cinput,' ')
	if(i.gt.0) cinput(i:i)=char(0)
	read(cinput,*,iostat=ios) igrid
	if(ios.ne.0) igrid=-1
	if(igrid.lt.1 .or. igrid.gt.32767) ierror=1
	call getarg(4,cinput)
	i=index(cinput,' ')
	if(i.gt.0) cinput(i:i)=char(0)
	read(cinput,*,iostat=ios) imonth
	if(ios.ne.0) imonth=-1
	if(imonth.lt.1 .or. imonth.gt.12) ierror=1
	call getarg(5,cinput)
	i=index(cinput,' ')
	if(i.gt.0) cinput(i:i)=char(0)
	read(cinput,*,iostat=ios) iprod
	if(ios.ne.0) iprod=-1
	if(iprod.lt.1 .or. iprod.gt.99) ierror=1
      else
	ierror=1
      end if
c
      if(ierror.ne.0) then
	write(6,*) '  usage: mergeclim <par.dat> <clim.dat>',
     +		   ' <grid> <month> <producer.clim>' 
	stop
      end if
c
c read from dnmi parameter file
c -----------------------------
c
      write(*,*) '********** mergeclim:'
      write(*,*) '********** read from dnmi parameter file:'
c
c open file
      call mrfelt(1,filarg,luarg,in(1),ipack,1,1,1,1,1,ierror)
      if (ierror.ne.0) stop 'open error parameter file'
c
c prepare for input
      in(1)=88
      in(2)=igrid
      in(3)=-32767
      in(4)=-32767
      in(5)=-32767
      in(9)=4
      in(10)=0
      in(11)=2
      in(13)=1000
      in(14)=0
      in(15)=2
c
c read fields
c
c topography
      in(12)=101
      call mrfelt(2,filarg,luarg,in(1),ipack,maxhor,topo(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error topography'
c
c surface types
      in(12)=102
      call mrfelt(2,filarg,luarg,in(1),ipack,maxhor,znull(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error surface types'
c
c sea surface temperature
      in(12)=103
      call mrfelt(2,filarg,luarg,in(1),ipack,maxhor,sst(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error sst'
c
c snow cover
      in(12)=104
      call mrfelt(2,filarg,luarg,in(1),ipack,maxhor,snowc(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error snow cover'
c
cvo +++ snow depth on DNMI parameter files
c snow depth
      in(12)=105
      call mrfelt(2,filarg,luarg,in(1),ipack,maxhor,snowd(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error snow cover'
cvo --- snow depth on DNMI parameter files
c
c number of horisontal points
      nx=ident(10)
      ny=ident(11)
c
c compute and print min/max
      call minmax('topo    ',topo(1),nx,ny,udef)
      call minmax('znull   ',znull(1),nx,ny,udef)
      call minmax('sst     ',sst(1),nx,ny,udef)
      call minmax('snowc   ',snowc(1),nx,ny,udef)
      call minmax('snowd   ',snowd(1),nx,ny,udef)
c
c close file
      call mrfelt(3,filarg,luarg,in(1),ipack,1,1,1,1,1,ierror)
      if (ierror.ne.0) stop 'close error parameter file'
c
c read from from hirlam climate file
c ----------------------------------
c
      write(*,*) '********** read from hirlam climate file:'
c
c open file
      call mrfelt(1,filres,lures,in(1),ipack,1,1,1,1,1,ierror)
      if (ierror.ne.0) stop 'open error climate file'
c
c prepare for input
      in(1)=iprod
      in(2)=igrid
      in(3)=-32767
      in(4)=-32767
      in(5)=-32767
      in(9)=4
      in(10)=0
      in(11)=2
      in(13)=1000
      in(14)=0
      in(15)=2
c
c read fields
c
c fraction of land
      in(12)=181
      call mrfelt(2,filres,lures,in(1),ipack,maxhor,frl(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error fraction of land'
c
c fraction of ice
      in(12)=191
      call mrfelt(2,filres,lures,in(1),ipack,maxhor,fri(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error fraction of ice'
c
c albedo
      in(12)=184
      call mrfelt(2,filres,lures,in(1),ipack,maxhor,alb(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'read error albedo'
c
cvo +++ snow depth read from DNMI parameter file
c snow depth
c      in(9)=1
c      in(12)=66
c      call mrfelt(2,filres,lures,in(1),ipack,maxhor,snowd(1),1.,
c     +            mdata,idata(1),ierror)
c      if (ierror.ne.0) stop 'read error snow depth'
cvo ---
c
c number of horisontal points
      nx=ident(10)
      ny=ident(11)
c
c compute and print min/max
      call minmax('frl     ',frl(1),nx,ny,udef)
      call minmax('fri     ',fri(1),nx,ny,udef)
      call minmax('alb     ',alb(1),nx,ny,udef)
c     call minmax('snowd   ',snowd(1),nx,ny,udef)
c
c close file
      call mrfelt(3,filres,lures,in(1),ipack,1,1,1,1,1,ierror)
      if (ierror.ne.0) stop 'close error climate file'
c
c merge dnmi and hirlam fields
c ----------------------------
c
c redefine fraction of land for 1997-grid (Hirlam_0.1 T3E)
c in areas outside Scandinavia (approx. outside 2010-grid)
c
      if (igrid.eq.1997) then
c
      do j=1,ny
      do i=1,nx
         if (i.gt.100 .and. j.gt.100) goto 1000
            k = (j-1)*nx+i
            if (nint(znull(k)).le.1) then
               frl(k) = 0.
            else
               frl(k) = 100.
            endif
 1000    continue
      enddo
      enddo
c
      write(*,*) '***********************************************'
      write(*,*) 'FRACTION OF LAND RECOMPUTED OUTSIDE SCANDINAVIA'
      write(*,*) '***********************************************'
c
      endif
c
      do i=1,nx*ny
c sea surface temperature
         sst(i)=sst(i)+273.15
c
c add snow over landpoints
cvo +++ merging snow cover and snow depth to depth
c       snow depth is 15cm at snow cover 100% in input
c       (critical value adapted from HIRLAM phys routines radia.f and surf.f)
c       NB assuming that snowd is in unit m water equivalent in output
c       Albedo over snow is set in phys routine newalb.F
         if (snowd(i).le.15. .and. snowc(i).ge.10.) then
            snowd(i)=0.015*snowc(i)/100.
         else
            snowd(i)=snowd(i)*1.e-3
         endif
cvo ---
c
c add ice over seapoints
         if (frl(i).lt.5.) then
             if (nint(znull(i)).eq.1) then
                fri(i)=100.
cvo .. albedo over ice is set in HIRLAM phys routine newalb
c                alb(i)=70.
             else
                fri(i)=0.
cvo .. albedo over ice is set in HIRLAM phys routine newalb
c                alb(i)=10.
             endif
         endif
      enddo
c
c write to hirlam climate file
c -----------------------------
c
      write(*,*) '******** write to hirlam climate file:'
c
      call mwfelt(1,filres,lures,ipack,1,1,1,1,1,ierror)
      if (ierror.ne.0) stop 'open error climate file'
c
c prepare for output
      ident(1)=iprod
      ident(12)=0
      ident(13)=imonth*100+1
      ident(14)=0
c
c compute and print min/max
      call minmax('topo    ',topo(1),nx,ny,udef)
      call minmax('sst     ',sst(1),nx,ny,udef)
      call minmax('snowd   ',snowd(1),nx,ny,udef)
      call minmax('frl     ',frl(1),nx,ny,udef)
      call minmax('fri     ',fri(1),nx,ny,udef)
      call minmax('alb     ',alb(1),nx,ny,udef)
      call minmax('znull   ',znull(1),nx,ny,udef)
c
c write fields
c
c topography
      ident(3)=4
      ident(6)=101
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,topo(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error topography'
c
c sea surface temperature
      ident(3)=4
      ident(6)=103
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,sst(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error sst'
c
c snow depth
      ident(3)=1
      ident(6)=66
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,snowd(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error snow depth'
c
      if (igrid.eq.1997) then
c
c fraction of land
      ident(3)=4
      ident(6)=181
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,frl(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error fraction of land' 
c
      endif
c
c fraction of ice
      ident(3)=4
      ident(6)=191
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,fri(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error fraction of ice' 
c
c albedo
      ident(3)=4
      ident(6)=184
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,alb(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error albedo' 
c
c surfacer classes
      ident(3)=4
      ident(6)=102
      ident(20)=-32767
      call mwfelt(2,filres,lures,ipack,nx*ny,znull(1),1.,
     +            mdata,idata(1),ierror)
      if (ierror.ne.0) stop 'write error surface classes' 
c
c close file
      call mwfelt(3,filres,lures,ipack,1,1,1,1,1,ierror)
      if (ierror.ne.0) stop 'close error climate file'
c
      stop
      end
*deck minmax
      subroutine minmax (
     +   kname, pa, klon, klat, udef )
c
c**********************************************************************
c
c     minmax - statistics computations
c
c  purpose:
c
c     compute and print
c
c     mean-value
c     minimum -value and position
c     maximum -value and position
c
c  input:
c
c     kname     message to be printed
c     pa        array with data
c     klon      number of gridpoints in x-direction
c     klat      number of gridpoints in y-direction
c     udef      skip points with value udef
c
c  history:
c
c     j.e. haugen/hirlam     010290
c
c**********************************************************************
c
      character*8 kname
      real pa(klon*klat)
c
      zsum = 0.
      isum = 0
      zmin =  1.e+35
      zmax = -1.e+35
c
      do i = 1,klon*klat
         if (pa(i).ne.udef) then
            zsum = zsum + pa(i)
            isum = isum + 1
            if (pa(i).lt.zmin) then
               zmin = pa(i)
               imin = i
            endif
            if (pa(i).gt.zmax) then
               zmax = pa(i)
               imax = i
            endif
         endif
      enddo
c
      if (isum.gt.0) then
         zsum = zsum/real(isum)
         zmin = pa(imin)
         zmax = pa(imax)
         iymin = (imin-1)/klon + 1
         ixmin =  imin-(iymin-1)*klon
         iymax = (imax-1)/klon + 1
         ixmax =  imax-(iymax-1)*klon
      else
         zsum = udef
         zmin = udef
         zmax = udef
         iymin = 0
         ixmin = 0
         iymax = 0
         ixmax = 0
      endif
c
      write(*,'(1x,a8)') kname
      write(*,'(1x,''mean-value: '',g20.8,'' in '',i5,''/'',i5)')
     +      zsum,isum,klon*klat
      write(*,'(1x,''min -value: '',g20.8,'' at (x,y) = '',2i5)')
     +      zmin,ixmin,iymin
      write(*,'(1x,''max -value: '',g20.8,'' at (x,y) = '',2i5)')
     +      zmax,ixmax,iymax
c
      return
      end
