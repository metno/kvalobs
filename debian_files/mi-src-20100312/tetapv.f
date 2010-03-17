      program tetapv
c
c        interpolasjon av vind fra sigma/eta-flater til isentrop-flater.
c        legger ogsaa ut felt med trykk i isentrop-flatene
c        og potensiell virvling i isentrop-flatene
c        (pv beregnes ikke i 1. og siste)
c
c        input grid: polarstereografisk, geografisk, rotert sfaerisk
c
c        u,v og pi interpoleres lineaert i potensiell temperatur (th).
c
c        hvis under nederste innleste flate: verdi = 'udefinert'
c        hvis over  oeverste innleste flate: verdi = 'udefinert'
c
c
c----------------------------------------------------------------------
c      DNMI library subroutines:  mrfelt (+rfelt)
c                                 mwfelt (+wfelt)
c                                 qfelt
c                                 cmdarg
c                                 daytim
c                                 gridpar
c                                 mapfield
c                                 sphrot
c                                 qfelt
c
c-----------------------------------------------------------------------
c  DNMI/FoU  26.09.1991  Anstein Foss ... IBM
c  DNMI/FoU  27.10.1992  Anstein Foss ... Unix
c  DNMI/FoU  14.03.1994  Anstein Foss ... 31 levels and new WFELT
c  DNMI/FoU  22.08.1994  Anstein Foss ... p(pv) fields & rewrite
c  DNMI/FoU  26.05.1995  Anstein Foss ... for Hirlam (eta & hor.grid)
c  DNMI/FoU  02.01.1996  Anstein Foss ... Montgomery pot., exner interp.
c  DNMI/FoU  09.03.1998  Anstein Foss ... input pressure levels
c  DNMI/FoU  10.12.2002  Anstein Foss ... max.wind (p,u,v)
c  DNMI/FoU  05.02.2003  Anstein Foss ... pressure,height for temperatures
c  DNMI/FoU  13.03.2003  Anstein Foss ... tetapvtab.inc with pitab etc
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c met.no/FoU 04.01.2010  Ole Vignes ..... min.p for max.wind (-W option)
c-----------------------------------------------------------------------
c
      implicit none
c
      include 'tetapv.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c  tetapv.inc ... include file for tetapv.f
c
c  max2d  : max size of 2d-fields (nx*ny)
c  max3d  : max size of 3d-fields (nx*ny*kk) in sigma/eta levels
c  kkmax  : max no. of input sigma/eta levels
c  mpvlev : max no. of output pv levels (p(pv=constant) fields)
c  mtclev : max no. of output t  levels (p,z(t=constant) fields)
c
ccc   integer max2d,max3d,kkmax,mpvlev,mtclev
c
ccc   parameter (max2d=200000,max3d=200000*40)
ccc   parameter (kkmax=100,mpvlev=6,mtclev=20)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'tetapvtab.inc'
c
      integer ldata
c
      parameter (ldata=20+max2d+50)
c
      real     us(max3d),vs(max3d),ths(max3d),zs(max3d)
     +        ,psurf(max2d),zsurf(max2d)
     +        ,uth(max2d),vth(max2d),pth(max2d),mth(max2d),pv(max2d)
     +        ,pth2(max2d),pth3(max2d)
     +        ,xmd2h(max2d),ymd2h(max2d),f(max2d)
     +        ,pv3(max2d),ppvlev(max2d,mpvlev),pvnext(max2d)
c
      common/work/us,vs,ths,zs,psurf,zsurf
     +		 ,uth,vth,pth,mth,pv,pth2,pth3
     +		 ,xmd2h,ymd2h,f,pv3,ppvlev,pvnext
c
      integer  knum(max2d),nextpv(max2d)
c
      integer*2 idata(ldata)
      integer*2 inr(16),inps(16)
      integer*2 idfile(32),idato(7)
      integer   levels(kkmax),ipvlev(mpvlev)
      integer   itlevels(mtclev)
c
      real      alevel(kkmax),blevel(kkmax),grid(6),rpvlev(mpvlev)
      real      tlevels(mtclev)
      real      ahalf(kkmax+1),bhalf(kkmax+1)
c
      integer nx,ny,i,j,k,istop,n,ioutuv,ioutp,ioutpv,ioutm,npvlev,
     +        ntlevels,nsurfp,isurfp,ioutwm,iprod,igrid,ivcoor,kk,
     +        ithmin,ithmax,ithstp,ihmin,ihmax,ihstp,nt,it,
     +        maincomp,iunitr,iunitw,mnd,idag,ipartemp,
     +	      izsurf,izread,izcomp,izzero,ihour,igtype,n3d,idatyp
      real    rt,undef
      real    r,cp,rcp,cpr,p0,p,pi,pwmin
c
      character*256 filein,fileot
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=12)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=2)
      integer       nargs
      character*256 cargs(margs)
      integer       mispec
      parameter    (mispec=100)
      integer       ispec(mispec)
      integer       mrspec
      parameter    (mrspec=5+mpvlev+mtclev)
      real          rspec(mrspec)
      integer       mcspec
      parameter    (mcspec=1)
      character*1   cspec(mcspec)
      integer       ierror
      integer       nerror
c..cmdarg................................................
c
c..cmdarg...................................................
      data copt/'g','s','e','l','t','o','u','x','p','w','c','W'/
      data iopt/ 1 , 1 , 1 , 2 , 1 , 1 , 2 , 1 , 0 , 0 , 2 , 2 /
c..cmdarg........1...2...3...4...5...6...7...8...9..10..11..12..
c
      data undef/+1.e+35/
c
      istop=1
c
c..cmdarg................................................
c..get command line arguments
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c..cmdarg................................................
c
      if(nargs.ne.2) ierror=-1
      if(iopts(1,1).ne.2) ierror=-1
      n=0
      if(iopts(1,2).eq.1) n=n+1
      if(iopts(1,3).eq.1) n=n+1
      if(iopts(1,9).eq.1) n=n+1
      if(n.ne.1) ierror=-1
      if(iopts(1,4).ne.3) ierror=-1
      if(iopts(1,5).ne.3) ierror=-1
