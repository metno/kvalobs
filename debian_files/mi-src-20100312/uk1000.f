        program uk1000
c
c  Copy UK surface fields to 1000 hPa.
c  z1000=(mslp-1000.)*8   u1000=u10m   v1000=v10m   t1000=t2m
c  Input from and output to the same felt file.
c  All forecast lengths and grids for producer 74 (UK) are treated.
c  Works on any grid type (polarstereographic, geographic etc.)
c  No output field are made if there are missing values in input field.
c  No quality control.
c
c-----------------------------------------------------------------------
c      DNMI library subroutines:  mwfelt (+rlunit,wcfelt,wfelt,daytim)
c                                 qfelt
c                                 rfelt
c
c-----------------------------------------------------------------------
c  DNMI/FoU  17.01.1995  Anstein Foss
c  DNMI/FoU  24.10.2003  Anstein Foss ... SGI+Linux (a few "1" -> "1.")
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c-----------------------------------------------------------------------
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for uk1000.f
c
c  maxsiz : max. field size
c..maxinh : the maximum no. of grids, forecast lengths and levels
c           (when requesting felt file information)
c
ccc     integer maxsiz,maxinh
ccc     parameter (maxsiz=32000)
ccc     parameter (maxinh=200)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
        include 'uk1000.inc'
c
	parameter (npar=4)
c
        integer*2   inhsurf(3,npar,2)
        character*1 par1000(npar)
c
        integer   ierr(3),ihelp(6)
        integer*2 idata(20+maxsiz)
c
        integer   ifound(maxinh)
        integer*2 inh(16,maxinh)
        integer*2 ingrid(2,maxinh),intime(2,maxinh)
c
        character*256 option,filnam
	character*8  parout
c
	data ihelp/6*0/
c
c....................mslp.......u10m.......v10m.......t2m.......
	data inhsurf/2,58,1000, 2,33,1000, 2,34,1000, 2,31,1000,
c....................z1000......u1000......v1000......t1000.....
     +		     1, 1,1000, 1, 2,1000, 1, 3,1000, 1, 4,1000/
c
	data par1000/'z','u','v','t'/
c
c..possibly force copy even when output fields exist (iforce=1)
        iforce=0
c
c--------------------------------------------
        narg=iargc()
        ierror=0
	filnam='*'
	do n=1,narg
	  call getarg(n,option)
	  if(option.eq.'-e') then
	    iforce=1
	  elseif(option(1:1).ne.'-' .and. filnam.eq.'*') then
	    filnam=option
	  else
	    ierror=1
	  end if
	end do
	if(filnam.eq.'*') ierror=1
c
        if(ierror.ne.0) then
          write(6,*) ' usage:  uk1000 felt_file'
          write(6,*) '    or:  uk1000 -e felt_file'
          write(6,*) '   option -e : copy also if fields exist'
          stop
        endif
c
        write(6,*) 'Felt file: ',filnam(1:lenstr(filnam,1))
c--------------------------------------------
c
	if(maxinh.lt.npar*2) call exit(201)
c
c  error exit control
        istop=1
c
c  file unit for input/output felt file
        iunit=20
c
c  open the input/output file
c
        call mwfelt(1,filnam,iunit,1,1,1.,1.,1,idata,ierror)
        if(ierror.ne.0) goto 910
c
c
c  find the UK grids on the felt file
c
        ireq=2
        iexist=0
        ninh=maxinh
        do i=1,16
          inh(i,1)=-32767
        end do
        inh( 1,1)=74
        inh( 2,1)=-32766
        inh(11,1)=1
        inh(13,1)=1000
        inh(14,1)=0
c
        call qfelt(iunit,ireq,iexist,ninh,inh,ifound,nfound,
     +             iend,ierror,ioerr)
c
        if(ierror.ne.0) then
          write(6,*) 'qfelt failure - inquire UK grids'
          write(6,*) '  nfound,ierror,ioerr: ',nfound,ierror,ioerr
          goto 910
        end if
c
        if(nfound.eq.0) then
          write(6,*) 'no UK 1000 hPa height fields defined in file'
          istop=0
          goto 910
        end if
c
        do n=1,nfound
          ingrid(1,n)=inh(1,n)
          ingrid(2,n)=inh(2,n)
        end do
        ngrid=nfound
c
        do 100 ngrd=1,ngrid
c
        iprod=ingrid(1,ngrd)
        igrid=ingrid(2,ngrd)
        write(6,*) 'Producer and grid: ',iprod,igrid
