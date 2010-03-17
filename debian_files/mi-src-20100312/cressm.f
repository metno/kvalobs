      program cressm
c
c        skalar-analyse, 'observasjoner' -> felt
c
c        grids: polarstereographic, geographic, spherical (rotated)
c
c--------------
c cressm.input:
c======================================================================
c *** cressm.input  ('cressm.input' for the cressm program, cressm.f)
c ***
c *=> Cressman analysis of SST
c *=>
c *=> Command format:
c *=> cressm cressm.input 1814 sstdig.dat ect0m2006.dat par1814.dat
c *=>                    <grid> <input>   <felt.in>     <felt.out>
c *** .......(#1).........#2...#3.........#4............#5.........
c ***---------------------------------------------------------------
c **  '#n' = command line arg. no n (n>1)    '$var' = env. var.
c **---------------------------------------------------------------
c *>
c 1000, '#2'           <-- 1000, 'grid_nummer' ... brukes hvis 88,0,...
c 1, '#3'              <-- file_id, 'file' ... input digitalisert SST
c 2, '#4'              <-- file_id, 'file' ... input/output felt-file
c 3, '#5'              <-- file_id, 'file' ... input/output felt-file
c 0, '*'               <-- file_id, 'file' ... end of file list
c 6, 1                 <-- ant. scan(0=end), print.statistikk (0,1,2)
c  1,  1,  2,   2,   2,  1           <-- ant. iterasjoner
c 16.,11., 7.,  4.,  2., 1.          <-- scan-radius (grid enheter)
c -1, 0., 00., 1                 <-- kguess(-1:K->C),fguess,rguess, ishap(0/1)
c 0, 0.                          <-- kondat(0/1),diflim
c 1, 1,0                         <-- input  line file_id, kobs1,kobs2
c 2, 98,0000,1,0,2, 30,1000,0    <-- input  felt file_id, in(1:2,9:14) ec.t0m
c 3, 88,0000,4,0,2,103,1000,0    <-- output felt file_id, in(1:2,9:14)
c 0, 0                 <-- ant. skalaer(0=end), statistikk (0,1,2)
c======================================================================
c
c----------------------------------------------------------------------
c
c    subrutiner: interp
c                getdat .... digitaliserte data ('linjer')
c                feltin
c                feltut
c                stati
c                cress
c                cwtab
c                blxy1
c                guess1
c                shap2
c                rdat2 ..... temp(2m) evt. justert med 'tsvar'
c                rdat3 ..... tw (sst) og alt mulig annet !!! (ascii file)
c
c----------------------------------------------------------------------
c  DNMI library subroutines: rlunit
c                            mrfelt
c                            mwfelt
c                            daytim
c                            prhelp
c                            rcomnt
c                            getvar
c                            gridpar
c                            mapfield
c                            xyconvert
c
c----------------------------------------------------------------------
c  DNMI/FoU  xx.xx.1989  Anstein Foss ... ibm
c  DNMI/FoU  25.09.1993  Anstein Foss ... unix
c  DNMI/FoU  07.12.1993  Anstein Foss ... new rdat3
c  DNMI/FoU  23.03.1994  Anstein Foss ... mrfelt/mwfelt
c  DNMI/FoU  16.05.1995  Anstein Foss ... misc. grids
c  DNMI/FoU  14.06.1996  Anstein Foss ... xyconvert
c  DNMI/FoU  04.09.1997  Anstein Foss ... kguess=-1 : kelvin->celsius
c  DNMI/FoU  22.10.1997  Anstein Foss ... kguess=-2 : k->c + sea_extend
c  DNMI/FoU  30.10.2003  Anstein Foss ... SGI+Linux version
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c----------------------------------------------------------------------
c
c
      include 'cressm.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c  include file for cressm.f
c
c  maxsiz: max field size
c  maxobs: max no. of observations or line positions
c  mxscan: max no. scans in each analysis
c  maxfil: max no. files
c  maxtab: table size for weights
c  ldata:  max size of buffer for field i/o
c
ccc   parameter (maxsiz=100000)
ccc   parameter (maxobs=50000)
ccc   parameter (mxscan=24)
ccc   parameter (maxfil=16)
ccc   parameter (maxtab=2500)
ccc   parameter (ldata=20+maxsiz+50)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      common/fl/felt(maxsiz),felt2(maxsiz),felt3(maxsiz)
      common/map/xm(maxsiz),ym(maxsiz)
      common/cgrid/igtype,grid(6)
c
      real      fsave(maxsiz)
c
      integer   niter(mxscan)
      real      rscan(mxscan)
c
      integer*2 ini(16),ino(16),inx(16)
c
      character*256 filnam(maxfil)
      character*256 finput,cinput
c
c
      istop=0
c
      do n=1,maxfil
        filnam(n)='*'
      end do
c
c--------------------------------------------------------------------
c..file unit for 'cressm.input'
      iuinp=9
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: cressm <cressm.input>'
        write(6,*) '      or: cressm <cressm.input> <arguments>'
        write(6,*) '      or: cressm <cressm.input> ?     (to get help)'
        write(6,*)
        stop 1
      end if
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error:'
        write(6,*) finput
        call exit(1)
      end if
c
      if(narg.gt.1) then
        call getarg(2,cinput)
        if(cinput.eq.'?') then
          call prhelp(iuinp,'*=>')
          close(iuinp)
          stop
        end if
      end if
c
      write(6,*) 'reading input file:'
      write(6,*) finput
      nlines=0
      iprhlp=0
      ierror=0
      iend=0
      igrid=0
c
c..read comment lines
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) write(6,*) 'error reading comment lines.'
c
c..read file_id's and file names
c
      do while (iend.eq.0 .and. ierror.eq.0)
c
        nlines=nlines+1
        read(iuinp,*,iostat=ios) idfile,cinput
        if(ios.ne.0) then
          ierror=1
	elseif(idfile.eq.1000) then
c..grid no.
          call getvar(1,cinput,1,1,1,ierror)
	  if(ierror.eq.0) then
	    read(cinput,*,iostat=ios) igrid
	    if(ios.ne.0) ierror=1
	  end if
          if(ierror.ne.0) iprhlp=1
        elseif(idfile.lt.1) then
          iend=1
        elseif(idfile.gt.maxfil) then
          write(6,*) 'illegal file_id: ',idfile
          write(6,*) '    max allowed: ',maxfil
          ierror=1
        elseif(filnam(idfile).eq.'*') then
          call getvar(1,cinput,1,1,1,ierror)
          if(ierror.ne.0) iprhlp=1
          filnam(idfile)=cinput
        else
          write(6,*) 'file_id defined more than once: ',idfile
          ierror=1
        end if
c
      end do