c
      if(ierror.ne.0) then
        write(6,*)
        write(6,*) 'usage:  tetapv [options] input_file output_file'
        write(6,*)
        write(6,*) ' input and output files are existing felt files'
        write(6,*)
        write(6,*) ' options:    (required: -g -s/-e -l -t)'
        write(6,*) '  -g <producer_no>,<grid_no>'
        write(6,*) '  -s <no_of_model_sigma_levels>  (e.g. norlam)'
        write(6,*) '  -e <no_of_model_eta_levels>    (e.g. hirlam)'
        write(6,*) '  -p : using all pressure levels (e.g. Ecmwf)'
        write(6,*) '  -l <theta_min>,<theta_max>,<theta_step>'
        write(6,*) '      with theta in unit kelvin (one decimal ok)'
        write(6,*) '  -t <hour_min>,<hour_max>,<hour_step>'
        write(6,*) '  -o1,1,1,1 : output wind,p,pv,Montgomery.potential'
        write(6,*) '            with  0=no  1=yes  (default -o1,1,1,0)'
        write(6,*) '  -u <pv1,pv2,...> : p in levels pv=const.'
        write(6,*) '           (unit pvu, one decimal ok, default none)'
        write(6,*) '  -w : output of max.wind and p.max.wind'
        write(6,*) '  -W <pmin> : like -w, but only where p > pmin[hPa]'
        write(6,*) '  -c <t1,t2,...> : p,h in levels temp=const.'
        write(6,*) '       (unit Celsius, one decimal ok, default none)'
        write(6,*) '  -x <param_no,...> : transfer surface parameters'
        write(6,*) '  e.g.'
        write(6,*) '  -g88,1814 -s31 -l270,370,5 -t0,48,6 -u1,1.5,2,2.5'
        write(6,*) '  -g88,2006 -e31 -l270,370,5 -t0,48,6 -u1,1.5,2,2.5'
        write(6,*)
        write(6,*) '  ( tetapv limits: max(nx*ny):    ',max2d, ' )'
        write(6,*) '  (                max(nx*ny*kk): ',max3d, ' )'
        write(6,*) '  (                max(kk):       ',kkmax, ' )'
        write(6,*) '  (         max no. of pv levels: ',mpvlev,' )'
        write(6,*) '  (         max no. of t  levels: ',mtclev,' )'
        write(6,*)
        stop 1
      end if
c
c
      ioutuv=1
      ioutp =1
      ioutpv=1
      ioutm =0
      npvlev=0
      nsurfp=0
      isurfp=1
      ioutwm=0
      ntlevels=0
c
c..cmdarg...................................................
c     data copt/'g','s','e','l','t','o','u','x','p','w','c'/
c     data iopt/ 1 , 1 , 1 , 2 , 1 , 1 , 2 , 1 , 0 , 0 , 2 /
c..cmdarg........1...2...3...4...5...6...7...8...9..10..11..
c
      iprod=ispec(iopts(2,1))
      igrid=ispec(iopts(2,1)+1)
      if(iopts(1,2).eq.1) then
        ivcoor=2
        kk=ispec(iopts(2,2))
      elseif(iopts(1,3).eq.1) then
        ivcoor=10
        kk=ispec(iopts(2,3))
      elseif(iopts(1,9).eq.1) then
        ivcoor=1
        kk=0
      end if
      ithmin=nint(10.*rspec(iopts(2,4)))
      ithmax=nint(10.*rspec(iopts(2,4)+1))
      ithstp=nint(10.*rspec(iopts(2,4)+2))
      ihmin=ispec(iopts(2,5))
      ihmax=ispec(iopts(2,5)+1)
      ihstp=ispec(iopts(2,5)+2)
      if(iopts(1,6).ge.1) ioutuv=ispec(iopts(2,6)+0)
      if(iopts(1,6).ge.2) ioutp =ispec(iopts(2,6)+1)
      if(iopts(1,6).ge.3) ioutpv=ispec(iopts(2,6)+2)
      if(iopts(1,6).ge.4) ioutm =ispec(iopts(2,6)+3)
c
      npvlev=iopts(1,7)
      if(npvlev.gt.mpvlev) npvlev=mpvlev
      do n=1,npvlev
        rpvlev(n)=rspec(iopts(2,7)+n-1)
        ipvlev(n)=nint(rpvlev(n)*10.)
        if(ipvlev(n).lt.01) ipvlev(n)=01
        rpvlev(n)=ipvlev(n)*0.1
      end do
c
      nsurfp=iopts(1,8)
      if(nsurfp.gt.0) isurfp=iopts(2,8)
      if(iopts(1,10).eq.1) ioutwm=1
c
      ntlevels=iopts(1,11)
      if(ntlevels.gt.mtclev) ntlevels=mtclev
      do n=1,ntlevels
        tlevels(n)=rspec(iopts(2,11)+n-1)
        itlevels(n)=nint(tlevels(n)*10.)
        tlevels(n)=itlevels(n)*0.1
      end do
      if(ntlevels.gt.1) then
      	if(tlevels(1).lt.tlevels(2)) then
      	  nt= ntlevels/2
      	  do n=1,nt
      	    it=itlevels(n)
      	    rt= tlevels(n)
      	    itlevels(n)=itlevels(ntlevels+1-n)
      	     tlevels(n)= tlevels(ntlevels+1-n)
      	    itlevels(ntlevels+1-n)=it
      	     tlevels(ntlevels+1-n)=rt
      	  end do
      	end if
      end if
      pwmin = 0.0
      if(iopts(1,12).eq.1) then
         pwmin=rspec(iopts(2,12))
         ioutwm=1
      endif
c
      filein=cargs(1)
      fileot=cargs(2)
c
      write(6,*) 'input  file:'
      write(6,*)  filein
      write(6,*) 'output file:'
      write(6,*)  fileot
      write(6,*) 'producer,grid:        ',iprod,igrid
      write(6,*) 'vertical coordinate:  ',ivcoor,' (1=p,2=sigma,10=eta)'
      write(6,*) 'no. of input levels:  ',kk,' (0=all)'
      write(6,*) 'theta1,theta2,dtheta: ',ithmin,ithmax,ithstp
      write(6,*) 'hour1,hour2,hour_step:',ihmin,ihmax,ihstp
      write(6,*) 'output:'
      write(6,*) 'wind, pressure, pv, M:',ioutuv,ioutp,ioutpv,ioutm
      write(6,*) 'no. of pv levels:     ',npvlev
      if(npvlev.gt.0) then
        write(6,*) 'pv levels in unit 1/10 pvu:'
        write(6,*) (ipvlev(i),i=1,npvlev)
      end if
      if(ntlevels.gt.0) then
        write(6,*) 't levels in unit 1/10 degree Celsius:'
        write(6,*) (itlevels(i),i=1,ntlevels)
      end if
      if(nsurfp.gt.0) then
        write(6,*) 'transfer surface parameters:'
        write(6,*) (ispec(i),i=isurfp,isurfp+nsurfp-1)
      end if
      if(ioutwm.eq.0) then
	write(6,*) 'output of max.wind (u,v,p) fields : no'
      else
	write(6,*) 'output of max.wind (u,v,p) fields : yes'
      end if
c
      if(ihmin.eq.ihmax   .and. ihstp.le.0)  ihstp=1
      if(ithmin.eq.ithmax .and. ithstp.le.0) ithstp=1
c
      if(ihmin.gt.ihmax   .or. ihstp.le.0) stop 1
      if(ithmin.gt.ithmax .or. ithstp.le.0) stop 1
      if(kk.gt.0 .and. kk.lt.2) stop 1
      if(kk.gt.kkmax) stop 1
c
      maincomp=1
      if(ioutuv.eq.0 .and. ioutp.eq.0 .and.
     +   ioutpv.eq.0 .and. ioutm.eq.0 .and. npvlev.eq.0) maincomp=0
c
      if(maincomp.eq.0 .and. ioutwm.eq.0 .and. ntlevels.lt.1) then
        write(6,*) 'no output specified'
        stop 1
      end if
