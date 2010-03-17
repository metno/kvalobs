      program flt2v5d
c
c  Read Hirlam fields from felt file and make a Vis5d data file
c
c--------------------------------------------------------------------------
c  DNMI/FoU  03.08.1995  Anstein Foss
c  DNMI/FoU  31.08.1995  Anstein Foss
c  DNMI/FoU  13.09.1995  Anstein Foss
c  DNMI/FoU  31.10.1996  Anstein Foss ... vis5d p-levels, cat, ...
c  DNMI/FoU  21.05.1997  Anstein Foss ... interp. to pressure levels
c  DNMI/FoU  26.05.1997  Anstein Foss ... new compw
c  DNMI/FoU  08.12.1997  Anstein Foss ... surface data: mslp,u10m,v10m,pr6h
c  DNMI/FoU  15.12.1997  Anstein Foss ... ptop option
c  DNMI/FoU  07.01.1998  Anstein Foss ... ff10m and -o option
c  DNMI/FoU  29.12.1999  Anstein Foss
c  DNMI/FoU  01.11.2003  Anstein Foss ... SGI+Linux update
c  DNMI/FoU  10.05.2005  Anstein Foss ... float() -> real()
c--------------------------------------------------------------------------
c
      include 'flt2v5d.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..flt2v5d.inc .... include file for flt2v5d.f
c
c..max2d  : max size of 2d fields
c..max3d  : max size of 3d fields
c..maxpar : max no. of parameters
c..maxlev : max no. of levels
c..maxtim : max no. of timesteps
c
ccc   parameter (max2d=188*152)
ccc   parameter (max3d=max2d*31)
ccc   parameter (maxpar=25)
ccc   parameter (maxlev=50)
ccc   parameter (maxtim=50)
c
ccc   parameter (ldata=20+max2d+50)
ccc   parameter (maxinh=512)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      parameter (undef=+1.e+35)
c
      integer*2 inh(16,maxinh)
      integer   ifound(maxinh)
c
      integer*2 idata(ldata)
      real      grid(6)
      real      f2d(max2d),f3d(max3d),f3dv5d(max3d)
c
      real      alevel(maxlev),blevel(maxlev),plevel(maxlev)
      real      alvls(3),blvls(3)
      real      zs(max2d),ps(max2d)
      real      u3d(max3d),v3d(max3d),q3d(max3d),th3d(max3d)
c.old real      om3d(max3d)
      real	xmd2h(max2d),ymd2h(max2d),fc(max2d)
      real      ahalf(maxlev+1),bhalf(maxlev+1),work2d(max2d,6)
c
      integer   iparam(maxpar),nplevel(maxpar)
      integer   ilevel(maxlev),idlevel(2,maxlev)
      integer   iprog(5,maxtim),itime(5,maxtim)
c
      integer*2 in(16)
c
      integer   initv5d
c
      character*10 cparam(maxpar)
      character*256 feltfile,v5dfile,seqfile
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=14)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=2)
      integer       nargs
      character*256 cargs(margs)
      integer       mispec
      parameter    (mispec=300)
      integer       ispec(mispec)
      integer       mrspec
      parameter    (mrspec=10)
      real          rspec(mrspec)
      integer       mcspec
      parameter    (mcspec=1)
      character*256 cspec(mcspec)
      integer       ierror
      integer       nerror
c..cmdarg................................................
c
      integer inpos(nopt),iloop(3,nopt)
c
      real    ewt(41)
c
c..metningstrykk for t=-100,-95,-90,...+100 gr.c.
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
c..cmdarg...........................................................
      data copt/
     +     'n','g','d','t','v','p','l','s','r','o','f','z','x','y'/
      data iopt/
     +      1 , 1 , 1 , 1 , 1 , 1 , 1 , 2 , 1 , 1 , 4 , 1 , 0 , 1 /
c..cmdarg...1...2...3...4...5...6...7...8...9..10..11..12..13..14...
c
      data inpos/
     +      1,  2,  9, 10, 11, 12, 13 , 0 , 0 , 0 , 0 , 0 , 0 , 0 /
c
c
c..cmdarg................................................
c..get command line arguments
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c..cmdarg................................................
c
      if(ierror.eq.0 .and. nargs.ne.2) ierror=-1
c
      if(ierror.ne.0) then
	write(6,*) ' usage: flt2v5d felt_file vis5d_file'
	write(6,*) '    or: flt2v5d felt_file vis5d_file [options]'
	write(6,*) '   options:'
        write(6,*) '     -n <producer_no>      : producer (1-99)'
        write(6,*) '     -g <grid_no>          : grid'
        write(6,*) '     -d <data_type>        : data type'
        write(6,*) '     -t <forecast_hour(s)> : forecast time'
cc      write(6,*) '     -v <v.coord._no>      : vertical coordinate'
cc      write(6,*) '     -p <parameter_no(s)>  : parameter no.'
        write(6,*) '     -l <level_no(s)>      : levels (no.)'
        write(6,*) '     -s <x1%,x2%,y1%,y2%>  : subarea (percentage)'
        write(6,*) '     -r <x_and_y_step>     : resolution (1,2,...)'
        write(6,*) '     -o0                   : all fields'
        write(6,*) '     -o1                   : skip omega,etadot,CAT',
     +						' (default)'
        write(6,*) '     -o2                   : not compute any fields'
        write(6,*) '     -x                    : exclude surface fields'
        write(6,*) '     -y <pressure.hpa>     : output top pressure',
     +						' (default 200)'
        write(6,*) '     -f <file_name>        : make sequential file',
     +					       ' with computed W,PV,CAT'
        write(6,*) '     -z1                   : pressure as vertical ',
     +						'coordinate (default)'
        write(6,*) '     -z2                   : height as vertical ',
     +						'coordinate'
cc      write(6,*) '   (e.g. -t12,18,24 -v 10 -p2,3,9,18 -l10,31)'
        write(6,*) '   (e.g. -t12,18,24 -l10,31)'
	write(6,*) '   NOTE: 2 arguments means the min and max value,'
	write(6,*) '         repeat one of the two values if two wanted'
	write(6,*) '   Default: first producer and grid on file,'
	write(6,*) '            vertical coordinate 10 (eta/hybrid),'
	write(6,*) '            all timesteps, parameters and levels,'
	write(6,*) '            total area in full resolution'
	write(6,*)
	call exit(1)
      end if
c
      feltfile=cargs(1)
      v5dfile =cargs(2)
c
c..defaults
      iprod=-32767
      igrid=-32767
      idatype=-32767
      ntinp1=0
      ntinp2=0
      ioutput=1
      isegfile=0
      seqfile=' '
      x1p=  0.
      x2p=100.
      y1p=  0.
      y2p=100.
      iresol=1
      nlevel=0
      level1=-32767
      level2=+32767
      ivertical=1
      isurf=1
      ptop=200.
c
c..producer no.
      n=1
      if(iopts(1,n).gt.0) iprod=ispec(iopts(2,n))
c
c..grid no.
      n=2
      if(iopts(1,n).gt.0) igrid=ispec(iopts(2,n))
c
c..data type
      n=3
      if(iopts(1,n).gt.0) idatype=ispec(iopts(2,n))
c
c..forecast hours
      n=4
      if(iopts(1,n).gt.0) then
        ntinp1=iopts(2,n)
        ntinp2=iopts(2,n)+iopts(1,n)-1
      end if