c
      if(ierror.ne.0) then
        write(6,*) 'error at line no. ',nlines,'   (or below)'
        if(iprhlp.eq.1) then
          write(6,*) 'Help from ''cressm.input'' file:'
          write(6,*) finput
          call prhelp(iuinp,'*=>')
        end if
        istop=1
        goto 990
      end if
c--------------------------------------------------------------------
c
c..file unit for all data files
      iunit=20
c
   50 continue
c
c        nscan: antall analyser (dvs. analyse-skalaer)
c        istat: statistikk
c               istat=0: ingen statistikk
c                    =1: f@r f@rste og etter hvert scan (hver radius)
c                    =2: f@r f@rste og etter hver iterasjon
c
c
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) nscan, istat
c
c..end ?
      if(nscan.lt.1) goto 990
c
      if(nscan.lt.0 .or. nscan.gt.mxscan) then
        write(6,*) ' **** antall scan:',nscan
        write(6,*) ' ****  max lovlig:',mxscan
        istop=3
        goto 990
      end if
c
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) (niter(i),i=1,nscan)
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) (rscan(i),i=1,nscan)
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) kguess,fguess,rguess, ishap
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) kondat,diflim
c
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) idobs,kobs1,kobs2
      if(idobs.lt.1 .or. idobs.gt.maxfil) goto 910
      if(filnam(idobs).eq.'*') goto 910
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) idfi,ini(1),ini(2),(ini(i),i=9,14)
      if(idfi.lt.1 .or. idfi.gt.maxfil) goto 910
      if(filnam(idfi).eq.'*') goto 910
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) idfo,ino(1),ino(2),(ino(i),i=9,14)
      if(idfo.lt.1 .or. idfo.gt.maxfil) goto 910
      if(filnam(idfo).eq.'*') goto 910
c
      if(ini(2).eq.0) ini(2)=igrid
      if(ino(2).eq.0) ino(2)=igrid
c
c
      if(kobs1.eq.1) then
c......... leser inn gjetnings-felt
        call feltin(iunit,filnam(idfi),ini,nx,ny,ierror)
        if(ierror.ne.0) stop 117
c......... leser inn observasjoner (posisjon gitt med b,l)
c                file fra 'digit1' e.l.  ('linje'-data)
        call getdat(iunit,filnam(idobs))
c ........ fra b,l til x,y og fjerner observasjoner utenfor gridet'
        call blxy1(nx,ny)
c
      elseif(kobs1.eq.2) then
c..leser inn observasjoner (posisjon gitt med b,l)
c                formatert file fra 't2save' e.l.  ('punkt'-data)
c          leser inn gjetning-felt og foretar b,l -> x,y
        call rdat2(iunit,filnam(idobs),kobs2,filnam(idfi),ini,nx,ny)
c
      elseif(kobs1.eq.3) then
c..leser inn observasjoner (posisjon gitt med b,l)
c                formatert file fra 'twsave' e.l.  ('punkt'-data)
c          leser inn gjetning-felt og foretar b,l -> x,y
        call rdat3(iunit,filnam(idobs),kobs2,filnam(idfi),ini,nx,ny)
c
      else
        write(6,*) ' ****** ukjent input:  kobs1= ',kobs1
        stop 117
      end if
c
c..beregner kartfaktor
      call mapfield(1,0,igtype,grid,nx,ny,xm,ym,0.,hx,hy,ierror)
      if(ierror.ne.0) then
        write(6,*) 'MAPFIELD ERROR: ',ierror
        write(6,*) '     grid type: ',igtype
        stop 255
      end if
c
      if(kguess.gt.0) then
c..lager gjetningsfelt
        if(rguess.lt.1.) rguess=0.
        write(6,*) '   kguess: ',kguess
        write(6,*) '   fguess: ',fguess
        write(6,*) '   rguess: ',rguess
        call guess1(fguess,rguess,nx,ny,xm,ym,felt,felt2,felt3)
      elseif(kguess.eq.-1) then
	write(6,*) '   kguess = -1 : guess field, Kelvin -> Celsius'
	do i=1,nx*ny
	  felt(i)=felt(i)-273.15
	end do
      elseif(kguess.eq.-2) then
	write(6,*) '   kguess = -2 : guess field, Kelvin -> Celsius'
	write(6,*) '       then read land/sea(102) from output file'
	write(6,*) '       and put sea values on nearest landpoints'
	do i=1,nx*ny
	  felt3(i)=felt(i)-273.15
	end do
	inx( 1)=ino(1)
	inx( 2)=ino(2)
	inx( 9)=4
	inx(10)=0
	inx(11)=2
	inx(12)=102
	inx(13)=1000
	inx(14)=0
        call feltin(iunit,filnam(idfo),inx,nx2,ny2,ierror)
        if(ierror.eq.0) then
	  do i=1,nx*ny
	    felt2(i)=felt(i)
	    felt(i)=felt3(i)
	  end do
	  nextend=nint(rguess)
	  if(nextend.lt.1 .or. nextend.gt.nx/4
     +			  .or. nextend.gt.ny/4) nextend=4
	  call seaextend(nextend,nx,ny,felt,felt2,felt3)
	else
	  write(6,*) '  continue without sea extension'
	  do i=1,nx*ny
	    felt(i)=felt3(i)
	  end do
	end if
      end if
c
      kinter=0
c
      if(istat.ne.0) then
        write(6,*) ' statistics of input data'
        if(kinter.ne.1) call interp(nx,ny,felt)
        kinter=1
        call stati
      end if
c
      konitr=0
c
c        'data-kontroll' ( /obs-felt/ < diflim => o.k.)
c
   80 if(kondat.eq.1 .and. diflim.gt.0.) then
        if(kinter.ne.1) call interp(nx,ny,felt)
        kinter=1
        call datkon(diflim,nremov)
        if(konitr.ne.0) then
c..get guess field if iterations before data control
          do i=1,nx*ny
            felt(i)=fsave(i)
          end do
          kinter=0
        end if
        if(nremov.gt.0 .and. istat.ne.0) then
          write(6,*) ' statistics after data control'
          call interp(nx,ny,felt)
          kinter=1
          call stati
        elseif(nremov.gt.0) then
          kinter=0
        end if
      elseif(kondat.lt.0 .and. diflim.gt.0.) then
        nscana=nscan
        nscan=-kondat
        write(6,*) ' iterations before data control: ',nscan
c..store guess field
        do i=1,nx*ny
          fsave(i)=felt(i)
        end do
      else
        kondat=0
      end if
c
      do iscan=1,nscan
c
        call cwtab(rscan(iscan))
c
c..iterasjoner
        do n=1,niter(iscan)
c
          write(6,*) ' scan,iteration: ',iscan,n
          if(kinter.ne.1) call interp(nx,ny,felt)
          kinter=1
          call cress(rscan(iscan),nx,ny,xm,ym,felt,felt2,felt3)
          kinter=0