c
c..make some tables, exner function (pi) and pi/cp
c..the tables much faster than "**rcp" etc.
c..very long tables to avoid crash with bad data
      r  =287.
      cp =1004.
      rcp=r/cp
      cpr=cp/r
      p0 =1000.
      pitab(0)=0.
      pktab(0)=0.
      do lt=1,mpitab
        p=dptab*real(lt)
        pitab(lt)=cp*(p/p0)**rcp
        pktab(lt)=   (p/p0)**rcp
ccc     write(6,*) 'lt,p,pi,pk:',lt,p,pitab(lt),pktab(lt)
      end do
      pptab(0)=0.
      do lt=1,mpptab
        pi=dpitab*real(lt)
        pptab(lt)=p0*(pi/cp)**cpr
ccc     write(6,*) 'lt,pi,p:',lt,pi,pptab(lt)
      end do
c
c
c..input felt-file
      iunitr=20
c
c..output felt-file
      iunitw=30
c
c..open input felt file
      call mrfelt(1,filein,iunitr,inr,0,1,1.,1.,32,idfile,ierror)
      if(ierror.ne.0) stop 2
c
      mnd=idfile(6)/100
      idag=idfile(6)-mnd*100
      write(6,1020)idag,mnd,idfile(5),idfile(7)
 1020 format(' input  date/time is ',2i4,2i6,' utc')
c
      do i=1,7
        idato(i)=idfile(i)
      end do
c
      if(kk.eq.0) then
	call findlevels(iunitr,iprod,igrid,ihmin,ihmax,ihstp,
     +			ivcoor,kkmax,levels,kk,ipartemp,ierror)
	if(ierror.ne.0 .or. kk.lt.2) stop 2
      else
	do k=1,kk
	  levels(k)=k
	end do
	ipartemp=18
	if(iprod.ne.88) ipartemp=4
      end if
c
c..open (existing) output felt file
      call mwfelt(1,fileot,iunitw,0,1,1.,1.,32,idfile,ierror)
      if(ierror.ne.0) stop 2
c
      mnd=idfile(6)/100
      idag=idfile(6)-mnd*100
      write(6,1021)idag,mnd,idfile(5),idfile(7)
 1021 format(' output date/time is ',2i4,2i6,' utc')
c
      n=0
      do i=5,7
        if(idato(i).ne.idfile(i)) n=1
      end do
      if(n.eq.1) then
        write(6,*) ' ---- date/time error ----'
        stop 2
      end if
c
c
c..time loop
c
      nerror=0
      izsurf=0
      izread=0
      izcomp=0
      if(ioutm.gt.0 .or. ntlevels.gt.0) then
        if(ivcoor.eq.1) then
          izread=1
        else
          izsurf=1
          izcomp=1
        end if
      end if
      izzero=0
      if(izread.eq.0 .and. izcomp.eq.0) izzero=1
c
      do 100 ihour=ihmin,ihmax,ihstp
c
        idatyp=3
        if(ihour.gt.0) idatyp=2
        if(iprod.ne.88) idatyp=-32767
c
        inr( 1)=iprod
        inr( 2)=igrid
        inr( 3)=-32767
        inr( 4)=-32767
        inr( 5)=-32767
        inr( 9)=idatyp
        inr(10)=ihour
c
        write(6,*) 'prod,grid,prog:',inr(1),inr(2),inr(10)
c
c..read data (and possibly transfer surface data to output file)
        call rdata(filein,iunitr,ivcoor,ipartemp,izread,izsurf,
     +             inr,kk,levels,ldata,idata,max2d,max3d,
     +             us,vs,ths,zs,psurf,zsurf,xmd2h,ymd2h,f,
     +             nx,ny,alevel,blevel,igtype,grid,
     +		   nsurfp,ispec(isurfp),fileot,iunitw,ierror)
        if(ierror.ne.0) then
          nerror=nerror+1
          if(ierror.lt.0) goto 900
          goto 100
        end if
c
        izsurf=0
c
	if(izcomp.eq.1) then
c
	  call zcompute(ivcoor,nx,ny,kk,alevel,blevel,ahalf,bhalf,
     +			ths,zs,psurf,zsurf,pv3)
c
	else if (izzero.eq.1) then
c
	  n3d=nx*ny*kk
	  do i=1,n3d
	    zs(i)=0.
	  end do
	  izzero=0
c
	end if
c
	if(maincomp.ne.0) then
c
c..interpolate/compute and write data
          call ipvcomp(fileot,iunitw,
     +                 ithmin,ithmax,ithstp,ioutuv,ioutp,ioutpv,ioutm,
     +                 npvlev,mpvlev,rpvlev,ldata,idata,
     +                 nx,ny,kk,alevel,blevel,
     +                 us,vs,ths,zs,psurf,zsurf,xmd2h,ymd2h,f,
     +                 uth,vth,pth,mth,pv,pth2,pth3,knum,
     +                 pv3,ppvlev,pvnext,nextpv)
c
	end if
c
	if(ioutwm.ne.0) then
c
	  call windmax(fileot,iunitw,
     +                 nx,ny,kk,alevel,blevel,
     +                 us,vs,psurf, uth,vth,pth,
     +		       mth,pwmin,ldata,idata)
c
	end if
c
	if(ntlevels.gt.0) then
c
	  call templevels(fileot,iunitw,ntlevels,tlevels,
     +                    nx,ny,kk,alevel,blevel,
     +                    ths,zs,psurf, uth,vth, knum, ldata,idata)
c
	end if
c
  100 continue
c
  900 if(nerror.eq.0) istop=0
c
c..update file header and close output file
      call mwfelt(3,fileot,iunitw,0,1,1.,1.,1,idfile,ierror)
c
c..close input file
      call mrfelt(3,filein,iunitr,inr,0,1,1.,1.,1,idfile,ierror)
c
      if(istop.ne.0) then
        write(6,*) '******** TETAPV ERROR EXIT ********'
        stop 3
      end if
c
      end
c
c***********************************************************************
c
      subroutine findlevels(iunit,iprod,igrid,ihmin,ihmax,ihstp,
     +			    ivcoor,kkmax,levels,kk,ipartemp,ierror)
c
c	find existing levels with necessary parameters,
c	u,v and th or t
c
      implicit none
c
      integer iunit,iprod,igrid,ihmin,ihmax,ihstp
      integer ivcoor,kkmax,kk,ipartemp,ierror
      integer levels(kkmax)
c
      integer maxinh
c
      parameter (maxinh=1024)
c
      integer   ifound(maxinh)
      integer*2 inh(16,maxinh)
      integer   ireq,iexist,nin,nfound,iend,ioerr,idatyp,ihour,
     +          i,k,n,iu,iv,it,ith,lastlev,newlev
c
c..temperature parameter no., 18=th  4=t  0=unknown
      ipartemp=0
c
c..possibly later: check timstep 'ihmin' and 'ihmin+ihstp'
c..here only the first
      ihour=ihmin
c
      idatyp=3
      if(ihour.gt.0) idatyp=2
      if(iprod.ne.88) idatyp=-32767
c
      do i=1,16
	inh(i,1)=-32767
      end do
      inh( 1,1)=iprod
      inh( 2,1)=igrid
      inh( 9,1)=idatyp
      inh(10,1)=ihour
      inh(11,1)=ivcoor
      ireq=1
      iexist=1
      nin=maxinh
c
      call qfelt(iunit,ireq,iexist,nin,inh,ifound,nfound,
     +           iend,ierror,ioerr)
