      program dig2felt
c
c	Input:  Digitized lines of Sea Surface Temperature,
c	        Ice edge (closed lines), Snow coverage (closed lines)
c               and Snow depth (closed lines).
c	        Fields with surface types (land/ice/sea) and
c	        topography.
c	Output: Fields with updated surface types (land/ice/sea),
c		sea surface temperature (Celsius), snow coverage and
c		snow depth.
c
c	Fields are read from and written to the same Felt file.
c
c----------------------------------------------------------------------
c  DNMI library subroutines: mrfelt
c                            mwfelt
c                            lenstr
c                            gridpar
c                            xyconvert
c                            mapfield
c
c----------------------------------------------------------------------
c  DNMI/FoU   1989-1997  Anstein Foss ... cressm and digpar programs
c  DNMI/FoU  28.08.1998  Anstein Foss ... new SST analysis method
c  DNMI/FoU  03.12.1998  Anstein Foss ... transfer ice edge to sst
c  DNMI/FoU  05.12.2000  J. E. Haugen ... add snow depth
c  DNMI/FoU  18.02.2001  Anstein Foss ... -a3 (sst for ecom3d)
c  DNMI/FoU  17.09.2001  Anstein Foss ... -a4 (for old datasets)
c  DNMI/FoU  30.10.2003  Anstein Foss ... very minor update
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c----------------------------------------------------------------------
c
      include 'dig2felt.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c  dig2felt.inc : include file for dig2felt.f
c
c  maxsiz: max field size
c  mfwork: max no. of work fields
c  maxlin: max no. of lines     from line file
c  maxpos: max no. of positions from line file
c  maxone: max no. of positions in one line
c  maxint: max no. of inetersections between lines and
c          a straight line through the grid (plus coast points)
c
ccc   parameter (maxsiz=100000)
ccc   parameter (mfwork=4)
ccc   parameter (maxlin=5000,maxpos=100000,maxone=3000)
ccc   parameter (maxint=1000)
c
ccc   parameter (undef=+1.e+35)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      parameter (ldata=20+maxsiz+50)
c
      integer*2 idfile(32),idata(ldata)
c
      real    fland(maxsiz),topo(maxsiz),tsea(maxsiz),snowc(maxsiz),
     +        snowd(maxsiz)
      real    temp(maxsiz),fwork(maxsiz,mfwork)
      integer iarea(maxsiz),iwork(maxsiz)
c
      integer nlin,npos,igscal,ivscal
      integer iline(3,maxlin)
      real    xpos(maxpos),ypos(maxpos)
c
c..for "transfer" of ice edge lines to sst.............
      integer nlinice,nposice,igscalice,ivscalice
      integer ilineice(3,maxlinice)
      real    xposice(maxposice),yposice(maxposice)
c......................................................
c
      character*256 feltfile,icefile,sstfile,snowfile,sndepfile
      character*256 text
c
      integer       iunit
      integer       itime(5)
      character*256 parnam,rstatus
c
      integer igeogrid,igtype
      real    geogrid(6),grid(6)
c
      integer*2 in(16)
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=4)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=6)
      integer       nargs
      character*256 cargs(margs)
      integer       mispec
      parameter    (mispec=10)
      integer       ispec(mispec)
      integer       mrspec
      parameter    (mrspec=1)
      real          rspec(mrspec)
      integer       mcspec
      parameter    (mcspec=1)
      character*1   cspec(mcspec)
      integer       ierror
      integer       nerror
c..cmdarg................................................
c
c..cmdarg................................................
      data copt/'a','d','g','s'/
      data iopt/ 1 , 1 , 1 , 1 /
c..cmdarg........1...2...3...4...........................
c
      data igeogrid/2/
      data geogrid/1.,1.,1.,1.,0.,0./
c
c..cmdarg................................................
c..get command line arguments
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c..cmdarg................................................
c
c..defaults
      ndirec=24
      nsmooths=1
      nsmoothl=3
c
      if(ierror.eq.0) then
c..-a
	if(iopts(1,1).ne.1) then
	  ierror=999
	else
	  idigit=ispec(iopts(2,1))
	  if(idigit.lt.1 .or. idigit.gt.4) ierror=999
	end if
c..-d
	if(iopts(1,2).eq.1) then
	  ndirec=ispec(iopts(2,2))
	  if(ndirec.lt.2 .or. ndirec.gt.360) ierror=999
	elseif(iopts(1,2).ne.0) then
	  ierror=999
	end if
c..-g
	if(iopts(1,3).ne.2) then
	  ierror=999
	else
	  iprod=ispec(iopts(2,3))
	  igrid=ispec(iopts(2,3)+1)
	  if(iprod.lt.1 .or. iprod.gt.99) ierror=999
	  if(igrid.lt.-32766 .or. igrid.gt.32767) ierror=999
	end if
c..-s
	if(iopts(1,4).eq.2) then
	  nsmooths=ispec(iopts(2,4))
	  nsmoothl=ispec(iopts(2,4)+1)
	  if(nsmooths.lt.0 .or. nsmooths.gt.100) ierror=999
	  if(nsmoothl.lt.0 .or. nsmoothl.gt.100) ierror=999
	elseif(iopts(1,4).ne.0) then
	  ierror=999
	end if
c
	if(ierror.eq.0) then
	  if(idigit.eq.1 .and. nargs.ne.5) ierror=999
	  if(idigit.eq.2 .and. nargs.ne.2) ierror=999
	  if(idigit.eq.3 .and. nargs.ne.3) ierror=999
	  if(idigit.eq.4 .and. nargs.ne.4) ierror=999
	end if
c
      end if
c
      if(ierror.ne.0) then
        write(6,*)
        write(6,*) 'usage:  dig2felt -a1 [options] parfelt.dat',
     +			' icedig.dat sstdig.dat snowdig.dat sndepdig.dat'
        write(6,*) '   or:  dig2felt -a2 [options]',
     +			' icefelt.dat icedig.dat'
        write(6,*) '   or:  dig2felt -a3 [options]',
     +			' sstfelt.dat icedig.dat sstdig.dat'
        write(6,*) '   or:  dig2felt -a4 [options] parfelt.dat',
     +			' icedig.dat sstdig.dat snowdig.dat'
        write(6,*)
        write(6,*) ' options:    (required: -a -g)'
        write(6,*) '  -a1 : make parameter fields for Norlam/Hirlam'
        write(6,*) '  -a2 : make ice field for Wam'
        write(6,*) '  -a3 : make SST field for Ecom3d/Mipom'
        write(6,*) '  -a4 : make parameter fields for Norlam/Hirlam ',
     +			' (no snowdepth)'
        write(6,*) '  -d <num_direc> : no. of directions in SST',
     +			' analysis (default is 24)'
        write(6,*) '  -g <producer>,<grid> : producer no. and grid no.'
        write(6,*) '  -s <num_smooth_T.sea>,<num_smooth_T.land>',
     +				      ' (default is 1,3)'
        write(6,*)
        stop
      end if
c
      feltfile=cargs(1)
      icefile =cargs(2)
      if(idigit.eq.1) then
	sstfile =cargs(3)
	snowfile=cargs(4)
	sndepfile=cargs(5)
      elseif(idigit.eq.3) then
	sstfile =cargs(3)
      elseif(idigit.eq.4) then
	sstfile =cargs(3)
	snowfile=cargs(4)
      end if
c
      iunit=20
c
      do i=1,16
	in(i)=-32767
      end do
c
      in( 1)=iprod
      in( 2)=igrid
      in( 9)=4
      in(10)=0
      ipack=2
c
      nlinice=0
      nposice=0
c
      if(idigit.eq.1 .or. idigit.eq.4) then
c
	in(11)=2
	in(13)=1000
	in(14)=0
c
c..read land/ice/sea matrix (0=sea 1=ice 2,3,...=land)
	in(12)=102
	call mrfelt(0,feltfile,iunit,in,ipack,maxsiz,fland,1.0,
     +              ldata,idata,ierror)
	if(ierror.ne.0) write(6,*) 'MRFELT ERROR'
	if(ierror.ne.0) call exit(2)
c
c..read topography
	in(12)=101
	call mrfelt(0,feltfile,iunit,in,ipack,maxsiz,topo,1.0,
     +              ldata,idata,ierror)
	if(ierror.ne.0) write(6,*) 'MRFELT ERROR'
	if(ierror.ne.0) call exit(2)
c
      elseif(idigit.eq.2) then
c
	in(11)=3
	in(13)=0
	in(14)=0
c
c..read land/ice/sea matrix (0=sea/land 1=ice)
	in(12)=293
	call mrfelt(0,feltfile,iunit,in,ipack,maxsiz,fland,1.0,
     +              ldata,idata,ierror)
	if(ierror.ne.0) write(6,*) 'MRFELT ERROR'
	if(ierror.ne.0) call exit(2)
c
      elseif(idigit.eq.3) then
c
	in(11)=8
	in(13)=0
	in(14)=0
c
c..read ocean bottom topography (undefined value = land)
	in(12)=351
	call mrfelt(0,feltfile,iunit,in,ipack,maxsiz,topo,1.0,
     +              ldata,idata,ierror)
	if(ierror.ne.0) write(6,*) 'MRFELT ERROR'
	if(ierror.ne.0) call exit(2)
c
      end if
c
c..get grid specifications
      call gridpar(+1,ldata,idata,igtype,nx,ny,grid,ierror)
      if(ierror.ne.0) write(6,*) 'GRIDPAR ERROR'
      if(ierror.ne.0) call exit(2)
c
      if(idigit.ne.2) then
c..find average grid resolution
	call mapfield(1,0,igtype,grid,nx,ny,
     +		      fwork(1,1),fwork(1,2),fc,hx,hy,ierror)
	if(ierror.ne.0) write(6,*) 'MAPFIELD ERROR'
	if(ierror.ne.0) call exit(2)
	xmavg=0.
	ymavg=0.
	do ij=1,nx*ny
	  xmavg=xmavg+fwork(ij,1)
	  ymavg=ymavg+fwork(ij,2)
	end do
	xmavg=xmavg/real(nx*ny)
	ymavg=ymavg/real(nx*ny)
	gresol=((hx/xmavg)+(hy/ymavg))*0.5
      end if