c
c  find all the forecast lengths on the file (with 1000 hPa index)
c
        ireq=2
        iexist=0
        ninh=maxinh
        do i=1,16
          inh(i,1)=-32767
        end do
        inh( 1,1)=iprod
        inh( 2,1)=igrid
        inh(10,1)=-32766
        inh(11,1)=1
        inh(13,1)=1000
        inh(14,1)=0
c
        call qfelt(iunit,ireq,iexist,ninh,inh,ifound,nfound,
     +             iend,ierror,ioerr)
c
        if(nfound.eq.0 .or. ierror.ne.0) then
          write(6,*) 'qfelt failure - inquire forecast lengths'
          write(6,*) '  nfound,ierror,ioerr: ',nfound,ierror,ioerr
          goto 100
        end if
c
        do n=1,nfound
          intime(1,n)=inh( 9,n)
          intime(2,n)=inh(10,n)
        end do
        ntime=nfound
c
c  time loop (forecast lengths)
c
        do nt=1,ntime
c
          ireq=3
          iexist=0
          ninh=0
	  do n=1,2
	    do np=1,npar
              ninh=ninh+1
              do i=1,16
                inh(i,ninh)=-32767
	      end do
              inh( 1,ninh)=iprod
              inh( 2,ninh)=igrid
              inh( 9,ninh)=intime(1,nt)
              inh(10,ninh)=intime(2,nt)
              inh(11,ninh)=inhsurf(1,np,n)
              inh(12,ninh)=inhsurf(2,np,n)
              inh(13,ninh)=inhsurf(3,np,n)
              inh(14,ninh)=0
	    end do
          end do
c
          call qfelt(iunit,ireq,iexist,ninh,inh,ifound,nfound,
     +               iend,ierror,ioerr)
c
          if(nfound.eq.0 .or. ierror.ne.0) then
            write(6,*) 'qfelt failure - inquire surface and 1000 hPa'
            write(6,*) '  nfound,ierror,ioerr: ',nfound,ierror,ioerr
          end if
c
	  if(iforce.eq.1) then
	    do n=npar+1,npar*2
	      if(ifound(n).gt.0) ifound(n)=0
	    end do
	  end if
c
	  parout=' '
	  numout=0
	  numok =0
c
	  do n=1,npar
c
	    if(ifound(n+npar).ne.0) then
	      numok=numok+1
	    elseif(ifound(n).gt.0 .and. ifound(n+npar).eq.0) then
              limit=20+maxsiz
              ierr(1)=0
              ierr(2)=0
              ierr(3)=0
              call rfelt(iunit,ip,inh(1,n),idata,limit,ierr,ihelp)
              if(ip.ne.1) then
                write(6,*) 'rfelt error ',ip,' for param. ',inh(12,n)
              else
                nx=idata(10)
                ny=idata(11)
                iscale=idata(20)
                scale=10.**iscale
                lfield=nx*ny
                nundef=0
                do i=1,lfield
                  if(idata(20+i).eq.-32767) nundef=1
                end do
		if(nundef.eq.0) then
		  if(inh(12,n).eq.58) then
c..from mslp to z(1000hPa)
                    do i=1,lfield
		      pmsl=scale*real(idata(20+i))
		      z1000=(pmsl-1000.)*8.
		      idata(20+i)=nint(z1000)
                    end do
		    idata(20)=0
		  end if
		  idata(5)=inhsurf(1,n,2)
		  idata(6)=inhsurf(2,n,2)
		  idata(7)=inhsurf(3,n,2)
                  ldata=20+nx*ny
                  call mwfelt(2,filnam,iunit,0,0,0.,
     +                        1.,ldata,idata,ierror)
                  if(ierror.ne.0) goto 300
	          numout=numout+1
	          parout(numout*2:numout*2)=par1000(n)
		end if
	      end if
	    end if
c
	  end do
c
	  if(numok.eq.npar) then
	    write(6,fmt='(1x,sp,i6,ss,2x,''ok'')') intime(2,nt)
	  elseif(numout.gt.0) then
	    write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''surface->1000hPa: '',a8)') intime(2,nt),parout
	  elseif(iforce.eq.0) then
	    write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''no (new) input data'')') intime(2,nt)
	  else
	    write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''no input data'')') intime(2,nt)
	  end if
c
c.......end do nt=1,ntime
        end do
c
  100   continue
c
c  error exit control: no severe error
        istop=0
c
c  update file header and close file
  300   call mwfelt(3,filnam,iunit,1,1,1.,1.,1,idata,ierror)
c
        if(istop.eq.0) goto 990
        goto 920
c
  910   close(iunit)
        if(istop.eq.0) goto 990
c
  920   continue
        write(6,*) 'uk1000 ERROR exit (2)'
ccc     stop 2
        call exit(2)
c
  990   continue
c
        end