c
c..input levels
      n=7
      if(iopts(1,n).eq.2) then
        level1=ispec(iopts(2,n)+0)
        level2=ispec(iopts(2,n)+1)
	if(level1.gt.level2) then
	  i=level1
	  level1=level2
	  level2=i
	end if
      elseif(iopts(1,n).gt.2) then
	nl1=iopts(2,n)
	nl2=iopts(2,n)+iopts(1,n)-1
	nlevel=1
	ilevel(nlevel)=ispec(iopts(2,n))
	do i=nl1+1,nl2
	  level=ispec(i)
	  k=0
	  do j=1,nlevel
	    if(ilevel(j).eq.level) k=1
	  end do
	  if(k.eq.0) then
	    nlevel=nlevel+1
	    ilevel(nlevel)=level
	  end if
	end do
      end if
c
c..subarea (x1,x2,y1,y2 in unit percent)
      n=8
      if(iopts(1,n).eq.4) then
	x1p=rspec(iopts(2,n)+0)
	x2p=rspec(iopts(2,n)+1)
	y1p=rspec(iopts(2,n)+2)
	y2p=rspec(iopts(2,n)+3)
	if(x1p.lt.0. .or. x1p.ge.x2p .or. x2p.gt.100. .or.
     +	   y1p.lt.0. .or. y1p.ge.y2p .or. y2p.gt.100.) then
          x1p=  0.
          x2p=100.
          y1p=  0.
          y2p=100.
	end if
      end if
c
c..resolution (in grid units)
      n=9
      if(iopts(1,n).gt.0) then
        iresol=ispec(iopts(2,n))
        if(iresol.lt.1) iresol=1
      end if
c
c..output parameters
c..(0=all  1=skip omega,etadot  2=not compute any fields)
      n=10
      if(iopts(1,n).gt.0) ioutput=ispec(iopts(2,n))
c
c..sequential outputfile (on/off)
      n=11
      if(iopts(1,n).gt.0 .and. ioutput.ne.2) then
	isegfile=1
	seqfile=cargs(iopts(2,n))
      end if
c
c..vis-5d vertical default presentation coordinate
      n=12
      if(iopts(1,n).gt.0) then
	ivertical=ispec(iopts(2,n))
	ivertical=max(ivertical,1)
	ivertical=min(ivertical,2)
      end if
c
c..surface parameters (on/off)
      n=13
      if(iopts(1,n).gt.0) isurf=0
c
c..top pressure (hPa)
      n=14
      if(iopts(1,n).gt.0) ptop=ispec(iopts(2,n))
c
c..fixed vertical coordinate (10=Hirlam eta/hybrid)
      ivcoord=10
c..and interpolation to pressure surfaces
      ivcout=1
c
c..input felt file unit
      iunitf=20
c
c..output sequential felt file unit
      iunits=30
c
c..open input felt file
      call mrfturbo(1,feltfile,iunitf,in,0,1,1.,1.,1,idata,ierror)
      if(ierror.ne.0) stop 2
c
      if(iprod.eq.-32767 .or. igrid.eq.-32767) then
c
c..find producer and/or grid no. (the first existing field in the file)
        ireq=1
        iexist=1
        nin=1
        do i=1,16
	  inh(i,1)=-32767
        end do
	inh( 1,1)=iprod
	inh( 2,1)=igrid
	inh( 9,1)=idatype
        inh(11,1)=ivcoord
        call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +             iend,ierror,ioerr)
        if(ierror.ne.0) write(6,*) 'ERROR.  qfelt ierror= ',ierror
        if(ierror.ne.0) call exit(2)
        if(nfound.lt.1) write(6,*) 'ERROR.  No existing fields.'
        if(nfound.lt.1) call exit(2)
c
        iprod=inh(1,1)
        igrid=inh(2,1)
        write(6,*) 'iprod,igrid: ',iprod,igrid
	write(6,*) 'ptop: ',ptop
c
        if(iend.ne.1) then
          ireq=0
          call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +               iend,ierror,ioerr)
        end if
c
      end if
c
c..find all parameters (first timestep and level)
      ireq=1
      iexist=1
      nin=maxinh
      inh(12,1)=-32767
      call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +           iend,ierror,ioerr)
      if(ierror.ne.0) write(6,*) 'ERROR.  qfelt ierror= ',ierror
      if(ierror.ne.0) call exit(2)
      if(nfound.lt.1) write(6,*) 'ERROR.  No existing fields.'
      if(nfound.lt.1) call exit(2)
      if(nfound.gt.maxpar) write(6,*) 'ERROR.  Too many parameters.'
      if(nfound.gt.maxpar) call exit(3)
c
c
      if(ioutput.eq.1) then
c..skip etadot and omega
	nparam=0
        do n=1,nfound
	  if(inh(12,n).ne.11 .and. inh(12,n).ne.13) then
	    nparam=nparam+1
	    iparam(nparam)=inh(12,n)
	  end if
        end do
      else
c..keep all input parameters
        do n=1,nfound
	  iparam(n)=inh(12,n)
        end do
        nparam=nfound
      end if
c
      nparin=nparam
c
      nparw3d =0
      nparpv3d=0
      nparrh3d=0
      npart3d =0
      npartd3d=0
      nparff3d=0
      nparcat3d=0
      ncompseq=0
c
      iu3d=0
      iv3d=0
      iq3d=0
      ith3d=0
c.old iom3d=0
c
      if(ivcoord.eq.10 .and. ioutput.ne.2) then
c
	itinp=0
c
        do n=1,nparin
	  if(iparam(n).eq. 2) iu3d=1
	  if(iparam(n).eq. 3) iv3d=1
	  if(iparam(n).eq. 9) iq3d=1
	  if(iparam(n).eq.18) ith3d=1
c.old	  if(iparam(n).eq.13) iom3d=1
	  if(iparam(n).eq. 4) itinp=1
	end do
c
	if(nparam.lt.maxpar .and.
c.old+	   ith3d.eq.1 .and. iom3d.eq.1) then
     +	   ith3d.eq.1 .and. iu3d.eq.1 .and. iv3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=12
	  nparw3d=nparam
	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing W.'
	end if