c
      if(idigit.eq.3) then
c..for ocean SST
	do ij=1,nx*ny
	  if (topo(ij).ne.undef) then
	    fland(ij)= 0.
	  else
	    fland(ij)= 2.
	  end if
	  topo(ij)=0.
	  tsea(ij)=0.
	end do
      end if
c
c...................................................................
c
c..ice
      call rline(icefile,iunit,parnam,itime,igscal,ivscal,
     +		 maxpos,maxlin,npos,nlin,iline,ypos,xpos,
     +		 rstatus,ierror)
      if(ierror.ne.0) write(6,*) 'RLINE ERROR'
      if(ierror.ne.0) call exit(2)
c
c.."transfer" of ice edge lines to sst.................
      if(idigit.ne.2) then
	nlinice=min(nlin,maxlinice)
	nposice=min(npos,maxposice)
	igscalice=igscal
	ivscalice=ivscal
	nl=0
	do l=1,nlinice
	  ilineice(1,l)=iline(1,l)
	  ilineice(2,l)=iline(2,l)
	  ilineice(3,l)=iline(3,l)
	  if(ilineice(2,l).le.maxposice) nl=l
	end do
	nlinice=nl
	nposice=ilineice(2,nl)
	do k=1,nposice
	  xposice(k)=xpos(k)
	  yposice(k)=ypos(k)
	end do
      end if
c
c......................................................
c
      call xyconvert(npos,xpos,ypos,igeogrid,geogrid,
     +		     igtype,grid,ierror)
      if(ierror.ne.0) write(6,*) 'XYCONVERT ERROR'
      if(ierror.ne.0) call exit(2)
c
      vscale=10.**ivscal
c
      call ice(nx,ny,fland,iarea,iwork,
     +	       nlin,iline,npos,xpos,ypos,vscale)
c
      if(idigit.eq.2) goto 800
c
      if(idigit.eq.3) goto 200
c
c...................................................................
c
c..snowcover
      call rline(snowfile,iunit,parnam,itime,igscal,ivscal,
     +		 maxpos,maxlin,npos,nlin,iline,ypos,xpos,
     +		 rstatus,ierror)
      if(ierror.ne.0) write(6,*) 'RLINE ERROR'
      if(ierror.ne.0) call exit(2)
c
      call xyconvert(npos,xpos,ypos,igeogrid,geogrid,
     +		     igtype,grid,ierror)
      if(ierror.ne.0) write(6,*) 'XYCONVERT ERROR'
      if(ierror.ne.0) call exit(2)
c
      vscale=10.**ivscal
c
      call snow(nx,ny,snowc,fland,iarea,iwork,
     +	        nlin,iline,npos,xpos,ypos,vscale)
c
      if(idigit.eq.4) goto 200
c
c...................................................................
c
c..snowdepth
      call rline(sndepfile,iunit,parnam,itime,igscal,ivscal,
     +		 maxpos,maxlin,npos,nlin,iline,ypos,xpos,
     +		 rstatus,ierror)
      if(ierror.ne.0) write(6,*) 'RLINE ERROR'
      if(ierror.ne.0) call exit(2)
c
      call xyconvert(npos,xpos,ypos,igeogrid,geogrid,
     +		     igtype,grid,ierror)
      if(ierror.ne.0) write(6,*) 'XYCONVERT ERROR'
      if(ierror.ne.0) call exit(2)
c
      vscale=10.**ivscal
c
      call sndep(nx,ny,snowd,fland,iarea,iwork,
     +	         nlin,iline,npos,xpos,ypos,vscale)
c
c...................................................................
c
  200 continue
c
c..sst
      call rline(sstfile,iunit,parnam,itime,igscal,ivscal,
     +		 maxpos,maxlin,npos,nlin,iline,ypos,xpos,
     +		 rstatus,ierror)
      if(ierror.ne.0) write(6,*) 'RLINE ERROR'
      if(ierror.ne.0) call exit(2)
c
c.."transfer" of ice edge lines to sst.................
      newice=0
      nfound=0
      if(nlinice.gt.0 .and. nposice.gt.0) then
c..may have been added before
	vscale=10.**(-ivscal)
	tice=-2.
	ivalue=nint(tice*vscale)
	nlinsst=nlin
	do nl=1,nlinice
	  k1=ilineice(1,nl)-1
	  k2=ilineice(2,nl)
	  kk=k2-k1
	  ifound=0
	  do l=1,nlinsst
	    if(iline(2,l)-iline(1,l)+1.eq.kk) then
	      k2=iline(1,l)-1
	      new=0
	      do k=1,kk
		if(xposice(k1+k).ne.xpos(k2+k) .or.
     +		   yposice(k1+k).ne.ypos(k2+k)) new=1
	      end do
	      if(new.eq.0) ifound=1
	    end if
	  end do
	  nfound=nfound+ifound
	  if(ifound.eq.0 .and. nlin+1 .le.maxlin
     +			 .and. npos+kk.le.maxpos) then
	    nlin=nlin+1
	    iline(1,nlin)=npos+1
	    iline(2,nlin)=npos+kk
	    iline(3,nlin)=ivalue
	    do k=1,kk
	      xpos(npos+k)=xposice(k1+k)
	      ypos(npos+k)=yposice(k1+k)
	    end do
	    npos=npos+k
	    newice=newice+1
	  end if
	end do
      end if
      if(newice.gt.0) then
	write(6,*) newice,' ice edge lines added to sst data, t=',tice
	write(6,*) '    (',nfound,' matching lines)'
      else
	write(6,*) 'No ice edge lines added to sst data'
	write(6,*) '    (',nfound,' matching lines)'
      end if
c
c......................................................
c
      call xyconvert(npos,xpos,ypos,igeogrid,geogrid,
     +		     igtype,grid,ierror)
      if(ierror.ne.0) write(6,*) 'XYCONVERT ERROR'
      if(ierror.ne.0) call exit(2)
c
      vscale=10.**ivscal
c
      call sst(nx,ny,fland,topo,gresol,
     +	       ndirec,nsmooths,nsmoothl,
     +	       nlin,iline,npos,xpos,ypos,vscale,
     +	       mfwork,fwork,temp,tsea)
c
c...................................................................
c
  800 continue
c
c..open felt file
      call mwfelt(1,feltfile,iunit,1,1,1.,1.,32,idfile,ierror)
      if(ierror.ne.0) write(6,*) 'MWFELT ERROR'
      if(ierror.ne.0) call exit(2)
c
      if(idigit.eq.1 .or. idigit.eq.4) then
c
c..write land/sea/ice matrix
	idata( 6)=102
	idata(20)=0
	call mwfelt(2,feltfile,iunit,ipack,nx*ny,fland,
     +              1.0,ldata,idata,ierror)
c
c..write SST (T.sea + T.land, deg. Celsius)
	idata( 6)=103
	idata(20)=-32767
	call mwfelt(2,feltfile,iunit,ipack,nx*ny,tsea,
     +              1.0,ldata,idata,ierror)
c
c..write snow coverage (%)
	idata( 6)=104
	idata(20)=0
	call mwfelt(2,feltfile,iunit,ipack,nx*ny,snowc,
     +              1.0,ldata,idata,ierror)
c
	if (idigit.ne.4) then
c..write snow depth (cm)
	  idata( 6)=105
	  idata(20)=0
	  call mwfelt(2,feltfile,iunit,ipack,nx*ny,snowd,
     +                1.0,ldata,idata,ierror)
	end if
c
      elseif(idigit.eq.2) then
c
c..write land/sea/ice matrix
	idata( 6)=293
	idata(20)=0
	call mwfelt(2,feltfile,iunit,ipack,nx*ny,fland,
     +              1.0,ldata,idata,ierror)
c
      elseif(idigit.eq.3) then
c
c..write SST (T.sea + T.land, deg. Celsius)
	idata( 6)=308
	idata(20)=-32767
	call mwfelt(2,feltfile,iunit,ipack,nx*ny,tsea,
     +              1.0,ldata,idata,ierror)
c
      end if
c
c..close felt file
      call mwfelt(3,feltfile,iunit,1,1,1.,1.,32,idfile,ierror)
c
      end
c
c***********************************************************************
c
      subroutine sst(nx,ny,fland,topo,gresol,
     +		     ndirec,nsmooths,nsmoothl,
     +		     nlin,iline,npos,xpos,ypos,vscale,
     +		     nfwork,fwork,temp,tsea)
c
      include 'dig2felt.inc'
c
c..input/output
      integer nx,ny,nlin,npos
      integer ndirec,nsmooths,nsmoothl
      integer iline(3,nlin)
      real    gresol,vscale
      real    xpos(npos),ypos(npos)
      real    fland(nx,ny),topo(nx,ny)
      real    fwork(nx,ny,nfwork)
      real    temp(nx,ny),tsea(nx,ny)
c
      if(nx.lt.1 .or. ny.lt.1 .or. nfwork.lt.6) then
	write(6,*) 'SSTANA ERROR'
	write(6,*) '   nx,ny,nfwork: ',nx,ny,nfwork
	call exit(5)
      end if
c
      write(6,*) 'input: nlin,npos: ',nlin,npos
c
      call joinline(nx,ny,fland,gresol,
     +		    nlin,iline,npos,xpos,ypos)
c
      write(6,*) 'used:  nlin,npos: ',nlin,npos
c
      do j=1,ny
	do i=1,nx
	  tsea(i,j)   = undef
	  fwork(i,j,5)=-undef
	  fwork(i,j,6)=+undef
	end do
      end do
c
      dangle=180./real(ndirec)
c
      do n=1,ndirec
c
	angle=-90.+n*dangle