c
      if(ierror.ne.0 .or. nfound.eq.0) then
	write(6,*) 'Error when searching for levels in input file'
	return
      end if
c
      kk=0
      lastlev=0
c
      do n=1,nfound
	if(inh(13,n).gt.lastlev) then
	  newlev=inh(13,n)
	  do i=n+1,nfound
	    if(inh(13,i).gt.lastlev .and.
     +	       inh(13,i).lt.newlev) newlev=inh(13,i)
	  end do
	  iu=0
	  iv=0
	  it=0
	  ith=0
	  do i=n,nfound
	    if(inh(13,i).eq.newlev) then
	      if(inh(12,i).eq. 2) iu=1
	      if(inh(12,i).eq. 3) iv=1
	      if(inh(12,i).eq. 4) it=1
	      if(inh(12,i).eq.18) ith=1
	    end if
	  end do
	  if(ipartemp.eq.0 .and. ith.eq.1) ipartemp=18
	  if(ipartemp.eq.0 .and. it .eq.1) ipartemp= 4
	  if(ipartemp.eq.18) it =0
	  if(ipartemp.eq. 4) ith=0
	  if(iu+iv.eq.2 .and. it+ith.gt.0 .and. kk.lt.kkmax) then
	    kk=kk+1
	    levels(kk)=newlev
	  end if
	  lastlev=newlev
	end if
      end do
c
      write(6,*) 'levels found: ',kk
      if(kk.gt.0) write(6,fmt='(1x,11i6)') (levels(k),k=1,kk)
c
c..reset qfelt
      call qfelt(0,0,iexist,nin,inh,ifound,nfound,
     +           iend,ierror,ioerr)
c
      return
      end
c
c***********************************************************************
c
      subroutine rdata(filein,iunitr,ivcoor,ipartemp,izread,izsurf,
     +                 inr,kk,levels,ldata,idata,max2d,max3d,
     +                 us,vs,ths,zs,psurf,zsurf,xmd2h,ymd2h,f,
     +                 nx,ny,alevel,blevel,igtype,grid,
     +		       nsurfpar,isurfpar,fileot,iunitw,ierror)
c
c..read data in model levels and surface pressure for one timestep
c
      implicit none
c
      include 'tetapvtab.inc'
c
      integer   iunitr,ivcoor,ipartemp,kk,ldata,max2d,max3d,izsurf
      integer   nx,ny,igtype,nsurfpar,iunitw,ierror,izread
      integer   levels(kk),isurfpar(nsurfpar)
      integer*2 inr(16),idata(ldata)
      real      us(max3d),vs(max3d),ths(max3d),zs(max3d)
      real      psurf(max2d),zsurf(max2d)
      real      xmd2h(max2d),ymd2h(max2d),f(max2d)
      real      alevel(kk),blevel(kk),grid(6)
      character*(*) filein
      character*(*) fileot
c
      integer*2 inz(16)
c
      integer   ipackr,ipackw,i,k,n,np,lfield,i2d,i3d,nxx,nyy,newgrid
      real      ptop,pbottom,p,const,hx,hy,pk
c
      integer   lasttype,lastnx,lastny
      real      gridlast(6)
c
      data lasttype,lastnx,lastny/3*0/
      data gridlast/6*0./
c
      ierror=1
      nx=0
      ny=0
c
      if(nsurfpar.gt.0) then
c..transfer surface fields from input to output file
        ipackr=0
        ipackw=0
	do i=1,16
	  inz(i)=inr(i)
	end do
	do np=1,nsurfpar
          inz(11)=2
          inz(12)=isurfpar(np)
          inz(13)=1000
          inz(14)=0
          call mrfelt(2,filein,iunitr,inz,ipackr,1,0.,1.,
     +                  ldata,idata,ierror)
          if(ierror.ne.0) goto 980
          call mwfelt(2,fileot,iunitw,ipackw,1,0.,1.,
     +                  ldata,idata,ierror)
          if(ierror.ne.0) goto 980
	end do
      end if
c
c..no undefined values in input data
      ipackr=1
c
      inr(11)=ivcoor
c
      if(izsurf.eq.1) then
c..zsurf (topography)
	do i=1,16
	  inz(i)=inr(i)
	end do
        inz( 9)=4
        inz(10)=0
        inz(11)=2
        inz(12)=101
        inz(13)=1000
        inz(14)=0
        lfield=max2d
        call mrfelt(2,filein,iunitr,inz,ipackr,lfield,zsurf(1),1.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) goto 980
        nx=idata(10)
        ny=idata(11)
        if(nx*ny.gt.max2d .or. nx*ny*kk.gt.max3d) goto 940
      end if
c
      do k=1,kk
c
        i3d=nx*ny*(k-1)+1
        lfield=max3d-i3d+1
c
        inr(13)=levels(k)
        inr(14)=-32767
c
	if(izread.gt.0) then
c..z
          inr(12)=1
          call mrfelt(2,filein,iunitr,inr,ipackr,lfield,zs(i3d),1.,
     +                ldata,idata,ierror)
          if(ierror.ne.0) goto 980
          if(nx.eq.0 .and. ny.eq.0) then
            nx=idata(10)
            ny=idata(11)
            if(nx*ny.gt.max2d .or. nx*ny*kk.gt.max3d) goto 940
          end if
          if(idata(10).ne.nx .or. idata(11).ne.ny) goto 950
        end if
c
c..u
        inr(12)=2
        call mrfelt(2,filein,iunitr,inr,ipackr,lfield,us(i3d),1.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) goto 980
        if(nx.eq.0 .and. ny.eq.0) then
          nx=idata(10)
          ny=idata(11)
          if(nx*ny.gt.max2d .or. nx*ny*kk.gt.max3d) goto 940
        end if
        if(idata(10).ne.nx .or. idata(11).ne.ny) goto 950
c
c..v
        inr(12)=3
        call mrfelt(2,filein,iunitr,inr,ipackr,lfield,vs(i3d),1.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) goto 980
        if(idata(10).ne.nx .or. idata(11).ne.ny) goto 950
c
c..th(18) or t(4)
        inr(12)=ipartemp
        call mrfelt(2,filein,iunitr,inr,ipackr,lfield,ths(i3d),1.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) goto 980
        if(idata(10).ne.nx .or. idata(11).ne.ny) goto 950
c
        alevel(k)=idata( 8)*0.1
        blevel(k)=idata(19)*0.0001
c
      end do
c
c..psurf
      if(ivcoor.eq.2 .or. ivcoor.eq.10) then
c
        inr(11)=2
        inr(12)=8
        inr(13)=1000
        inr(14)=0
        lfield=max2d
        call mrfelt(2,filein,iunitr,inr,ipackr,lfield,psurf(1),1.,
     +                ldata,idata,ierror)
        if(ierror.ne.0) goto 980
        if(idata(10).ne.nx .or. idata(11).ne.ny) goto 950
c
        if(ivcoor.eq.2) then
          ptop=idata(19)
          do k=1,kk
            alevel(k)=ptop*(1.-blevel(k))
          end do
        end if
c
      elseif(ivcoor.eq.1) then