c
          if(istat.eq.2 .or. (istat.eq.1 .and. n.eq.niter(iscan))) then
            write(6,*) ' statistics'
            call interp(nx,ny,felt)
            kinter=1
            call stati
          end if
c
        end do
c
      end do
c
      if(kondat.lt.0) then
        write(6,*) ' finished iterations before data control'
        nscan=nscana
        kondat=1
        konitr=1
        goto 80
      end if
c
      if(ishap.eq.1) then
        write(6,*) ' smoothing: shap2 filter'
        call shap2(nx,ny,felt,felt2)
        if(istat.gt.0) then
          write(6,*) ' statistics after smoothing'
          call interp(nx,ny,felt)
          kinter=1
          call stati
        end if
      end if
c
c..analysert felt til file
      call feltut(iunit,filnam(idfo),ino,nx,ny)
c
      goto 50
c
  910 write(6,*) 'error at line no. ',nlines,'  (or below)  in file:'
      write(6,*) finput
      istop=2
c
  990 continue
      close(iuinp)
      if(istop.gt.0) then
        write(6,*) 'cressm ***** ERROR *****'
        if(istop.gt.255) istop=255
        call exit(istop)
      end if
c
      end
c
c***********************************************************************
c
      subroutine interp(nx,ny,f)
c
c        interpolasjon
c
      include 'cressm.inc'
c
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
c
      integer nx,ny
      real    f(nx,ny)
c
      nxm1=nx-1
      nym1=ny-1
c
      do n=1,nobs
c
        x=xobs(n)
        y=yobs(n)
        i=x
        j=y
        if(i.gt.1 .and. i.lt.nxm1 .and. j.gt.1 .and. j.lt.nym1) then
c..bessel interpolation (using the 16 nearest gridpoints)
          x1=x-i
          x2=1.-x1
          x3=-0.25*x1*x2
          x4=-0.1666667*x1*x2*(x1-0.5)
          a=x3-x4
          b=x2-x3+3.*x4
          c=x1-x3-3.*x4
          d=x3+x4
          y1=y-j
          y2=1.-y1
          y3=-0.25*y1*y2
          y4=-0.1666667*y1*y2*(y1-0.5)
          ay=y3-y4
          by=y2-y3+3.*y4
          cy=y1-y3-3.*y4
          dy=y3+y4
          t1=a*f(i-1,j-1)+b*f(i,j-1)+c*f(i+1,j-1)+d*f(i+2,j-1)
          t2=a*f(i-1,j  )+b*f(i,j  )+c*f(i+1,j  )+d*f(i+2,j  )
          t3=a*f(i-1,j+1)+b*f(i,j+1)+c*f(i+1,j+1)+d*f(i+2,j+1)
          t4=a*f(i-1,j+2)+b*f(i,j+2)+c*f(i+1,j+2)+d*f(i+2,j+2)
          obsint(n)=ay*t1+by*t2+cy*t3+dy*t4
        else
c..bilinear interpolation (using the 4 nearest gridpoints)
          x1=x-i
          y1=y-j
          a=(1.-y1)*(1.-x1)
          b=(1.-y1)*x1
          c=y1*(1.-x1)
          d=y1*x1
          obsint(n)=a*f(i,j)+b*f(i+1,j)+c*f(i,j+1)+d*f(i+1,j+1)
        end if
c
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine getdat(iunit,filnam)
c
c        input fra file 'filnam'
c
c        nb| posisjoner gitt som b,l (evt. ogs$ punkt utenfor gridet)
c
      include 'cressm.inc'
c
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
c
      common/st/nrst(maxobs)
c
      character*(*) filnam
c
      parameter (maxout=256)
      integer*2 iout(2,maxout)
c
      integer   itime(4)
      character*50 text
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
c             (2,3): klokke
c             (1,4): antall linjer
c             (2,4): totalt antall posisjoner
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
      write(6,*) ' -----  getdat  -----'
c
      nobs=0
c
      write(6,*) 'reading file:'
      write(6,*) filnam
c
      call rlunit(lrunit)
      irec=0
      open(iunit,file=filnam,
     *           access='direct',form='unformatted',
     *           recl=1024/lrunit,
     *           status='old',iostat=ios,err=900)
c
      irec=1
      read(iunit,rec=irec,iostat=ios,err=910) iout
c
      nrec=    iout(1,1)
      lpout=   iout(2,1)
      itime(1)=iout(1,2)
      itime(2)=iout(2,2)
      itime(3)=iout(1,3)
      itime(4)=iout(2,3)
      nlin=    iout(1,4)
      npos=    iout(2,4)
      igscal=  iout(1,5)
      ivscal=  iout(2,5)
      nptxt=   iout(1,6)
      lntxt=   iout(2,6)
c
      gscal=10.**igscal
      vscal=10.**ivscal
c
      l=len(text)
      lntxt=min(lntxt,l)
      text=' '
      k=0
      do n=1,nptxt
	ichr=iout(1,6+n)
	ichr1=iand(ishft(ichr,-8),255)
	ichr2=iand(ichr,255)
	ichr=iout(2,6+n)
	ichr3=iand(ishft(ichr,-8),255)
	ichr4=iand(ichr,255)
	if(k+1.le.lntxt) text(k+1:k+1)=char(ichr1)
	if(k+2.le.lntxt) text(k+2:k+2)=char(ichr2)
	if(k+3.le.lntxt) text(k+3:k+3)=char(ichr3)
	if(k+4.le.lntxt) text(k+4:k+4)=char(ichr4)
	k=k+4
      end do
c
      write(6,*) 'identifikasjon: ',text(1:lntxt)
      write(6,*) '           tid: ',(itime(i),i=1,4)
c
      if(npos.gt.maxobs) goto 920
c
      np=6+nptxt
      iobs=0
c
      do 100 l=1,nlin
c
      if(np.eq.maxout) then
      irec=irec+1
      read(iunit,rec=irec,iostat=ios,err=910) iout
      np=0
      end if
      np=np+1
c
      ivalue=iout(2,np)
      value=vscal*ivalue
c
      linlen=iout(1,np)
      nr=(np+linlen+maxout-1)/maxout
      if(np.eq.maxout) nr=nr-1
      i2=0
c
      do 110 ir=1,nr
      if(np.eq.maxout) then
      irec=irec+1
      read(iunit,rec=irec,iostat=ios,err=910) iout
      np=0
      end if
      i0=i2
      i2=min(i2+maxout-np,linlen)
      ni=i2-i0
      do 120 i=1,ni
c..NOTE: xobs()=longitude  yobs()=latitude
      iobs=iobs+1
      yobs(iobs)=gscal*iout(1,np+i)
      xobs(iobs)=gscal*iout(2,np+i)
  120 dobs(iobs)=value
      np=np+ni
  110 continue