c
	write(6,*) 'SST analysis ... angle= ',angle
c
	do k=1,4
          do j=1,ny
	    do i=1,nx
	      fwork(i,j,k)=undef
	    end do
	  end do
	end do
c
	call ppcross(angle,nx,ny,fland,fwork(1,1,1),gresol,
     +		     nlin,iline,npos,xpos,ypos,vscale)
c
c..fwork(i,j,1) and  tsea(i,j)   : temperature
c..fwork(i,j,2) and fwork(i,j,5) : temp. gradient
c..fwork(i,j,3) and fwork(i,j,6) : distance for gradient computation
c..				   or distance from nearest point if
c..				   the gradient is 0
c..				   (the smaller the better)
c..fwork(i,j,4)                  : work array in ppcross/ppinter
c
	do j=1,ny
	  do i=1,nx

	    if(fwork(i,j,1).ne.undef) then
	      if(tsea(i,j).eq.undef) then
		ibest=1
	      elseif(fwork(i,j,2).gt.0. .and. fwork(i,j,5).gt.0. .and.
     +		     fwork(i,j,3).lt.fwork(i,j,6)) then
		ibest=1
	      elseif(fwork(i,j,2).eq.0. .and. fwork(i,j,5).eq.0. .and.
     +		     fwork(i,j,3).lt.fwork(i,j,6)) then
		ibest=1
	      elseif(fwork(i,j,2).gt.0. .and. fwork(i,j,5).eq.0.) then
		ibest=1
	      else
		ibest=0
	      end if
	      if(ibest.eq.1) then
	        tsea(i,j)   =fwork(i,j,1)
	        fwork(i,j,5)=fwork(i,j,2)
	        fwork(i,j,6)=fwork(i,j,3)
	      end if
	    end if
	  end do
	end do
c
      end do
c
c...................................................................
c
      tmin=+undef
      tmax=-undef
      nundef=0
      do j=1,ny
	do i=1,nx
	  if(tsea(i,j).ne.undef) then
	    tmin=min(tmin,tsea(i,j))
	    tmax=max(tmax,tsea(i,j))
	  else
	    nundef=nundef+1
	  end if
	end do
      end do
      write(6,*) 'temp  min,max,nundef: ',tmin,tmax,nundef
c
c...................................................................
c
c..fill remaining sea points (from sea neighbours)
      missing=1
      nset=1
      nloop=0
      do while (missing.gt.0 .and. nset.gt.0)
	nloop=nloop+1
	do j=1,ny
	  do i=1,nx
	    temp(i,j)=tsea(i,j)
	  end do
	end do
	missing=0
	nset=0
	do j=1,ny
	  do i=1,nx
	    if(fland(i,j).lt.0.5 .and. tsea(i,j).eq.undef) then
	      i1=max(i-1,1)
	      i2=min(i+1,nx)
	      j1=max(j-1,1)
	      j2=min(j+1,ny)
	      ts=0.
	      nt=0
	      do js=j1,j2
		do is=i1,i2
		  if(fland(i,j).lt.0.5 .and. temp(is,js).ne.undef) then
		    ts=ts+temp(is,js)
		    nt=nt+1
		  end if
		end do
	      end do
	      if(nt.gt.0) then
		tsea(i,j)=ts/real(nt)
		nset=nset+1
	      else
		missing=missing+1
	      end if
	    elseif(fland(i,j).ge.0.5) then
	      tsea(i,j)=undef
	    end if
	  end do
	end do
      end do
c
      write(6,*) 'tsea  missing,nset,nloop: ',missing,nset,nloop
c
c...................................................................
c
c..data from lines on land
      do j=1,ny
	do i=1,nx
	  fwork(i,j,1)=0.
	  fwork(i,j,2)=0.
	end do
      end do
c
      do l=1,nlin
	k1=iline(1,l)
	k2=iline(2,l)-1
	value=iline(3,l)*vscale
	do k=k1,k2
	  x1=xpos(k)
	  y1=ypos(k)
	  x2=xpos(k+1)
	  y2=ypos(k+1)
	  nstep=max(abs(x2-x1),abs(y2-y1))*2.+2.
	  dx=(x2-x1)/real(nstep-1)
	  dy=(y2-y1)/real(nstep-1)
	  x=x1-dx
	  y=y1-dy
	  do n=1,nstep
	    x=x+dx
	    y=y+dy
	    i=nint(x)
	    j=nint(y)
	    if(i.ge.1 .and. i.le.nx .and. j.ge.1 .and. j.le.ny) then
	      if(tsea(i,j).eq.undef) then
		fwork(i,j,1)=fwork(i,j,1)+value
		fwork(i,j,2)=fwork(i,j,2)+1.
	      end if
	    end if
	  end do
	end do
      end do
c
      do j=1,ny
	do i=1,nx
	  if(fwork(i,j,2).gt.0.) tsea(i,j)=fwork(i,j,1)/fwork(i,j,2)
	end do
      end do
c
c...................................................................
c
c..fill still remaining sea/land points (from neighbours)
      do j=1,ny
	do i=1,nx
	  fwork(i,j,1)=tsea(i,j)
	end do
      end do
      iloop=0
      mta=1
      nta=0
c
      do while (nta.lt.mta)
c
	iloop=iloop+1
	do j=1,ny
	  do i=1,nx
	    tsea(i,j)=fwork(i,j,1)
	    fwork(i,j,2)=0.
	  end do
	end do
	mta=0
	nta=0
        missing=1
        nset=1
        nloop=0
c
        do while (missing.gt.0 .and. nset.gt.0)
c
	  nloop=nloop+1
	  do j=1,ny
	    do i=1,nx
	      temp(i,j)=tsea(i,j)
	    end do
	  end do
	  missing=0
	  nset=0
	  do j=1,ny
	    do i=1,nx
	      if(tsea(i,j).eq.undef) then
	        i1=max(i-1,1)
	        i2=min(i+1,nx)
	        j1=max(j-1,1)
	        j2=min(j+1,ny)
	        ts=0.
	        nt=0
	        do js=j1,j2
		  do is=i1,i2
		    if(temp(is,js).ne.undef) then
		      ts=ts+temp(is,js)
		      nt=nt+1
		    end if
		  end do
	        end do
	        if(nt.gt.0) then
		  tsea(i,j)=ts/real(nt)
		  nset=nset+1
		  fwork(i,j,2)=nloop
	        else
		  missing=missing+1
	        end if
	      end if
	    end do
	  end do
c
        end do
c
        write(6,*) 'temp  missing,nset,nloop: ',missing,nset,nloop
c
c..keep the last extrapolated values, influenced by extrapolation
c..from more or less all directions.
c..there will be more and more of these points in each iteration,
c..and the final result will get much smoother than the first iteration
c
        do j=1,ny
	  do i=1,nx
	    if(fwork(i,j,1).eq.undef) then
	      i1=max(i-2,1)
	      i2=min(i+2,nx)
	      j1=max(j-2,1)
	      j2=min(j+2,ny)
	      zmax=fwork(i,j,2)
	      do js=j1,j2
	        do is=i1,i2
		  zmax=max(zmax,fwork(is,js,2))
	        end do
	      end do
	      if(fwork(i,j,2).eq.zmax) then
	        ts=0.
	        nt=0
	        do js=j1,j2
	          do is=i1,i2
		    ts=ts+tsea(is,js)
		    nt=nt+1
	          end do
	        end do
	        fwork(i,j,1)=ts/real(nt)
	        nta=nta+1
	      end if
	      mta=mta+1
	    end if
	  end do
        end do
c
        write(6,*) 'iloop,mta,nta: ',iloop,mta,nta
c
      end do
c
      do j=1,ny
	do i=1,nx
	  tsea(i,j)=fwork(i,j,1)
	end do
      end do
c
c..................................................................
c
      write(6,*) 'Temp.sea  smoothing iterations: ',nsmooths
      write(6,*) 'Temp.land smoothing iterations: ',nsmoothl
c
      if(nsmooths.gt.0 .or. nsmoothl.gt.0) then
c..separate smoothing of Temp.sea and Temp.land
	do j=1,ny
	  do i=1,nx
	    if(fland(i,j).lt.0.5) then
	      temp(i,j)=undef
	    else
	      temp(i,j)=tsea(i,j)
	      tsea(i,j)=undef
	    end if
	  end do
	end do
	nundef=1
	call fsmooth(nsmooths,nx,ny,tsea,fwork(1,1,1),
     +		     nundef,undef,fwork(1,1,2),fwork(1,1,3))
	call fsmooth(nsmoothl,nx,ny,temp,fwork(1,1,1),
     +		     nundef,undef,fwork(1,1,2),fwork(1,1,3))
	do j=1,ny
	  do i=1,nx
	    if(fland(i,j).ge.0.5) tsea(i,j)=temp(i,j)
	  end do
	end do
      end if
c
c...................................................................
c
c..adjust land temperature according to topography (0.6 grader/100m)
      do j=1,ny
	do i=1,nx
	  if(fland(i,j).ge.0.5) tsea(i,j)=tsea(i,j)-topo(i,j)*0.006
	end do
      end do
c
c...................................................................
c
c..check minimum allowed sea temp.
      tmin=-1.5
      do j=1,ny
	do i=1,nx
	  if(fland(i,j).lt.0.5) tsea(i,j)=max(tsea(i,j),tmin)
	end do
      end do
c
c...................................................................
c
      tmin=+undef
      tmax=-undef
      nundef=0
      do j=1,ny
	do i=1,nx
	  if(tsea(i,j).ne.undef) then
	    tmin=min(tmin,tsea(i,j))
	    tmax=max(tmax,tsea(i,j))
	  else
	    nundef=nundef+1
	  end if
	end do
      end do
      write(6,*) 'temp  min,max,nundef: ',tmin,tmax,nundef
c
      return
      end
c
c***********************************************************************
c
      subroutine ppcross(angle,nx,ny,fland,fwork,gresol,
     +		         nlin,iline,npos,xpos,ypos,vscale)