c
c..following will make computations equal to those in other level types
	pbottom=levels(kk)
	do i=1,nx*ny
	  psurf(i)=pbottom
	end do
        do k=1,kk
          alevel(k)=levels(k)
          blevel(k)=0.
        end do
c
      end if
c
      if(ipartemp.eq.4) then
c..convert input abs.temperature to potential temperature
	if(ivcoor.eq.1) then
c..pressure levels
	  do k=1,kk
	    i3d=nx*ny*(k-1)
	    p=alevel(k)
	    const=1./((p*0.001)**0.28585)
	    do i=1,nx*ny
	      ths(i3d+i)=ths(i3d+i)*const
	    end do
	  end do
	else
c..other level types
	  do k=1,kk
	    i3d=nx*ny*(k-1)
	    do i=1,nx*ny
	      p=alevel(k)+blevel(k)*psurf(i)
ccc	      ths(i3d+i)=ths(i3d+i)/((p*0.001)**0.28585)
              xt=p*dpinv
              lt=xt
              pk=pktab(lt)+(pktab(lt+1)-pktab(lt))*(xt-lt)
	      ths(i3d+i)=ths(i3d+i)/pk
	    end do
	  end do
	end if
      end if
c
      call gridpar(+1,ldata,idata,igtype,nxx,nyy,grid,ierror)
      if(ierror.ne.0) then
        write(6,*) 'GRIDPAR ERROR: ',ierror
        write(6,*) '    grid type: ',igtype
        igtype=0
        do i=1,6
          grid(i)=0.
        end do
      end if
c
      newgrid=0
      if(lasttype.ne.igtype) newgrid=1
      if(lastnx.ne.nx) newgrid=1
      if(lastny.ne.ny) newgrid=1
      do i=1,6
        if(gridlast(i).ne.grid(i)) newgrid=1
      end do
c
      if(newgrid.eq.1) then
c
        lasttype=igtype
        lastnx=nx
        lastny=ny
        do i=1,6
          gridlast(i)=grid(i)
        end do
c
c..compute map ratio (mapratio/2h) and coriolis parameter
        call mapfield(3,1,igtype,grid,nx,ny,xmd2h,ymd2h,f,hx,hy,ierror)
        if(ierror.ne.0) then
          write(6,*) 'MAPFIELD ERROR: ',ierror
          write(6,*) '     grid type: ',igtype
          stop 255
        end if
c
      end if
c
      ierror=0
      goto 990
c
  940 i2d=nx*ny
      i3d=nx*ny*kk
      write(6,*) 'ERROR. too much data.'
      write(6,*) '     last producer,grid: ',idata(1),idata(2)
      write(6,*) '     last     field dimensions: ',nx,ny
      write(6,*) '     no. of levels to read:     ',kk
      write(6,*) '     current    2d-field size:  ',i2d
      write(6,*) '     current    3d-field size:  ',i3d
      write(6,*) '     max allowd 2d-field size:  ',max2d
      write(6,*) '     max allowd 3d-field size:  ',max3d
      ierror=-1
      goto 990
c
  950 nxx=idata(10)
      nyy=idata(11)
      write(6,*) 'ERROR. field dimensions has changed.'
      write(6,*) '     last producer,grid: ',idata(1),idata(2)
      write(6,*) '     last     field dimensions: ',nxx,nyy
      write(6,*) '     previous field dimensions: ',nx,ny
      ierror=-1
      goto 990
c
  980 ierror=1
c
  990 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine zcompute(ivcoor,nx,ny,kk,alevel,blevel,ahalf,bhalf,
     +			  ths,zs,psurf,zsurf,ztmp)
c
c  compute height in sigma or eta levels
c
      implicit none
c
      include 'tetapvtab.inc'
c
      integer ivcoor,nx,ny,kk
      real    alevel(kk),blevel(kk),ahalf(kk+1),bhalf(kk+1)
      real    ths(nx,ny,kk),zs(nx,ny,kk)
      real    psurf(nx,ny),zsurf(nx,ny),ztmp(nx,ny)
c
      integer i,j,k
      real    r,cp,g,p0,rcp,ginv,p0inv
      real    po,p,pu,pio,pi,piu
c
      if(ivcoor.ne.2 .and. ivcoor.ne.10) stop 117
c
      r=287.04
      cp=1004.6
      g=9.80665
      p0=1000.
      rcp=r/cp
      ginv=1./g
      p0inv=1./p0
c
c..sigma_1/eta_half (where height can be computed)
      ahalf(kk+1)=0.
      bhalf(kk+1)=1.
      do k=kk,1,-1
        ahalf(k)=alevel(k)+(alevel(k)-ahalf(k+1))
        bhalf(k)=blevel(k)+(blevel(k)-bhalf(k+1))
      end do
      ahalf(1)=max(ahalf(1),0.)
      bhalf(1)=max(bhalf(1),0.)
c
c..temporary store integrated height in mth
      do j=1,ny
        do i=1,nx
          ztmp(i,j)=zsurf(i,j)
        end do
      end do
c
      do k=kk,1,-1
        do j=1,ny
          do i=1,nx
            po=ahalf(k)+bhalf(k)*psurf(i,j)
            p =alevel(k)+blevel(k)*psurf(i,j)
            pu=ahalf(k+1)+bhalf(k+1)*psurf(i,j)
ctab        pio=cp*(po*p0inv)**rcp
ctab        pi =cp*(p *p0inv)**rcp
ctab        piu=cp*(pu*p0inv)**rcp
            xt=po*dpinv
            lt=xt
            pio=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
            xt=p*dpinv
            lt=xt
            pi=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
            xt=pu*dpinv
            lt=xt
            piu=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
            zs(i,j,k)=ztmp(i,j)+ths(i,j,k)*(piu-pi )*ginv
            ztmp(i,j)=ztmp(i,j)+ths(i,j,k)*(piu-pio)*ginv
          end do
        end do
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine ipvcomp(fileot,iunitw,
     +                   ithmin,ithmax,ithstp,ioutuv,ioutp,ioutpv,ioutm,
     +                   npvlev,mpvlev,rpvlev,ldata,idata,
     +                   nx,ny,kk,alevel,blevel,
     +                   us,vs,ths,zs,psurf,zsurf,xmd2h,ymd2h,f,
     +                   uth,vth,pth,mth,pv,pth2,pth3,knum,
     +                   pv3,ppvlev,pvnext,nextpv)
c
c  interpolate wind from sigma/eta to isentropic surfaces,
c  compute potential vorticity (pv) and Montgomery potential (M=cp*T+g*z)
c  and find pressure at constant pv (potential vorticity unit) levels
c  (no fields are smoothed).
c  output to felt file here.
c
c  when finding pressure at pv levels, the 'theta' loop will always
c  cover all possible levels (with a fixed increment), but without
c  output below and above the specified output levels.
c  (when above the specified output levels, a test is done to stop
c   the loop when no more pv levels can be found).
c
c  the 'theta' loop is also extended to compute pv in the upper and
c  lower output levels.
c
c  computation of height and Montgomery potential could be more
c  accurate (by interpolation/integration between sigma_2/eta_half
c  levels), but this would take more cpu time too.
c
      implicit none
c
      include 'tetapvtab.inc'