c
  100 continue
c
      nobs=iobs
c
c        sjekk input
c
      if(irec.ne.nrec .or. nobs.ne.npos) goto 930
c
      write(6,*)
      write(6,*) 'input fra file:'
      write(6,*) '      antall posisjoner:',nobs
c
c        'stasjons-nr'
      do 180 iobs=1,nobs
  180 nrst(iobs)=iobs
c
      close(iunit)
c
      return
c
  900 write(6,*) 'open error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      goto 980
c
  910 write(6,*) 'read error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      write(6,*) 'record: ',irec
      goto 980
c
  920 write(6,*) 'error'
      write(6,*) 'file: ',filnam
      write(6,*) '      no. of positions:',npos
      write(6,*) 'program:'
      write(6,*) '  max no. of positions:',maxobs
      goto 980
c
  930 write(6,*) 'error'
      write(6,*) 'file: ',filnam
      write(6,*) 'record 1:  no. of records:   ',nrec
      write(6,*) '           no. of positions: ',npos
      write(6,*) 'read:      no. of records:   ',irec
      write(6,*) '           no. of positions: ',nobs
      goto 980
c
  980 stop 117
c
      end
c
c***********************************************************************
c
      subroutine feltin(iunit,filnam,in,nx,ny,ierror)
c
      include 'cressm.inc'
c
      common/cgrid/igtype,grid(6)
      common/fl/felt(maxsiz),felt2(maxsiz),felt3(maxsiz)
      common/inout/idata(ldata)
      integer*2 idata
c
      integer   iunit,nx,ny,ierror
      integer*2 in(16)
      character*(*) filnam
c
      integer*2 idfile(32)
      double precision sum
c
      write(6,*) ' -----  feltin  -----'
c
      call mrfelt(1,filnam,iunit,in,2,1,1.,1.,32,idfile,ierror)
      if(ierror.ne.0) goto 900
c
      iyear =idfile(5)
      month =idfile(6)/100
      iday  =idfile(6)-(idfile(6)/100)*100
      iutc  =idfile(7)
      iyearu=idfile(2)
      monthu=idfile(3)/100
      idayu =idfile(3)-(idfile(3)/100)*100
      iutcu =idfile(4)
      write(6,*) 'file:'
      write(6,*) filnam
      write(6,fmt='(6x,''date/time'',16x,''last update'')')
      write(6,fmt='(2(6x,2(i2.2,'':''),i4.4,1x,i4.4,'' utc''))')
     *        iday,month,iyear,iutc,idayu,monthu,iyearu,iutcu
c
      in(3)=-32767
      in(4)=-32767
      in(5)=-32767
      call mrfelt(2,filnam,iunit,in,2,maxsiz,felt,1.,
     *              ldata,idata,ierror)
      ierr=ierror
c
      call mrfelt(3,filnam,iunit,in,2,1,1.,1.,1,idfile,ierror)
      if(ierror.eq.0) ierror=ierr
      if(ierror.ne.0) goto 900
c
      call gridpar(+1,ldata,idata,igtype,nx,ny,grid,ierror)
      if(ierror.ne.0) write(6,*) 'GRIDPAR ERROR: ',ierror
      if(ierror.ne.0) goto 900
c
      sum=0.
      fmin=felt(1)
      fmax=felt(1)
      do i=1,nx*ny
        if(felt(i).lt.fmin) fmin=felt(i)
        if(felt(i).gt.fmax) fmax=felt(i)
        sum=sum+felt(i)
      end do
c
      fmid=sum/real(nx*ny)
      write(6,*) ' felt.  min,max,middel: ',fmin,fmax,fmid
c
  900 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine feltut(iunit,filnam,in,nx,ny)
c
      include 'cressm.inc'
c
      common/cgrid/igtype,grid(6)
      common/fl/felt(maxsiz),felt2(maxsiz),felt3(maxsiz)
      common/inout/idata(ldata)
      integer*2 idata
c
      integer*2 in(16)
      character*(*) filnam
c
      double precision sum
c
      write(6,*) ' -----  feltut  -----'
c
      sum=0.
      fmin=felt(1)
      fmax=felt(1)
      do i=1,nx*ny
        if(felt(i).lt.fmin) fmin=felt(i)
        if(felt(i).gt.fmax) fmax=felt(i)
        sum=sum+felt(i)
      end do
c
      fmid=sum/(nx*ny)
      write(6,*) ' felt.  min,max,middel: ',fmin,fmax,fmid
c
      idata(1)=in( 1)
      idata(2)=in( 2)
      idata(3)=in( 9)
      idata(4)=in(10)
      idata(5)=in(11)
      idata(6)=in(12)
      idata(7)=in(13)
      idata(8)=in(14)
      idata(20)=-32767
c
      call mwfelt(0,filnam,iunit,2,nx*ny,felt,1.,
     *              ldata,idata,ierror)
      if(ierror.ne.0) stop 117
c
      return
      end
c
c***********************************************************************
c
      subroutine stati
c
c        statistikk for skalar-felt
c
      include 'cressm.inc'
c
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
c
      double precision ana,pro,asum,psum,apsum,aasum,ppsum
c
      iobs=nobs
c
      asum=0.
      psum=0.
      apsum=0.
      aasum=0.
      ppsum=0.
c
      do i=1,iobs
        ana=dobs(i)
        pro=obsint(i)
        asum=asum+ana
        psum=psum+pro
        apsum=apsum+ana*pro
        aasum=aasum+ana*ana
        ppsum=ppsum+pro*pro
ccc     np=np+1
      end do
c

      if(iobs.gt.0) then
        np=iobs
        rnp=np
c..mean values
ccc     amid=asum/rnp
ccc     pmid=psum/rnp
c..standard deviation
        asums=asum*asum
        psums=psum*psum
ccc     asta=(aasum-asums/rnp)/rnp
ccc     asta=sqrt(asta)
ccc     psta=(ppsum-psums/rnp)/rnp
ccc     psta=sqrt(psta)
c..root mean square error
        rmse=(aasum+ppsum-2.*apsum)/rnp
        rmse=sqrt(rmse)
c..correlation
        cnev=(aasum-asums/rnp)*(ppsum-psums/rnp)
        cnev=rnp*sqrt(cnev)
        corr=-1.e+20
        if(cnev.ne.0.) corr=100.*(rnp*apsum-asum*psum)/cnev
      else
        np=0
        rmse=-1.e+20
        corr=-1.e+20
      end if
c
      write(6,1010) np,rmse,corr
 1010 format('  antall:',i7,'    rmse: ',f8.2,'    corr: ',f7.3)
c
      return
      end
