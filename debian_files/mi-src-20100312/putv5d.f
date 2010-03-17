      subroutine putv5d(initv5d,v5dfile,
     +		        nx,ny,nlevel,ntime,nparam,nt,np,f3d,
     +		        iparam,cparam,nplevel,
     +                  ilevel,alevel,blevel,plevel,
     +			itime,ivcoord,igtype,grid,ivertical)
c
c  Write data to a vis5d data file (create,write,close)
c
c--------------------------------------------------------------------------
c  DNMI/FoU  03.08.1995  Anstein Foss
c  DNMI/FoU  31.10.1996  Anstein Foss
c  DNMI/FoU  21.05.1997  Anstein Foss ... pressure levels (interpolated)
c  DNMI/FoU  08.12.1997  Anstein Foss ... surface data
c  DNMI/FoU  01.11.2003  Anstein Foss ... Vis5d+-1.2.1, include path
c--------------------------------------------------------------------------
c
      implicit none
c
      integer initv5d,iunitv,nx,ny,nlevel,ntime,nparam,nt,np
      integer ivcoord,igtype,ivertical
      integer iparam(nparam),nplevel(nparam),ilevel(nlevel)
      integer itime(5,ntime)
      real    alevel(nlevel),blevel(nlevel),plevel(nlevel)
      real    f3d(nx,ny,nlevel),grid(6)
      character*(*) v5dfile
      character*(*) cparam(nparam)
c
      integer n,i,j,k,nhours,ndays,ierr1,ierr2,istatus
      integer itime1(5),itime2(5)
c
      integer ix1,ix2,ixs,iy1,iy2,iys,iz1,iz2,izs
      integer izz1,izz2,izzs
      real    p,hkm
c
c..................................................................
      include 'v5df.h'
c
      real*4 G(MAXROWS*MAXCOLUMNS*MAXLEVELS)
c
C  THE FOLLOWING VARIABLES DESCRIBE THE DATASET TO BE CONVERTED.  YOU
C  MUST INITIALIZE ALL THESE VARIABLES WITH VALUES FROM YOUR FILE OR
C  ASSIGN SUITABLE CONSTANTS.  SEE THE README FILE FOR DESCRIPTIONS
C  OF THESE VARIABLES.
      integer nr, nc, nl(MAXVARS)
      integer numtimes
      integer numvars
      character*10 varname(MAXVARS)
      integer dates(MAXTIMES)
      integer times(MAXTIMES)
      integer compressmode
      integer projection
      real proj_args(100)
      integer vertical
      real vert_args(MAXLEVELS)
c
c  initialize the variables to missing values
      data nr,nc / IMISSING, IMISSING /
      data (nl(i),i=1,MAXVARS) / MAXVARS*IMISSING /
      data numtimes,numvars / IMISSING, IMISSING /
      data (varname(i),i=1,MAXVARS) / MAXVARS*"          " /
      data (dates(i),i=1,MAXTIMES) / MAXTIMES*IMISSING /
      data (times(i),i=1,MAXTIMES) / MAXTIMES*IMISSING /
      data compressmode / 1 /
      data projection / IMISSING /
      data (proj_args(i),i=1,100) / 100*MISSING /
      data vertical / IMISSING /
      data (vert_args(i),i=1,100) / 100*MISSING /
c..................................................................
c
      data ix1,ix2,ixs,iy1,iy2,iys,iz1,iz2,izs/9*-1/
c
      if(initv5d.eq.0) then
c
c..create vis5d data file
c
	nr=ny
	nc=nx
	do n=1,nparam
	  nl(n)=nplevel(n)
	end do
	numtimes=ntime
	numvars=nparam
	do n=1,nparam
	  varname(n)=cparam(n)
	end do
	do n=1,ntime
	  itime1(1)=itime(1,n)
	  itime1(2)=1
	  itime1(3)=1
	  itime1(4)=0
	  itime1(5)=0
	  itime2(1)=itime(1,n)
	  itime2(2)=itime(2,n)
	  itime2(3)=itime(3,n)
	  itime2(4)=0
	  itime2(5)=0
	  call hrdiff(0,0,itime1,itime2,nhours,ierr1,ierr2)
	  ndays=(nhours/24)+1
	  dates(n)=(itime(1,n)-(itime(1,n)/100)*100)*1000+ndays
	  times(n)=itime(4,n)*10000+itime(5,n)*100
	end do