c
	if(nparam.lt.maxpar .and.
     +	   iu3d.eq.1 .and. iv3d.eq.1 .and. ith3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=80
	  nparpv3d=nparam
	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing PV.'
	end if
c
	if(nparam.lt.maxpar .and.
     +	   ith3d.eq.1 .and. iq3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=10
	  nparrh3d=nparam
ccc	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing RH.'
	end if
c
	if(nparam.lt.maxpar .and. itinp.eq.0 .and.
     +	   ith3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=4
	  npart3d=nparam
ccc	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing T.'
	end if
c
	if(nparam.lt.maxpar .and.
     +	   ith3d.eq.1 .and. iq3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=5
	  npartd3d=nparam
ccc	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing Td.'
	end if
c
	if(nparam.lt.maxpar .and.
     +	   iu3d.eq.1 .and. iv3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=-1
	  nparff3d=nparam
ccc	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing FF.'
	end if
c
	if(ioutput.ne.1 .and. nparam.lt.maxpar .and.
     +	   iu3d.eq.1 .and. iv3d.eq.1 .and. ith3d.eq.1) then
	  nparam=nparam+1
	  iparam(nparam)=91
	  nparcat3d=nparam
	  ncompseq=ncompseq+1
	elseif(nparam.ge.maxpar) then
	  write(6,*) 'Too many parameters. Not computing CAT.'
	end if
c
      end if
c
      if(isurf.eq.1 .and. nparam+6.le.maxpar) then
c..mslp(58) u10m(33) v10m(34) ff10m(-2) prec(17)
	iparam(nparam+1)=58
	iparam(nparam+2)=31
	iparam(nparam+3)=33
	iparam(nparam+4)=34
	iparam(nparam+5)=-2
	iparam(nparam+6)=17
	npsurf1=nparam+1
	npsurf2=nparam+6
	nparam=nparam+6
      else
        npsurf1=nparam+1
        npsurf2=nparam
      end if
c
      do n=1,nparam
	cparam(n)='?'
	if(iparam(n).eq.-1) cparam(n)='FF'
	if(iparam(n).eq. 2) cparam(n)='U'
	if(iparam(n).eq. 3) cparam(n)='V'
	if(iparam(n).eq. 4) cparam(n)='T'
	if(iparam(n).eq. 5) cparam(n)='TD'
	if(iparam(n).eq. 9) cparam(n)='Q'
	if(iparam(n).eq.10) cparam(n)='RH'
	if(iparam(n).eq.11 .and. ivcoord.eq.10) cparam(n)='ETADOT'
	if(iparam(n).eq.12) cparam(n)='W'
	if(iparam(n).eq.13) cparam(n)='OMEGA'
	if(iparam(n).eq.18) cparam(n)='THETA'
	if(iparam(n).eq.22) cparam(n)='CLOUDWATER'
	if(iparam(n).eq.23) cparam(n)='PRECIP.3D'
	if(iparam(n).eq.80) cparam(n)='PV'
	if(iparam(n).eq.91) cparam(n)='CAT'
c..surface
	if(iparam(n).eq.-2) cparam(n)='FF10M'
	if(iparam(n).eq.58) cparam(n)='MSLP'
	if(iparam(n).eq.31) cparam(n)='T2M'
	if(iparam(n).eq.33) cparam(n)='U10M'
	if(iparam(n).eq.34) cparam(n)='V10M'
	if(iparam(n).eq.17) cparam(n)='PRECIP'
c..unknown parameters
	if(cparam(n).eq.'?') then
	  write(cparam(n),fmt='(''PAR.'',i6)') iparam(n)
	  k1=5
	  k2=10
	  do while (cparam(n)(k1:k1).eq.' ')
	    k1=k1+1
	  end do
	  k=4
	  do i=k1,k2
	   k=k+1
	   cparam(n)(k:k)=cparam(n)(i:i)
	   cparam(n)(i:i)=' '
	  end do
	end if
      end do
c
      if(iend.ne.1) then
        ireq=0
        call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +             iend,ierror,ioerr)
      end if
c
      if(nlevel.eq.0) then
c
c..find all levels (first parameter and timestep)
	ireq=1
	iexist=1
	nin=maxinh
	inh(13,1)=-32767
	inh(14,1)=-32767
	call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +             iend,ierror,ioerr)
	if(ierror.ne.0) write(6,*) 'ERROR.  qfelt ierror= ',ierror
	if(ierror.ne.0) call exit(2)
	if(nfound.lt.1) write(6,*) 'ERROR.  No existing fields.'
	if(nfound.lt.1) call exit(2)
	if(nfound.gt.maxlev) write(6,*) 'ERROR.  Too many levels.'
	if(nfound.gt.maxlev) call exit(3)
c
	do n=1,nfound
	  level=inh(13,n)
	  if(level.ge.level1 .and. level.le.level2) then
	    nlevel=nlevel+1
	    ilevel(nlevel)=level
	  end if
	end do
c
	write(6,*) 'levels:      nlevel= ',nlevel
	write(6,*) (ilevel(i),i=1,nlevel)
c
	if(iend.ne.1) then
	  ireq=0
	  call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +               iend,ierror,ioerr)
	end if
c
      end if
c
      write(6,*) 'nparam: ',nparam
c
      do n=1,nparam
	if(n.lt.npsurf1) then
	  nplevel(n)=nlevel
	  write(6,*) iparam(n),'  ',cparam(n)
	else
	  nplevel(n)=1
	  write(6,*) iparam(n),'  ',cparam(n),'  (surface)'
	end if
      end do
c
c..find all timesteps (first level and first parameter)
      ireq=1
      iexist=1
      nin=maxinh
      inh( 9,1)=-32767
      inh(10,1)=-32767
      call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +           iend,ierror,ioerr)
      if(ierror.ne.0) write(6,*) 'ERROR.  qfelt ierror= ',ierror
      if(ierror.ne.0) call exit(2)
      if(nfound.lt.1) write(6,*) 'ERROR.  No existing fields.'
      if(nfound.lt.1) call exit(2)
      if(nfound.gt.maxtim) write(6,*) 'ERROR.  Too many timesteps.'
      if(nfound.gt.maxtim) call exit(3)
c
      if(ntinp1.gt.0) then
c..check found timesteps against input list
	nt=0
	do n=1,nfound
	  igood=0
	  if(ntinp2.eq.ntinp1+1) then
	    if(ispec(ntinp1).le.inh(10,n) .and.
     +	       ispec(ntinp2).ge.inh(10,n)) igood=1
	  else
	    do i=ntinp1,ntinp2
	      if(ispec(i).eq.inh(10,n)) igood=1
	    end do
	  end if
	  if(igood.eq.1) then
	    nt=nt+1
	    if(nt.ne.n) then
	      do i=1,16
		inh(i,nt)=inh(i,n)
	      end do
	      ifound(nt)=ifound(n)
	    end if
	  end if
	end do
	nfound=nt
        if(nfound.lt.1) write(6,*) 'ERROR.  No existing fields.'
        if(nfound.lt.1) call exit(2)
      end if
c
      ntime=nfound
      write(6,*) 'ntime: ',ntime
      do n=1,nfound
	iprog(1,n)=inh(3,n)
	iprog(2,n)=inh(4,n)
	iprog(3,n)=inh(5,n)
	iprog(4,n)=inh(9,n)
	iprog(5,n)=inh(10,n)
	itime(1,n)=inh(3,n)
	itime(2,n)=inh(4,n)/100
	itime(3,n)=inh(4,n)-(inh(4,n)/100)*100
	itime(4,n)=inh(5,n)/100
	minute=inh(5,n)-(inh(5,n)/100)*100
	itime(5,n)=inh(10,n)
	call vtime(itime(1,n),ierror)
	itime(5,n)=minute
	write(6,*) (itime(i,n),i=1,5)
      end do
c
      if(iend.ne.1) then
        ireq=0
        call qfelt(iunitf,ireq,iexist,nin,inh,ifound,nfound,
     +             iend,ierror,ioerr)
      end if
c
      do i=1,16
	in(i)=-32767
      end do
      in( 1)=iprod
      in( 2)=igrid
c
      imap=0
      if(nparpv3d.gt.0 .or. naprcat3d.gt.0) imap=1
c
      initv5d=0
      initgrid=0
      isubarea=0
      initlevels=0
c
c
      if(nparw3d.gt.0) then
c..read topography (zs)
        in( 3)=iprog(1,1)
        in( 4)=iprog(2,1)
        in( 5)=iprog(3,1)
        in( 9)=4
        in(10)=0
        in(11)=2
        in(12)=101
        in(13)=1000
        in(14)=0
        call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,zs,1.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) call exit(2)
c
	if(initgrid.eq.0) then
	  call gridcheck(ldata,idata,igtype,nxin,nyin,grid,
     +			 nlevel,x1p,x2p,y1p,y2p,iresol,
     +			 nx,ny,ix1,ix2,iy1,iy2,isubarea,
     +			 max2d,max3d,imap,xmd2h,ymd2h,fc,hx,hy)
	  initgrid=1
	end if
c
	if(isubarea.ne.0) then
	  nn=0
	  do j=iy1,iy2,iresol
	    js=(j-1)*nxin
	    do i=ix1,ix2,iresol
	      nn=nn+1
	      zs(nn)=zs(js+i)
	    end do
	  end do
	end if

      end if
c
c
      if(iseqfile.eq.1 .and. ncompseq.gt.0) then
	open(iunits,file=seqfile,
     +		    access='sequential',form='unformatted',
     +		    status='unknown')
      end if
c
c
c..time loop, read all Ps fields and find maximum surface pressure
c
      write(6,*) 'Finding min,max surface pressure for all timesteps'