c
c***********************************************************************
c
      subroutine cress(rscan,nx,ny,xm,ym,felt,felt2,felt3)
c
c        analyse: et skalar-felt
c
      include 'cressm.inc'
c
      common/cgrid/igtype,grid(6)
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
c
      common/tab/ctabw,wtab(maxtab)
c
      integer nx,ny
      real    rscan
      real    xm(nx,ny),ym(nx,ny)
      real    felt(nx,ny),felt2(nx,ny),felt3(nx,ny)
c
      rs=rscan
      rs2=rs*rs
c
      ctab=ctabw
c
      iobs=nobs
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c     iwmin=maxtab
c     iwmax=1
c     nloop=0
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      do j=1,ny
        do i=1,nx
          felt2(i,j)=0.
          felt3(i,j)=0.
        end do
      end do
c
      do n=1,iobs
c
        d=dobs(n)-obsint(n)
        xo=xobs(n)
        yo=yobs(n)
        io=nint(xo)
        jo=nint(yo)
c
        rsx=rs*xm(io,jo)
        rsy=rs*ym(io,jo)
        xmi=1./xm(io,jo)
        ymi=1./ym(io,jo)
c
	i1=max(int(xo-rsx+1.), 1)
	i2=min(int(xo+rsx),   nx)
	j1=max(int(yo-rsy+1.), 1)
	j2=min(int(yo+rsy),   ny)
c
        do j=j1,j2
          dy=(j-yo)*ymi
          dy2=dy*dy
          do i=i1,i2
            dx=(i-xo)*xmi
            r2=dx*dx+dy2
            xw=ctab*r2+1.
            iw=xw
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
ccc         if(iw.lt.1 .or. iw.gt.maxtab-1)
ccc  *         write(6,*) '**cress**  iw,r2: ',iw,r2
c           if(iwmin.gt.iw) iwmin=iw
c           if(iwmax.lt.iw) iwmax=iw
c           nloop=nloop+1
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            w=wtab(iw)+(wtab(iw+1)-wtab(iw))*(xw-iw)
            felt2(i,j)=felt2(i,j)+w*d
            felt3(i,j)=felt3(i,j)+w
          end do
        end do
c
      end do
c
      do j=1,ny
        do i=1,nx
          if(felt3(i,j).ne.0.) felt(i,j)=felt(i,j)+felt2(i,j)/felt3(i,j)
        end do
      end do
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c     if(iwmin.lt.1 .or. iwmax.gt.maxtab-1) then
c       write(6,*) ' **** iwmin,iwmax,maxtab: ',iwmin,iwmax,maxtab
c     else
c       write(6,*) ' .... iwmin,iwmax,maxtab: ',iwmin,iwmax,maxtab
c     end if
c     write(6,*) ' ................. nloop: ',nloop
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      return
      end
c
c***********************************************************************
c
      subroutine cwtab(rscan)
c
c        beregner vekt-tabell
c
      include 'cressm.inc'
c
      common/tab/ctabw,wtab(maxtab)
c
      rs=rscan
      rs2=rs*rs
      dr=rs+0.2
      rmax2=2.*dr*dr
      ctab=(maxtab-1)/rmax2
      ntab=ctab*rs2+1.
      ctabw=ctab
c
      do n=1,ntab
        r2=(n-1)/ctab
        wtab(n)=(rs2-r2)/(rs2+r2)
      end do
      do n=ntab+1,maxtab
        wtab(n)=0.
      end do
c
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c     write(6,*) '------------ cwtab ----------------------'
c     write(6,*) 'ntab,maxtab,rs:  ',ntab,maxtab,rs
c     write(6,*) 'ctab,rmax2:      ',ctab,rmax2
c     n1=1
c     n2=10
c     if(n2.gt.maxtab) n2=maxtab
c     do n=n1,n2
c       r2=(n-1)/ctab
c       r=sqrt(r2)
c       write(6,*) ' r,r2,n,wtab: ',r,r2,n,wtab(n)
c     end do
c     n1=ntab-6
c     n2=ntab+3
c     if(n2.gt.maxtab) n2=maxtab
c     do n=n1,n2
c       r2=(n-1)/ctab
c       r=sqrt(r2)
c       write(6,*) ' r,r2,n,wtab: ',r,r2,n,wtab(n)
c     end do
c     n=maxtab
c     r2=(n-1)/ctab
c     r=sqrt(r2)
c     write(6,*) ' r,r2,n,wtab: ',r,r2,n,wtab(n)
c     write(6,*) '-----------------------------------------'
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      return
      end
c
c**********************************************************************
c
      subroutine blxy1(nx,ny)
c
c        posisjoner fra b,l til x,y
c        og fjerner observasjoner utenfor gridet
c
c
      include 'cressm.inc'
c
      integer nx,ny
c
      common/cgrid/igtype,grid(6)
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/st/nrst(maxobs)
c
c..define a grid matching geographic coordinates (for xyconvert)
      integer igeogrid
      real     geogrid(6)
      data igeogrid/2/
      data  geogrid/1.,1.,1.,1.,0.,0./
c
c..fra (l,b) til (x,y)
c
      call xyconvert(nobs,xobs,yobs,
     +               igeogrid,geogrid,igtype,grid,ierror)
      if(ierror.ne.0) write(6,*) 'XYCONVERT ERROR: ',ierror
      if(ierror.ne.0) stop 117
c
c..fjerner posisjoner utenfor gridet
c
      xmin=1.
      xmax=real(nx)
      ymin=1.
      ymax=real(ny)
c
      k=0
      do n=1,nobs
        if(xobs(n).gt.xmin .and. xobs(n).lt.xmax .and.
     +     yobs(n).gt.ymin .and. yobs(n).lt.ymax) then
          k=k+1
          xobs(k)=xobs(n)
          yobs(k)=yobs(n)
          dobs(k)=dobs(n)
          nrst(k)=nrst(n)
        end if
      end do
c
      write(6,*)
      write(6,*) ' antall observasjoner input:          ',nobs
      write(6,*) ' antall observasjoner innenfor gridet:',k
      write(6,*)
c
      if(k.lt.1) stop 117
c
      nobs=k
c
      return
      end
c
c***********************************************************************
c
      subroutine guess1(fguess,rguess,nx,ny,xm,ym,felt,felt2,felt3)
c
c        genererer gjentningsfelt.
c
      include 'cressm.inc'
c
      common/cgrid/igtype,grid(6)
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
c
      integer nx,ny
      real    fguess,rguess
      real    xm(nx,ny),ym(nx,ny)
      real    felt(nx,ny),felt2(nx,ny),felt3(nx,ny)
c
      fg=fguess
      rs=rguess
c
      do j=1,ny
        do i=1,nx
          felt(i,j)=fg
        end do
      end do
