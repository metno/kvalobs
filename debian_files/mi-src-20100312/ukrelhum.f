        program ukrelhum
c
c  Extrapolation of UK relative humidity fields.
c  All missing levels wil be filled if fields exist in the
c  500, 700 and 850 hPa levels (without any undefined values).
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
c  DNMI/FoU  01.10.1993  Anstein Foss
c  DNMI/FoU  07.10.1993  Anstein Foss
c  DNMI/FoU  12.03.1994  Anstein Foss
c  DNMI/FoU  17.01.1995  Anstein Foss
c  DNMI/FoU  24.10.2003  Anstein Foss ... SGI+Linux (a few "1" -> "1.")
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c-----------------------------------------------------------------------
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for ukrelhum.f
c
c  maxsiz : max. field size
c..maxinh : the maximum no. of grids, forecast lengths and levels
c           (when requesting felt file information)
c
ccc     integer maxsiz,maxinh
ccc     parameter (maxsiz=32000)
ccc     parameter (maxinh=128)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
        include 'ukrelhum.inc'
c
        integer   ierr(3),ihelp(6)
        integer*2 idata(20+maxsiz)
        real      field(maxsiz,3)
c
        integer   ifound(maxinh)
        integer*2 inh(16,maxinh)
        integer*2 ingrid(2,maxinh)
        integer*2 intime(2,maxinh)
c
        character*256 option,filnam
c
c..possibly force extrapolation even when levels exist (iforce=1)
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
          write(6,*) ' usage:  ukrelhum felt_file'
          write(6,*) '    or:  ukrelhum -e felt_file'
          write(6,*) '  option -e : extrapolation also if fields exist'
          stop
        endif
c
        write(6,*) 'Felt file: ',filnam(1:lenstr(filnam,1))
c--------------------------------------------
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
        iexist=1
        ninh=maxinh
        do i=1,16
          inh(i,1)=-32767
        end do
        inh( 1,1)=74
        inh( 2,1)=-32766
        inh(11,1)=1
        inh(12,1)=10
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
          write(6,*) 'no UK relative humidity fields found'
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
c  find all the forecast lengths on the file
c  with existing rel.hum. fields (param. 10) in p levels
c
        ireq=2
        iexist=1
        ninh=maxinh
        do i=1,16
          inh(i,1)=-32767
        end do
        inh( 1,1)=iprod
        inh( 2,1)=igrid
        inh(10,1)=-32766
        inh(11,1)=1
        inh(12,1)=10
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
c  find all the levels
c  with existing rel.hum. fields (param. 10) in p levels
c
          ireq=1
          iexist=0
          ninh=maxinh
          do i=1,16
            inh(i,1)=-32767
          end do
          inh( 1,1)=iprod
          inh( 2,1)=igrid
          inh( 9,1)=intime(1,nt)
          inh(10,1)=intime(2,nt)
          inh(11,1)=1
          inh(12,1)=10
          inh(14,1)=0
c
          call qfelt(iunit,ireq,iexist,ninh,inh,ifound,nfound,
     +               iend,ierror,ioerr)
c
          if(nfound.eq.0 .or. ierror.ne.0) then
            write(6,*) 'qfelt failure - inquire rel.hum. levels'
            write(6,*) '  nfound,ierror,ioerr: ',nfound,ierror,ioerr
          end if
c
          n500=0
          n700=0
          n850=0
          missin=0
          do n=1,nfound
            if(ifound(n).gt.0) then
              if(inh(13,n).eq.500) n500=n
              if(inh(13,n).eq.700) n700=n
              if(inh(13,n).eq.850) n850=n
            else
              missin=missin+1
            end if
          end do
          nlevels=nfound
c
          if(iforce.eq.1) then
            missin=0
            do n=1,nfound
              if(n.ne.n500 .and. n.ne.n700 .and. n.ne.n850)
     +                                            ifound(n)=0
              if(ifound(n).lt.1) missin=missin+1
            end do
          end if
c
c  don't start extrapolation if there are undefined values
c  in input levels
c
          nundef=0
c
          if(n500.gt.0 .and. n700.gt.0 .and. n850.gt.0
     +                                 .and. missin.gt.0) then
            do n=1,nlevels
              k=0
              if(n.eq.n500) k=1
              if(n.eq.n700) k=2
              if(n.eq.n850) k=3
              if(k.gt.0 .and. nundef.eq.0) then
                limit=20+maxsiz
                ierr(1)=0
                ierr(2)=0
                ierr(3)=0
                call rfelt(iunit,ip,inh(1,n),idata,limit,ierr,ihelp)
                if(ip.ne.1) then
                  write(6,*) 'rfelt error ',ip,' for ',inh(13,n)
                  nundef=1
                else
                  nx=idata(10)
                  ny=idata(11)
                  iscale=idata(20)
                  scale=10.**iscale
                  lfield=nx*ny
                  do i=1,lfield
                    if(idata(20+i).eq.-32767) nundef=1
                  end do
                  if(nundef.eq.0) then
                    do i=1,lfield
                      field(i,k)=scale*real(idata(20+i))
                    end do
                  end if
                end if
              end if
            end do
          end if
c
          if(nlevels.eq.0) then
            continue
c
          elseif(missin.eq.0) then
            write(6,fmt='(1x,sp,i6,ss,2x,''ok'')') intime(2,nt)
c
          elseif(n500.eq.0 .and. n700.eq.0 .and. n850.eq.0) then
            write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''missing all  input levels'')') intime(2,nt)
c
          elseif(n500.eq.0 .or. n700.eq.0 .or. n850.eq.0) then
            write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''missing some input levels'')') intime(2,nt)
c
          elseif(nundef.gt.0) then
            write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''undefined values in input'')') intime(2,nt)
c
          else
            write(6,fmt='(1x,sp,i6,ss,2x,
     +            ''extrapolation now'')') intime(2,nt)
c
            ihpain=0
            ilevel=0
c
            do n=1,nlevels
c
              if(ifound(n).lt.1) then
c
                ihpa=inh(13,n)
                k=1
                if(ihpa.gt.700) k=2
                if(ihpa.gt.850) k=3
c
                reduc=1.
                if(ihpa.lt.500) reduc=0.6
                if(ihpa.lt.400) reduc=0.4
                if(ihpa.lt.300) reduc=0.2
                if(ihpa.lt.250) reduc=0.05
c
                idata(7)=ihpa
                ldata=20+maxsiz
                call mwfelt(2,filnam,iunit,1,lfield,field(1,k),
     +                      reduc,ldata,idata,ierror)
                if(ierror.ne.0) goto 300
c
              end if
c
            end do
c
          end if
c
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
        write(6,*) 'ukrelhum ERROR exit (2)'
ccc     stop 2
        call exit(2)
c
  990   continue
c
        end