c
      include 'dig2felt.inc'
c
c..input/output
      integer nx,ny,nlin,npos
      integer iline(3,nlin)
      real    angle,gresol,vscale
      real    fland(nx,ny),fwork(nx,ny,4)
      real    xpos(npos),ypos(npos)
c
c..local
      real    pp(maxint),pv(maxint)
      integer ipg(maxint)
c######################################################################
      integer iline2(3,maxlin)
c######################################################################
c
      iascale=10000
      iangle=nint(angle*iascale)
      if(iangle.le.-90*iascale .or. iangle.gt.+90*iascale) then
	write(6,*) 'PPCROSS ERROR:   angle= ',angle
	write(6,*) '     angle*scale= ',iangle
	write(6,*) '           scale= ',iascale
	write(6,*) '  Required:  -90. < angle <= +90.'
	call exit(5)
      end if
c
      rad=asin(1.)/90.
      alimit=1.e-8
c
      if(iangle.gt.-45*iascale .and. iangle.lt.+45*iascale) then
	dya=1.
	dxa=tan(angle*rad)
	if(iangle.lt.0) then
	  xa=1
	  xb=nx-dxa*real(ny-1)
	else
	  xa=1-dxa*real(ny-1)
	  xb=nx
	end if
	if(iangle.eq.0) then
	  xstep=1.
	else
	  xstep=0.5
	end if
	nstep=int((xb-xa)/xstep)+1
	xstep=(xb-xa)/real(nstep-1)
	ystep=0.
	ya=1.
	ixydir=2
      else
	dxa=1.
	dya=0.
	if(iangle.ne.+90*iascale) dya=1./tan(angle*rad)
	if(iangle.lt.0) then
	  ya=1
	  yb=ny-dya*real(nx-1)
	else
	  ya=1-dya*real(nx-1)
	  yb=ny
	end if
	if(mod(iangle,45*iascale).eq.0) then
	  ystep=1.
	else
	  ystep=0.5
	end if
	nstep=int((yb-ya)/ystep)+1
	ystep=(yb-ya)/real(nstep-1)
	xstep=0.
	xa=1.
	ixydir=1
      end if
c
c.. y=ya+(x-xa)*ay, ay=dya/dxa
c.. x=xa+(y-ya)*ax, ax=dxa/dya
c
      xa=xa-xstep
      ya=ya-ystep
c
      do n=1,nstep
c
	xa=xa+xstep
	ya=ya+ystep
	np=0
c
c######################################################################
	sinang=dxa/sqrt(dxa*dxa+dya*dya)
	cosang=dya/sqrt(dxa*dxa+dya*dya)
cxxx	npos2=0
	nlin2=0
	do l=1,nlin
	  k1=iline(1,l)
	  k2=iline(2,l)
	  kk1=0
	  kk2=0
	  yr2=(ypos(k1)-ya)*sinang - (xpos(k1)-xa)*cosang
	  do k=k1+1,k2
	    yr1=yr2
	    yr2=(ypos(k)-ya)*sinang - (xpos(k)-xa)*cosang
	    if(yr1*yr2.le.0.) then
	      if(kk1.eq.0) kk1=k-1
	      kk2=k
	    end if
	  end do
	  if(kk1.gt.0) then
	    nlin2=nlin2+1
	    iline2(1,nlin2)=kk1
	    iline2(2,nlin2)=kk2
	    iline2(3,nlin2)=iline(3,l)
cxxx	    npos2=npos2+(kk2-kk1+1)
	  end if
	end do
cxxx	write(6,*) 'nlin,npos,nlin2,npos2:',nlin,npos,nlin2,npos2
c######################################################################
c
        if(iangle.eq.+90*iascale) then
c
	  do l=1,nlin2
	    k1=iline2(1,l)
	    k2=iline2(2,l)-1
	    value=iline2(3,l)*vscale
	    do k=k1,k2
	      dx=xpos(k+1)-xpos(k)
	      dy=ypos(k+1)-ypos(k)
	      if(abs(dx).gt.alimit .and. abs(dy).gt.alimit) then
	        x=xpos(k)+(ya-ypos(k))*dx/dy
	        if(x.ge.min(xpos(k),xpos(k+1)) .and.
     +		   x.le.max(xpos(k),xpos(k+1))) then
		  np=min(np+1,maxint)
		  pp(np)=x
		  pv(np)=value
	        end if
	      end if
	    end do
	  end do
c
        elseif(iangle.eq.0) then
c
	  do l=1,nlin2
	    k1=iline2(1,l)
	    k2=iline2(2,l)-1
	    value=iline2(3,l)*vscale
	    do k=k1,k2
	      dx=xpos(k+1)-xpos(k)
	      dy=ypos(k+1)-ypos(k)
	      if(abs(dx).gt.alimit .and. abs(dy).gt.alimit) then
	        y=ypos(k)+(xa-xpos(k))*dy/dx
	        if(y.ge.min(ypos(k),ypos(k+1)) .and.
     +		   y.le.max(ypos(k),ypos(k+1))) then
		  np=min(np+1,maxint)
		  pp(np)=y
		  pv(np)=value
	        end if
	      end if
	    end do
	  end do
c
	elseif(iangle.gt.-45*iascale .and. iangle.lt.+45*iascale) then
c
	  ax=dxa/dya
	  ay=dya/dxa
	  do l=1,nlin2
	    k1=iline2(1,l)
	    k2=iline2(2,l)-1
	    value=iline2(3,l)*vscale
	    do k=k1,k2
	      dx=xpos(k+1)-xpos(k)
	      dy=ypos(k+1)-ypos(k)
	      if(abs(dx).gt.alimit .and. abs(dy).gt.alimit) then
	        bx=dx/dy
	        if(abs(ay-by).gt.alimit) then
		  y=(xpos(k)-bx*ypos(k)-xa+ax*ya)/(ax-bx)
		  x=xpos(k)+(y-ypos(k))*bx
	          if(x.ge.min(xpos(k),xpos(k+1)) .and.
     +		     x.le.max(xpos(k),xpos(k+1))) then
		    np=min(np+1,maxint)
		    pp(np)=y
		    pv(np)=value
	          end if
	        end if
	      elseif(abs(dx).gt.alimit) then
		y=ypos(k)
	        x=xa+(y-ya)*ax
	        if(x.ge.min(xpos(k),xpos(k+1)) .and.
     +		   x.le.max(xpos(k),xpos(k+1))) then
		  np=min(np+1,maxint)
		  pp(np)=y
		  pv(np)=value
	        end if
	      elseif(abs(dy).gt.alimit) then
	        x=xpos(k)
	        y=ya+(x-xa)*ay
	        if(y.ge.min(ypos(k),ypos(k+1)) .and.
     +		   y.le.max(ypos(k),ypos(k+1))) then
		  np=min(np+1,maxint)
		  pp(np)=y
		  pv(np)=value
	        end if
	      end if
	    end do
	  end do
c
	else
c
	  ax=dxa/dya
	  ay=dya/dxa
	  do l=1,nlin2
	    k1=iline2(1,l)
	    k2=iline2(2,l)-1
	    value=iline2(3,l)*vscale
	    do k=k1,k2
	      dx=xpos(k+1)-xpos(k)
	      dy=ypos(k+1)-ypos(k)
	      if(abs(dx).gt.alimit .and. abs(dy).gt.alimit) then
	        by=dy/dx
	        if(abs(ay-by).gt.alimit) then
		  x=(ypos(k)-by*xpos(k)-ya+ay*xa)/(ay-by)
		  y=ypos(k)+(x-xpos(k))*by
	          if(x.ge.min(xpos(k),xpos(k+1)) .and.
     +		     x.le.max(xpos(k),xpos(k+1))) then
		    np=min(np+1,maxint)
		    pp(np)=x
		    pv(np)=value
	          end if
	        end if
	      elseif(abs(dx).gt.alimit) then
		y=ypos(k)
	        x=xa+(y-ya)*ax
	        if(x.ge.min(xpos(k),xpos(k+1)) .and.
     +		   x.le.max(xpos(k),xpos(k+1))) then
		  np=min(np+1,maxint)
		  pp(np)=x
		  pv(np)=value
	        end if
	      elseif(abs(dy).gt.alimit) then
	        x=xpos(k)
	        y=ya+(x-xa)*ay
	        if(y.ge.min(ypos(k),ypos(k+1)) .and.
     +		   y.le.max(ypos(k),ypos(k+1))) then
		  np=min(np+1,maxint)
		  pp(np)=x
		  pv(np)=value
	        end if
	      end if
	    end do
	  end do
c
	end if
c
	if(np.ge.maxint) then
	  write(6,*) 'MAXINT TOO SMALL.   maxint: ',maxint
	  call exit(5)
	end if
c
	call ppinter(maxint,np,pp,pv,ipg,xa,ya,dxa,dya,ixydir,
     +               nx,ny,fland,fwork,gresol,undef)
c
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine ppinter(mp,np,pp,pv,ipg,xa,ya,dxa,dya,ixydir,
     +			 nx,ny,fland,fwork,gresol,undef)
c
      integer mp,np,ixydir,nx,ny
      real    xa,ya,dxa,dya,gresol,undef
      real    pp(mp),pv(mp)
      integer ipg(mp)
      real    fland(nx,ny),fwork(nx,ny,4)
c
      real    xg(4),yg(4)
c
      if(np.lt.2) return
c
c..the pp cordinates are either x or y, but the lines may not
c..be along x or y constant, distance scaling with dscale >= 1.
c
      dscale=sqrt(dxa*dxa+dya*dya)
c
      if(ixydir.eq.1) then
	sgx=1.
	sgy=0.
      else
	sgx=0.
	sgy=1.
      end if