c
      if(rs.lt.1.) return
c
      rs2=rs*rs
c
      iobs=nobs
c
      do j=1,ny
        do i=1,nx
          felt2(i,j)=0.
          felt3(i,j)=0.
        end do
      end do
c
      do n=1,iobs
c
        d=dobs(n)
        xo=xobs(n)
        yo=yobs(n)
        io=nint(xo)
        jo=nint(yo)
c
        rsx=rs*xm(io,jo)
        rsy=rs*ym(io,jo)
c
        imin=xo-rsx+1.
        imax=xo+rsx
        jmin=yo-rsy+1.
        jmax=yo+rsy
        if(imin.lt.1)  imin=1
        if(imax.gt.nx) imax=nx
        if(jmin.lt.1)  jmin=1
        if(jmax.gt.ny) jmax=ny
c
        do j=jmin,jmax
          do i=imin,imax
            dx=(i-xo)*2./(xm(i,j)+xm(io,jo))
            dy=(j-yo)*2./(ym(i,j)+ym(io,jo))
            r2=dx*dx+dy*dy
            if(r2.lt.rs2) then
cccc          w=(rs2-r2)/(rs2+r2)
              w=1./(1.+0.01*r2**4)
              felt2(i,j)=felt2(i,j)+w*d
              felt3(i,j)=felt3(i,j)+w
            end if
          end do
        end do
c
      end do
c
      do j=1,ny
        do i=1,nx
          if(felt3(i,j).gt.0.) felt(i,j)=felt2(i,j)/felt3(i,j)
        end do
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine shap2(nx,ny,felt,a)
c
c        shapiro-filter
c
      integer nx,ny
      real    felt(nx,ny),a(nx,ny)
c
      nxm1=nx-1
      nym1=ny-1
      s=0.5
c
      do k=1,2
c
        s=s*0.5
c
        do j=1,ny
          do i=2,nxm1
            a(i,j)=felt(i,j)+s*(felt(i-1,j)+felt(i+1,j)-2.*felt(i,j))
          end do
        end do
        do j=1,ny
          a(1,j)=felt(1,j)
          a(nx,j)=felt(nx,j)
        end do
        do j=2,nym1
          do i=1,nx
            felt(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
          end do
        end do
        do i=1,nx
          felt(i,1)=a(i,1)
          felt(i,ny)=a(i,ny)
        end do
c
        s=-0.5
c
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine datkon(diflim,nremov)
c
      include 'cressm.inc'
c
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
      common/st/nrst(maxobs)
c
      write(6,*) ' -----  datkon -- data-kontroll -----'
      write(6,*) '   diflim= ',diflim
c
      dif=diflim
      iobs=0
      do n=1,nobs
        if(abs(dobs(n)-obsint(n)).lt.dif) then
          iobs=iobs+1
          xobs(iobs)=xobs(n)
          yobs(iobs)=yobs(n)
          dobs(iobs)=dobs(n)
          nrst(iobs)=nrst(n)
        end if
      end do
c
      nremov=nobs-iobs
      nobs=iobs
c
      write(6,*) '   antall fjernet:  ',nremov
      write(6,*) '   antall benyttes: ',nobs
c
      datmin=+1.e+35
      datmax=-1.e+35
      do n=1,nobs
        if(datmin.gt.dobs(n)) datmin=dobs(n)
        if(datmax.lt.dobs(n)) datmax=dobs(n)
      end do
      write(6,*) '   min obs-verdi som benyttes: ',datmin
      write(6,*) '   max obs-verdi som benyttes: ',datmax
c
      write(6,*) ' --------- datkon - end ----------'
c
      return
      end
c
c***********************************************************************
c
      subroutine rdat2(iunit,filnam,kobs2,feltfile,ini,nx,ny)
c
c        input fra file 'filnam'
c        formatert file fra 't2save' e.l.
c
c        nb| posisjoner gitt som b,l (evt. ogs$ punkt utenfor gridet)
c
c        kobs2 = 0: observerte verdier justeres ikke
c              = 1: observerte verdier = t2m justeres med 'tsvar'
c                   til modell-h@yde.
c
      include 'cressm.inc'
c
      common/fl/felt(maxsiz),felt2(maxsiz),felt3(maxsiz)
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
      common/st/nrst(maxobs)
c
      integer*2 ini(16)
      character*(*) filnam,feltfile
c
      real       hobs(maxobs)
      integer    itime1(4),itime2(4),indat(4,3)
      integer*2 in(16)
      character*72 txt
c
      write(6,*) ' -----  rdat2  -----'
c
      nobs=0
c
      write(6,*) 'reading file:'
      write(6,*) filnam
c
      irec=0
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios,err=900)
c
 1200 format(a72)
 1201 format(12i6)
c
      irec=1
      read(iunit,1200,iostat=ios,err=910) txt
c
      write(6,*) 'file identifikasjon:'
      write(6,*) txt
c
      irec=irec+1
      read(iunit,1201,iostat=ios,err=910) nobs,ndata
      irec=irec+1
      read(iunit,1201,iostat=ios,err=910) itime1
      irec=irec+1
      read(iunit,1201,iostat=ios,err=910) itime2
c
      write(6,*) 'første tid: ',itime1
      write(6,*) 'siste  tid: ',itime2
      write(6,*) 'antall observasjoner: ',nobs
c
      if(ndata.gt.nobs) write(6,*) '******** ndata: ',ndata
c
      if(nobs.gt.maxobs) then
        write(6,*) '******   nobs= ',nobs
        write(6,*) '****** maxobs= ',maxobs
        stop 117
      end if
c
c..skalering av geografisk bredde og lengde
      gscal=0.01
c
c..skalering av temperatur
      tscal=0.1
c
      nl=3
      nread=nobs/nl
      iobs=0
      do iread=1,nread
        irec=irec+1
        read(iunit,1201,iostat=ios,err=910) indat
        do n=1,nl
c..NOTE: xobs()=longitude  yobs()=latitude
          iobs=iobs+1
          yobs(iobs)=gscal*indat(1,n)
          xobs(iobs)=gscal*indat(2,n)
          dobs(iobs)=tscal*indat(3,n)
          hobs(iobs)=indat(4,n)
        end do
      end do
      nn=nobs-iobs
      if(nn.ge.1 .and. nn.le.nl) then
        irec=irec+1
        read(iunit,1201,iostat=ios,err=910) ((indat(i,n),i=1,4),n=1,nn)
        do n=1,nn
c..NOTE: xobs()=longitude  yobs()=latitude
          iobs=iobs+1
          yobs(iobs)=gscal*indat(1,n)
          xobs(iobs)=gscal*indat(2,n)
          dobs(iobs)=tscal*indat(3,n)
          hobs(iobs)=indat(4,n)
        end do
      end if