c
      psmin=+1.e+35
      psmax=-1.e+35
c
      do nt=1,ntime
	in( 3)=iprog(1,nt)
	in( 4)=iprog(2,nt)
	in( 5)=iprog(3,nt)
	in( 9)=iprog(4,nt)
	in(10)=iprog(5,nt)
        in(11)=2
        in(12)=8
        in(13)=1000
        in(14)=0
	ipack=1
        call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,ps,100.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) call exit(2)
c
	if(initgrid.eq.0) then
	  call gridcheck(ldata,idata,igtype,nxin,nyin,grid,
     +			 nlevel,x1p,x2p,y1p,y2p,iresol,
     +			 nx,ny,ix1,ix2,iy1,iy2,isubarea,
     +			 max2d,max3d,imap,xmd2h,ymd2h,fc,hx,hy)
	  initgrid=1
	end if
c
	if(isubarea.eq.0) then
	  do i=1,nx*ny
	    psmin=min(psmin,ps(i))
	    psmax=max(psmax,ps(i))
	  end do
	else
	  do j=iy1,iy2,iresol
	    js=(j-1)*nxin
	    do i=ix1,ix2,iresol
	      psmin=min(psmin,ps(js+i))
	      psmax=max(psmax,ps(js+i))
	    end do
	  end do
	end if
c
      end do
c
      write(6,*) '   Ps min = ',psmin*0.01,' hPa'
      write(6,*) '   Ps max = ',psmax*0.01,' hPa'
c
c
c..time loop
c
      do nt=1,ntime
c
	write(6,*) 'Read: ',(iprog(i,nt),i=1,5)
c
	in( 3)=iprog(1,nt)
	in( 4)=iprog(2,nt)
	in( 5)=iprog(3,nt)
	in( 9)=iprog(4,nt)
	in(10)=iprog(5,nt)
c
c..read surface pressure (ps), scale from unit hPa to Pa
	jps=0
        in(11)=2
        in(12)=8
        in(13)=1000
        in(14)=0
	ipack=1
        call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,ps,100.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) call exit(2)
	if(isubarea.ne.0) then
	  nn=0
	  do j=iy1,iy2,iresol
	    js=(j-1)*nxin
	    do i=ix1,ix2,iresol
	      nn=nn+1
	      ps(nn)=ps(js+i)
	    end do
	  end do
	end if
	jps=1
c
	ju3d =0
	jv3d =0
	jth3d=0
c.old	jom3d=0
c
c..parameter loop
	do np=1,nparin
c
ccc	  write(6,*) '     parameter: ',iparam(np),'  ',cparam(np)
c
	  in(11)=ivcoord
	  in(12)=iparam(np)
c
c..level loop
	  do nl=1,nlevel
c
	    in(13)=ilevel(nl)
	    in(14)=-32767
c
	    ipack=1
	    call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,f2d,1.,
     +                    ldata,idata,ierror)
	    if(ierror.ne.0) call exit(2)
c
	    if(initgrid.eq.0) then
	      call gridcheck(ldata,idata,igtype,nxin,nyin,grid,
     +			     nlevel,x1p,x2p,y1p,y2p,iresol,
     +			     nx,ny,ix1,ix2,iy1,iy2,isubarea,
     +			     max2d,max3d,imap,xmd2h,ymd2h,fc,hx,hy)
	      initgrid=1
	    end if
c
	    if(isubarea.eq.0) then
	      nn=nx*ny*(nl-1)
	      do i=1,nx*ny
	        f3d(nn+i)=f2d(i)
	      end do
	    else
	      nn=nx*ny*(nl-1)
	      do j=iy1,iy2,iresol
		js=(j-1)*nxin
		do i=ix1,ix2,iresol
		  nn=nn+1
	          f3d(nn)=f2d(js+i)
		end do
	      end do
	    end if
c
	    idlevel(1,nl)=idata( 8)
	    idlevel(2,nl)=idata(19)
	    alevel(nl)=idata( 8)*10.
	    blevel(nl)=idata(19)*0.0001
c
c.........end do nl=1,nlevel
	  end do
c
c..store data for computation of new parameters
c
	  if(iparam(np).eq.2 .and. iu3d.eq.1) then
	    do i=1,nx*ny*nlevel
	      u3d(i)=f3d(i)
	    end do
	    ju3d=1
	  end if
	  if(iparam(np).eq.3 .and. iv3d.eq.1) then
	    do i=1,nx*ny*nlevel
	      v3d(i)=f3d(i)
	    end do
	    jv3d=1
	  end if
	  if(iparam(np).eq.9 .and. iq3d.eq.1) then
	    do i=1,nx*ny*nlevel
	      q3d(i)=f3d(i)
	    end do
	    jq3d=1
	  end if
	  if(iparam(np).eq.18 .and. ith3d.eq.1) then
	    do i=1,nx*ny*nlevel
	      th3d(i)=f3d(i)
	    end do
	    jth3d=1
	  end if
c.old	  if(iparam(np).eq.13 .and. iom3d.eq.1) then
c.old	    do i=1,nx*ny*nlevel
c.old	      om3d(i)=f3d(i)
c.old	    end do
c.old	    jom3d=1
c.old	  end if
c
	  if(iparam(np).eq.4) then
c..temperature input (not computed),  from kelvin to celsius
	    do i=1,nx*ny*nlevel
	      f3d(i)=f3d(i)-273.15
	    end do
	  end if
c
	  if(initlevels.eq.0) then
	    do nl=1,nlevel
	      plevel(nl)=alevel(nl)+blevel(nl)*psmax
	    end do
	    if(nlevel.gt.2) then
c..almost never data at lower level, adjust the two lower levels
	      p1=plevel(nlevel-2)*0.25 + plevel(nlevel-1)*0.75
	      p2=plevel(nlevel-1)*0.50 + plevel(nlevel)  *0.50
	      plevel(nlevel-1)=p1
	      plevel(nlevel)  =p2
	    end if
c
	    if(ptop.gt.0. .and. ptop.lt.900.) then
	      p1=plevel(1)
	      p2=plevel(nlevel)
	      pscale=(p2-ptop*100.)/(p2-p1)
	      do nl=1,nlevel
	        plevel(nl)=p2-(p2-plevel(nl))*pscale
	      end do
	    end if
c
c..get nice values
	    do nl=1,nlevel
	      ip=nint(plevel(nl)*0.01)
	      plevel(nl)=ip*100.
	    end do
	    write(6,*) 'Output levels (hPa):    nlevel= ',nlevel
	    write(6,*) (plevel(nl)*0.01,nl=1,nlevel)
	    initlevels=1
	  end if
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(np)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,np,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
c.......end do np=1,nparin
	end do
c
c..compute parameters
c
	if(nparw3d.gt.0 .and. jps.eq.1 .and.
c.old +	   jth3d.eq.1 .and. jom3d.eq.1) then
     +	   jth3d.eq.1 .and. ju3d.eq.1 .and. jv3d.eq.1) then
c
c..compute w
c
c.oldc..from potential temperature (K) to absolute temperture (K)
c.oldc..and compute w, omega from unit hPa/s to Pa/s
c.old	  r=287.04
c.old	  cp=1004.6
c.old	  g=9.80665
c.old	  p0=1.e+5
c.old	  p0inv=1./p0
c.old	  rkap=r/cp
c.old	  const=-(r/g)*100.
c.old	  do nl=1,nlevel
c.old	    i0=nx*ny*(nl-1)
c.old	    do i=1,nx*ny
c.old	      p=alevel(nl)+blevel(nl)*ps(i)
c.old	      t        =th3d(i0+i)*(p*p0inv)**rkap
c.old	      w        = const*(t/p) * om3d(i0+i)
c.old	      f3d(i0+i)=w
c.old	    end do
c.old	  end do
c
	  call compw(nx,ny,nlevel, hx,hy,
     +		     alevel,blevel, u3d,v3d,th3d,zs,ps, xmd2h,ymd2h,
     +		     ahalf, bhalf,  work2d(1,1),work2d(1,2),
     +		     work2d(1,3),work2d(1,4),work2d(1,5),work2d(1,6),
     +		     f3dv5d, f3d)