c
      ng=0
      if(dxa.ne.0.) then
	xg(ng+1)=1
	yg(ng+1)=ya+(xg(ng+1)-xa)*dya/dxa
	xg(ng+2)=nx
	yg(ng+2)=ya+(xg(ng+2)-xa)*dya/dxa
	ng=ng+2
      end if
      if(dya.ne.0.) then
	yg(ng+1)=1
	xg(ng+1)=xa+(yg(ng+1)-ya)*dxa/dya
	yg(ng+2)=ny
	xg(ng+2)=xa+(yg(ng+2)-ya)*dxa/dya
	ng=ng+2
      end if
      ngg=ng
      ng=0
      do n=1,ngg
ccc	i=nint(xg(n))
ccc	j=nint(yg(n))
ccc	if(i.ge.1 .and. i.le.nx .and. j.ge.1 .and. j.le.ny) then
	if(xg(n).ge.0.999 .and. xg(n).le.nx+0.001 .and.
     +     yg(n).gt.0.999 .and. yg(n).le.ny+0.001) then
	  ng=ng+1
	  xg(ng)=xg(n)
	  yg(ng)=yg(n)
	end if
      end do
      if(ng.gt.2) then
	ngg=ng
	ng=1
	do n=2,ngg
	  m=1
	  do i=1,ng
	   if(abs(xg(i)-xg(n)).lt.0.01 .and.
     +	      abs(yg(i)-yg(n)).lt.0.01) m=0
	  end do
	  if(m.eq.1) then
	    ng=ng+1
	    xg(ng)=xg(n)
	    yg(ng)=yg(n)
	  end if
	end do
      elseif(ng.eq.1) then
	xg(2)=xg(1)
	yg(2)=yg(1)
	ng=2
      end if
      if(ng.eq.0) then
ccc	write(6,*) 'PPINTER WARNING.  ng= ',ng
ccc	write(6,*) '   xa,ya:    ',xa,ya
ccc	write(6,*) '   dxa,dya:  ',dxa,dya
	return
      end if
      if(ng.ne.2) then
	write(6,*) 'PPINTER ERROR.  ng= ',ng
	do n=1,ng
	  write(6,*) 'n,xg,yg: ',n,xg(n),yg(n)
	end do
	write(6,*) 'xa,ya:    ',xa,ya
	write(6,*) 'dxa,dya:  ',dxa,dya
	call exit(5)
      end if
c
      if((ixydir.eq.1 .and. xg(1).le.xg(2)) .or.
     +   (ixydir.ne.1 .and. yg(1).le.yg(2))) then
	ng1=1
	ng2=2
      else
	ng1=2
	ng2=1
      end if
      if(ixydir.eq.1) then
	nstep=(xg(ng2)-xg(ng1))*2.+2.
	ppfirst=xg(ng1)
	pplast =xg(ng2)
      else
	nstep=(yg(ng2)-yg(ng1))*2.+2.
	ppfirst=yg(ng1)
	pplast =yg(ng2)
      end if
      dx=(xg(ng2)-xg(ng1))/real(nstep-1)
      dy=(yg(ng2)-yg(ng1))/real(nstep-1)
c#####################################################################
c     nerr=0
c     nerrp=0
c     x=xg(ng1)-dx
c     y=yg(ng1)-dy
c     ip=nint(x+dx)
c     jp=nint(y+dy)
c     do n=1,nstep
c	x=x+dx
c	y=y+dy
c	i=nint(x)
c	j=nint(y)
c	if(i.lt.1 .or. i.gt.nx .or. j.lt.1 .or. j.gt.ny) nerr=nerr+1
c	if(iabs(i-ip).gt.1 .or. iabs(j-jp).gt.1) nerrp=nerrp+1
c	ip=i
c	jp=j
c     end do
c     if(nerr.gt.0 .or. nerrp.gt.0) then
c	x=xg(ng1)-dx
c	y=yg(ng1)-dy
c	ip=nint(x+dx)
c	jp=nint(y+dy)
c	do n=1,nstep
c	  x=x+dx
c	  y=y+dy
c	  i=nint(x)
c	  j=nint(y)
c	  write(6,*) 'PPINTER. ',n,ip,jp,i,j,x,y
c	  ip=i
c	  jp=j
c	end do
c	write(6,*) 'xg(ng1),yg(ng1): ',xg(ng1),yg(ng1)
c	write(6,*) 'xg(ng2),yg(ng2): ',xg(ng2),yg(ng2)
c	write(6,*) 'xa,ya:           ',xa,ya
c	write(6,*) 'dxa,dya:         ',dxa,dya
c	write(6,*) 'dx,dy:           ',dx,dy
c	write(6,*) 'nstep,nerr,nerrp:',nstep,nerr,nerrp
c	if(nerr.gt.0 .or. nerrp.gt.0) stop 'BAD.ENOUGH'
c     end if
c#####################################################################
c
c..remove data far ouside grid (dangerous as land/sea is unknown)
c..(limit is 12 grid units if 50km grid)
      dplim=12.*(50000./gresol)/dscale
      ppmin=ppfirst-dplim
      ppmax=pplast +dplim
      ip=np
      np=0
      do n=1,ip
	if(pp(n).gt.ppmin .and. pp(n).lt.ppmax) then
	  np=np+1
	  pp(np)=pp(n)
	  pv(np)=pv(n)
	end if
      end do
c
      if(np.eq.0) return
c
      if(np.eq.mp) then
	write(6,*) 'MAXINT TOO SMALL.   maxint: ',mp
	call exit(5)
      end if
c
c..sort positions
      do n=1,np-1
	m=n
	do i=n+1,np
	  if(pp(i).lt.pp(m)) m=i
	end do
	hpp=pp(n)
	hpv=pv(n)
	pp(n)=pp(m)
	pv(n)=pv(m)
	pp(m)=hpp
	pv(m)=hpv
      end do
c
      do n=1,np
	ipg(n)=0
      end do
      npp=np
c
c..a distance that is 3.5 grid units in a 50km grid
      plandl=3.5*(50000./gresol)/dscale
c
      do loop=1,2
c
      n1=1
      n2=npp-1
      if(loop.eq.2) then
	if(ppfirst.lt.pp(1))   n1=0
	if(pplast .gt.pp(npp)) n2=npp
      end if
c
      do n=n1,n2
c
	if(n.eq.0) then
	  pp1=ppfirst
	  pp2=pp(1)
	  ipgg=0
	elseif(n.eq.npp) then
	  pp1=pp(npp)
	  pp2=pplast
	  ipgg=0
	else
	  pp1=pp(n)
	  pp2=pp(n+1)
	  ipgg=ipg(n)
	end if
c
	if(ipgg.eq.0) then
c
	  if(ixydir.eq.1) then
	    px1=pp1
	    py1=ya+(px1-xa)*dya/dxa
	    px2=pp2
	    py2=ya+(px2-xa)*dya/dxa
	    kstep=(px2-px1)*2.+2.
	  else
	    py1=pp1
	    px1=xa+(py1-ya)*dxa/dya
	    py2=pp2
	    px2=xa+(py2-ya)*dxa/dya
	    kstep=(py2-py1)*2.+2.
	  end if
	  dpx=(px2-px1)/real(kstep-1)
	  dpy=(py2-py1)/real(kstep-1)
	  px=px1-dpx
	  py=py1-dpy
	  nsea=0
	  isea=0
	  nland=0
	  iland=0
	  plandm=0.
	  pland1=pp1-999.
	  pland2=pp2+999.
	  do k=1,kstep
	    px=px+dpx
	    py=py+dpy
	    i=nint(px)
	    j=nint(py)
	    if(i.ge.1 .and. i.le.nx .and. j.ge.1 .and. j.le.ny) then
	      if(fland(i,j).lt.0.5) then
	        if(isea.eq.0) nsea=nsea+1
	        isea=1
	        iland=0
	      else
	        isea=0
	        p=sgx*px+sgy*py
	        if(iland.eq.0) then
		  if(nland.eq.0) pland1=p
		  nland=nland+1
		  pland=p
	        end if
	        plandm=max(plandm,p-pland)
	        pland2=p
	        iland=1
	      end if
	    end if
	  end do
c
	  if(loop.eq.1) then
c
	    if((nsea.eq.2 .and. nland.eq.1 .and. plandm.lt.plandl) .or.
     +         (nsea.lt.2 .and. plandm.lt.plandl)) then
	      if(abs(pv(n+1)-pv(n)).gt.0.01) then
		ipg(n)=1
	      else
		ipg(n)=2
	      end if
	    end if
c
	  elseif(loop.eq.2) then
c
	    if(n.gt.0 .and. pland1.gt.pp1) then
	      grad=0.
	      dvmax=0.
	      if(n.gt.1) then
		if(ipg(n-1).eq.1) then
	          grad=(pv(n)-pv(n-1))/(pp(n)-pp(n-1))
ccc	          dvmax=abs(pv(n)-pv(n-1))*0.8
	          dvmax=0.8
		end if
	      end if
	      ipg(n)=3
	      np=min(np+1,mp)
	      pp(np)=pland1
	      pv(np)=pv(n)+grad*(pland1-pp(n))
	      pv(np)=min(pv(np),pv(n)+dvmax)
	      pv(np)=max(pv(np),pv(n)-dvmax)
	      ipg(np)=0
	    end if
	    if(n.lt.npp .and. pland2.lt.pp2) then
	      grad=0.
	      dvmax=0.
	      if(n+1.lt.npp) then
		if(ipg(n+1).eq.1) then
	          grad=(pv(n+2)-pv(n+1))/(pp(n+2)-pp(n+1))
ccc	          dvmax=abs(pv(n+2)-pv(n+1))*0.8
	          dvmax=0.8
		end if
	      end if
	      np=min(np+1,mp)
	      pp(np)=pland2
	      pv(np)=pv(n+1)+grad*(pland2-pp(n+1))
	      pv(np)=min(pv(np),pv(n+1)+dvmax)
	      pv(np)=max(pv(np),pv(n+1)-dvmax)
	      ipg(np)=4
	    end if
c
	  end if
c
	end if
c
      end do
c
      end do