c
      if(iobs.ne.nobs) then
        write(6,*) ' ***** nobs,iobs: ',nobs,iobs
        stop 117
      end if
c
c..'stasjons-nr'
      do iobs=1,nobs
        nrst(iobs)=iobs
      end do
c
      close(iunit)
c
      if(kobs2.eq.0) then
c..leser inn gjetnings-felt
        call feltin(iunit,feltfile,ini,nx,ny,ierror)
	if(ierror.ne.0) stop 117
c..fra b,l til x,y og fjerner observasjoner utenfor gridet'
        call blxy1(nx,ny)
        goto 800
c
      elseif(kobs2.eq.1) then
        goto 200
c
      else
         write(6,*) ' **** rdat2 **** ukjent kobs2= ',kobs2
      end if
c
      stop 117
c
c..justerer t2m med 'tsvar' iflg. stasjons- og modell-h@yde
  200 write(6,*)
      write(6,*) ' obs: t2m ... justeres med "tsvar"'
      write(6,*) '              iflg. stasjons- og modell-h@yde'
c
c..fjerner stasjoner som mangler stasjons-h@yde
      hmin=+99999.
      hmax=-99999.
      iobs=0
      do n=1,nobs
        if(hobs(n).gt.-100.) then
          iobs=iobs+1
          xobs(iobs)=xobs(n)
          yobs(iobs)=yobs(n)
          dobs(iobs)=dobs(n)
          hobs(iobs)=hobs(n)
          if(hmin.gt.hobs(n)) hmin=hobs(n)
          if(hmax.lt.hobs(n)) hmax=hobs(n)
          nrst(iobs)=nrst(n)
        end if
      end do
      n=nobs-iobs
      nobs=iobs
      write(6,*) ' antall fjernet p.g.a. manglende h@yde: ',n
      write(6,*) ' antall stasjoner benyttes:             ',nobs
      write(6,*) ' min stasjons-h@yde: ',hmin
      write(6,*) ' max stasjons-h@yde: ',hmax
c
c..les topografi
      in( 1)=ini(1)
      in( 2)=ini(2)
      in( 9)=4
      in(10)=0
      in(11)=2
      in(12)=101
      in(13)=1000
      in(14)=0
      call feltin(iunit,feltfile,in,nx,ny,ierror)
      if(ierror.ne.0) stop 117
c..fra l,b til x,y og fjerner observasjoner utenfor gridet'
      call blxy1(nx,ny)
c..interpoler til obs-posisjoner
      call interp(nx,ny,felt)
c..beregn differanse mellom stasjons- og modell-h@yde
      hmin=+99999.
      hmax=-99999.
      do n=1,nobs
        hobs(n)=obsint(n)-hobs(n)
        if(hmin.gt.hobs(n)) hmin=hobs(n)
        if(hmax.lt.hobs(n)) hmax=hobs(n)
      end do
      write(6,*) ' min h@yde-justering: ',hmin
      write(6,*) ' max h@yde-justering: ',hmax
c
c..les tsvar
      in( 1)=ini( 1)
      in( 2)=ini( 2)
      in( 9)=ini( 9)
      in(10)=ini(10)
      in(11)=ini(11)
      in(12)=ini(12)+1
      in(13)=ini(13)
      in(14)=ini(14)
      call feltin(iunit,feltfile,in,nx,ny,ierror)
      if(ierror.ne.0) stop 117
c        interpoler til obs-posisjoner
      call interp(nx,ny,felt)
c..beregn temperatur-justering
      tmin=+99999.
      tmax=-99999.
      do n=1,nobs
        dt=hobs(n)*obsint(n)
        dobs(n)=dobs(n)+dt
        if(tmin.gt.dt) tmin=dt
        if(tmax.lt.dt) tmax=dt
      end do
      write(6,*) ' min temperatur-justering: ',tmin
      write(6,*) ' max temperatur-justering: ',tmax
c
c..leser inn gjetnings-felt
      call feltin(iunit,feltfile,ini,nx,ny,ierror)
      if(ierror.ne.0) stop 117
c
ccc   goto 800
c
c..sjekk min/max temperatur og gj@r om til grader kelvin
  800 tmin=+99999.
      tmax=-99999.
      do n=1,nobs
        if(tmin.gt.dobs(n)) tmin=dobs(n)
        if(tmax.lt.dobs(n)) tmax=dobs(n)
        dobs(n)=273.15+dobs(n)
      end do
      write(6,*) ' antall stasjoner benyttes: ',nobs
      write(6,*) ' min temperatur (c): ',tmin
      write(6,*) ' max temperatur (c): ',tmax
      tmin=273.15+tmin
      tmax=273.15+tmax
      write(6,*) ' min temperatur (k): ',tmin
      write(6,*) ' max temperatur (k): ',tmax
c
      write(6,*) ' --------- rdat2 - end ----------'
c
      return
c
  900 write(6,*)
      write(6,*) 'open error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      goto 980
c
  910 write(6,*)
      write(6,*) 'read error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      write(6,*) 'line: ',irec
      goto 980
c
  980 stop 117
c
      end
c
c***********************************************************************
c
      subroutine rdat3(iunit,filnam,kobs2,feltfile,ini,nx,ny)
c
c        input fra file 'filnam'
c        formatert file fra 'twsave' e.l.
c
c        nb| posisjoner gitt som b,l (evt. ogs$ punkt utenfor gridet)
c
c        kobs2:     benyttes ikke
c
      include 'cressm.inc'
c
      common/fl/felt(maxsiz),felt2(maxsiz),felt3(maxsiz)
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
      common/st/nrst(maxobs)
c
      integer*2 ini(16)
      character*(*) filnam,feltfile
c
      dimension itime1(4),itime2(4),indat(3,4)
      integer*2 in(16)
      character*72 txt
c
      write(6,*) ' -----  rdat3  -----'
c
      nobs=0
c
      write(6,*) 'reading file:'
      write(6,*) filnam
c
      irec=0
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios,err=900)
c
      do n=1,maxobs
        read(iunit,*,iostat=ios,err=910,end=200) glat,glon,value
c..NOTE: xobs()=longitude  yobs()=latitude
        xobs(n)=glon
        yobs(n)=glat
        dobs(n)=value
c..'stasjons-nr'
        nrst(n)=n
      end do
c
      n=maxobs+1
      read(iunit,*,iostat=ios,err=910,end=200) glat,glon,value
      write(6,*) 'Too many observations.  Max: ',maxobs
c
  200 nobs=n-1
c
      close(iunit)
c
      nobsi=nobs
c
c..leser inn gjetnings-felt
      call feltin(iunit,feltfile,ini,nx,ny,ierror)
      if(ierror.ne.0) stop 117