c
 	if(igtype.eq.2) then
	  projection=1
	  ix1=1
	  ix2=nx
	  ixs=+1
	  if(grid(4).gt.0.) then
	    proj_args(1)=grid(2)+(ny-1)*grid(4)
	    proj_args(2)=-grid(1)
	    proj_args(3)=grid(4)
	    proj_args(4)=grid(3)
	    iy1=ny
	    iy2=1
	    iys=-1
	  else
	    proj_args(1)=grid(2)
	    proj_args(2)=-grid(1)
	    proj_args(3)=-grid(4)
	    proj_args(4)=grid(3)
	    iy1=1
	    iy2=ny
	    iys=+1
	  end if
 	elseif(igtype.eq.3) then
	  projection=4
	  proj_args(1)=grid(2)+(ny-1)*grid(4)
	  proj_args(2)=-grid(1)
	  proj_args(3)=grid(4)
	  proj_args(4)=grid(3)
	  proj_args(5)=grid(6)
	  proj_args(6)=-grid(5)
	  proj_args(7)=0.
	  ix1=1
	  ix2=nx
	  ixs=+1
	  iy1=ny
	  iy2=1
	  iys=-1
	else
	  write(6,*) 'PUTV5D ERROR. Unknown grid type: ',igtype
	  call exit(4)
	end if
c
	if(ivcoord.eq.1 .or. ivcoord.eq.10) then
	  if(ilevel(1).lt.ilevel(nlevel)) then
	    iz1=nlevel
	    iz2=1
	    izs=-1
	  else
	    iz1=1
	    iz2=nlevel
	    izs=+1
	  end if
	else
	  write(6,*) 'PUTV5D ERROR.'
	  write(6,*) 'Unknown vertical coordinate: ',ivcoord
	  call exit(4)
	end if
c
	if(ivcoord.eq.1) then
	  if(ivertical.eq.1) then
c..pressure (mb) as vertical (display) coordinate
c..(available from Vis5d-4.2)
	    vertical=3
	    n=0
	    do k=iz1,iz2,izs
              p=plevel(k)*0.01
	      n=n+1
	      vert_args(n)=p
	    end do
	  else
c..height (km) as vertical (display) coordinate
	    vertical=2
	    n=0
	    do k=iz1,iz2,izs
              p=plevel(k)*0.01
c..as used in vis5d
	      hkm=-7.2*alog(p/1012.5)
	      n=n+1
	      vert_args(n)=hkm
	    end do
	  end if
	elseif(ivcoord.eq.10) then
	  if(ivertical.eq.1) then
c..pressure (mb) as vertical (display) coordinate
c..(available from Vis5d-4.2)
	    vertical=3
	    n=0
	    do k=iz1,iz2,izs
c..assume ps=1000 hPa
              p=alevel(k)*0.01+blevel(k)*1000.
	      n=n+1
	      vert_args(n)=p
	    end do
	  else
c..height (km) as vertical (display) coordinate
	    vertical=2
	    n=0
	    do k=iz1,iz2,izs
c..assume ps=1000 hPa
              p=alevel(k)*0.01+blevel(k)*1000.
c..as used in vis5d
	      hkm=-7.2*alog(p/1012.5)
	      n=n+1
	      vert_args(n)=hkm
	    end do
	  end if
	else
	  write(6,*) 'PUTV5D ERROR.'
	  write(6,*) 'Unknown vertical coordinate: ',ivcoord
	  call exit(4)
	end if
c
c..Create the v5d file.
      istatus = v5dcreate( v5dfile, numtimes, numvars, nr, nc, nl,
     *                     varname, times, dates, compressmode,
     *                     projection, proj_args, vertical, vert_args )
	if(istatus.eq.0) then
	  write(6,*) 'PUTV5D ERROR. v5dcreate failure.'
	  write(6,*) v5dfile
	  call exit(5)
	end if
c
	initv5d=1
c
      end if
c
      if(initv5d.gt.0) then
c
c..write data
c
	if(nplevel(np).gt.1) then
	  izz1=iz1
	  izz2=iz2
	  izzs=izs
	else
	  izz1=1
	  izz2=1
	  izzs=1
	end if
        n=0
        do k=izz1,izz2,izzs
	  do i=ix1,ix2,ixs
	    do j=iy1,iy2,iys
	      n=n+1
	      G(n)=f3d(i,j,k)
	    end do
	  end do
        end do
c
        istatus = v5dwrite( nt, np, G )
        if(istatus.eq.0) then
	  write(6,*) 'PUTV5D ERROR. v5dwrite failure.'
	  call exit(5)
        end if
c
      end if
c
      if(initv5d.lt.0) then
c
c..close vis5d data file
c
	istatus = v5dclose()
	if(istatus.eq.0) then
	  write(6,*) 'PUTV5D ERROR. v5dclose failure.'
	  call exit(5)
	end if
c
	return
c
      end if
c
      return
      end