c
      if(np.lt.2) return
c
      if(np.eq.mp) then
	write(6,*) 'MAXINT TOO SMALL.   maxint: ',mp
	call exit(5)
      end if
c
      if(np.gt.npp) then
c..sort positions
        do n=1,np-1
	  m=n
	  do i=n+1,np
	    if(pp(i).lt.pp(m)) m=i
	  end do
	  hpp=pp(n)
	  hpv=pv(n)
	  ipgg=ipg(n)
	  pp(n)=pp(m)
	  pv(n)=pv(m)
	  ipg(n)=ipg(m)
	  pp(m)=hpp
	  pv(m)=hpv
	  ipg(m)=ipgg
        end do
      end if
c
      x=xg(ng1)-dx
      y=yg(ng1)-dy
      ip=1
      do n=1,nstep
	x=x+dx
	y=y+dy
	i=nint(x)
	j=nint(y)
	rg2=(x-i)**2+(y-j)**2
	if(rg2.lt.fwork(i,j,4)) then
ccc       p=sgx*real(i)+sgy*real(j)
          p=sgx*x+sgy*y
	  if(p.ge.pp(1) .and. p.le.pp(np)) then
	    do while (pp(ip+1).lt.p .and. ip.lt.np-1)
	      ip=ip+1
	    end do
	    if(ipg(ip).gt.0) then
	      value=pv(ip)+(pv(ip+1)-pv(ip))
     +			  *(p-pp(ip))/(pp(ip+1)-pp(ip))
	      if(ipg(ip).eq.1) then
	        grad=abs(pv(ip+1)-pv(ip))/((pp(ip+1)-pp(ip))*dscale)
	        dist=(pp(ip+1)-pp(ip))*dscale
	      elseif(ipg(ip).eq.2) then
		grad=0.
		dist=min(p-pp(ip),pp(ip+1)-p)*dscale
	      elseif(ipg(ip).eq.3) then
		grad=0.
		dist=(p-pp(ip))*dscale
	      else
		grad=0.
		dist=(pp(ip+1)-p)*dscale
	      end if
	      fwork(i,j,1)=value
	      fwork(i,j,2)=grad
	      fwork(i,j,3)=dist
	      fwork(i,j,4)=rg2
	    end if
	  end if
	end if
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine joinline(nx,ny,fland,gresol,
     +			  nlin,iline,npos,xpos,ypos)
c
      include 'dig2felt.inc'
c
c..input/output
      integer nx,ny,nlin,npos
      integer iline(3,nlin)
      real    fland(nx,ny),gresol
      real    xpos(npos),ypos(npos)
c
c..local
      real    xone(maxone),yone(maxone)
c
c..remove lines far outside the grid (12 grid units if 50 km grid)
c
      dremove=12.*(50000./gresol)
      dremove=max(dremove,0.)
      xmin= 1-dremove
      xmax=nx+dremove
      ymin= 1-dremove
      ymax=ny+dremove
      np=0
      nl=0
      do l=1,nlin
	k1=iline(1,l)
	k2=iline(2,l)
	x1=xpos(k1)
	x2=xpos(k1)
	y1=ypos(k1)
	y2=ypos(k1)
	do k=k1+1,k2
	  x1=min(x1,xpos(k))
	  x2=max(x2,xpos(k))
	  y1=min(y1,ypos(k))
	  y2=max(y2,ypos(k))
	end do
	if(x1.lt.xmax .and. x2.gt.xmin .and.
     +	   y1.lt.ymax .and. y2.gt.ymin) then
	  if(l.ne.nl+1 .or. k1.ne.np+1) then
	    nl=nl+1
	    iline(1,nl)=np+1
	    iline(2,nl)=np+(k2-k1+1)
	    iline(3,nl)=iline(3,l)
	    do k=k1,k2
	      np=np+1
	      xpos(np)=xpos(k)
	      ypos(np)=ypos(k)
	    end do
	  else
	    nl=nl+1
	    np=k2
	  end if
	end if
      end do
      npos=np
      nlin=nl
c
c..join lines with close endpoints
c
c..(limits are 1.5 and 8.0 grid units if 50km grid)
      d2lim1=(1.5*(50000./gresol))**2
      d2lim2=(8.0*(50000./gresol))**2
      l1=0
c
      do while (l1.lt.nlin-1)
	l1=l1+1
	k1=iline(1,l1)
	k2=iline(2,l1)
	x1=xpos(k1)
	y1=ypos(k1)
	x2=xpos(k2)
	y2=ypos(k2)
	d2min=(x2-x1)**2+(y2-y1)**2
	ljoin=l1
	kjoin=0
	ijoin=0
	do l=l1+1,nlin
	  if(iline(3,l).eq.iline(3,l1)) then
	    k1=iline(1,l)
	    k2=iline(2,l)
	    d2=(xpos(k1)-x1)**2+(ypos(k1)-y1)**2
	    if(d2.lt.d2min) then
	      d2min=d2
	      ljoin=l
	      kjoin=k1
	      ijoin=1
	    end if
	    d2=(xpos(k1)-x2)**2+(ypos(k1)-y2)**2
	    if(d2.lt.d2min) then
	      d2min=d2
	      ljoin=l
	      kjoin=k1
	      ijoin=2
	    end if
	    d2=(xpos(k2)-x1)**2+(ypos(k2)-y1)**2
	    if(d2.lt.d2min) then
	      d2min=d2
	      ljoin=l
	      kjoin=k2
	      ijoin=1
	    end if
	    d2=(xpos(k2)-x2)**2+(ypos(k2)-y2)**2
	    if(d2.lt.d2min) then
	      d2min=d2
	      ljoin=l
	      kjoin=k2
	      ijoin=2
	    end if
	  end if
	end do
	if(d2min.ge.d2lim1 .and. d2min.lt.d2lim2) then
c..lines further apart only joined if land between the endpoints
	  if(ljoin.eq.l1) then
	    xj1=x1
	    yj1=y1
	    xj2=x2
	    yj2=y2
	  else
	    if(ijoin.eq.1) then
	      xj1=x1
	      yj1=y1
	    else
	      xj1=x2
	      yj1=y2
	    end if
	    xj2=xpos(kjoin)
	    yj2=ypos(kjoin)
	  end if
	  nstep=int(sqrt(d2min)/0.5)+2
	  dx=(xj2-xj1)/real(nstep-1)
	  dy=(yj2-yj1)/real(nstep-1)
	  xj=xj1-dx
	  yj=yj1-dy
	  nsea=0
	  do n=1,nstep
	    xj=xj+dx
	    yj=yj+dy
	    i=nint(xj)
	    j=nint(yj)
	    if(i.ge.1 .and. i.le.nx .and. j.ge.1 .and. j.le.ny) then
	      if(fland(i,j).lt.0.5) nsea=nsea+1
	    end if
	  end do
	  if(nsea.eq.0) d2min=0.
	end if
	if(d2min.lt.d2lim1) then
	  if(ljoin.eq.l1) then
	    npos=npos+1
	    if(npos.gt.maxpos) then
	      write(6,*)  'MAXPOS TOO SMALL.  maxpos= ',maxpos
	      call exit(5)
	    end if
	    k1=iline(2,l1)+1
	    k2=iline(2,nlin)
	    do k=k2,k1,-1
	      xpos(k+1)=xpos(k)
	      ypos(k+1)=ypos(k)
	    end do
	    do l=l1+1,nlin
	      iline(1,l)=iline(1,l)+1
	      iline(2,l)=iline(2,l)+1
	    end do
	    iline(2,l1)=iline(2,l1)+1
	    k1=iline(1,l1)
	    k2=iline(2,l1)
	    xpos(k2)=xpos(k1)
	    ypos(k2)=ypos(k1)
	  else
	    k1=iline(1,ljoin)
	    k2=iline(2,ljoin)
	    nk=k2-k1+1
	    if(nk.gt.maxone) then
	      write(6,*) 'MAXONE TOO SMALL.   maxone= ',maxone
	      call exit(5)
	    end if
	    if((kjoin.eq.k1 .and. ijoin.eq.2) .or.
     +	       (kjoin.eq.k2 .and. ijoin.eq.1)) then
	      do k=1,nk
	        xone(k)=xpos(k1-1+k)
	        yone(k)=ypos(k1-1+k)
	      end do
	    else
	      do k=1,nk
	        xone(k)=xpos(k2+1-k)
	        yone(k)=ypos(k2+1-k)
	      end do
	    end if
	    if(ljoin.gt.l1+1) then
	      k1=iline(1,l1+1)
	      k2=iline(2,ljoin-1)
	      do k=k2,k1,-1
		xpos(k+nk)=xpos(k)
		ypos(k+nk)=ypos(k)
	      end do
	      do l=l1+1,ljoin-1
		iline(1,l)=iline(1,l)+nk
		iline(2,l)=iline(2,l)+nk
	      end do
	    end if
	    do l=ljoin,nlin-1
	      iline(1,l)=iline(1,l+1)
	      iline(2,l)=iline(2,l+1)
	      iline(3,l)=iline(3,l+1)
	    end do
	    nlin=nlin-1
	    k1=iline(1,l1)
	    k2=iline(2,l1)
	    if(ijoin.eq.1) then
	      do k=k2,k1,-1
		xpos(k+nk)=xpos(k)
		ypos(k+nk)=ypos(k)
	      end do
	      k0=k1-1
	    else
	      k0=k2
	    end if
	    do k=1,nk
	      xpos(k0+k)=xone(k)
	      ypos(k0+k)=yone(k)
	    end do
	    iline(2,l1)=iline(2,l1)+nk
c..this "new" line must be rechecked 
	    l1=l1-1
	  end if
	end if
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine ice(nx,ny,fland,iarea,iwork,
     +		     nlin,iline,npos,xpos,ypos,vscale)
c
      include 'dig2felt.inc'