c
      integer   iunitw,ithmin,ithmax,ithstp,ioutuv,ioutp,ioutpv,ioutm
      integer   npvlev,mpvlev,ldata,nx,ny,kk
      integer   knum(nx,ny)
      integer   nextpv(nx,ny)
      integer*2 idata(ldata)
      real      alevel(kk),blevel(kk)
      real      rpvlev(mpvlev)
      real      us(nx,ny,kk),vs(nx,ny,kk),ths(nx,ny,kk),zs(nx,ny,kk)
      real      psurf(nx,ny),zsurf(nx,ny)
      real      xmd2h(nx,ny),ymd2h(nx,ny),f(nx,ny)
      real      uth(nx,ny),vth(nx,ny),pth(nx,ny),mth(nx,ny),pv(nx,ny)
      real      pth2(nx,ny),pth3(nx,ny)
      real      pv3(nx,ny),ppvlev(nx,ny,mpvlev),pvnext(nx,ny)
      character*(*) fileot
c
      integer i,j,k,n,lfield,ipackw,ith,ipv,ith1,ith2,ithout,k1,k2
      integer nthlev,nundef,nudef2,nudef3,next,iend,ierror
      real    undef,udef,r,cp,g,p0,rcp,cpr,ginv,p0inv,cpinv
      real    th,thmin,thmax,c1,c2,p1,p2,pi,pi1,pi2,z,dth,pvscal,thtest
c
      undef=+1.e+35
      udef=undef*0.9
c
      r=287.04
      cp=1004.6
      g=9.80665
      p0=1000.
      rcp=r/cp
      cpr=cp/r
      ginv=1./g
      p0inv=1./p0
      cpinv=1./cp
c
      do j=1,ny
        do i=1,nx
          knum(i,j)=kk-1
        end do
      end do
c
c..field identfication changes from input to output
c..(-32767 : set this element for each field)
      idata( 5)=4
      idata( 6)=-32767
      idata( 7)=-32767
      idata( 8)=0
      idata(19)=0
      idata(20)=-32767
c
c..undefined values in output fields
      ipackw=2
c
      lfield=nx*ny
c
      ipv=1
      if(ioutpv.eq.0 .and. npvlev.eq.0) ipv=0
c
      if(npvlev.gt.0) then
        do j=1,ny
          do i=1,nx
            pv3(i,j)=undef
            pvnext(i,j)=rpvlev(1)
            nextpv(i,j)=1
          end do
        end do
        do n=1,npvlev
          do j=1,ny
            do i=1,nx
              ppvlev(i,j,n)=undef
            end do
          end do
        end do
        thmin=+undef
        thmax=-undef
        do j=1,ny
          do i=1,nx
            thmin=min(thmin,ths(i,j,kk))
            thmax=max(thmax,ths(i,j,1))
          end do
        end do
        ith1=nint(thmin*10.)
        ith2=nint(thmax*10.)
        ith1=((ith1+ithstp-1)/ithstp)*ithstp
        ith2=(ith2/ithstp)*ithstp
        if(ioutpv.ne.0) then
          ith1=min(ith1,ithmin-ithstp)
          ith2=max(ith2,ithmax+ithstp)
        else
          ith1=min(ith1,ithmin)
          ith2=max(ith2,ithmax)
        end if
      elseif(ioutpv.ne.0) then
        ith1=ithmin-ithstp
        ith2=ithmax+ithstp
      else
        ith1=ithmin
        ith2=ithmax
      end if
c
c..interpolerer ett og ett theta-nivaa
c
      nthlev=0
      ith=ith1-ithstp
c
      do while (ith.lt.ith2)
c
      ith=ith+ithstp
      th=ith*0.1
      nthlev=nthlev+1
      ithout=1
      if(ith.lt.ithmin .or. ith.gt.ithmax) ithout=0
      nundef=0
c
      do j=1,ny
        do i=1,nx
          if(th.gt.ths(i,j,1) .or. th.lt.ths(i,j,kk)) then
            uth(i,j)=undef
            vth(i,j)=undef
            pth(i,j)=undef
            mth(i,j)=undef
            nundef=nundef+1
          else
            k=knum(i,j)
            do while (th.ge.ths(i,j,k) .and. k.gt.1)
              k=k-1
            end do
            knum(i,j)=k
            k1=k+1
            k2=k
            c1=(ths(i,j,k2)-th)/(ths(i,j,k2)-ths(i,j,k1))
            c2=(th-ths(i,j,k1))/(ths(i,j,k2)-ths(i,j,k1))
c..lineaer interpolasjon i pot.temp.
            uth(i,j)=c1*us(i,j,k1)+c2*us(i,j,k2)
            vth(i,j)=c1*vs(i,j,k1)+c2*vs(i,j,k2)
            p1=alevel(k1)+blevel(k1)*psurf(i,j)
            p2=alevel(k2)+blevel(k2)*psurf(i,j)
ctab        pi1=cp*(p1*p0inv)**rcp
ctab        pi2=cp*(p2*p0inv)**rcp
            xt=p1*dpinv
            lt=xt
            pi1=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
            xt=p2*dpinv
            lt=xt
            pi2=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
            pi=c1*pi1+c2*pi2
ctab        pth(i,j)=p0*(pi*cpinv)**cpr
            xt=pi*dpiinv
            lt=xt
            pth(i,j)=pptab(lt)+(pptab(lt+1)-pptab(lt))*(xt-lt)
            z=c1*zs(i,j,k1)+c2*zs(i,j,k2)
            mth(i,j)=th*pi+g*z
          end if
        end do
      end do
c
      if(ioutuv.ne.0 .and. ithout.eq.1) then
c..u(th)
        idata( 6)=2
        idata( 7)=ith
        idata(20)=-2
        call mwfelt(2,fileot,iunitw,ipackw,lfield,uth(1,1),1.,
     +                ldata,idata,ierror)
c..v(th)
        idata( 6)=3
        idata( 7)=ith
        idata(20)=-2
        call mwfelt(2,fileot,iunitw,ipackw,lfield,vth(1,1),1.,
     +                ldata,idata,ierror)
      end if
c
      if(ioutp.ne.0 .and. ithout.eq.1) then
c..p(th)
        idata( 6)=8
        idata( 7)=ith
        idata(20)=-1
        call mwfelt(2,fileot,iunitw,ipackw,lfield,pth(1,1),1.,
     +                ldata,idata,ierror)
      end if
c
      if(ioutm.ne.0 .and. ithout.eq.1) then
c..m(th) ... Montgomery potential
        idata( 6)=90
        idata( 7)=ith
        idata(20)=-32767
        call mwfelt(2,fileot,iunitw,ipackw,lfield,mth(1,1),1.,
     +                ldata,idata,ierror)
      end if
c
c..potensiell virvling..................
c
      if(nthlev.ge.3 .and. ipv.eq.1) then
c
        dth=2*ithstp*0.1
        g=9.8
c
c..1 pvu = 1.e-6 m**2 s**-1 k kg**-1 ....... 1 mb = 100 pa
c
        pvscal=g*dth*1.e+6/100.