c
	  if(iseqfile.eq.1) then
c..output to sequential file with fields (automatic scaling)
	    if(isubarea.ne.0) 
     +	       call gridpar(-1,ldata,idata,igtype,nx,ny,grid,ierror)
	    idata(5)=ivcoord
	    idata(6)=iparam(nparw3d)
	    ldout=20+nx*ny
	    if(idata(9).gt.1000)
     +	       ldout=ldout + (idata(9)-(idata(9)/1000)*1000)
	    do nl=1,nlevel
	      i=nx*ny*(nl-1)+1
	      call asr4i2(2,nx*ny,f3d(i),idata(21),0,undef,
     +			  iscale,nundef)
	      idata( 7)=ilevel(nl)
	      idata( 8)=idlevel(1,nl)
	      idata(19)=idlevel(2,nl)
	      idata(20)=iscale
	      write(iunits) (idata(i),i=1,20)
	      write(iunits) (idata(i),i=21,ldout)
	    end do
	  end if
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(nparw3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,nparw3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
	if(nparpv3d.gt.0 .and. jps.eq.1 .and.
     +	   ju3d.eq.1 .and. jv3d.eq.1) then
c
c..compute potential vorticity (pv)
c
	  do nl=1,nlevel
	    k1=min(nl+1,nlevel)
	    k2=nl
	    k3=max(nl-1,1)
	    alvls(1)=alevel(k1)
	    blvls(1)=blevel(k1)
	    alvls(2)=alevel(k2)
	    blvls(2)=blevel(k2)
	    alvls(3)=alevel(k3)
	    blvls(3)=blevel(k3)
	    k1=nx*ny*(k1-1)+1
	    k2=nx*ny*(k2-1)+1
	    k3=nx*ny*(k3-1)+1
	    call potv(u3d(k1),v3d(k1),th3d(k1),
     +		      u3d(k2),v3d(k2),th3d(k2),
     +		      u3d(k3),v3d(k3),th3d(k3),ps,f3d(k2),
     +		      xmd2h,ymd2h,fc,nx,ny,alvls,blvls,0)
	  end do
c
	  if(iseqfile.eq.1) then
c..output to sequential file with fields (automatic scaling)
	    if(isubarea.ne.0) 
     +	       call gridpar(-1,ldata,idata,igtype,nx,ny,grid,ierror)
	    idata(5)=ivcoord
	    idata(6)=iparam(nparpv3d)
	    ldout=20+nx*ny
	    if(idata(9).gt.1000)
     +	       ldout=ldout + (idata(9)-(idata(9)/1000)*1000)
	    do nl=1,nlevel
	      i=nx*ny*(nl-1)+1
	      call asr4i2(2,nx*ny,f3d(i),idata(21),0,undef,
     +			  iscale,nundef)
	      idata( 7)=ilevel(nl)
	      idata( 8)=idlevel(1,nl)
	      idata(19)=idlevel(2,nl)
	      idata(20)=iscale
	      write(iunits) (idata(i),i=1,20)
	      write(iunits) (idata(i),i=21,ldout)
	    end do
	  end if
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(nparpv3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,nparpv3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
	if(nparrh3d.gt.0 .and. jps.eq.1 .and.
     +	   jth3d.eq.1 .and. jq3d.eq.1) then
c
c..compute rh
c
	  r=287.04
	  cp=1004.6
	  p0=1.e+5
	  p0inv=1./p0
	  rkap=r/cp
	  t0=273.15
	  eps=0.622
	  hpa=100.
	  const=100./eps
	  do nl=1,nlevel
	    i0=nx*ny*(nl-1)
	    do i=1,nx*ny
	      p=alevel(nl)+blevel(nl)*ps(i)
	      t=th3d(i0+i)*(p*p0inv)**rkap - t0
              xl=(t+105.)*0.2
              lx=xl
	      esat=(ewt(lx)+(ewt(lx+1)-ewt(lx))*(xl-lx))*hpa
              rh=const*p*q3d(i0+i)/esat
cc	      rh=max(rh,  0.)
cc	      rh=min(rh,100.)
	      f3d(i0+i)=rh
	    end do
	  end do
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(nparrh3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,nparrh3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
	if(npart3d.gt.0 .and. jps.eq.1 .and.
     +	   jth3d.eq.1) then
c
c..compute t (temp.celsius) from th (pot.temp.)
c
	  r=287.04
	  cp=1004.6
	  p0=1.e+5
	  p0inv=1./p0
	  rkap=r/cp
	  t0=273.15
	  do nl=1,nlevel
	    i0=nx*ny*(nl-1)
	    do i=1,nx*ny
	      p=alevel(nl)+blevel(nl)*ps(i)
ccc	      f3d(i0+i)=th3d(i0+i)*(p*p0inv)**rkap - t0
	      f3d(i0+i)=th3d(i0+i)*(p*p0inv)**rkap
	    end do
	  end do
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(npart3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,npart3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
	if(npartd3d.gt.0 .and. jps.eq.1 .and.
     +	   jth3d.eq.1 .and. jq3d.eq.1) then
c
c..compute Td, dew point temperature
c
	  r=287.04
	  cp=1004.6
	  p0=1.e+5
	  p0inv=1./p0
	  rkap=r/cp
	  t0=273.15
	  eps=0.622
	  hpa=100.
	  const=1./eps
	  do nl=1,nlevel
	    i0=nx*ny*(nl-1)
	    do i=1,nx*ny
	      p=alevel(nl)+blevel(nl)*ps(i)
	      t=th3d(i0+i)*(p*p0inv)**rkap - t0
              xl=(t+105.)*0.2
              lx=xl
	      esat=(ewt(lx)+(ewt(lx+1)-ewt(lx))*(xl-lx))*hpa
              rh=const*p*q3d(i0+i)/esat
	      rh=max(rh,0.01)
	      rh=min(rh,1.00)
	      etd=rh*esat/hpa
	      do while (ewt(lx).gt.etd .and. lx.gt.1)
		lx=lx-1
	      end do
	      xl=(etd-ewt(lx))/(ewt(lx+1)-ewt(lx))
	      td=-105.+(real(lx)+xl)*5.
	      f3d(i0+i)=td+t0
	    end do
	  end do
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(npartd3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,npartd3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
	if(nparff3d.gt.0 .and.
     +	   ju3d.eq.1 .and. jv3d.eq.1) then
c
c..compute ff
c
	  do i=1,nx*ny*nlevel
	    f3d(i)=sqrt(u3d(i)*u3d(i)+v3d(i)*v3d(i))
	  end do
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(nparff3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,nparff3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
	if(nparcat3d.gt.0 .and. jps.eq.1 .and.
     +	        ju3d.eq.1 .and. jv3d.eq.1) then
c
c..compute potential clear air turbulence (cat)
c
	  do nl=1,nlevel
	    k1=min(nl+1,nlevel)
	    k2=nl
	    k3=max(nl-1,1)
	    alvls(1)=alevel(k1)
	    blvls(1)=blevel(k1)
	    alvls(2)=alevel(k2)
	    blvls(2)=blevel(k2)
	    alvls(3)=alevel(k3)
	    blvls(3)=blevel(k3)
	    k1=nx*ny*(k1-1)+1
	    k2=nx*ny*(k2-1)+1
	    k3=nx*ny*(k3-1)+1
	    call cat(nx,ny,u3d(k1),v3d(k1),th3d(k1),
     +		           u3d(k2),v3d(k2),th3d(k2),
     +		           u3d(k3),v3d(k3),th3d(k3),ps,f3d(k2),
     +		           xmd2h,ymd2h,alvls,blvls,0)
	  end do
c
	  if(iseqfile.eq.1) then
c..output to sequential file with fields (automatic scaling)
	    if(isubarea.ne.0) 
     +	       call gridpar(-1,ldata,idata,igtype,nx,ny,grid,ierror)
	    idata(5)=ivcoord
	    idata(6)=iparam(nparcat3d)
	    ldout=20+nx*ny
	    if(idata(9).gt.1000)
     +	       ldout=ldout + (idata(9)-(idata(9)/1000)*1000)
	    do nl=1,nlevel
	      i=nx*ny*(nl-1)+1
	      call asr4i2(2,nx*ny,f3d(i),idata(21),0,undef,
     +			  iscale,nundef)
	      idata( 7)=ilevel(nl)
	      idata( 8)=idlevel(1,nl)
	      idata(19)=idlevel(2,nl)
	      idata(20)=iscale
	      write(iunits) (idata(i),i=1,20)
	      write(iunits) (idata(i),i=21,ldout)
	    end do
	  end if
c
c..vertical interpolation to fixed levels required by vis-5d
	  call vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		      ps,alevel,blevel,plevel)
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(nparcat3d)
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,nparcat3d,f3dv5d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end if
c
c..surface fields.........................................................
c
      if(isurf.eq.1) then
c
        in(11)=2
        in(13)=1000
        in(14)=0
c
	do np=npsurf1,npsurf2
c
	  if(iparam(np).eq.17) then
c
c..precipitation
	    ihr2=iprog(5,nt)
	    ihr1=ihr2-6
	    if(nt.gt.1) then
	      ihr1=iprog(5,nt-1)
	    elseif(nt.lt.ntime) then
	      ihr1=ihr2-(iprog(5,nt+1)-iprog(5,nt))
	    end if
	    if(ihr1.gt.0) then
              in(10)=ihr1
              in(12)=iparam(np)
	      ipack=1
              call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,f3d,1.,
     +                      ldata,idata,ierror)
              if(ierror.ne.0) call exit(2)
              in(10)=ihr2
	    else
	      do i=1,nxin*nyin
		f3d(i)=0.
	      end do
	    end if
	    if(ihr2.gt.0) then
              in(12)=iparam(np)
	      ipack=1
              call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,f2d,1.,
     +                      ldata,idata,ierror)
              if(ierror.ne.0) call exit(2)
	    else
	      do i=1,nxin*nyin
		f2d(i)=0.
	      end do
	    end if
	    do i=1,nxin*nyin
	      f2d(i)=max(f2d(i)-f3d(i),0.)
	    end do
c
	  elseif(iparam(np).ne.-2) then
c
            in(12)=iparam(np)
	    ipack=1
            call mrfturbo(2,feltfile,iunitf,in,ipack,max2d,f2d,1.,
     +                    ldata,idata,ierror)
            if(ierror.ne.0) call exit(2)
c
	  end if
c
	  if(isubarea.ne.0 .and. iparam(np).ne.-2) then
	    nn=0
	    do j=iy1,iy2,iresol
	      js=(j-1)*nxin
	      do i=ix1,ix2,iresol
	        nn=nn+1
	        f2d(nn)=f2d(js+i)
	      end do
	    end do
	  end if
c
	  if(iparam(np).eq.31) then
c..temp.2m
	    do i=1,nx*ny
	      f2d(i)=f2d(i)-273.15
	    end do
	  elseif(iparam(np).eq.33) then
c..u10m, save for ff10m
	    do i=1,nx*ny
	      u3d(i)=f2d(i)
	    end do
	  elseif(iparam(np).eq.34) then
c..v10m, save for ff10m
	    do i=1,nx*ny
	      v3d(i)=f2d(i)
	    end do
	  elseif(iparam(np).eq.-2) then
c..compute ff10m
	    do i=1,nx*ny
	      f2d(i)=sqrt(u3d(i)*u3d(i)+v3d(i)*v3d(i))
	    end do
	  end if
c
c..VIS-5D output
	  write(6,*) 'putv5d ... ',cparam(np),'  (surface)'
	  call putv5d(initv5d,v5dfile,
     +		      nx,ny,nlevel,ntime,nparam,nt,np,f2d,
     +		      iparam,cparam,nplevel,
     +		      ilevel,alevel,blevel,plevel,
     +		      itime,ivcout,igtype,grid,ivertical)
c
	end do
c
      end if
c.........................................................................
c
c.....end do nt=1,ntime
      end do
c
c..close input felt file
      call mrfturbo(3,feltfile,iunitf,in,0,1,1.,1.,1,idata,ierror)
c
c..close output vis5d file
      initv5d=-1
      call putv5d(initv5d,v5dfile,
     +		  nx,ny,nlevel,ntime,nparam,nt,np,f3dv5d,
     +		  iparam,cparam,nplevel,
     +		  ilevel,alevel,blevel,plevel,
     +		  itime,ivcout,igtype,grid,ivertical)
c
      end
c
c***********************************************************************
c
      subroutine gridcheck(ldata,idata,igtype,nxin,nyin,grid,
     +			   nlevel,x1p,x2p,y1p,y2p,iresol,
     +			   nx,ny,ix1,ix2,iy1,iy2,isubarea,
     +			   max2d,max3d,imap,xmd2h,ymd2h,fc,hx,hy)
c
      integer   ldata,igtype,nxin,nyin,nlevel,iresol
      integer   nx,ny,ix1,ix2,iy1,iy2,isubarea,max2d,max3d,imap
      integer*2 idata(ldata)
      real      x1p,x2p,y1p,y2p,hx,hy
      real      grid(6),xmd2h(max2d),ymd2h(max2d),fc(max2d)
c
      call gridpar(+1,ldata,idata,igtype,nxin,nyin,grid,ierror)
      if(ierror.ne.0) then
	write(6,*) 'GRIDPAR ERROR: ',ierror
	write(6,*) '    grid type: ',igtype
	call exit(2)
      end if
c
      ix1=nint(1.+(nxin-1.)*x1p*0.01)
      ix2=nint(1.+(nxin-1.)*x2p*0.01)
      iy1=nint(1.+(nyin-1.)*y1p*0.01)
      iy2=nint(1.+(nyin-1.)*y2p*0.01)
      nx=1+(ix2-ix1)/iresol
      ny=1+(iy2-iy1)/iresol
      ix2=ix1+(nx-1)*iresol
      iy2=iy1+(ny-1)*iresol
      if(nx.ne.nxin .or. ny.ne.nyin) isubarea=1
c#######################################################################
      write(6,*) 'x1p,x2p,y1p,y2p,iresol:'
      write(6,*)  x1p,x2p,y1p,y2p,iresol
      write(6,*) 'nxin,nyin,nx,ny: ',nxin,nyin,nx,ny
      write(6,*) 'ix1,ix2,iy1,iy2: ',ix1,ix2,iy1,iy2
      write(6,*) 'input  grid: ',grid
c#######################################################################
      if(isubarea.eq.1) then
	if(ix1.lt.1 .or. ix1.ge.ix2 .or. ix2.gt.nxin .or.
     +	   iy1.lt.1 .or. iy1.ge.iy2 .or. iy2.gt.nyin) then
	  write(6,*) 'ERROR. BAD subarea'
	  call exit(2)
	end if
	if(igtype.eq.1 .or. igtype.eq.4) then
c..polarstereographic grid
	  grid(1)=grid(1)-(ix1-1)
	  grid(2)=grid(2)-(iy1-1)
	  grid(3)=grid(3)*iresol
	elseif(igtype.eq.2 .or. igtype.eq.3) then
c..geographic or spherical (rotated) grid
	  grid(1)=grid(1)+grid(3)*(ix1-1)
	  grid(2)=grid(2)+grid(4)*(iy1-1)
	  grid(3)=grid(3)*iresol
	  grid(4)=grid(4)*iresol
	else
	  write(6,*) 'ERROR. Unknow gridtype for subarea'
	  call exit(2)
	end if
c#######################################################################
	write(6,*) 'output grid: ',grid
c#######################################################################
      end if
c
      if(nx*ny.gt.max2d .or. nx*ny*nlevel.gt.max3d) then
	write(6,*) 'ERROR.  Too much data'
	write(6,*) '   nx,ny,nx*ny,max2d:'
	write(6,*) ' ',nx,ny,nx*ny,max2d
	write(6,*) '   nx,ny,nlevel,nx*ny*nlevel,max3d:'
	write(6,*) ' ',nx,ny,nlevel,nx*ny*nlevel,max3d
	call exit(3)
      end if
c
      if(imap.gt.0) then
	call mapfield(3,1,igtype,grid,nx,ny,xmd2h,ymd2h,fc,
     +		      hx,hy,ierror)
	if(ierror.ne.0) then
	  write(6,*) 'MAPFIELD ERROR: ',ierror
	  write(6,*) '    grid type: ',igtype
	  call exit(2)
	end if
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine vinter(nx,ny,nlevel,f3d,f3dv5d,
     +		        ps,alevel,blevel,plevel)
c
c..vertical interpolation from model eta levels to fixed pressure levels
c..(linear in pressure, same number of levels input and output)
c
      integer nx,ny,nlevel
      real    f3d(nx,ny,nlevel),f3dv5d(nx,ny,nlevel),ps(nx,ny)
      real    alevel(nlevel),blevel(nlevel),plevel(nlevel)
c
      undef=+1.e+35
c
      do j=1,ny
	do i=1,nx
c
	  psurf=ps(i,j)
	  k=2
c
	  do nl=1,nlevel
	    p=plevel(nl)
	    do while (p.gt.alevel(k)+blevel(k)*psurf
     +		                   .and. k.lt.nlevel)
	      k=k+1
	    end do
	    p1=alevel(k-1)+blevel(k-1)*psurf
	    p2=alevel(k)+blevel(k)*psurf
	    if(p.gt.p1 .and. p.le.p2) then
	      f3dv5d(i,j,nl)=( f3d(i,j,k-1)*(p2-p)
     +			      +f3d(i,j,k)  *(p-p1))/(p2-p1)
	    elseif(p.le.p1) then
	      f3dv5d(i,j,nl)=f3d(i,j,k-1)
	    else
	      f3dv5d(i,j,nl)=undef
	    end if
c
	  end do
c
	end do
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine compw(nx,ny,ne, hx,hy,
     +		       af,bf, u,v,th,zs,ps, xmd2h,ymd2h,
     +		       ah,bh, dp,dlnp,alfa,sum,uu,vv, z, w)
c
c*****************************************************************
c
c compw - compute w
c
c purpose:
c
c compute vertical velocity w from variables at model levels
c
c input parameters:
c
c nx - number of gridpoints in x-direction
c ny - number of gridpoints in y-direction
c ne - number of vertical full levels
c hx - grid distance in x-direction in meters (where xm=ym=1)
c hy - grid distance in y-direction in meters (where xm=ym=1)
c af - definition of model full levels a(ne)
c bf - definition of model full levels b(ne)
c zs - surface geopotential in meters
c ps - surface pressure in Pa
c u  - velocity component in x-direction in m/s
c v  - velocity component in y-direction in m/s
c th - potential temperature in K
c dp,dlnp,alfa,sum,uu,vv - 2d work arrays
c z                      - 3d work array
c
c output parameters:
c
c w  - vertical velocity in m/s
c
c history:
c
c j.e. haugen    dnmi    aug 1995
c   a. foss      dnmi    26.05.1997
c
c*****************************************************************
c
      implicit none
c
c declaration of input/output and work parameters
c
      integer nx,ny,ne
      real hx,hy,
     +     af(ne),bf(ne),
     +     zs(nx,ny),ps(nx,ny),xmd2h(nx,ny),ymd2h(nx,ny),
     +     u(nx,ny,ne),v(nx,ny,ne),th(nx,ny,ne),
     +     ah(ne+1),bh(ne+1),
     +     dp(nx,ny),dlnp(nx,ny),alfa(nx,ny),
     +     sum(nx,ny),uu(nx,ny),vv(nx,ny),
     +     z(nx,ny,ne),
     +     w(nx,ny,ne)
c
c local declarations
c
      integer i,j,k
      real    r,g,cp,p0,p0inv,rkap,twohx,twohy,
     +        p,rt,ln2,da,db,pm,pp,div,w1,w2
c
c setup of constants
c
      g=9.80665
      r=287.04
      cp=1004.6
      p0=1.e+5
      p0inv=1./p0
      rkap=r/cp
c
      twohx=2.*hx
      twohy=2.*hy
c
c compute model half levels
c
      ah(1)=0.
      bh(1)=0.
      ah(ne+1)=0.
      bh(ne+1)=1.
c
      do k=ne,2,-1
         ah(k)=2.*af(k)-ah(k+1)
         bh(k)=2.*bf(k)-bh(k+1)
      enddo
c
c
c vertical integration of hydrostatic equation
c
      do j=1,ny
      do i=1,nx
         sum(i,j)=zs(i,j)*g
      enddo
      enddo
      do k=ne,1,-1
c--------------------------------------------------------------------
c version with 2d arrays of dp,dlnp,alfa to save space,
c values computed twice
         if ( k.eq.1 ) then
            ln2=alog(2.)
            da=ah(2)-ah(1)
            db=bh(2)-bh(1)
            do j=1,ny
            do i=1,nx
               dp(i,j)=da+db*ps(i,j)
               dlnp(i,j)=0.
               alfa(i,j)=ln2
            enddo
            enddo
         else
            da=ah(k+1)-ah(k)
            db=bh(k+1)-bh(k)
            do j=1,ny
            do i=1,nx
               pm=ah(k)+bh(k)*ps(i,j)
               pp=ah(k+1)+bh(k+1)*ps(i,j)
               dp(i,j)=da+db*ps(i,j)
               dlnp(i,j)=alog(pp/pm)
               alfa(i,j)=1.-pm*dlnp(i,j)/dp(i,j)
            enddo
            enddo
         endif
c--------------------------------------------------------------------
         do j=1,ny
         do i=1,nx
	    p=af(k)+bf(k)*ps(i,j)
	    rt=r*th(i,j,k)*(p*p0inv)**rkap
            z(i,j,k)=sum(i,j)+alfa(i,j)*rt
            sum(i,j)=sum(i,j)+dlnp(i,j)*rt
         enddo
         enddo
      enddo
c
c vertical integral of divergence, compute w
c
      do j=2,ny-1
      do i=2,nx-1
         sum(i,j)=0.
      enddo
      enddo
      do k=1,ne
c--------------------------------------------------------------------
c version with 2d arrays of dp,dlnp,alfa to save space,
c values computed twice
         if ( k.eq.1 ) then
            ln2=alog(2.)
            da=ah(2)-ah(1)
            db=bh(2)-bh(1)
            do j=1,ny
            do i=1,nx
               dp(i,j)=da+db*ps(i,j)
               dlnp(i,j)=0.
               alfa(i,j)=ln2
            enddo
            enddo
         else
            da=ah(k+1)-ah(k)
            db=bh(k+1)-bh(k)
            do j=1,ny
            do i=1,nx
               pm=ah(k)+bh(k)*ps(i,j)
               pp=ah(k+1)+bh(k+1)*ps(i,j)
               dp(i,j)=da+db*ps(i,j)
               dlnp(i,j)=alog(pp/pm)
               alfa(i,j)=1.-pm*dlnp(i,j)/dp(i,j)
            enddo
            enddo
         endif
c--------------------------------------------------------------------
         do j=1,ny
         do i=1,nx
            uu(i,j)=u(i,j,k)*dp(i,j)/(ymd2h(i,j)*twohy)
            vv(i,j)=v(i,j,k)*dp(i,j)/(xmd2h(i,j)*twohx)
         enddo
         enddo
         do j=2,ny-1
         do i=2,nx-1
            div=xmd2h(i,j)*ymd2h(i,j)
     +			  *( (uu(i+1,j)-uu(i-1,j))*twohy
     +                      +(vv(i,j+1)-vv(i,j-1))*twohx)
	    p=af(k)+bf(k)*ps(i,j)
	    rt=r*th(i,j,k)*(p*p0inv)**rkap
            w1=rt*(dlnp(i,j)*sum(i,j)+alfa(i,j)*div)/dp(i,j)
            w2= xmd2h(i,j)*(z(i+1,j,k)-z(i-1,j,k))
     +         +ymd2h(i,j)*(z(i,j+1,k)-z(i,j-1,k))
            w(i,j,k)=(w1+w2)/g
            sum(i,j)=sum(i,j)+div
         enddo
         enddo
         do i=2,nx-1
            w(i,1,k)=w(i,2,k)
            w(i,ny,k)=w(i,ny-1,k)
         enddo
         do j=1,ny
            w(1,j,k)=w(2,j,k)
            w(nx,j,k)=w(nx-1,j,k)
         enddo
      enddo
c
      return
      end
c
c***********************************************************************
c
      subroutine potv(u1,v1,th1,u2,v2,th2,u3,v3,th3,ps,pv,
     *                xmd2h,ymd2h,f,nx,ny,alvls,blvls,ipress)
c
c       compute potential vorticity (pv)
c
c       sigma/eta levels.
c
c       u,v,th fields: 3 levels (1,2,3 in 'lower to upper' sequence)
c       computing pv in level 2.
c       (for levels k=1 and k=kk: copy fields from the
c        closest level to the level 'k=0' or 'k=kk+1')
c
c	ipress=0: ps and alvls in unit Pa
c	      =2: ps and alvls in unit hPa
c
      integer nx,ny,ipress
      real    u1(nx,ny),v1(nx,ny),th1(nx,ny)
      real    u2(nx,ny),v2(nx,ny),th2(nx,ny)
      real    u3(nx,ny),v3(nx,ny),th3(nx,ny)
      real    ps(nx,ny),pv(nx,ny)
      real    xmd2h(nx,ny),ymd2h(nx,ny),f(nx,ny)
      real    alvls(3),blvls(3)
c
      dalvl = alvls(3)-alvls(1)
      dblvl = blvls(3)-blvls(1)
c
c..1 pvu = 1.e-6 m**2 s**-1 k kg**-1 ....... (1 hPa = 100 Pa)
c
      pvu = 1.e-6
      g=9.8
c
      pvscal=-g/pvu
      if(ipress.eq.2) pvscal=pvscal/100.
c
      do j=2,ny-1
        do i=2,nx-1
c
          dth=max((th3(i,j)-th1(i,j)),0.001)
c
          dvdxth = xmd2h(i,j)*((v2(i+1,j)-v2(i-1,j))
     1                        -(v3(i,j)-v1(i,j))/dth
     2                        *(th2(i+1,j)-th2(i-1,j)))
c
          dudyth = ymd2h(i,j)*((u2(i,j+1)-u2(i,j-1))
     1                        -(u3(i,j)-u1(i,j))/dth
     2                        *(th2(i,j+1)-th2(i,j-1)))
c
          dp = dalvl+dblvl*ps(i,j)
c
          pv(i,j) = pvscal*(f(i,j)+(dvdxth - dudyth))*dth/dp
c
        end do
      end do
c
c..lateral boundaries (no values computed)
      do j=2,ny-1
        pv( 1,j)=pv(   2,j)
        pv(nx,j)=pv(nx-1,j)
      end do
      do i=1,nx
        pv(i, 1)=pv(i,   2)
        pv(i,ny)=pv(i,ny-1)
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine cat(nx,ny,u1,v1,th1,u2,v2,th2,u3,v3,th3,ps,fcat,
     *               xmd2h,ymd2h,alvls,blvls,ipress)
c
c	compute cat, clear air turbulence
c
c        CAT: 'THE METEOROLOGICAL MAGAZINE' (UK Met. Office)
c              - Met.O. 931, No. 1299, Vol. 109, October   1980, p. 308
c              - Met.O. 971, No. 1370, Vol. 115, September 1986, p. 276
c
c       sigma/eta levels.
c
c       u,v,th fields: 3 levels (1,2,3 in 'lower to upper' sequence)
c       computing cat in level 2.
c       (for levels k=1 and k=kk: copy fields from the
c        closest level to the level 'k=0' or 'k=kk+1')
c
c	ipress=0: ps and alvls in unit Pa
c	      =2: ps and alvls in unit hPa
c
      integer nx,ny,ipress
      real    u1(nx,ny),v1(nx,ny),th1(nx,ny)
      real    u2(nx,ny),v2(nx,ny),th2(nx,ny)
      real    u3(nx,ny),v3(nx,ny),th3(nx,ny)
      real    ps(nx,ny),fcat(nx,ny)
      real    xmd2h(nx,ny),ymd2h(nx,ny)
      real    alvls(3),blvls(3)
c
      g=9.8
      r=287.
      cp=1004.
      rcp=r/cp
      ginv=1./g
      p0=100000.
      if(ipress.eq.2) p0=1000.
      p0inv=1./p0
c
      csh=1.25*1.e+5
      csv=0.25*1.e+6
      c=10.5
c
      do j=2,ny-1
        do i=2,nx-1
c
          p1=alvls(1)+blvls(1)*ps(i,j)
          p3=alvls(3)+blvls(3)*ps(i,j)
	  pi1=cp*(p1*p0inv)**rcp
	  pi3=cp*(p3*p0inv)**rcp
c
	  duz=u3(i,j)-u1(i,j)
	  dvz=v3(i,j)-v1(i,j)
c
	  dz=th2(i,j)*(pi3-pi1)*ginv
	  sv2=(duz*duz+dvz*dvz)/(dz*dz)
c
	  dux=(u2(i+1,j)-u2(i-1,j))*xmd2h(i,j)
	  dvx=(v2(i+1,j)-v2(i-1,j))*xmd2h(i,j)
	  duy=(u2(i,j+1)-u2(i,j-1))*ymd2h(i,j)
	  dvy=(v2(i,j+1)-v2(i,j-1))*ymd2h(i,j)
c
	  sh=sqrt(dux*dux+dvx*dvx+duy*duy+dvy*dvy)
c
ccc	  sh=sqrt(dux*dux+dvx*dvx+duy*duy+dvy*dvy
ccc  +		               +2.*(dvx*duy-dux*dvy))
c
          fcat(i,j)=csh*sh+csv*sv2+c
c
	end do
      end do
c
      do j=2,ny-1
        fcat( 1,j)=fcat(   2,j)
        fcat(nx,j)=fcat(nx-1,j)
      end do
      do i=1,nx
        fcat(i, 1)=fcat(i,   2)
        fcat(i,ny)=fcat(i,ny-1)
      end do
c
      return
      end