c
c..fra l,b til x,y og fjerner observasjoner utenfor gridet'
      call blxy1(nx,ny)
c
c
c..sjekk min/max verdi
      vmin=dobs(1)
      vmax=dobs(1)
      do 210 n=1,nobs
        if(vmin.gt.dobs(n)) vmin=dobs(n)
        if(vmax.lt.dobs(n)) vmax=dobs(n)
  210 continue
      write(6,*) ' antall data input:    ',nobsi
      write(6,*) ' antall data benyttes: ',nobs
      write(6,*) ' min verdi: ',vmin
      write(6,*) ' max verdi: ',vmax
c
      write(6,*) ' --------- rdat3 - end ----------'
c
      return
c
  900 write(6,*)
      write(6,*) 'open error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      goto 980
c
  910 write(6,*)
      write(6,*) 'read error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      write(6,*) 'line: ',n
      goto 980
c
  980 stop 117
c
      end
c
c***********************************************************************
c
      subroutine rdat3old(iunit,filnam,kobs2,feltfile,ini,nx,ny)
c
c        input fra file 'filnam'
c        formatert file fra 'twsave' e.l.
c
c        nb| posisjoner gitt som b,l (evt. ogs$ punkt utenfor gridet)
c
c        kobs2:     benyttes ikke
c
      include 'cressm.inc'
c
      common/fl/felt(maxsiz),felt2(maxsiz),felt3(maxsiz)
      common/ob/nobs,xobs(maxobs),yobs(maxobs),dobs(maxobs)
      common/it/obsint(maxobs)
      common/st/nrst(maxobs)
c
      integer*2 ini(16)
      character*(*) filnam,feltfile
c
      dimension itime1(4),itime2(4),indat(3,4)
      integer*2 in(16)
      character*72 txt
c
      write(6,*) ' -----  rdat3  -----'
c
      nobs=0
c
      write(6,*) 'reading file:'
      write(6,*) filnam
c
      irec=0
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios,err=900)
c
 1200 format(a72)
 1201 format(12i6)
c
      irec=1
      read(iunit,1200,iostat=ios,err=910) txt
c
      write(6,*) 'file identifikasjon:'
      write(6,*) txt
c
      irec=irec+1
      read(iunit,1201,iostat=ios,err=910) nobs,ndata
      irec=irec+1
      read(iunit,1201,iostat=ios,err=910) itime1
      irec=irec+1
      read(iunit,1201,iostat=ios,err=910) itime2
c
      write(6,*) 'frste tid: ',itime1
      write(6,*) 'siste  tid: ',itime2
      write(6,*) 'antall observasjoner: ',nobs
c
      if(ndata.gt.nobs) write(6,*) '******** ndata: ',ndata
c
      if(nobs.gt.maxobs) then
        write(6,*) '******   nobs= ',nobs
        write(6,*) '****** maxobs= ',maxobs
        stop 117
      end if
c
c..skalering av geografisk bredde og lengde
      gscal=0.01
c
c..skalering av temperatur
      tscal=0.1
c
      nl=4
      nread=nobs/nl
      iobs=0
      do iread=1,nread
        irec=irec+1
        read(iunit,1201,iostat=ios,err=910) indat
        do n=1,nl
c..NOTE: xobs()=longitude  yobs()=latitude
          iobs=iobs+1
          yobs(iobs)=gscal*indat(1,n)
          xobs(iobs)=gscal*indat(2,n)
          dobs(iobs)=tscal*indat(3,n)
        end do
      end do
      nn=nobs-iobs
      if(nn.ge.1 .and. nn.le.nl) then
        irec=irec+1
        read(iunit,1201,iostat=ios,err=910) ((indat(i,n),i=1,3),n=1,nn)
        do n=1,nn
c..NOTE: xobs()=longitude  yobs()=latitude
          iobs=iobs+1
          yobs(iobs)=gscal*indat(1,n)
          xobs(iobs)=gscal*indat(2,n)
          dobs(iobs)=tscal*indat(3,n)
        end do
      end if
c
      if(iobs.ne.nobs) then
        write(6,*) ' ***** nobs,iobs: ',nobs,iobs
        stop 117
      end if
c
c..'stasjons-nr'
      do iobs=1,nobs
        nrst(iobs)=iobs
      end do
c
      close(iunit)
c
c..leser inn gjetnings-felt
      call feltin(iunit,feltfile,ini,nx,ny,ierror)
      if(ierror.ne.0) stop 117
c
c..fra l,b til x,y og fjerner observasjoner utenfor gridet'
      call blxy1(nx,ny)
c
c
c..sjekk min/max temperatur
      tmin=dobs(1)
      tmax=dobs(1)
      do n=1,nobs
        if(tmin.gt.dobs(n)) tmin=dobs(n)
        if(tmax.lt.dobs(n)) tmax=dobs(n)
      end do
      write(6,*) ' antall stasjoner benyttes: ',nobs
      write(6,*) ' min temperatur (c): ',tmin
      write(6,*) ' max temperatur (c): ',tmax
c
      write(6,*) ' --------- rdat3 - end ----------'
c
      return
c
  900 write(6,*)
      write(6,*) 'open error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      goto 980
c
  910 write(6,*)
      write(6,*) 'read error.  iostat= ',ios
      write(6,*) 'file: ',filnam
      write(6,*) 'line: ',irec
      goto 980
c
  980 stop 117
c
      end
c
c***********************************************************************
c
      subroutine seaextend(nextend,nx,ny,field,fsea,fwork)
c
c	put sea values in the nearest land gridpoints,
c	land point if fsea()>0.5,
c	fsea() will be destroyed (fsea()=-1. in updated gridpoints)
c
      integer nextend,nx,ny
      real    field(nx,ny),fsea(nx,ny),fwork(nx,ny)
c
      nchange=0
c
      do iextend=1,nextend
c
	do j=1,ny
	  do i=1,nx
	    fwork(i,j)=fsea(i,j)
	  end do
	end do
c
	do j=1,ny
	  j1=max(j-1, 1)
	  j2=min(j+1,ny)
	  do i=1,nx
	    if(fwork(i,j).ge.0.5) then
	      i1=max(i-1, 1)
	      i2=min(i+1,nx)
	      nsum=0
	      fsum=0.
	      do jp=j1,j2
		do ip=i1,i2
		  if(fwork(ip,jp).lt.0.5) then
		    fsum=fsum+field(ip,jp)
		    nsum=nsum+1
		  end if
		end do
	      end do
	      if(nsum.gt.0) then
		field(i,j)=fsum/real(nsum)
		fsea(i,j)=-1.
		nchange=nchange+1
	      end if
	    end if
	  end do
	end do
c
      end do
c
      write(6,*) '  seaextend.  nextend,nchange: ',nextend,nchange
c
      return
      end