c
c..absolutt virvling er naa beregnet og er lagret i pv-arrayen
c
        if(nundef+nudef2+nudef3.eq.0) then
          do j=1,ny
            do i=1,nx
              pv(i,j)=pvscal*pv(i,j)/(pth3(i,j)-pth(i,j))
            end do
          end do
        else
          do j=1,ny
            do i=1,nx
              if(pth(i,j).lt.udef .and. pth3(i,j).lt.udef
     *                            .and. pv(i,j).lt.udef) then
                pv(i,j)=pvscal*pv(i,j)/(pth3(i,j)-pth(i,j))
              else
                pv(i,j)=undef
              end if
            end do
          end do
        end if
c
        if(npvlev.gt.0) then
c..note:
c..pv() is at level pth2()  and  pv3() at level pth3().
c..if pv() is defined then pth2() and pth3() are defined.
c..using both pvnext() and nextpv() to make a rather simple
c..first if-test
          do j=1,ny
            do i=1,nx
              if(pv(i,j).lt.udef .and. pv(i,j).gt.pvnext(i,j)) then
                next=nextpv(i,j)
                if(pv3(i,j).gt.udef) pv3(i,j)=0.
                do n=nextpv(i,j),npvlev
                  if(pv(i,j).gt.rpvlev(n)) then
                    ppvlev(i,j,n)=pth3(i,j)+(pth2(i,j)-pth3(i,j))
     +                                     *(rpvlev(n)-pv3(i,j))
     +                                     /(pv(i,j)-pv3(i,j))
                    next=n+1
                  end if
                end do
                nextpv(i,j)=next
                if(next.le.npvlev) then
                  pvnext(i,j)=rpvlev(next)
                else
                  pvnext(i,j)=undef
                end if
              end if
            end do
          end do
          do j=1,ny
            do i=1,nx
              pv3(i,j)=pv(i,j)
            end do
          end do
          if(ith.gt.ithmax) then
c..check if necessary to continue loop
            thtest=(ith+ithstp)*0.1
            iend=1
            do j=1,ny
              do i=1,nx
                if(nextpv(i,j).le.npvlev .and.
     +             ths(i,j,1).gt.thtest) iend=0
              end do
            end do
            if(iend.eq.1) ith2=ith
          end if
        end if
c
        if(ioutpv.ne.0 .and. ith-ithstp.ge.ithmin .and.
     +                       ith-ithstp.le.ithmax) then
c..pv(th)
          idata( 6)=80
          idata( 7)=ith-ithstp
          idata(20)=-2
          call mwfelt(2,fileot,iunitw,ipackw,lfield,pv(1,1),1.,
     +                  ldata,idata,ierror)
        end if
c
      end if
c
      if(nthlev.eq.1 .and. ipv.eq.1) then
        do j=1,ny
          do i=1,nx
            pth2(i,j)=pth(i,j)
          end do
        end do
        nudef2=nundef
      elseif(ith.lt.ith2 .and. ipv.eq.1) then
        do j=1,ny
          do i=1,nx
            pth3(i,j)=pth2(i,j)
            pth2(i,j)=pth(i,j)
          end do
        end do
        nudef3=nudef2
        nudef2=nundef
c..absolutt virvling  (lagres i pv)
        if(nundef.eq.0) then
          do j=2,ny-1
            do i=2,nx-1
              pv(i,j)=f(i,j)+xmd2h(i,j)*(vth(i+1,j)-vth(i-1,j))
     *                      -ymd2h(i,j)*(uth(i,j+1)-uth(i,j-1))
            end do
          end do
        else
          do j=2,ny-1
            do i=2,nx-1
              if(vth(i+1,j).lt.udef .and. vth(i-1,j).lt.udef .and.
     *           uth(i,j+1).lt.udef .and. uth(i,j-1).lt.udef) then
                pv(i,j)=f(i,j)+xmd2h(i,j)*(vth(i+1,j)-vth(i-1,j))
     *                        -ymd2h(i,j)*(uth(i,j+1)-uth(i,j-1))
              else
                pv(i,j)=undef
              end if
            end do
          end do
        end if
c..sett inn verdier langs ytterkanter av gridet (ikke beregnet)
        do i=2,nx-1
          pv(i, 1)=pv(i,2)
          pv(i,ny)=pv(i,ny-1)
        end do
        do j=1,ny
          pv( 1,j)=pv(2,j)
          pv(nx,j)=pv(nx-1,j)
        end do
      end if
c
c.....end do while (ith.lt.ith2)
      end do
c
      if(npvlev.gt.0) then
c..p(pv=const) output ... vertical coordinate 9, levls in unit 1/10 pvu
        idata( 5)=9
        idata( 8)=0
        idata(19)=0
        do n=1,npvlev
c..p(pv=const)  (pressure is parameter 8 in any vertical coordinate)
          idata( 6)=8
          idata( 7)=nint(rpvlev(n)*10.)
          idata(20)=-1
          call mwfelt(2,fileot,iunitw,ipackw,lfield,ppvlev(1,1,n),1.,
     +                  ldata,idata,ierror)
        end do
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine windmax(fileot,iunitw,
     +                   nx,ny,kk,alevel,blevel,
     +                   us,vs,psurf, umax,vmax,pmax,
     +			 ffmax,pwmin,ldata,idata)
c
c  max wind (in the vertical)
c  write wind components (u,v) and pressure fields
c
      implicit none
c
      integer   iunitw,nx,ny,kk,ldata
      integer*2 idata(ldata)
      real      alevel(kk),blevel(kk)
      real      us(nx,ny,kk),vs(nx,ny,kk),psurf(nx,ny)
      real      umax(nx,ny),vmax(nx,ny),pmax(nx,ny)
      real      ffmax(nx,ny)
      real      pwmin
      character*(*) fileot
c
      integer i,j,k,ipackw,lfield,ierror
      real    undef,udef,ff,pp
c#################################################################
c     real    ppmin,ppmax,fffmax
c#################################################################
c
      undef=+1.e+35
      udef=undef*0.9
c
      k=kk
      do j=1,ny
        do i=1,nx
          umax(i,j)=us(i,j,k)
          vmax(i,j)=vs(i,j,k)
          pmax(i,j)=alevel(k)+blevel(k)*psurf(i,j)
	  ff=us(i,j,k)*us(i,j,k)+vs(i,j,k)*vs(i,j,k)
	  ffmax(i,j)=ff
        end do
      end do
c
      do k=kk-1,1,-1
        do j=1,ny
          do i=1,nx
	    ff=us(i,j,k)*us(i,j,k)+vs(i,j,k)*vs(i,j,k)
            pp=alevel(k)+blevel(k)*psurf(i,j)
	    if(ff.gt.ffmax(i,j).and.pp.ge.pwmin) then
	      ffmax(i,j)=ff
              umax(i,j)=us(i,j,k)
              vmax(i,j)=vs(i,j,k)
              pmax(i,j)=pp
	    end if
          end do
        end do
      end do