c
c..input/output
      integer nx,ny,nlin,npos
      integer iarea(nx,ny),iwork(nx,ny)
      integer iline(3,nlin)
      real    vscale
      real    fland(nx,ny)
      real    xpos(npos),ypos(npos)
c
c..local
      integer isequence(2,maxlin)
c
c..just one ice type, only take care of ice removal
      nsequence=0
      do l=1,nlin
	if(iline(3,l).gt.0) then
	  nsequence=nsequence+1
	  isequence(1,nsequence)=l
	  isequence(2,nsequence)=1
	end if
      end do
      do l=1,nlin
	if(iline(3,l).le.0) then
	  nsequence=nsequence+1
	  isequence(1,nsequence)=l
	  isequence(2,nsequence)=0
	end if
      end do
c
c..initialize as sea
      do j=1,ny
	do i=1,nx
	  iarea(i,j)=0
	end do
      end do
c
      call markarea(nx,ny,iarea,nx*ny,iwork,
     +              nlin,iline,npos,xpos,ypos,
     +		    nsequence,isequence)
c
c..update ice/sea
      nsea1=0
      nice1=0
      nsea2=0
      nice2=0
c
      do j=1,ny
	do i=1,nx
c
	  iland=nint(fland(i,j))
c
	  if(iland.eq.0) nsea1=nsea1+1
	  if(iland.eq.1) nice1=nice1+1
c
c..update sea/ice
	  if(iland.eq.0 .or. iland.eq.1) then
	    iland=iarea(i,j)
	    fland(i,j)=iland
	  end if
c
	  if(iland.eq.0) nsea2=nsea2+1
	  if(iland.eq.1) nice2=nice2+1
c
	end do
      end do
c
      write(6,*) 'Sea grid points input,output: ',nsea1,nsea2
      write(6,*) 'Ice grid points input,output: ',nice1,nice2
c
      return
      end
c
c***********************************************************************
c
      subroutine snow(nx,ny,snowc,fland,iarea,iwork,
     +		      nlin,iline,npos,xpos,ypos,vscale)
c
      include 'dig2felt.inc'
c
c..input/output
      integer nx,ny,nlin,npos
      integer iarea(nx,ny),iwork(nx,ny)
      integer iline(3,nlin)
      real    vscale
      real    snowc(nx,ny),fland(nx,ny)
      real    xpos(npos),ypos(npos)
c
c..local
      integer isequence(2,maxlin)
c
c..The following approach seems appropriate for data from VNN !!!!
c..sort according to increasing value (0 - 100 %),
c..except for 0% at the end
      ivmax=nint(100./vscale)
      nsequence=0
      do l=1,nlin
	if(iline(3,l).gt.0) then
	  nsequence=nsequence+1
	  isequence(1,nsequence)=l
	  isequence(2,nsequence)=min(iline(3,l),ivmax)
	end if
      end do
c..sort
      do n=1,nsequence-1
	m=n
	do i=n+1,nsequence
	  if(isequence(2,i).lt.isequence(2,m)) m=i
	end do
	i1=isequence(1,n)
	i2=isequence(2,n)
	isequence(1,n)=isequence(1,m)
	isequence(2,n)=isequence(2,m)
	isequence(1,m)=i1
	isequence(2,m)=i2
      end do
      do l=1,nlin
	if(iline(3,l).le.0) then
	  nsequence=nsequence+1
	  isequence(1,nsequence)=l
	  isequence(2,nsequence)=0
	end if
      end do
c
c..initialize as no snow
      do j=1,ny
	do i=1,nx
	  iarea(i,j)=0
	end do
      end do
c
      call markarea(nx,ny,iarea,nx*ny,iwork,
     +              nlin,iline,npos,xpos,ypos,
     +		    nsequence,isequence)
c
c..update the snow coverage field,
c..set 0% on sea and 100% on ice
      nsnow1=0
      nsnow2=0
      nsnow3=0
      scmin=+9999.
      scmax=-9999.
c
      do j=1,ny
	do i=1,nx
	  snowc(i,j)=iarea(i,j)*vscale
	  iland=nint(fland(i,j))
	  if(iland.eq.0) snowc(i,j)=0.
	  if(iland.eq.1) snowc(i,j)=100.
	  if(snowc(i,j).gt. 0.) nsnow1=nsnow1+1
	  if(snowc(i,j).gt.50.) nsnow2=nsnow2+1
	  if(snowc(i,j).gt.80.) nsnow3=nsnow3+1
	  scmin=min(scmin,snowc(i,j))
	  scmax=max(scmax,snowc(i,j))
	end do
      end do
c
      write(6,*) 'Snow coverage min,max: ',scmin,scmax
      write(6,*) 'No. of grid points with snow >  0%: ',nsnow1
      write(6,*) 'No. of grid points with snow > 50%: ',nsnow2
      write(6,*) 'No. of grid points with snow > 80%: ',nsnow3
c
      return
      end
c
c***********************************************************************
c
      subroutine sndep(nx,ny,snowd,fland,iarea,iwork,
     +		       nlin,iline,npos,xpos,ypos,vscale)
c
      include 'dig2felt.inc'
c
c..input/output
      integer nx,ny,nlin,npos
      integer iarea(nx,ny),iwork(nx,ny)
      integer iline(3,nlin)
      real    vscale
      real    snowd(nx,ny),fland(nx,ny)
      real    xpos(npos),ypos(npos)
c
c..local
      integer isequence(2,maxlin)
c
c..The following approach seems appropriate for data from VNN !!!!
c..sort according to increasing value (0 - 1000 cm),
c..except for 0 cm at the end
      ivmax=nint(1000./vscale)
      nsequence=0
      do l=1,nlin
	if(iline(3,l).gt.0) then
	  nsequence=nsequence+1
	  isequence(1,nsequence)=l
	  isequence(2,nsequence)=min(iline(3,l),ivmax)
	end if
      end do
c..sort
      do n=1,nsequence-1
	m=n
	do i=n+1,nsequence
	  if(isequence(2,i).lt.isequence(2,m)) m=i
	end do
	i1=isequence(1,n)
	i2=isequence(2,n)
	isequence(1,n)=isequence(1,m)
	isequence(2,n)=isequence(2,m)
	isequence(1,m)=i1
	isequence(2,m)=i2
      end do
      do l=1,nlin
	if(iline(3,l).le.0) then
	  nsequence=nsequence+1
	  isequence(1,nsequence)=l
	  isequence(2,nsequence)=0
	end if
      end do
c
c..initialize as no snow
      do j=1,ny
	do i=1,nx
	  iarea(i,j)=0
	end do
      end do
c
      call markarea(nx,ny,iarea,nx*ny,iwork,
     +              nlin,iline,npos,xpos,ypos,
     +		    nsequence,isequence)
c
c..update the snow depth field,
c..set 0 cm on sea and 100 cm on ice
      nsnow1=0
      nsnow2=0
      nsnow3=0
      nsnow4=0
      nsnow5=0
      nsnow6=0
      nsnow7=0
      scmin=+9999.
      scmax=-9999.
c
      do j=1,ny
	do i=1,nx
	  snowd(i,j)=iarea(i,j)*vscale
	  iland=nint(fland(i,j))
	  if(iland.eq.0) snowd(i,j)=0.
	  if(iland.eq.1) snowd(i,j)=100.
	  if(snowd(i,j).ge.   0.) nsnow1=nsnow1+1
	  if(snowd(i,j).ge.  15.) nsnow2=nsnow2+1
	  if(snowd(i,j).ge.  30.) nsnow3=nsnow3+1
	  if(snowd(i,j).ge.  70.) nsnow4=nsnow4+1
	  if(snowd(i,j).ge. 100.) nsnow5=nsnow5+1
	  if(snowd(i,j).ge. 300.) nsnow6=nsnow6+1
	  if(snowd(i,j).ge.1000.) nsnow7=nsnow7+1
	  scmin=min(scmin,snowd(i,j))
	  scmax=max(scmax,snowd(i,j))
	end do
      end do
c
      write(6,*) 'Snow depth min,max: ',scmin,scmax
      write(6,*) 'No. of grid points with snow >=    0 cm: ',nsnow1
      write(6,*) 'No. of grid points with snow >=   15 cm: ',nsnow2
      write(6,*) 'No. of grid points with snow >=   30 cm: ',nsnow3
      write(6,*) 'No. of grid points with snow >=   70 cm: ',nsnow4
      write(6,*) 'No. of grid points with snow >=  100 cm: ',nsnow5
      write(6,*) 'No. of grid points with snow >=  300 cm: ',nsnow6
      write(6,*) 'No. of grid points with snow >= 1000 cm: ',nsnow7
c
      return
      end
c
c**********************************************************************
c
      subroutine markarea(nx,ny,iarea,lwork,iwork,
     +			  nlin,iline,npos,xpos,ypos,
     +			  nsequence,isequence)
c
c	put a value in gridpoints inside a close line.
c	using a fine mesh grid (5x5 instead of 1x1 point, to check
c	if the 'inside line area' covers more than half of the
c	original grid square)
c
c..input/output
      integer nx,ny,lwork,nlin,npos,nsequence
      integer iarea(nx,ny),iwork(lwork)
      integer iline(3,nlin)
      integer isequence(2,nsequence)
      real    xpos(npos),ypos(npos)
c
c..no. of bits in each word (of iwork)
      parameter (nbitwd=32)
c
c..the fine mesh grid: xb = (x-1)*5 + 3
c..                    yb = (y-1)*5 + 3
      nxb=nx*5
      nyb=ny*5
      nword=(nxb*nyb+nbitwd-1)/nbitwd
      if(nword.gt.lwork) then
	write(6,*) 'MARKAREA.  LWORK TOO SMALL.  lwork,nword: ',
     +						 lwork,nword
	call exit(5)
      end if
c
      do ls=1,nsequence
c
        l   =isequence(1,ls)
        iset=isequence(2,ls)
c
        k1=iline(1,l)
        k2=iline(2,l)
c
c..handling each line separate
c
        do i=1,nword
	  iwork(i)=0
        end do
c
c..note: closed lines, first a line from endpoint to startpoint
c
        x2=(xpos(k2)-1.)*5.+3.
        y2=(ypos(k2)-1.)*5.+3.
        j2=int(y2)
c
        do k=k1,k2
c
          x1=x2
          y1=y2
          j1=j2
          x2=(xpos(k)-1.)*5.+3.
          y2=(ypos(k)-1.)*5.+3.
          j2=int(y2)
 	  jb1=min(j1,j2)+1
	  jb2=max(j1,j2)
          jb1=max(jb1,  1)
          jb2=min(jb2,nyb)
c
          do jb=jb1,jb2
            y=jb
            ib=int(x1+(x2-x1)*(y-y1)/(y2-y1))
	    ib=max(ib+1,1)
	    if(ib.le.nxb) then
	      ibit=(jb-1)*nxb+ib
	      iwrd=(ibit+nbitwd-1)/nbitwd
	      ibit=ibit-(iwrd-1)*nbitwd-1
	      if(btest(iwork(iwrd),ibit)) then
		iwork(iwrd)=ibclr(iwork(iwrd),ibit)
	      else
		iwork(iwrd)=ibset(iwork(iwrd),ibit)
	      end if
	    end if
          end do
c
        end do
c
c..mark fine mesh grid area inside the closed line
	do jb=1,nyb
	  k=0
	  do ib=1,nxb
	    ibit=(jb-1)*nxb+ib
	    iwrd=(ibit+nbitwd-1)/nbitwd
	    ibit=ibit-(iwrd-1)*nbitwd-1
	    if(btest(iwork(iwrd),ibit)) then
	      k=mod(k+1,2)
	      if(k.eq.0) iwork(iwrd)=ibclr(iwork(iwrd),ibit)
	    elseif(k.eq.1) then
	      iwork(iwrd)=ibset(iwork(iwrd),ibit)
	    end if
	  end do
	end do
c
c..set value in output grid
	do j=1,ny
	  jb =(j-1)*5+3
	  jb1=jb-2
	  jb2=jb+2
	  do i=1,nx
	    ib =(i-1)*5+3
	    ib1=ib-2
	    ib2=ib+2
	    isum=0
	    do jb=jb1,jb2
	      do ib=ib1,ib2
		ibit=(jb-1)*nxb+ib
		iwrd=(ibit+nbitwd-1)/nbitwd
		ibit=ibit-(iwrd-1)*nbitwd-1
		if(btest(iwork(iwrd),ibit)) isum=isum+1
	      end do
	    end do
	    if(isum.gt.12) iarea(i,j)=iset
	  end do
	end do
c
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine fsmooth(nsmooth,nx,ny,z,a,nundef,undef,w1,w2)
c
c        low-bandpass filter, removing short wavelengths
c        (not a 2. or 4. order shapiro filter)
c
c        G.J.Haltiner, Numerical Weather Prediction,
c                         Objective Analysis,
c                            Smoothing and filtering
c
c        input:   nsmooth  - no. of iterations
c                 z(nx,ny) - the field
c                 a(nx,ny) - a work matrix
c                w1(nx,ny) - a work matrix (nundef>0)
c                w2(nx,ny) - a work matrix (nundef>0)
c        output:  z(nx,ny) - the field
c
      integer nsmooth,nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
c
      if(nsmooth.lt.1) return
c
      nxm1=nx-1
      nym1=ny-1
c
      s=0.25
c
      if(nundef.le.0) then
c
        do n=1,nsmooth
c
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
c
        end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
          do i=2,nxm1
            w1(i,j)=1.
            if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
          end do
        end do
        do j=2,nym1
          do i=1,nx
            w2(i,j)=1.
            if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
          end do
        end do
c
        do n=1,nsmooth
c
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
c
        end do
c
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine rline(filnam,iunit,parnam,itime,igscal,ivscal,
     *		       mpos,mlin,npos,nlin,ilin,glat,glon,
     *		       rstatus,ierror)
c
c        read data from line file
c
c
      integer       iunit,igscal,ivscal,mpos,mlin,npos,nlin,ierror
      integer       itime(4),ilin(3,mlin)
      real          glat(mpos),glon(mpos)
      character*(*) filnam,parnam,rstatus
c
      parameter (maxout=256)
      integer*2 iout(2,maxout)
c
c-------------------------------------------------------------------
c        max record-lengde = 1024 bytes
c                          =  512 integer*2 tall
c                          =  256 posisjoner
c
c         iout(1,1): siste record benyttet
c             (2,1): siste 'posisjon' benyttet i siste record
c             (1,2): $r
c             (2,2): m$ned
c             (1,3): dag
c             (2,3): time
c             (1,4): antall linjer
c             (2,4): totalt antall posisjoner (hvis < 32767)
c             (1,5): skalering av b,l
c             (2,5): skalering av 'verdi'/'type'
c             (1,6): antall 'posisjoner' for 'tekst' (nptxt)
c             (2,6): antall tegn i 'tekst'
c             (1,6+1):     start 'tekst'
c             ........
c             (2,6+nptxt): slutt 'tekst'
c             (1,n): antall posisjoner for f@rste linje (npos)
c             (2,n): 'verdi'/'type'
c             (1,n+1):  b ) f@rste posisjon
c             (2,n+1):  l )
c             ......
c             (1,n+npos):  b ) siste posisjon
c             (2,n+npos):  l )
c             (1,n+npos+1):  antall posisjoner neste linje
c             (2,n+npos+1):  'verdi'/'type'
c             .....osv.
c-------------------------------------------------------------------
c
      ierror=0
      irec=0
      nlin=0
      npos=0
      nli=0
      npi=0
c
      call rlunit(lrunit)
c
      write(6,*) 'RLINE: ',filnam(1:lenstr(filnam,1))
c
      open(iunit,file=filnam,
     *           access='direct',form='unformatted',
     *		 recl=1024/lrunit,
     *           status='old',iostat=ios,err=900)
c
      irec=1
      read(iunit,rec=irec,iostat=ios,err=910) iout
      nreci=irec
c
      nlin=     iout(1,4)
      npos=     iout(2,4)
c
      write(6,*) 'no. of lines:     ',nlin
      write(6,*) 'no. of positions: ',npos,'  (if < 32767)'
c
      if(nlin.lt.1 .or. npos.lt.1) then
	rstatus='Empty file'
	ierror=-1
        goto 980
      end if
c
      nrec=     iout(1,1)
      lpout=    iout(2,1)
      itime(1)= iout(1,2)
      itime(2)= iout(2,2)
      itime(3)= iout(1,3)
      itime(4)= iout(2,3)
      nlin=     iout(1,4)
      npos=     iout(2,4)
      igscal=   iout(1,5)
      ivscal=   iout(2,5)
      nptxt=    iout(1,6)
      lntxt=    iout(2,6)
c
      l=len(parnam)
      lntxt=min(lntxt,l)
      parnam=' '
      k=0
      do n=1,nptxt
	ichr=iout(1,6+n)
	ichr1=iand(ishft(ichr,-8),255)
	ichr2=iand(ichr,255)
	ichr=iout(2,6+n)
	ichr3=iand(ishft(ichr,-8),255)
	ichr4=iand(ichr,255)
	if(k+1.le.lntxt) parnam(k+1:k+1)=char(ichr1)
	if(k+2.le.lntxt) parnam(k+2:k+2)=char(ichr2)
	if(k+3.le.lntxt) parnam(k+3:k+3)=char(ichr3)
	if(k+4.le.lntxt) parnam(k+4:k+4)=char(ichr4)
	k=k+4
      end do
c
      write(6,*) 'text: ',parnam(1:lntxt)
      write(6,*) 'time: ',(itime(i),i=1,4)
c
      if(npos.gt.mpos) then
	write(6,*) 'too many positions. max= ',mpos
      end if
c
      nnlin=nlin
      if(nnlin.gt.mlin) then
	write(6,*) 'too many lines. max= ',mlin
	nnlin=maxlin
	ierror=1
      end if
c
      np=6+nptxt
      ip=0
c
      gscale=10.**igscal
c
      do l=1,nnlin
c
        if(np.ge.maxout) then
          irec=irec+1
          read(iunit,rec=irec,iostat=ios,err=910) iout
          np=0
        end if
        np=np+1
        length=iout(1,np)
	if(ip+length.gt.mpos) then
	  ierror=1
	  goto 500
	end if
c
        ilin(1,l)=ip+1
        ilin(2,l)=ip+length
        ilin(3,l)=iout(2,np)
c
        nr=(np+length+maxout-1)/maxout
        if(np.eq.maxout) nr=nr-1
        i2=0
c
        do ir=1,nr
          if(np.ge.maxout) then
            irec=irec+1
            read(iunit,rec=irec,iostat=ios,err=910) iout
            np=0
          end if
          i0=i2
          i2=i2+maxout-np
          if(i2.gt.length) i2=length
          ni=i2-i0
          do i=1,ni
            glat(ip+i)=gscale*iout(1,np+i)
            glon(ip+i)=gscale*iout(2,np+i)
	  end do
          ip=ip+ni
          np=np+ni
        end do
c
        nreci=irec
        nli=l
        npi=ip
c
      end do
c
  500 if(ierror.eq.0) then
        rstatus='OK'
      else
        rstatus='Too much data'
      end if
      goto 980
c
  900 write(6,*) 'open error.  iostat: ',ios
      rstatus='New file'
      ierror=-1
      goto 980
c
  910 write(6,*) 'read error.  iostat,record: ',ios,irec
      rstatus='Read ERROR'
      ierror=1
      if(irec.eq.1) ierror=-1
      goto 980
c
  980 close(iunit)
c
      write(6,*) 'input no. of lines:     ',nli
      write(6,*) 'input no. of positions: ',npi
      write(6,*) 'status: ',rstatus(1:lenstr(rstatus,1))
c
      nlin=nli
      npos=npi
c
      return
      end