c#################################################################
c     ppmin=+1.e+35
c     ppmax=-1.e+35
c     fffmax=-1.e+35
c     do j=1,ny
c       do i=1,nx
c	  if(ppmin.gt.pmax(i,j)) ppmin=pmax(i,j)
c	  if(ppmax.lt.pmax(i,j)) ppmax=pmax(i,j)
c	  if(fffmax.lt.ffmax(i,j)) fffmax=ffmax(i,j)
c       end do
c     end do
c     write(6,*) 'FFmax,Pn,Px: ',sqrt(fffmax),ppmin,ppmax
c#################################################################
c
c..field identfication changes from input to output
c..(-32767 : set this element for each field)
      idata( 5)=2
      idata( 6)=-32767
      idata( 7)=1000
      idata( 8)=0
      idata(19)=0
      idata(20)=-32767
c
c..undefined values in output fields
      ipackw=2
c
      lfield=nx*ny
c
c..u.max output
      idata( 6)=152
      idata(20)=-32767
      call mwfelt(2,fileot,iunitw,ipackw,lfield,umax(1,1),1.,
     +            ldata,idata,ierror)
c
c..v.max output
      idata( 6)=153
      idata(20)=-32767
      call mwfelt(2,fileot,iunitw,ipackw,lfield,vmax(1,1),1.,
     +            ldata,idata,ierror)
c
c..p.max output
      idata( 6)=154
      idata(20)=-32767
      call mwfelt(2,fileot,iunitw,ipackw,lfield,pmax(1,1),1.,
     +            ldata,idata,ierror)
c
      return
      end
c
c***********************************************************************
c
      subroutine templevels(fileot,iunitw,ntlevels,tlevels,
     +                      nx,ny,kk,alevel,blevel,
     +                      ths,zs,psurf, plev,zlev, knum, ldata,idata)
c
c  find pressure and height for specified temperatures (in unit Celsius)
c
c  WARNING: ths() recomputed to degrees Celsius
c
      implicit none
c
      include 'tetapvtab.inc'
c
      integer iunitw,ntlevels,nx,ny,kk,ldata
      real    tlevels(ntlevels),alevel(kk),blevel(kk)
      real    ths(nx,ny,kk),zs(nx,ny,kk),psurf(nx,ny)
      real    plev(nx,ny),zlev(nx,ny)
      integer knum(nx,ny)
      integer*2 idata(ldata)
      character*(*) fileot
c
      integer i,j,k,n,lfield,ipackw,k1,k2,nt
      integer ierror
      real    undef,t0,r,cp,g,p0,rcp,cpr,ginv,p0inv,cpinv
      real    xpi,p,t,c1,c2,p1,p2,pi,pi1,pi2,z
c#################################################################
c     real    pmin,pmax,zmin,zmax
c     integer ii,jj
c#################################################################
c
      undef=+1.e+35
c
      t0=273.15
      r=287.04
      cp=1004.6
      g=9.80665
      p0=1000.
      rcp=r/cp
      cpr=cp/r
      ginv=1./g
      p0inv=1./p0
      cpinv=1./cp
c
c..pot.temp -> temp.celius
      do k=1,kk
        do j=1,ny
          do i=1,nx
	    p=alevel(k)+blevel(k)*psurf(i,j)
ctab        xpi=(p*p0inv)**rcp
            xt=p*dpinv
            lt=xt
            xpi=pktab(lt)+(pktab(lt+1)-pktab(lt))*(xt-lt)
            ths(i,j,k)=ths(i,j,k)*xpi - t0
	  end do
        end do
      end do
c#################################################################
c	ii=nx/2
c	jj=ny/2
c	do k=1,kk
c	  write(6,*) '>>>>> k,p,z,t: ',
c    +			k,alevel(k)+blevel(k)*psurf(ii,jj),
c    +			zs(ii,jj,k),ths(ii,jj,k)
c	end do
c#################################################################
c
      do j=1,ny
        do i=1,nx
          knum(i,j)=kk-1
        end do
      end do
c
c..field identfication changes from input to output
c..(-32767 : set this element for each field)
c..Temperature levels is vertical coordinate 15,
c..with level n unit 1/10 degree Celsius
      idata( 5)=15
      idata( 6)=-32767
      idata( 7)=-32767
      idata( 8)=0
      idata(19)=0
      idata(20)=-32767
c
c..undefined values in output fields
      ipackw=2
c
      lfield=nx*ny
c
c
      do nt=1,ntlevels
c
        t=tlevels(nt)
c#################################################################
c	pmin=+1.e+35
c	pmax=-1.e+35
c	zmin=+1.e+35
c	zmax=-1.e+35
c#################################################################
c
        do j=1,ny
          do i=1,nx
            if(t.gt.ths(i,j,kk)) then
              plev(i,j)=undef
              zlev(i,j)=undef
            else
              k=knum(i,j)
              do while (t.lt.ths(i,j,k) .and. k.gt.1)
                k=k-1
              end do
              knum(i,j)=k
              k1=k+1
              k2=k
              c1=(ths(i,j,k2)-t)/(ths(i,j,k2)-ths(i,j,k1))
              c2=(t-ths(i,j,k1))/(ths(i,j,k2)-ths(i,j,k1))
              p1=alevel(k1)+blevel(k1)*psurf(i,j)
              p2=alevel(k2)+blevel(k2)*psurf(i,j)
ctab          pi1=cp*(p1*p0inv)**rcp
ctab          pi2=cp*(p2*p0inv)**rcp
              xt=p1*dpinv
              lt=xt
              pi1=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
              xt=p2*dpinv
              lt=xt
              pi2=pitab(lt)+(pitab(lt+1)-pitab(lt))*(xt-lt)
	      pi= c1*pi1+c2*pi2
ctab          plev(i,j)=p0*(pi*cpinv)**cpr
              xt=pi*dpiinv
              lt=xt
              plev(i,j)=pptab(lt)+(pptab(lt+1)-pptab(lt))*(xt-lt)
              zlev(i,j)=c1*zs(i,j,k1)+c2*zs(i,j,k2)
c#################################################################
c	      if(i.eq.ii .and. j.eq.jj) then
c		write(6,*) 'k1,k2,p1,p2,t1,t2,p,z,c1,c2,t: ',
c    +			k1,k2,p1,p2,ths(i,j,k1),ths(i,j,k2),
c    +			plev(i,j),zlev(i,j),t,c1,c2,t
c	      end if
c#################################################################
c#################################################################
c	      if(pmin.gt.plev(i,j)) pmin=plev(i,j)
c	      if(pmax.lt.plev(i,j)) pmax=plev(i,j)
c	      if(zmin.gt.zlev(i,j)) zmin=zlev(i,j)
c	      if(zmax.lt.zlev(i,j)) zmax=zlev(i,j)
c#################################################################
            end if
          end do
        end do
c#################################################################
c	write(6,*) 'T,Pn,Px,Zn,Zx: ',t,pmin,pmax,zmin,zmax
c#################################################################
c
c..p(t=const), parameter 8
        idata( 6)=8
        idata( 7)=nint(t*10.)
        idata(20)=-32767
        call mwfelt(2,fileot,iunitw,ipackw,lfield,plev(1,1),1.,
     +              ldata,idata,ierror)
c..z(t=const), parameter 1
        idata( 6)=1
        idata( 7)=nint(t*10.)
        idata(20)=-32767
        call mwfelt(2,fileot,iunitw,ipackw,lfield,zlev(1,1),1.,
     +              ldata,idata,ierror)
c
      end do
c
      return
      end
