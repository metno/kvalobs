c***********************************************************************
c vcdsub.f : subroutines for vcdata
c ---------------------------------
c subroutines:  rcdata - read and store data for one timestep
c		rcinit - initialization
c               hermit - hermit interpolation
c		comput - divergence and vorticity
c		gwindp - geostrophic wind in pressure levels
c		gwinds - geostrophic wind in sigma/eta levels
c		tabdef - tables
c		potv   - potential vorticity
c		horvort- horizontal vorticity components
c
c-----------------------------------------------------------------------
c  DNMI/FoU  21.09.1987  Anstein Foss
c  DNMI/FoU  03.03.1992  Anstein Foss
c  DNMI/FoU  27.02.1994  Anstein Foss
c  DNMI/FoU  07.03.1994  Anstein Foss
c  DNMI/FoU  30.03.1994  Anstein Foss
c  DNMI/FoU  31.05.1994  Anstein Foss
c  DNMI/FoU  26.08.1994  Anstein Foss ... single level topography
c  DNMI/FoU  18.05.1995  Anstein Foss ... eta levels, misc. grids, bput*
c  DNMI/FoU  20.09.1995  Anstein Foss ... interp.bilinear, rm bug.undef
c  DNMI/FoU  19.12.1995  Anstein Foss
c  DNMI/FoU  04.01.1996  Anstein Foss ... geost. wind in pot.temp levels
c  DNMI/FoU  14.06.1996  Anstein Foss ... minor int2pos,pos2pos update
c  DNMI/FoU  02.09.1996  Anstein Foss ... better height in eta levels
c  DNMI/FoU  20.09.1996  Anstein Foss ... hermit interpolation (again)
c  DNMI/FoU  17.11.1996  Anstein Foss ... only hermit interpolation
c  DNMI/FoU  28.05.1997  Anstein Foss ... sigma height levels (MEMO)
c  DNMI/FoU  08.10.1997  Anstein Foss ... rotation to E/W and N/S
c  DNMI/FoU  01.12.1998  Anstein Foss ... straight.lines.geographic.grid
c  DNMI/FoU  10.12.1999  Anstein Foss ... sigma.MM5
c  DNMI/FoU  10.10.2002  Anstein Foss ... bug fix (undef testing)
c  DNMI/FoU  03.02.2003  Anstein Foss ... param.min/max/min.max=...
c  DNMI/FoU  10.06.2004  Anstein Foss ... horizontal.vorticity
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c  DNMI/FoU  26.10.2005  Anstein Foss ... surface.parameter.level option
c  DNMI/FoU  07.12.2005  Anstein Foss ... posinp(2,max) -> posinp(3,max)
c met.no/FoU 02.12.2008  Ole Vignes ..... lambert (tangent) grids
c-----------------------------------------------------------------------
c***********************************************************************
c
      subroutine rcdata(ifirst,info,npar,nlev,ipar,iparlev,ilev,ilevot,
     +                  numpos,posinp,posmap,
     +                  inter,isline,alevel,blevel,
     +                  lcdat,cdat,itime,indat,f,fmap,
     +                  ntotal,geopos,gxypos,pardat,
     +                  nhermit,khermit,ihermit,dhermit,
     +                  igtype,grid,igsize,igtyp2,grid2,igsiz2,
     +                  nx,ny,nparlim,iparlim,parlim,ierr)
c
c        read fields and interpolate to crossection positions.
c
c        output.  ierr=0: o.k.
c                     =1: not o.k.  -  error in parameter sequence
c                     =2: not o.k.  -  positions
c                     =3: not o.k.  -  total length of crossections
c                     =4: not o.k.  -  too small data area
c                     =5: not o.k.  -  data not found
c
c        warning! levels are reordered.
c                 levels are usually stored in order top - bottom
c                 on felt files (and it is most efficient to read
c                 in this order), while the plotting program uses
c                 the order bottom - top.
c                 (geost. wind: read bottom - top)
c
c        compute parameters:  -1: vorticity   (after u,v input)
c                             -2: divergence  (after u,v input)
c                             -3: x component of geostrophic wind (ug)
c                             -4: y component of geostrophic wind (vg)
c                             -5: potential vorticity (after u,v,th input)
c                             -6: x comp. horizontal vorticity (z,u,v,w input)
c                             -7: y comp. horizontal vorticity (z,u,v,w input)
c
c
      include 'vcdata.inc'
c
      integer   ifirst,npar,nlev,inter,isline,lcdat,ntotal,nhermit
      integer   igtype,igtyp2,nx,ny,nparlim,ierr
      integer   info(12),ipar(npar),ilev(nlev),ilevot(nlev)
      integer   iparlev(npar)
      integer   numpos(9,maxcrs),itime(5)
      integer   khermit(2,maxpos,2),ihermit(2,maxpos)
      integer   igsize(4),igsiz2(4),iparlim(2,nparlim)
      real      posinp(3,maxinp),posmap(2,maxpos),cdat(lcdat)
      real      alevel(nlev),blevel(nlev)
      real      geopos(maxpos,2),gxypos(maxpos,2)
      real      pardat(maxpos,4)
      real      f(maxij,maxflt),fmap(maxij,3)
      real      dhermit(2,maxpos)
      real      grid(6),grid2(6),parlim(2,nparlim)
      integer*2 indat(mindat)
c
      real      alvls(5),blvls(5),ptop
      integer*2 in(16)
      integer   ierrf(3),ih(6)
      integer   iparhv(4)
c
      data ih/6*0/, ierrf/3*0/
      data iparhv/1,2,3,12/
c
      ih(1)=0
c
      iunitf=info(1)
      in( 1)=info(2)
      in( 2)=info(3)
      in( 9)=info(4)
      in(10)=info(5)
      ivcoor=info(6)
      in(11)=ivcoor
      if(ivcoor.eq.22) in(11)=2
      if(ivcoor.eq.10 .and. nlev.eq.1) in(11)=2
      if(ivcoor.eq.12 .and. nlev.eq.1) in(11)=2
      in(14)=0
ccc   nlev=  info(7)
ccc   npar=  info(8)
      ncross=info(9)
c
c..defaults
      htop=-1.
      ptop=-1.
      do k=1,nlev
        alevel(k)=0.
        blevel(k)=0.
      end do
c
      level2=0
      if(ivcoor.eq.10 .and. nlev.gt.1) level2=-32767
c
c..save fields for later computations
c..name(parameter no.): u(2) v(3) th(18) z(1)
      nsaveu=0
      nsavev=0
      nsavet=0
c..save 3 levels of u,v and pot.temp. for computation of pot.vorticity
      ipotv=0
c..sigma-dot (input in sig1-levels 2:ks, output in sig2-levels 1:ks)
c..  eta-dot (input in half-levels 2:kk, output in full-levels 1:kk)
      nsaved=0
c
c..horizontal vorticity
      ihorvort=0
c
c..save u and v if vorticity or divergence
c..save u,v and th if potential vorticity
      k1=1
      k2=1
      iu=0
      iv=0
      it=0
      do np=1,npar
        if(ipar(np).eq. 2) iu=np
        if(ipar(np).eq. 3) iv=np
        if(ipar(np).eq.18) it=np
        if(ivcoor.eq.5 .and. ipar(np).eq.302) iu=np
        if(ivcoor.eq.5 .and. ipar(np).eq.303) iv=np
        if(ipar(np).eq.-1 .or. ipar(np).eq.-2) then
          nsaveu=iu
          nsavev=iv
          if(iu.eq.0 .or. iv.eq.0) k1=0
        end if
        if(ipar(np).eq.-5) then
          nsaveu=iu
          nsavev=iv
          nsavet=it
          ipotv=1
          if(iu.eq.0 .or. iv.eq.0 .or. it.eq.0) k2=0
        end if
c..sigma-dot (parameter 11)
        if(ipar(np).eq.11 .and.
     *     (ivcoor.eq.2 .or. ivcoor.eq.22)) nsaved=np
c..horizontal vorticity
        if(ipar(np).eq.-6 .and. np.lt.npar) then
          if(ipar(np+1).eq.-7) ihorvort=1
        end if
      end do
      if(k1.eq.0) then
        if(ivcoor.eq.5) then
          write(6,*) 'specify vorticity(-1),divergence(-2) after',
     *               ' u(302) and v(303)'
        else
          write(6,*) 'specify vorticity(-1),divergence(-2) after',
     *               ' u(2) and v(3)'
        end if
      end if
      if(k2.eq.0)
     *  write(6,*) 'specify potential vorticity(-5) after u(2), v(3)',
     *             ' and th(18)'
      if(k1.eq.0 .or. k2.eq.0) then
        ierr=1
        return
      end if
c
c..check components of geostrophic wind
      igwind=0
      k=1
      if(ipar(1).eq.-4) k=0
      if(ipar(npar).eq.-3) k=0
      do np=1,npar-1
        if(ipar(np).eq.-3 .and. ipar(np+1).ne.-4) k=0
        if(ipar(np).eq.-3 .and. ipar(np+1).eq.-4) igwind=1
      end do
      do np=2,npar
        if(ipar(np).eq.-4 .and. ipar(np-1).ne.-3) k=0
      end do
      if(k.eq.0) then
        write(6,*) 'specify geost. wind with ug(-3) before vg(-4)'
        ierr=1
        return
      end if
c
      igwinds=0
      igwindp=0
      if(igwind.ne.0) then
        if(ivcoor.eq.2 .or. ivcoor.eq.22 .or. ivcoor.eq.10) igwinds=1
        if(ivcoor.eq.1) igwindp=1
        if(ivcoor.eq.4) igwindp=1
        igwind=0
      end if
c
      mf=1
      if(nsaveu.gt.0) mf=2
      if(nsavev.gt.0) mf=3
      if(nsavet.gt.0) mf=4
      if(igwindp.gt.0) mf=7
      if(igwinds.gt.0) mf=13
      if(igwinds.gt.0 .and. ivcoor.eq.22) mf=14
      if(ipotv .gt.0) mf=11
      if(nsaved.gt.0) mf=12
c
      if(ihorvort.eq.1 .and. mf.lt.14) mf=14
c
      if(mf.gt.maxflt) then
        write(6,*) ' no. of fields required:   ',mf
        write(6,*) ' max (''parameter'' maxflt): ',maxflt
        stop 117
      end if
c
      undef=+1.e+35
      iundef=0
c
c..potential vorticity:  read ps (surface pressure)
      if(ipotv.gt.0) then
        if(ivcoor.eq.10) in(11)=2
        in(12)=8
        in(13)=1000
        in(14)=0
c..read data from felt file
        call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
        if(ip.ne.1) goto 910
        if(ivcoor.eq.10) in(11)=ivcoor
        if(ifirst.eq.0) then
          ifirst=1
          call rcinit(indat,info,numpos,posinp,posmap,ntotal,
     +                inter,isline,geopos,gxypos,pardat,
     +                nx,ny,fmap(1,1),fmap(1,2),fmap(1,3),
     +                khermit,ihermit,dhermit,nhermit,
     +                igtype,grid,igsize,igtyp2,grid2,igsiz2,ierr)
          if(ierr.ne.0) then
            ierr=2
            return
          end if
        end if
        ptop=indat(19)
        scal=10.**indat(20)
        do i=1,nx*ny
          if(indat(20+i).ne.-32767) then
            f(i,11)=scal*indat(20+i)
          else
            f(i,11)=undef
            iundef=1
          end if
        end do
      end if
c
c..sigma-dot
      if(nsaved.gt.0) then
        if(ilev(1).eq.1) then
          do i=1,maxij
            f(i,12)=0.
          end do
        else
          in(12)=ipar(nsaved)
          in(13)=ilev(1)
          in(14)=level2
c..read data from felt file
          call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
          if(ip.ne.1) goto 910
          if(ifirst.eq.0) then
            ifirst=1
            call rcinit(indat,info,numpos,posinp,posmap,ntotal,
     +                  inter,isline,geopos,gxypos,pardat,
     +                  nx,ny,fmap(1,1),fmap(1,2),fmap(1,3),
     +                  khermit,ihermit,dhermit,nhermit,
     +                  igtype,grid,igsize,igtyp2,grid2,igsiz2,ierr)
            if(ierr.ne.0) then
              ierr=2
              return
            end if
          end if
          scal=10.**indat(20)
          do i=1,nx*ny
            if(indat(20+i).ne.-32767) then
              f(i,12)=scal*indat(20+i)
            else
              f(i,12)=undef
              iundef=1
            end if
          end do
        end if
      end if
c
      nread=0
      nz=0
      nug=0
      nvg=0
c
      do 100 nli=1,nlev
c
      nlo=ilevot(nli)
c
c..save 3 levels of u,v,th for computation of pot.vorticity
      if(ipotv.eq.1 .and. nli.gt.1) then
        do n=10,5,-1
          do i=1,nx*ny
            f(i,n)=f(i,n-3)
          end do
        end do
        alvls(3)=alvls(2)
        blvls(3)=blvls(2)
        alvls(2)=alvls(1)
        blvls(2)=blvls(1)
      end if
c
      do 110 np=1,npar
c
      if((ipar(np).eq.-3 .or. ipar(np).eq.-4)
     *                   .and. igwinds.eq.1) goto 110
c
      if((ipar(np).eq.-6 .or. ipar(np).eq.-7)
     *                   .and. ihorvort.eq.1) goto 110
c
      nl=nlo
      in( 9)=info(4)
      in(10)=info(5)
      in(11)=info(6)
      in(12)=ipar(np)
      in(13)=ilev(nli)
      if (nlev.eq.1) in(13)=iparlev(np)
      in(14)=level2
      notime=0
      if(ivcoor.eq.5 .and. nlev.eq.1) then
c..sea surface elevation and sea bottom
        in(11)=8
c..sea bottom depth (constant in time, 'parameter field')
        if(in(12).eq.351) notime=1
      end if
      if((ivcoor.eq.2 .or. ivcoor.eq.22) .and. nlev.eq.1) then
c..parameter fields (constant in time)
        if(in(12).eq.101) notime=1
        if(in(12).eq.102) notime=1
        if(in(12).eq.103) notime=1
        if(in(12).eq.104) notime=1
        if(in(12).eq.183) notime=1
      end if
      if(ivcoor.eq.11 .and. nlev.eq.1) then
c..topography, sigma height levels
        in(11)=2
        if(in(12).eq.101) notime=1
      end if
      if(ivcoor.eq.12 .and. nlev.eq.1) then
c..topography, MM5 model ("sigma") levels
        in(11)=2
        if(in(12).eq.101) notime=1
      end if
      if(notime.eq.1) then
        in( 9)=4
        in(10)=0
      end if
c
c..read z (param. 1) if geostrophic wind in pressure levels
c..or Montgomery potential (param. 90) in isentropic (theta) levels
      if(ipar(np).eq.-3 .and. igwindp.eq.1) in(12)=1
      if(ipar(np).eq.-3 .and. igwindp.eq.1 .and. ivcoor.eq.4) in(12)=90
      if(ipar(np).eq.-4 .and. igwindp.eq.1) then
        do i=1,nx*ny
          f(i,1)=f(i,7)
        end do
        goto 160
      end if
c
c..sigmadot
      if(np.eq.nsaved) then
        if(nli.eq.nlev) goto 149
        in(13)=in(13)+1
        in(14)=level2
      end if
c
      if(ipar(np).eq.-1 .or. ipar(np).eq.-2) then
c..vorticity or divergence
        icompu=ipar(np)
        call comput(icompu,f(1,2),f(1,3),f(1,1),
     *              fmap(1,1),fmap(1,2),fmap(1,3),
     *              nx,ny,iundef,undef)
        goto 160
      end if
c
      if(ipar(np).eq.-5) then
c..potential vorticity
        alvls(1)=alevel(nl)
        blvls(1)=blevel(nl)
        if(nli.eq.1) goto 110
        if(nli.eq.2) then
c..copy fields from sigma/eta level '1' to level '0' (for potv)
          do n=10,8,-1
            do i=1,nx*ny
              f(i,n)=f(i,n-3)
            end do
          end do
          alvls(3)=alvls(2)
          blvls(3)=blvls(2)
        end if
        if(ivcoor.eq.2) then
          alvls(1)=ptop*(1.-blvls(1))
          alvls(2)=ptop*(1.-blvls(2))
          alvls(3)=ptop*(1.-blvls(3))
        elseif(ivcoor.eq.22) then
          alvls(1)=ptop
          alvls(2)=ptop
          alvls(3)=ptop
        end if
c..computing potential vorticity in the previous sigma/eta level
        if(iundef.ne.0) then
          write(6,*) 'ERROR: Cannot handle undefined values when ',
     +                                             'computing'
          write(6,*) '       potential vorticity in sigma or eta ',
     +                                             'levels.'
          stop 117
        end if
        call potv(f(1,2),f(1,3),f(1,4),
     *            f(1,5),f(1,6),f(1,7),
     *            f(1,8),f(1,9),f(1,10),
     *            f(1,11),f(1,1),
     *            fmap(1,1),fmap(1,2),fmap(1,3),
     *            nx,ny,alvls,blvls)
        nl=nlo+1
        goto 160
      end if
c
c..read data from felt file
      call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
      if(ip.ne.1) goto 910
c
c..store pt from ps identification when single level 'sigma' fields
c..(for ps is ident(19)=ptop in unit hPa)
      if(nlev.eq.1 .and. in(12).eq.8 .and.
     *   (ivcoor.eq.2 .or. ivcoor.eq.22)) ptop=indat(19)
c..store Htop from zs identification when single level 'sigma height'
c..(for zs is ident(19)=Htop in unit m)
      if(nlev.eq.1 .and. in(12).eq.101 .and. ivcoor.eq.11)
     *                                               htop=indat(19)
c
      if(ifirst.eq.0) then
        ifirst=1
        call rcinit(indat,info,numpos,posinp,posmap,ntotal,
     +              inter,isline,geopos,gxypos,pardat,
     +              nx,ny,fmap(1,1),fmap(1,2),fmap(1,3),
     +              khermit,ihermit,dhermit,nhermit,
     +              igtype,grid,igsize,igtyp2,grid2,igsiz2,ierr)
        if(ierr.ne.0) then
          ierr=2
          return
        end if
      end if
c
      nread=nread+1
      if(nread.eq.1) then
        ldat=ntotal*nlev*npar
        if(ldat.gt.maxd) then
          write(6,*) '*****(1)***** ldat,maxd: ',ldat,maxd
          ierr=3
          return
        end if
      end if
c
      if(notime.eq.0) then
        itime(1)=indat(12)
        itime(2)=indat(13)/100
        itime(3)=indat(13)-indat(13)/100*100
        itime(4)=indat(14)/100
        itime(5)=indat(4)
      end if
      if(np.ne.nsaved) then
        alevel(nl)=indat( 8)*0.1
        blevel(nl)=indat(19)*0.0001
      end if
c
      nx=indat(10)
      ny=indat(11)
      scal=10.**indat(20)
      do i=1,nx*ny
        if(indat(20+i).ne.-32767) then
          f(i,1)=scal*indat(20+i)
        else
          f(i,1)=undef
          iundef=1
        end if
      end do
c
      if(ipar(np).eq.-3 .and. igwindp.eq.1) then
        call gwindp(f(1,1),f(1,6),f(1,7),
     *              fmap(1,1),fmap(1,2),fmap(1,3),
     *              nx,ny,ivcoor,iundef,undef)
        do i=1,nx*ny
          f(i,1)=f(i,6)
        end do
        goto 160
      end if
c
  149 if(np.eq.nsaveu) then
        do i=1,nx*ny
          f(i,2)=f(i,1)
        end do
      elseif(np.eq.nsavev) then
        do i=1,nx*ny
          f(i,3)=f(i,1)
        end do
      elseif(np.eq.nsavet) then
        do i=1,nx*ny
          f(i,4)=f(i,1)
        end do
      elseif(np.eq.nsaved) then
        if(nli.eq.nlev) then
          do i=1,nx*ny
            f(i,1)=0.
          end do
        end if
        do i=1,nx*ny
          fhelp=f(i,1)
          f(i,1)=(f(i,1)+f(i,12))*0.5
          f(i,12)=fhelp
        end do
      end if
c
  160 continue
c
c..interpolate data for all crossections (one level, one parameter).
c..set pointer to first data area (parameter no. np)
c
      iadr=ntotal*npar*(nl-1)+ntotal*(np-1)+1
      call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +            cdat(iadr),nx,ny,f(1,1),iundef)
c
      iadrend=iadr+ntotal-1
      do n=1,nparlim
	if(iparlim(1,n).eq.ipar(np)) then
	  if(iparlim(2,n).eq.1) then
	    do i=iadr,iadrend
	      if(cdat(i).ne.undef) then
	        if(cdat(i).lt.parlim(1,n)) cdat(i)=parlim(1,n)
	      end if
	    end do
	  elseif(iparlim(2,n).eq.2) then
	    do i=iadr,iadrend
	      if(cdat(i).ne.undef) then
	        if(cdat(i).gt.parlim(2,n)) cdat(i)=parlim(2,n)
	      end if
	    end do
	  elseif(iparlim(2,n).eq.3) then
	    do i=iadr,iadrend
	      if(cdat(i).ne.undef) then
	        if(cdat(i).lt.parlim(1,n)) cdat(i)=parlim(1,n)
	        if(cdat(i).gt.parlim(2,n)) cdat(i)=parlim(2,n)
	      end if
	    end do
	  end if
	end if
      end do
c
c..potential vorticity
      if(ipar(np).eq.-5 .and. nl.eq.2) then
c..move fields (lower sigma level reached)
        do n=10,5,-1
          do i=1,nx*ny
            f(i,n)=f(i,n-3)
          end do
        end do
        alvls(3)=alvls(2)
        blvls(3)=blvls(2)
        alvls(2)=alvls(1)
        blvls(2)=blvls(1)
c..copy fields from sigma level 'ks' to sigma level 'ks+1' (for potv)
        do n=2,4
          do i=1,nx*ny
            f(i,n)=f(i,n+3)
          end do
        end do
        alvls(1)=alvls(2)
        blvls(1)=blvls(2)
        if(ivcoor.eq.2) then
          alvls(1)=ptop*(1.-blvls(1))
          alvls(2)=ptop*(1.-blvls(2))
          alvls(3)=ptop*(1.-blvls(3))
        elseif(ivcoor.eq.22) then
          alvls(1)=ptop
          alvls(2)=ptop
          alvls(3)=ptop
        end if
        call potv(f(1,2),f(1,3),f(1,4),
     *            f(1,5),f(1,6),f(1,7),
     *            f(1,8),f(1,9),f(1,10),
     *            f(1,11),f(1,1),
     *            fmap(1,1),fmap(1,2),fmap(1,3),
     *            nx,ny,alvls,blvls)
        nl=1
        goto 160
      end if
c
  110 continue
  100 continue
c
      if(igwinds.eq.0) goto 300
c
c-------------------------------------------------------------
c        geostrophic wind (sigma/eta levels)
c-------------------------------------------------------------
c
c          input parameters: 8=ps  101=zs  18=pot.temp.  ( 78=pb )
c
c..read ps (surface pressure)
      in(11)=ivcoor
      if(ivcoor.eq.10) in(11)=2
      in(12)=8
      in(13)=1000
      in(14)=0
c..read data from felt file
      call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
      if(ip.ne.1) goto 910
      if(ivcoor.eq.10) in(11)=ivcoor
      nx=indat(10)
      ny=indat(11)
      scal=10.**indat(20)
      do i=1,nx*ny
        if(indat(20+i).ne.-32767) then
          f(i,1)=scal*indat(20+i)
        else
          f(i,1)=undef
          iundef=1
        end if
      end do
c..pressure at top of model (sigma=0.)
      ptop=indat(19)
c..at surface
      alvls(4)=0.
      blvls(4)=1.
c
cpb--------------------------------------------------------------------
      if(ivcoor.eq.22) then
c..read pb (pressure at top of pbl)
        in(12)=78
        in(13)=1000
        in(14)=level2
c..read data from felt file
        call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
        if(ip.ne.1) goto 910
        nx=indat(10)
        ny=indat(11)
        scal=10.**indat(20)
        do i=1,nx*ny
          if(indat(20+i).ne.-32767) then
            f(i,14)=scal*indat(20+i)
          else
            f(i,14)=undef
            iundef=1
          end if
        end do
c..at surface
        alvls(4)=0.
        blvls(4)=2.
      end if
cpb--------------------------------------------------------------------
c..read zs (topography)
      in( 9)=4
      in(10)=0
      in(11)=ivcoor
      if(ivcoor.eq.22) in(11)=2
      if(ivcoor.eq.10) in(11)=2
      in(12)=101
      in(13)=1000
      in(14)=0
c..read data from felt file
      call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
      if(ip.ne.1) goto 910
      nx=indat(10)
      ny=indat(11)
      scal=10.**indat(20)
      do i=1,nx*ny
        if(indat(20+i).ne.-32767) then
          f(i,5)=scal*indat(20+i)
        else
          f(i,5)=undef
          iundef=1
        end if
      end do
c
      in( 9)=info(4)
      in(10)=info(5)
      in(11)=info(6)
      if(ivcoor.eq.22) in(11)=2
      in(12)=18
c
c..for improved vertical 'interpolation' of height 3 levels of
c..potential temperature are needed, read the first here
c..(for height at lower and upper level pot.temp. below or above
c.. is copied from the nearest level)
c..level below: f(i,2) ... (using level 1 for nl=1)
c..level nl   : f(i,3)
c..level above: f(i,4) ... (using level ilev for nl=ilev)
      in(13)=ilev(nlev)
      in(14)=level2
c..read data from felt file
      call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
      if(ip.ne.1) goto 910
      nx=indat(10)
      ny=indat(11)
      scal=10.**indat(20)
      do i=1,nx*ny
        if(indat(20+i).ne.-32767) then
          f(i,2)=scal*indat(20+i)
        else
          f(i,2)=undef
          iundef=1
        end if
        f(i,3)=f(i,2)
      end do
      alvls(5)=indat( 8)*0.1
      blvls(5)=indat(19)*0.0001
      alvls(3)=alvls(5)
      blvls(3)=blvls(5)
c
c
      do 230 np=1,npar-1
      if(ipar(np).ne.-3) goto 230
c
      do 240 nli=nlev,1,-1
      nl=nlev+1-nli
c
      do i=1,3
        alvls(i)=alvls(i+2)
        blvls(i)=blvls(i+2)
      end do
c
      if(nli.gt.1) then
c
c..read data from felt file
        in(13)=ilev(nli-1)
        in(14)=level2
        call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
        if(ip.ne.1) goto 910
c
        if(ifirst.eq.0) then
          ifirst=1
          call rcinit(indat,info,numpos,posinp,posmap,ntotal,
     +                inter,isline,geopos,gxypos,pardat,
     +                nx,ny,fmap(1,1),fmap(1,2),fmap(1,3),
     +                khermit,ihermit,dhermit,nhermit,
     +                igtype,grid,igsize,igtyp2,grid2,igsiz2,ierr)
          if(ierr.ne.0) then
            ierr=2
            return
          end if
        end if
c
        nread=nread+1
        if(nread.eq.1) then
          ldat=ntotal*nlev*npar
          if(ldat.gt.maxd) then
            write(6,*) '*****(1)***** ldat,maxd: ',ldat,maxd
            ierr=3
            return
          end if
        end if
c
        itime(1)=indat(12)
        itime(2)=indat(13)/100
        itime(3)=indat(13)-indat(13)/100*100
        itime(4)=indat(14)/100
        itime(5)=indat(4)
        alvls(5)=indat( 8)*0.1
        blvls(5)=indat(19)*0.0001
c
        nx=indat(10)
        ny=indat(11)
        scal=10.**indat(20)
        do i=1,nx*ny
          if(indat(20+i).ne.-32767) then
            f(i,4)=scal*indat(20+i)
          else
            f(i,4)=undef
            iundef=1
          end if
        end do
c
      end if
c
c..following o.k. for norlam sigma levels and hirlam eta levels
      alvls(4)=max(alvls(3)-(alvls(2)-alvls(3)),0.)
      blvls(4)=max(blvls(3)-(blvls(2)-blvls(3)),0.)
c
c..compute geostrophic wind (ug and vg)
c
c         levels: 5 - full level, th3 input (th3=th2 for last level)
c                 4 - half level,            height computed
c                 3 - full level, th2 input, height,ug,vg computed
c                 2 - half level, height input
c                 1 - full level, th1 input (th1=th2 for first level)
c
c         input.  f( , 1): ps
c                 f( , 2): pot.temp. at level 1
c                 f( , 3): pot.temp. at level 3
c                 f( , 4): pot.temp. at level 5
c                 f( , 5): z at level 2
c                 f( , 6): (nothing)
c                 f( , 7): (nothing)
c                 f( , 8): (nothing)
c                 f( , 9): (nothing)
c                 f( ,10): (nothing)
c                 f( ,11): exner function level 1 (except first level)
c                 f( ,12): exner function level 2 (except first level)
c                 f( ,13): exner function level 3 (except first level)
c                          NOTE: f(..,9) - f(..,13) is used as a
c                                continues 3d array, array(nx,ny,5)
cpb--------------------------------------------------
c                 f( ,14): pb
cpb--------------------------------------------------
c
      if(iundef.ne.0) then
        write(6,*) 'ERROR: Cannot handle undefined values when ',
     +                                           'computing'
        write(6,*) '       geostrophic wind in sigma or eta levels.'
        stop 117
      end if
c
      call gwinds(nx,ny,nl,nlev,ivcoor,alvls,blvls,
     +            f(1,1),f(1,2),f(1,3),f(1,4),f(1,5),f(1,6),
     +            f(1,7),f(1,8),f(1,9),f(1,14),
     +            fmap(1,1),fmap(1,2),fmap(1,3))
c
c         output. f( , 1): ps
c                 f( , 2): pot.temp. at level 3 (next level 1)
c                 f( , 3): pot.temp. at level 5 (next level 3)
c                 f( , 4): pot.temp. at level 5
c                 f( , 5): z at level 4 (next level 2)
c                 f( , 6): z at level 3
c                 f( , 7): ug
c                 f( , 8): vg
c                 f( , 9): exner function level 1
c                 f( ,10): exner function level 2
c                 f( ,11): exner function level 3 (next level 1)
c                 f( ,12): exner function level 4 (next level 2)
c                 f( ,13): exner function level 5 (next level 3)
c                          NOTE: f(..,9) - f(..,13) is used as a
c                                continues 3d array, array(nx,ny,5)
cpb--------------------------------------------------
c                 f( ,14): pb
cpb--------------------------------------------------
c
c..get data for all crossections (one level, one parameter) .. ug
c..set pointer to first data area (parameter no. np)
c
      iadr=ntotal*npar*(nl-1)+ntotal*(np-1)+1
      call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +            cdat(iadr),nx,ny,f(1,7),iundef)
c
c..get data for all crossections (one level, one parameter) .. vg
c..set pointer to first data area (parameter no. np+1)
c
      iadr=ntotal*npar*(nl-1)+ntotal*np+1
      call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +            cdat(iadr),nx,ny,f(1,8),iundef)
c
  240 continue
  230 continue
c-------------------------------------------------------------
c
  300 continue
      if (ihorvort.eq.0) goto 400
c
c..a bit fast and memory consuming coding of "horizontal vorticity"
c..due to urgent need
c
c################################################################
c     write(6,*) 'horizontal vorticity'
c     do nl=1,nlev
c	write(6,*) '    nl,lev,ilevot: ',nl,ilev(nl),ilevot(nl)
c     end do
c################################################################
c
      nhvx=0
      nhvy=0
      do np=1,npar
        if(ipar(np).eq.-6) nhvx=np
        if(ipar(np).eq.-7) nhvy=np
      end do
      if(nhvx.eq.0 .or. nhvy.eq.0) goto 400
c
      in( 1)=info(2)
      in( 2)=info(3)
      in( 9)=info(4)
      in(10)=info(5)
      in(11)=info(6)
      k3=0
      k2=4
      k1=8
c
      do 310 nl=1,nlev
c
        kh=k3
        k3=k2
        k2=k1
        k1=kh
        in(13)=ilev(nl)
        in(14)=-32767
c
        do 320 np=1,4
c
          in(12)=iparhv(np)
c
          call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
          if(ip.ne.1) goto 910
c
          if(ifirst.eq.0) then
            ifirst=1
            call rcinit(indat,info,numpos,posinp,posmap,ntotal,
     +                  inter,isline,geopos,gxypos,pardat,
     +                  nx,ny,fmap(1,1),fmap(1,2),fmap(1,3),
     +                  khermit,ihermit,dhermit,nhermit,
     +                  igtype,grid,igsize,igtyp2,grid2,igsiz2,ierr)
            if(ierr.ne.0) then
              ierr=2
              return
            end if
          end if
c
          nread=nread+1
          if(nread.eq.1) then
            ldat=ntotal*nlev*npar
            if(ldat.gt.maxd) then
              write(6,*) '*****(1)***** ldat,maxd: ',ldat,maxd
              ierr=3
              return
            end if
          end if
c
          itime(1)=indat(12)
          itime(2)=indat(13)/100
          itime(3)=indat(13)-indat(13)/100*100
          itime(4)=indat(14)/100
          itime(5)=indat(4)
c
          nx=indat(10)
          ny=indat(11)
          scal=10.**indat(20)
          k=k1+np
          do i=1,nx*ny
            if(indat(20+i).ne.-32767) then
              f(i,k)=scal*indat(20+i)
            else
              f(i,k)=undef
              iundef=1
            end if
          end do
c
  320   continue
c
        if(nl.eq.2) then
c
          nlout=ilevot(1)
	  call horvort(nx,ny,
     +		       f(1,k1+1),f(1,k1+2),f(1,k1+3), f(1,k1+4),
     +                 f(1,k2+1),f(1,k2+2),f(1,k2+3),
     +		       fmap(1,1),fmap(1,2),
     +		       f(1,13),f(1,14))
c..crossections hvx
          iadr=ntotal*npar*(nlout-1)+ntotal*(nhvx-1)+1
          call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                cdat(iadr),nx,ny,f(1,13),iundef)
c..crossections hvy
          iadr=ntotal*npar*(nlout-1)+ntotal*(nhvy-1)+1
          call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                cdat(iadr),nx,ny,f(1,14),iundef)
c
          nlout=ilevot(2)
	  call horvort(nx,ny,
     +		       f(1,k1+1),f(1,k1+2),f(1,k1+3), f(1,k2+4),
     +                 f(1,k2+1),f(1,k2+2),f(1,k2+3),
     +		       fmap(1,1),fmap(1,2),
     +		       f(1,13),f(1,14))
c..crossections hvx
          iadr=ntotal*npar*(nlout-1)+ntotal*(nhvx-1)+1
          call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                cdat(iadr),nx,ny,f(1,13),iundef)
c..crossections hvy
          iadr=ntotal*npar*(nlout-1)+ntotal*(nhvy-1)+1
          call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                cdat(iadr),nx,ny,f(1,14),iundef)
c
	elseif(nl.gt.2) then
c
	  nlout=ilevot(nl-1)
	  call horvort(nx,ny,
     +		       f(1,k1+1),f(1,k1+2),f(1,k1+3), f(1,k2+4),
     +                 f(1,k3+1),f(1,k3+2),f(1,k3+3),
     +		       fmap(1,1),fmap(1,2),
     +		       f(1,13),f(1,14))
c..crossections hvx
          iadr=ntotal*npar*(nlout-1)+ntotal*(nhvx-1)+1
          call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                cdat(iadr),nx,ny,f(1,13),iundef)
c..crossections hvy
          iadr=ntotal*npar*(nlout-1)+ntotal*(nhvy-1)+1
          call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                cdat(iadr),nx,ny,f(1,14),iundef)
c
	  if(nl.eq.nlev) then
c
c################################################################
c..maybe not good ????...........................................
c	  do i=1,nx*ny
c	    f(i,k1+4)= (f(i,k1+4)+f(i,k2+4))*0.5
c	  end do
c################################################################
c
	    nlout=ilevot(nlev)
	    call horvort(nx,ny,
     +		         f(1,k1+1),f(1,k1+2),f(1,k1+3), f(1,k1+4),
     +                   f(1,k2+1),f(1,k2+2),f(1,k2+3),
     +		         fmap(1,1),fmap(1,2),
     +		         f(1,13),f(1,14))
c..crossections hvx
            iadr=ntotal*npar*(nlout-1)+ntotal*(nhvx-1)+1
            call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                  cdat(iadr),nx,ny,f(1,13),iundef)
c..crossections hvy
            iadr=ntotal*npar*(nlout-1)+ntotal*(nhvy-1)+1
            call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                  cdat(iadr),nx,ny,f(1,14),iundef)
c
	  end if
c
	end if
c
  310 continue
c
  400 continue
c
      if((ivcoor.eq.2 .or. ivcoor.eq.22)) then
        if(ptop.lt.0.) then
c..read ps and get ptop from the field identification
c..(for ps is ident(19)=ptop in unit hPa)
          in(11)=ivcoor
          if(ivcoor.eq.10) in(11)=2
          in(12)=8
          in(13)=1000
          in(14)=0
c..read data from felt file
          call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
          if(ip.ne.1) goto 910
          ptop=indat(19)
        end if
        if(nlev.eq.1) then
          alevel(1)=ptop
        elseif(ivcoor.eq.2) then
          do k=1,nlev
            alevel(k)=ptop*(1.-blevel(k))
          end do
        elseif(ivcoor.eq.22) then
          do k=1,nlev
            alevel(k)=ptop
          end do
        end if
      elseif(ivcoor.eq.11 .and. nlev.eq.1) then
c..sigma height: this goes wrong if no surface param. are specified
        if(htop.lt.0.) then
c..read zs and get htop from the field identification
c..(for zs is ident(19)=htop in unit m)
	  in( 9)=4
	  in(10)=0
          in(11)=2
          in(12)=101
          in(13)=1000
          in(14)=0
c..read data from felt file
          call rfelt(iunitf,ip,in,indat,mindat,ierrf,ih)
          if(ip.ne.1) goto 910
          htop=indat(19)
        end if
        alevel(1)=htop
      end if
c
      ierr=0
      return
c
  910 continue
      write(6,1012) iunitf,ip,ierrf,in(1),in(2),(in(i),i=9,14)
 1012 format(' data not found.  file:',i3,'  ip=',i4,'  ierr=',3i8,/,
     *       '   innh.f.(1,2,9-14): ',8i7)
      if(ip.eq.-3) then
        write(6,*) '**** too much data.  max nx*ny:',maxij
        write(6,*) '(''parameter'' maxij)'
        stop 117
      end if
      ierr=5
c
      return
      end
c
c***********************************************************************
c
      subroutine rcinit(indat,info,numpos,posinp,posmap,ntotal,
     +                  inter,isline,geopos,gxypos,pardat,
     +                  nx,ny,xmd2h,ymd2h,fcor,
     +                  khermit,ihermit,dhermit,nhermit,
     +                  igtype,grid,igsize,igtyp2,grid2,igsiz2,ierr)
c
c        ierr=0: o.k.
c            =1: not o.k. positions
c            =2: too many positions
c
c        isline=0 : straight lines in input grid
c              =1 : straight lines in polarstereographic grid
c              =2 : straight lines in geographic grid
c                   (best if latitude or longitude is constant)
c
c        inter=1: best interpolation, using all types
c             =2: simple interpolation, not using types 1,2,3,4,5
c
c  output:
c     khermit(1,n,1) : hermit interpolation type,
c                        0 = gridpoint
c                        1 = y constant, 4 points used (hermit interp.)
c                        2 = x constant, 4 points used (hermit interp.)
c                        3 = 4*4 points used    (double hermit interp.)
c                        4 = 4*2 points used    (hermit+linear interp.)
c                        5 = 2*4 points used    (hermit+linear interp.)
c                        6 = y constant, 2 points used (linear interp.)
c                        7 = x constant, 2 points used (linear interp.)
c                        8 = 2*2 points used         (bilinear interp.)
c                        9 = no interpolation possible (outside/undef.)
c     khermit(2,n,1) : last position no. (for current type)
c               n=1,nhermit
c     khermit(1,n,2) : work space (used in subr. hermit if iundef>0)
c     khermit(2,n,2) : work space (used in subr. hermit if iundef>0)
c     ihermit(1,i)   : i
c     ihermit(2,i)   : j
c     dhermit(1,i)   : dx
c     dhermit(2,i)   : dy
c               i=1,total_no_of_positions
c
      include 'vcdata.inc'
c
c..input/output
      integer   ntotal,inter,isline,nx,ny,nhermit,igtype,igtyp2,ierr
      integer*2 indat(mindat)
      integer   info(12),numpos(9,maxcrs)
      integer   khermit(2,maxpos,2),ihermit(2,maxpos)
      integer   igsize(4),igsiz2(4)
      real      posinp(3,maxinp),posmap(2,maxpos)
      real      geopos(maxpos,2),gxypos(maxpos,2),pardat(maxpos,4)
      real      xmd2h(maxij),ymd2h(maxij),fcor(maxij)
      real      dhermit(2,maxpos)
      real      grid(6),grid2(6)
c
c..local
      real      gwork(maxij,2)
c
      parameter (mgrtest=8)
      real      grtest(mgrtest,2)
c
      integer  igeogrid
      real      geogrid(6)
c
c..geographic coordinates with a grid description (for xyconvert)
      data igeogrid/2/
      data  geogrid/1.,1.,1.,1.,0.,0./
c
      if(maxinp.gt.maxpos .or. maxinp.gt.maxij) then
        write(6,*) 'RCINIT: PROGRAM ERROR. '
        write(6,*) '  maxinp,maxpos,maxij: ',maxinp,maxpos,maxij
        stop 255
      end if
c
      ierr=0
c
c..isline=0 : straight lines in input grid
c..isline=1 : straight lines in a polarstereographic projection
c..isline=2 : straight lines in a geographic projection
c
      if(isline.eq.1 .and. (igtype.eq.1 .or. igtype.eq.4)) isline=0
      if(isline.eq.2 .and. (igtype.eq.2 .or. igtype.eq.3)
     +                                .and. grid(6).eq.0.) isline=0
c
      call gridpar(+1,mindat,indat,igtype,nx,ny,grid,ierror)
      if(ierror.ne.0) then
        write(6,*) 'RCINIT: GRIDPAR ERROR: ',ierror
        write(6,*) '    grid type: ',igtype
        stop 255
      end if
c
      if(nx*ny.gt.maxij) then
        write(6,*) 'ERROR: Field too big'
        write(6,*) '    nx*ny,nx,ny: ',nx*ny,nx,ny
        write(6,*) '    max size:    ',maxij
        stop 255
      end if
c
c..compute grid parameters (map ratios and coriolis parameter)
      call mapfield(3,1,igtype,grid,nx,ny,xmd2h,ymd2h,fcor,
     +              dxgrid,dygrid,ierror)
      if(ierror.ne.0) then
        write(6,*) 'RCINIT: MAPFIELD ERROR: ',ierror
        write(6,*) '     grid type: ',igtype
        stop 255
      end if
c
      igsize(1)=1
      igsize(2)=nx
      igsize(3)=1
      igsize(4)=ny
c
c..print definition of input grid
      write(*,*) 'input grid = ',indat(2)
      write(*,'(1x,''nx,ny = '',2i10)') nx,ny
      write(*,'(1x,''grid = '',6f10.2)') (grid(i),i=1,6)
c
      ncross=info(9)
      npinp =numpos(4,ncross)
      icvpos=0
c
      ip=1
      do n=1,ncross
        ityp=numpos(1,n)
        if(ityp.eq.1) then
c..x,y coordinates input
          do i=numpos(3,n),numpos(4,n)
            gxypos(i,1)=posinp(1,i)
            gxypos(i,2)=posinp(2,i)
            geopos(i,1)=posinp(1,i)
            geopos(i,2)=posinp(2,i)
          end do
        else
c..geographic latitude,longitude coordinates input
          do i=numpos(3,n),numpos(4,n)
            geopos(i,1)=posinp(2,i)
            geopos(i,2)=posinp(1,i)
            gxypos(i,1)=posinp(2,i)
            gxypos(i,2)=posinp(1,i)
          end do
        end if
        np=numpos(4,n)
        k=1
        if(n.lt.ncross) then
          if(numpos(1,n+1).eq.ityp) k=0
        end if
        if(k.eq.1) then
          if(ityp.eq.1) then
c..from x,y to longitude,latitude
            call xyconvert(np-ip+1,geopos(ip,1),geopos(ip,2),
     +                     igtype,grid,igeogrid,geogrid,ierror)
          else
c..from longitude,latitude to x,y
            call xyconvert(np-ip+1,gxypos(ip,1),gxypos(ip,2),
     +                     igeogrid,geogrid,igtype,grid,ierror)
          end if
          if(ierror.ne.0) then
            write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
            write(6,*) '    grid type: ',igtype
            stop 255
          end if
          ip=np+1
        end if
      end do
c
      xmin=1.
      xmax=real(nx)
      ymin=1.
      ymax=real(ny)
c
      do n=1,ncross
        do i=numpos(3,n),numpos(4,n)
          if(gxypos(i,1).lt.xmin .or. gxypos(i,1).gt.xmax .or.
     +       gxypos(i,2).lt.ymin .or. gxypos(i,2).gt.ymax) then
            j=i-numpos(3,n)+1
            write(6,*) 'Position outside grid area.'
            write(6,*) '    crossection no.,position no.: ',n,j
            ierr=1
          end if
        end do
        do i=numpos(3,n),numpos(4,n)-1
          dx=gxypos(i+1,1)-gxypos(i,1)
          dy=gxypos(i+1,2)-gxypos(i,2)
          if(dx*dx+dy*dy.lt.0.02) then
            j=i-numpos(3,n)+1
            write(6,*) 'Positions too close.'
            write(6,*) '    crossection no.,positions no.: ',n,j,j+1
            ierr=1
          end if
        end do
      end do
c
      if(ierr.ne.0) return
c
      if(igtype.eq.1 .or. igtype.eq.4) then
c
c..presentation map as input grid
        igtyp2=igtype
        do i=1,6
          grid2(i)=grid(i)
        end do
        do i=1,4
          igsiz2(i)=igsize(i)
        end do
c
	if(isline.eq.2) then
	  icvpos=3
	else
c..positions on map
          do n=1,ncross
            numpos(8,n)=numpos(3,n)
            numpos(9,n)=numpos(4,n)
          end do
          do i=1,npinp
            posmap(1,i)=gxypos(i,1)
            posmap(2,i)=gxypos(i,2)
          end do
 	end if
c
      elseif(igtype.gt.0 .and. igtype.le.6) then
c
c..a polarstereographic grid
        igtyp2=1
        grid2(1)=0.
        grid2(2)=0.
        grid2(5)=60.
        grid2(6)=0.
        if(igtype.eq.5 .or. igtype.eq.6) then
c..mercator or lambert (but is this right???)
          grid2(3)=79.*150./(min(grid(3),grid(4))*1000.)
          grid2(4)=grid(1)
        else
c..geographic(2) or speherical rotated(3)
c..(1 degree resolution gives a 100km polarstereographic grid)
          grid2(3)=79.*150./(min(grid(3),grid(4))*100.)
c.old     grid2(4)=grid(5)
c..find longitude of centre grid point, and set rotation
	  gwork(1,1)=(1+nx)*0.5
	  gwork(1,2)=(1+ny)*0.5
          call xyconvert(1,gwork(1,1),gwork(1,2),
     +                   igtype,grid,igeogrid,geogrid,ierror)
          if(ierror.ne.0) write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
          if(ierror.ne.0) stop 255
	  i=nint(gwork(1,1)/90.)
          grid2(4)=real(i)*90.
          if(grid(6).lt.0.) then
            igtyp2=4
            grid2(5)=-60.
          end if
        end if
c..find area limits
        np=0
        do j=1,ny,ny-1
          do i=1,nx
            gwork(np+i,1)=i
            gwork(np+i,2)=j
          end do
          np=np+nx
        end do
        do i=1,nx,nx-1
          do j=1,ny
            gwork(np+j,1)=i
            gwork(np+j,2)=j
          end do
          np=np+ny
        end do
        call xyconvert(np,gwork(1,1),gwork(1,2),
     +                 igtype,grid,igtyp2,grid2,ierror)
        if(ierror.ne.0) write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
        if(ierror.ne.0) stop 255
        xmin=gwork(1,1)
        xmax=gwork(1,1)
        ymin=gwork(1,2)
        ymax=gwork(1,2)
        do i=2,np
          xmin=min(xmin,gwork(i,1))
          xmax=max(xmax,gwork(i,1))
          ymin=min(ymin,gwork(i,2))
          ymax=max(ymax,gwork(i,2))
        end do
        igsiz2(1)=nint(xmin-1.5)
        igsiz2(2)=nint(xmax+1.5)
        igsiz2(3)=nint(ymin-1.5)
        igsiz2(4)=nint(ymax+1.5)
c
        if(isline.eq.1) then
c..straight lines on presentation map (polarstereographic)
c..curves in input grid
          do i=1,npinp
            gxypos(i,1)=geopos(i,1)
            gxypos(i,2)=geopos(i,2)
          end do
          call xyconvert(npinp,gxypos(1,1),gxypos(1,2),
     +                   igeogrid,geogrid,igtyp2,grid2,ierror)
          if(ierror.ne.0) then
            write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
            stop 255
          end if
          do i=1,npinp
            posmap(1,i)=gxypos(i,1)
            posmap(2,i)=gxypos(i,2)
          end do
          do n=1,ncross
            numpos(8,n)=numpos(3,n)
            numpos(9,n)=numpos(4,n)
          end do
          icvpos=1
        elseif(isline.eq.2) then
	  icvpos=3
        else
c..curves on presentation map (polarstereographic)
c..straight lines in input grid,
c..store map positions (curves) later
          icvpos=2
        end if
c
      else
c
        igtyp2=0
        grid2(1)=0.
        grid2(2)=0.
        grid2(3)=0.
        grid2(4)=0.
        grid2(5)=0.
        grid2(6)=0.
        igsiz2(1)=1
        igsiz2(2)=nx
        igsiz2(3)=1
        igsiz2(4)=ny
c
      end if
c
c..positions to interpolate
c
      if(icvpos.ne.3) then
c
        do i=1,npinp
          gwork(i,1)=gxypos(i,1)
          gwork(i,2)=gxypos(i,2)
        end do
c
        ntotal=0
c
        do n=1,ncross
c
          ntotal=ntotal+1
          nfirst=ntotal
          if(ntotal.le.maxpos) then
            i=numpos(3,n)
            gxypos(ntotal,1)=gwork(i,1)
            gxypos(ntotal,2)=gwork(i,2)
          end if
c
          do i=numpos(3,n),numpos(4,n)-1
c
	    posinp(3,i)= real(ntotal)
c
            dx=gwork(i+1,1)-gwork(i,1)
            dy=gwork(i+1,2)-gwork(i,2)
            if(abs(dx).ge.abs(dy)) then
              np=max(nint(abs(dx)),1)
            else
              np=max(nint(abs(dy)),1)
            end if
            nt=ntotal
            ntotal=ntotal+np
            if(ntotal.le.maxpos) then
              dx=dx/real(np)
              dy=dy/real(np)
              do j=1,np-1
                gxypos(nt+j,1)=gwork(i,1)+dx*j
                gxypos(nt+j,2)=gwork(i,2)+dy*j
              end do
              gxypos(ntotal,1)=gwork(i+1,1)
              gxypos(ntotal,2)=gwork(i+1,2)
            end if
c
          end do
c
          numpos(5,n)=ntotal-nfirst+1
          numpos(6,n)=nfirst
          numpos(7,n)=ntotal
c
	  i=numpos(4,n)
	  posinp(3,i)= real(ntotal)
c
        end do
c
      elseif(icvpos.eq.3) then
c
c..straight lines in geographic grid,
c..select points with approx. one gridunit separation
c
        do i=1,npinp
          gwork(i,1)=geopos(i,1)
          gwork(i,2)=geopos(i,2)
        end do
c
        ntotal=0
c
	s=1./real(mgrtest-1)
c
        do n=1,ncross
c
          ntotal=ntotal+1
          nfirst=ntotal
          if(ntotal.le.maxpos) then
            i=numpos(3,n)
            geopos(ntotal,1)=gwork(i,1)
            geopos(ntotal,2)=gwork(i,2)
          end if
c
	  do i=numpos(3,n),numpos(4,n)-1
c
	    posinp(3,i)= real(ntotal)
c
	    dlon=gwork(i+1,1)-gwork(i,1)
	    dlat=gwork(i+1,2)-gwork(i,2)
	    do j=1,mgrtest
	      grtest(j,1)=gwork(i,1)+dlon*s*(j-1)
	      grtest(j,2)=gwork(i,2)+dlat*s*(j-1)
	    end do
            call xyconvert(5,grtest(1,1),grtest(1,2),
     +                     igeogrid,geogrid,igtype,grid,ierror)
            if(ierror.ne.0) then
              write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
              stop 255
            end if
	    r=0.
	    do j=1,mgrtest-1
	      dx=grtest(j+1,1)-grtest(j,1)
	      dy=grtest(j+1,2)-grtest(j,2)
	      r=r+sqrt(dx*dx+dy*dy)
	    end do
	    ngr=max(nint(r),1)
	    dlon=dlon/real(ngr)
	    dlat=dlat/real(ngr)
c
	    if(ntotal+ngr.le.maxpos) then
	      do j=1,ngr
		ntotal=ntotal+1
		geopos(ntotal,1)=gwork(i,1)+dlon*j
		geopos(ntotal,2)=gwork(i,2)+dlat*j
	      end do
	    else
	      ntotal=ntotal+ngr
	    end if
c
	  end do
c
          numpos(5,n)=ntotal-nfirst+1
          numpos(6,n)=nfirst
          numpos(7,n)=ntotal
c
	  i=numpos(4,n)
	  posinp(3,i)= real(ntotal)
c
        end do
c
      end if
c
      if(ntotal.gt.maxpos) then
        write(6,*) 'ERROR: Too much data.   maxpos: ',maxpos
        write(6,*) '                        needed: ',ntotal
        write(6,*) '   Make fewer or shorter crossections.'
        ierr=2
        return
      end if
c
      if(icvpos.eq.1) then
c
c..convert from presentation map to input grid coordinates
        call xyconvert(ntotal,gxypos(1,1),gxypos(1,2),
     +                 igtyp2,grid2,igtype,grid,ierror)
        if(ierror.ne.0) then
          write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
          stop 255
        end if
c
      elseif(icvpos.eq.2) then
c
c..convert from input grid to presentation map coordinates
        do i=1,ntotal
          gwork(i,1)=gxypos(i,1)
          gwork(i,2)=gxypos(i,2)
        end do
        call xyconvert(ntotal,gwork(1,1),gwork(1,2),
     +                 igtype,grid,igtyp2,grid2,ierror)
        if(ierror.ne.0) then
          write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
          stop 255
        end if
        do i=1,ntotal
          posmap(1,i)=gwork(i,1)
          posmap(2,i)=gwork(i,2)
        end do
        do n=1,ncross
          numpos(8,n)=numpos(6,n)
          numpos(9,n)=numpos(7,n)
        end do
c
      elseif(icvpos.eq.3) then
c
c..convert from geographic to presentation map coordinates
        do i=1,ntotal
          gwork(i,1)=geopos(i,1)
          gwork(i,2)=geopos(i,2)
        end do
        call xyconvert(ntotal,gwork(1,1),gwork(1,2),
     +                 igeogrid,geogrid,igtyp2,grid2,ierror)
        if(ierror.ne.0) then
          write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
          stop 255
        end if
        do i=1,ntotal
          posmap(1,i)=gwork(i,1)
          posmap(2,i)=gwork(i,2)
        end do
        do n=1,ncross
          numpos(8,n)=numpos(6,n)
          numpos(9,n)=numpos(7,n)
        end do
c
c..convert from geographic to input grid coordinates
        do i=1,ntotal
          gxypos(i,1)=geopos(i,1)
          gxypos(i,2)=geopos(i,2)
        end do
        call xyconvert(ntotal,gxypos(1,1),gxypos(1,2),
     +                 igeogrid,geogrid,igtype,grid,ierror)
        if(ierror.ne.0) then
          write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
          stop 255
        end if
c
      end if
c
c..check that all positions are inside the field area
      xmin=1.-0.05
      xmax=nx+0.05
      ymin=1.-0.05
      ymax=ny+0.05
      ierror=0
      do n=1,ncross
        do i=numpos(6,n),numpos(7,n)
          if(gxypos(i,1).lt.xmin .or. gxypos(i,1).gt.xmax .or.
     +       gxypos(i,2).lt.ymin .or. gxypos(i,2).gt.ymax) then
            write(6,*) 'ERROR. Position outside field'
            write(6,*) '       Crossection no. ',n
            write(6,*) '       x,y,nx,ny: ',
     +                         gxypos(i,1),gxypos(i,2),nx,ny
            ierror=1
          end if
        end do
      end do
c
      if(ierror.ne.0) stop 255
c
c..convert to geographic longitude,latitude
      do i=1,ntotal
        geopos(i,1)=gxypos(i,1)
        geopos(i,2)=gxypos(i,2)
      end do
      call xyconvert(ntotal,geopos(1,1),geopos(1,2),
     +               igtype,grid,igeogrid,geogrid,ierror)
      if(ierror.ne.0) then
        write(6,*) 'RCINIT: XYCONVERT ERROR: ',ierror
        stop 255
      end if
c
c..find type of hermit (or (bi)linear) interpolation to use
c..(if there are no undefined values in the fields)
c
      precis=1.e-5
      nhermit=0
      lhtype=-1
c
      do i=1,ntotal
c
        ix = int(gxypos(i,1))
        iy = int(gxypos(i,2))
        dx = gxypos(i,1)-real(ix)
        dy = gxypos(i,2)-real(iy)
        ixn = nint(gxypos(i,1))
        iyn = nint(gxypos(i,2))
        xdist = abs(gxypos(i,1)-real(ixn))
        ydist = abs(gxypos(i,2)-real(iyn))
        xlim = gxypos(i,1)*precis
        ylim = gxypos(i,2)*precis
        ixborder = 0
        iyborder = 0
c
        if(ix.lt.1) then
          ix=1
          dx=0.
          xdist=0.
          ixborder=1
        elseif(ix.gt.nx-1) then
          ix=nx-1
          dx=1.
          xdist=0.
          ixborder=1
        elseif(ix.lt.2 .or. ix.gt.nx-2) then
          ixborder=1
        end if
        if(iy.lt.1) then
          iy=1
          dy=0.
          ydist=0.
          iyborder=1
        elseif(iy.gt.ny-1) then
          iy=ny-1
          dy=1.
          ydist=0.
          iyborder=1
        elseif(iy.lt.2 .or. iy.gt.ny-2) then
          iyborder=1
        end if
c
        if(xdist.lt.xlim .and. ydist.lt.ylim) then
          ihtype=0
          ix=ixn
          iy=iyn
          dx=0.
          dy=0.
        elseif(ydist.lt.ylim) then
          ihtype=1
          if(ixborder.eq.1 .or. inter.eq.2) ihtype=6
          iy=iyn
          dy=0.
        elseif(xdist.lt.xlim) then
          ihtype=2
          if(iyborder.eq.1 .or. inter.eq.2) ihtype=7
          ix=ixn
          dx=0.
        else
          ihtype=3
          if(ixborder.eq.0 .and. iyborder.eq.1) ihtype=4
          if(ixborder.eq.1 .and. iyborder.eq.0) ihtype=5
          if(ixborder.eq.1 .and. iyborder.eq.1) ihtype=8
          if(inter.eq.2) ihtype=8
        end if
c
        ihermit(1,i)=ix
        ihermit(2,i)=iy
        dhermit(1,i)=dx
        dhermit(2,i)=dy
c
        if(ihtype.ne.lhtype) then
          lhtype=ihtype
          nhermit=nhermit+1
          khermit(1,nhermit,1)=ihtype
        end if
        khermit(2,nhermit,1)=i
c
      end do
c
      iundef=0
c
c..map ratio x direction
      call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +            pardat(1,1),nx,ny,xmd2h,iundef)
c
c..map ratio y direction
      call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +            pardat(1,2),nx,ny,ymd2h,iundef)
c
c..compute length (in meters) along crossection (increments stored)
      do n=1,ncross
        i1=numpos(6,n)
        i2=numpos(7,n)
	xmm2=pardat(i1,1)
	ymm2=pardat(i1,2)
        pardat(i1,1)=0.
        do i=i1+1,i2
	  xmm1=xmm2
	  ymm1=ymm2
	  xmm2=pardat(i,1)
	  ymm2=pardat(i,2)
          xm=(xmm1+xmm2)*dxgrid
          ym=(ymm1+ymm2)*dygrid
          dx=(gxypos(i-1,1)-gxypos(i,1))*dxgrid/xm
          dy=(gxypos(i-1,2)-gxypos(i,2))*dygrid/ym
          pardat(i,1)=sqrt(dx*dx+dy*dy)
        end do
      end do
c
c..coriolis parameter
      call hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +            pardat(1,2),nx,ny,fcor,iundef)
c
c..compute factors for rotation to E/W and N/S vector components
c..after computation: u(e/w)=+pardat(i,3)*u(x)+pardat(i,4)*v(y)
c..                   v(n/s)=-pardat(i,4)*u(x)+pardat(i,3)*v(y)
c..(this is the same sign-system as used in vcross for Vt,Vn components)
      do i=1,ntotal
        pardat(i,3)=1.
        pardat(i,4)=0.
      end do
      undef=+1.e+35
      call uvconvert(ntotal,geopos(1,1),geopos(1,2),
     +               pardat(1,3),pardat(1,4),
     +               igtype,grid,igeogrid,geogrid,undef,ierror)
      if(ierror.ne.0) then
        write(6,*) 'RCINIT: UVCONVERT ERROR: ',ierror
        stop 255
      end if
      do i=1,ntotal
        pardat(i,4)=-pardat(i,4)
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine hermit(nhermit,khermit,ntotal,ihermit,dhermit,
     +                  result,nx,ny,f,iundef)
c
c       hermit interpolation ('splines' whenever possible)
c
c  input:
c     khermit(1,n,1) : hermit interpolation type,
c                        0 = gridpoint
c                        1 = y constant, 4 points used (hermit interp.)
c                        2 = x constant, 4 points used (hermit interp.)
c                        3 = 4*4 points used    (double hermit interp.)
c                        4 = 4*2 points used    (hermit+linear interp.)
c                        5 = 2*4 points used    (hermit+linear interp.)
c                        6 = y constant, 2 points used (linear interp.)
c                        7 = x constant, 2 points used (linear interp.)
c                        8 = 2*2 points used         (bilinear interp.)
c                        9 = no interpolation possible (outside/undef.)
c     khermit(2,n,1) : last position no. (for current type)
c               n=1,nhermit
c     khermit(1,n,2) : work space (used if iundef>0)
c     khermit(2,n,2) : work space (used if iundef>0)
c     ihermit(1,i)   : i
c     ihermit(2,i)   : j
c     dhermit(1,i)   : dx
c     dhermit(2,i)   : dy
c               i=1,total_no_of_positions
c
      include 'vcdata.inc'
c
      integer nhermit,ntotal,nx,ny,iundef
      integer khermit(2,maxpos,2)
      integer ihermit(2,ntotal)
      real    dhermit(2,ntotal),result(ntotal),f(nx,ny)
c
      real    ftmp(4)
c
      kh=1
      nherm=nhermit
c
      if(iundef.gt.0) then
c
c..check undefined values in the field,
c..and possibly choose another interpolation type
c..(set type = 9 if interpolation not possible)
c
        kh=2
        nherm=0
c
        undef=+1.e+35
        ud=0.9*undef
        lhtype=-1
        n2=0
c
        do nh=1,nhermit
c
          n1    =n2+1
          ihtype=khermit(1,nh,1)
          n2    =khermit(2,nh,1)
c
          if(ihtype.eq.0) then
c
            if(ihtype.ne.lhtype) then
              lhtype=ihtype
              nherm=nherm+1
              khermit(1,nherm,2)=ihtype
            end if
            khermit(2,nherm,2)=n2
c
          elseif(ihtype.eq.1) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i-1,j),f(i,j),f(i+1,j),f(i+2,j)).lt.ud) then
                ihtype=1
              elseif(max(f(i,j),f(i+1,j)).lt.ud) then
                ihtype=6
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.2) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i,j-1),f(i,j),f(i,j+1),f(i,j+2)).lt.ud) then
                ihtype=2
              elseif(max(f(i,j),f(i,j+1)).lt.ud) then
                ihtype=7
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.3) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i-1,j-1),f(i,j-1),f(i+1,j-1),f(i+2,j-1),
     +               f(i-1,j  ),f(i,j  ),f(i+1,j  ),f(i+2,j  ),
     +               f(i-1,j+1),f(i,j+1),f(i+1,j+1),f(i+2,j+1),
     +               f(i-1,j+2),f(i,j+2),f(i+1,j+2),f(i+2,j+2))
     +                                                   .lt.ud) then
                ihtype=3
              elseif(max(f(i-1,j  ),f(i,j  ),f(i+1,j  ),f(i+2,j  ),
     +                   f(i-1,j+1),f(i,j+1),f(i+1,j+1),f(i+2,j+1))
     +                                                   .lt.ud) then
                ihtype=4
              elseif(max(f(i,j-1),f(i+1,j-1),f(i,j  ),f(i+1,j  ),
     +                   f(i,j+1),f(i+1,j+1),f(i,j+2),f(i+1,j+2))
     +                                                   .lt.ud) then
                ihtype=5
              elseif(max(f(i,j),f(i+1,j),f(i,j+1),f(i+1,j+1))
     +                                                   .lt.ud) then
                ihtype=8
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.4) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i-1,j  ),f(i,j  ),f(i+1,j  ),f(i+2,j  ),
     +               f(i-1,j+1),f(i,j+1),f(i+1,j+1),f(i+2,j+1))
     +                                                   .lt.ud) then
                ihtype=4
              elseif(max(f(i,j),f(i+1,j),f(i,j+1),f(i+1,j+1))
     +                                                   .lt.ud) then
                ihtype=8
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.5) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i,j-1),f(i+1,j-1),f(i,j  ),f(i+1,j  ),
     +               f(i,j+1),f(i+1,j+1),f(i,j+2),f(i+1,j+2))
     +                                                   .lt.ud) then
                ihtype=5
              elseif(max(f(i,j),f(i+1,j),f(i,j+1),f(i+1,j+1))
     +                                                   .lt.ud) then
                ihtype=8
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.6) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i,j),f(i+1,j)).lt.ud) then
                ihtype=6
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.7) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i,j),f(i,j+1)).lt.ud) then
                ihtype=7
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.8) then
c
            do n=n1,n2
              i=ihermit(1,n)
              j=ihermit(2,n)
              if(max(f(i,j),f(i+1,j),f(i,j+1),f(i+1,j+1)).lt.ud) then
                ihtype=8
              else
                ihtype=9
              end if
              if(ihtype.ne.lhtype) then
                lhtype=ihtype
                nherm=nherm+1
                khermit(1,nherm,2)=ihtype
              end if
              khermit(2,nherm,2)=n
            end do
c
          elseif(ihtype.eq.9) then
c
            if(ihtype.ne.lhtype) then
              lhtype=ihtype
              nherm=nherm+1
              khermit(1,nherm,2)=ihtype
            end if
            khermit(2,nherm,2)=n2
c
          end if
c
c.......end do nh=1,nhermit
        end do
c
c.....end if(iundef.gt.0) then
      end if
c
c..interpolation............................................
c
      n2=0
c
      do nh=1,nherm
c
        n1    =n2+1
        ihtype=khermit(1,nh,kh)
        n2    =khermit(2,nh,kh)
c
        if(ihtype.eq.0) then
c
c..gridpoints (no interpolation)
          do n=n1,n2
            i=ihermit(1,n)
            j=ihermit(2,n)
            result(n)=f(i,j)
          end do
c
        elseif(ihtype.eq.1) then
c
c..hermit interpolation (4 points), y=constant
          do n=n1,n2
            i=ihermit(1,n)
            j=ihermit(2,n)
            d=dhermit(1,n)
            df1=0.5*(f(i+1,j)-f(i-1,j))
            df2=0.5*(f(i+2,j)-f(i,j))
            ca=2.*(f(i,j)-f(i+1,j))+df1+df2
            cb=3.*(f(i+1,j)-f(i,j))-2.*df1-df2
            result(n)=f(i,j)+d*(df1+d*(cb+d*ca))
          end do
c
        elseif(ihtype.eq.2) then
c
c..hermit interpolation (4 points), x=constant
          do n=n1,n2
            i=ihermit(1,n)
            j=ihermit(2,n)
            d=dhermit(2,n)
            df1=0.5*(f(i,j+1)-f(i,j-1))
            df2=0.5*(f(i,j+2)-f(i,j))
            ca=2.*(f(i,j)-f(i,j+1))+df1+df2
            cb=3.*(f(i,j+1)-f(i,j))-2.*df1-df2
            result(n)=f(i,j)+d*(df1+d*(cb+d*ca))
          end do
c
        elseif(ihtype.eq.3) then
c
c..double hermit interpolation (4*4 points), first y=constant
          do n=n1,n2
            i =ihermit(1,n)
            jy=ihermit(2,n)
            d =dhermit(1,n)
            dy=dhermit(2,n)
            jt=0
            do j=jy-1,jy+2
              df1=0.5*(f(i+1,j)-f(i-1,j))
              df2=0.5*(f(i+2,j)-f(i,j))
              ca=2.*(f(i,j)-f(i+1,j))+df1+df2
              cb=3.*(f(i+1,j)-f(i,j))-2.*df1-df2
              jt=jt+1
              ftmp(jt)=f(i,j)+d*(df1+d*(cb+d*ca))
            end do
            df1=0.5*(ftmp(3)-ftmp(1))
            df2=0.5*(ftmp(4)-ftmp(2))
            ca=2.*(ftmp(2)-ftmp(3))+df1+df2
            cb=3.*(ftmp(3)-ftmp(2))-2.*df1-df2
            result(n)=ftmp(2)+dy*(df1+dy*(cb+dy*ca))
          end do
c
        elseif(ihtype.eq.4) then
c
c..hermit(y=const.) + linear interpolation (4*2 points)
          do n=n1,n2
            i =ihermit(1,n)
            jy=ihermit(2,n)
            d =dhermit(1,n)
            dy=dhermit(2,n)
            jt=0
            do j=jy,jy+1
              df1=0.5*(f(i+1,j)-f(i-1,j))
              df2=0.5*(f(i+2,j)-f(i,j))
              ca=2.*(f(i,j)-f(i+1,j))+df1+df2
              cb=3.*(f(i+1,j)-f(i,j))-2.*df1-df2
              jt=jt+1
              ftmp(jt)=f(i,j)+d*(df1+d*(cb+d*ca))
            end do
            result(n)=ftmp(1)+dy*(ftmp(2)-ftmp(1))
          end do
c
        elseif(ihtype.eq.5) then
c
c..hermit(x=const.) + linear interpolation (2*4 points)
          do n=n1,n2
            ix=ihermit(1,n)
            j =ihermit(2,n)
            dx=dhermit(1,n)
            d =dhermit(2,n)
            it=0
            do i=ix,ix+1
              df1=0.5*(f(i,j+1)-f(i,j-1))
              df2=0.5*(f(i,j+2)-f(i,j))
              ca=2.*(f(i,j)-f(i,j+1))+df1+df2
              cb=3.*(f(i,j+1)-f(i,j))-2.*df1-df2
              it=it+1
              ftmp(it)=f(i,j)+d*(df1+d*(cb+d*ca))
            end do
            result(n)=ftmp(1)+dx*(ftmp(2)-ftmp(1))
          end do
c
        elseif(ihtype.eq.6) then
c
c..2 point linear interpolation, y=constant
          do n=n1,n2
            i=ihermit(1,n)
            j=ihermit(2,n)
            d=dhermit(1,n)
            result(n)=f(i,j)+d*(f(i+1,j)-f(i,j))
          end do
c
        elseif(ihtype.eq.7) then
c
c..2 point linear interpolation, x=constant
          do n=n1,n2
            i=ihermit(1,n)
            j=ihermit(2,n)
            d=dhermit(2,n)
            result(n)=f(i,j)+d*(f(i,j+1)-f(i,j))
          end do
c
        elseif(ihtype.eq.8) then
c
c..2*2 point bilinear interpolation
          do n=n1,n2
            i =ihermit(1,n)
            j =ihermit(2,n)
            d =dhermit(1,n)
            dy=dhermit(2,n)
            ft1=f(i,j)+d*(f(i+1,j)-f(i,j))
            ft2=f(i,j+1)+d*(f(i+1,j+1)-f(i,j+1))
            result(n)=ft1+dy*(ft2-ft1)
          end do
c
        elseif(ihtype.eq.9) then
c
          do n=n1,n2
            result(n)=undef
          end do
c
        end if
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c       do n=n1,n2
c	  write(91,6601) nh,ihtype,n,ihermit(1,n),ihermit(2,n),
c    +			 dhermit(1,n),dhermit(2,n),result(n),
c    +			 f(ihermit(1,n),ihermit(2,n))
c6601	  format(1x,i4,i3,i5,2i4,1x,2f8.4,1x,2f15.8)
c	end do
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
c.....end do nh=1,nherm
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine comput(ipar,u,v,fout,xmd2h,ymd2h,f,nx,ny,iundef,undef)
c
c        ipar=-1: vorticity  (input: u(nx,ny) v(nx,ny)
c            =-2: divergence ( -------- " ---------- )
c
c        output: fout(nx,ny)
c
      integer nx,ny,iundef
      real    undef
      real    u(nx,ny),v(nx,ny),fout(nx,ny)
      real    xmd2h(nx,ny),ymd2h(nx,ny),f(nx,ny)
c
      if(iundef.eq.0) then
c
c..no undefined values in input fields
c
        if(ipar.eq.-1) then
c..vorticity
          do j=2,ny-1
            do i=2,nx-1
              fout(i,j)= xmd2h(i,j)*(v(i+1,j)-v(i-1,j))
     *                  -ymd2h(i,j)*(u(i,j+1)-u(i,j-1))
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c	      write(92,fmt='(1x,''V '',2i4,1x,4f10.5,e16.9)')
c    +			i,j,u(i,j+1),u(i,j-1),v(i+1,j),v(i-1,j),
c    +			fout(i,j)
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            end do
          end do
        elseif(ipar.eq.-2) then
c..divergence
          do j=2,ny-1
            do i=2,nx-1
              fout(i,j)= xmd2h(i,j)*(u(i+1,j)-u(i-1,j))
     *                  +ymd2h(i,j)*(v(i,j+1)-v(i,j-1))
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c	      write(92,fmt='(1x,''D '',2i4,1x,4f10.5,e16.9)')
c    +			i,j,u(i,j+1),u(i,j-1),v(i+1,j),v(i-1,j),
c    +			fout(i,j)
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            end do
          end do
        else
          write(6,*) '***comput*** unknown parameter: ',ipar
          stop 117
        end if
c
      else
c
c..undefined values in input fields
c
        ud=0.9*undef
        do j=1,ny
          do i=1,nx
            fout(i,j)=undef
          end do
        end do
        if(ipar.eq.-1) then
c..vorticity
          do j=2,ny-1
            do i=2,nx-1
              if(max(v(i-1,j),v(i+1,j),u(i,j-1),u(i,j+1)).lt.ud)
     *          fout(i,j)= xmd2h(i,j)*(v(i+1,j)-v(i-1,j))
     *                    -ymd2h(i,j)*(u(i,j+1)-u(i,j-1))
            end do
          end do
        elseif(ipar.eq.-2) then
c..divergence
          do j=2,ny-1
            do i=2,nx-1
              if(max(u(i-1,j),u(i+1,j),v(i,j-1),v(i,j+1)).lt.ud)
     *          fout(i,j)= xmd2h(i,j)*(u(i+1,j)-u(i-1,j))
     *                    +ymd2h(i,j)*(v(i,j+1)-v(i,j-1))
            end do
          end do
        else
          write(6,*) '***comput*** unknown parameter: ',ipar
          stop 117
        end if
c
      end if
c
c..values on lateral boundaries (not computed above)
      do i=2,nx-1
        fout(i, 1)=fout(i,   2)
        fout(i,ny)=fout(i,ny-1)
      end do
      do j=1,ny
        fout(1, j)=fout(   2,j)
        fout(nx,j)=fout(nx-1,j)
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine gwindp(z,ug,vg,xmd2h,ymd2h,f,nx,ny,ivcoor,iundef,undef)
c
c  compute geostrophic wind from height in one pressure level
c  (ug = -g/f * dz/dy     vg = +g/f * dz/dx)
c  or
c  compute geostrophic wind from Montgomery potential in one theta level
c  (ug = -1/f * dM/dy     vg = +1/f * dM/dx)
c
      integer nx,ny,ivcoor,iundef
      real    undef
      real    z(nx,ny),ug(nx,ny),vg(nx,ny)
      real    xmd2h(nx,ny),ymd2h(nx,ny),f(nx,ny)
c
      g=9.8
c
c..isentropic (theta) levels, M=cp*T+g*z
      if(ivcoor.eq.4) g=1.0
c
      if(iundef.eq.0) then
c
c..no undefined values in the input field
        do j=2,ny-1
          do i=1,nx
            ug(i,j)=-g*ymd2h(i,j)*(z(i,j+1)-z(i,j-1))/f(i,j)
          end do
        end do
        do i=1,nx
          ug(i,1) =2.*ug(i,2)   -ug(i,3)
          ug(i,ny)=2.*ug(i,ny-1)-ug(i,ny-2)
        end do
        do j=1,ny
          do i=2,nx-1
            vg(i,j)=+g*xmd2h(i,j)*(z(i+1,j)-z(i-1,j))/f(i,j)
          end do
        end do
        do j=1,ny
          vg(1,j) =2.*vg(2,j)   -vg(3,j)
          vg(nx,j)=2.*vg(nx-1,j)-vg(nx-2,j)
        end do
c
      else
c
c..undefined values in the input field
        ud=0.9*undef
        do j=1,ny
          do i=1,nx
            ug(i,j)=undef
            vg(i,j)=undef
          end do
        end do
        do j=2,ny-1
          do i=1,nx
            if(max(z(i,j+1),z(i,j-1)).lt.ud)
     *        ug(i,j)=-g*ymd2h(i,j)*(z(i,j+1)-z(i,j-1))/f(i,j)
          end do
        end do
        do i=1,nx
          if(max(ug(i,2),ug(i,3)).lt.ud)
     *      ug(i,1)=2.*ug(i,2)-ug(i,3)
          if(max(ug(i,ny-1),ug(i,ny-2)).lt.ud)
     *      ug(i,ny)=2.*ug(i,ny-1)-ug(i,ny-2)
        end do
        do j=1,ny
          do i=2,nx-1
            if(max(z(i+1,j),z(i-1,j)).lt.ud)
     *        vg(i,j)=+g*xmd2h(i,j)*(z(i+1,j)-z(i-1,j))/f(i,j)
          end do
        end do
        do j=1,ny
          if(max(vg(2,j),vg(3,j)).lt.ud)
     *      vg(1,j)=2.*vg(2,j)-vg(3,j)
          if(max(vg(nx-1,j),vg(nx-2,j)).lt.ud)
     *      vg(nx,j)=2.*vg(nx-1,j)-vg(nx-2,j)
        end do
c
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine gwinds(nx,ny,nl,nlev,ivcoor,alvls,blvls,
     +                  ps,th1,th3,th5,z2,z3,ug,vg,pi,pb,
     +			xmd2h,ymd2h,f)
c
c        compute geostrophic wind (ug and vg) in one sigma/eta level
c
c
c	  levels: 5 - full level, th3 input (th3=th2 for last level)
c		  4 - half level,            height computed
c		  3 - full level, th2 input, ug,vg computed (the level)
c		  2 - half level, height input
c		  1 - full level, th1 input (th1=th2 for first level)
c
c         input.  ps:     surface pressure
c                 th1:    pot.temp. at level 1
c                 th3:    pot.temp. at level 3
c                 th5:    pot.temp. at level 5
c                 z2:     z at level 2
c                 z3:     (nothing)
c                 ug:     (nothing)
c                 vg:     (nothing)
c                 pi(,1): (nothing)
c                 pi(,2): (nothing)
c                 pi(,3): exner function level 1 (except first level)
c                 pi(,4): exner function level 2 (except first level)
c                 pi(,5): exner function level 3 (except first level)
c                 pb:     pressure at top of pbl, used if ivcoor=22
c
c         output. ps:     surface pressure
c                 th1:    pot.temp. at level 3 (next level 1)
c                 th3:    pot.temp. at level 5 (next level 3)
c                 th5:    pot.temp. at level 5
c                 z2:     z at level 4 (next level 2)
c                 z3:     z at level 3
c                 ug:     ug
c                 vg:     vg
c                 pi(,1): exner function level 1
c                 pi(,2): exner function level 2
c                 pi(,3): exner function level 3 (next level 1)
c                 pi(,4): exner function level 4 (next level 2)
c                 pi(,5): exner function level 5 (next level 3)
c                 pb:     pressure at top of pbl, used if ivcoor=22
c
c         ug = -xm/f * (g * dz/dy + th * dpi/dy)
c         vg = +ym/f * (g * dz/dx + th * dpi/dx)
c
      common/tab/tpi(128),pbas,pinc,te(41),tbas,tinc
c
      integer nx,ny,nl,nlev,ivcoor
      real    alvls(5),blvls(5)
      real    ps(nx,ny),th1(nx,ny),th3(nx,ny),th5(nx,ny)
      real    z2(nx,ny),z3(nx,ny),ug(nx,ny),vg(nx,ny)
      real    pi(nx,ny,5)
      real    pb(nx,ny)
      real    xmd2h(nx,ny),ymd2h(nx,ny),f(nx,ny)
c
      g=9.8
c
      pbase=-pbas
      pinci=1./pinc
      ginv=1./g
c
      if(nl.eq.1) then
	k1=2
	k2=5
      else
	do k=1,3
	  do j=1,ny
	    do i=1,nx
	      pi(i,j,k)=pi(i,j,k+2)
	    end do
	  end do
	end do
	k1=4
	k2=5
      end if
      if(nl.eq.nlev) k2=4
c
      if(ivcoor.eq.2 .or. ivcoor.eq.10) then
c
	do k=k1,k2
	  do j=1,ny
	    do i=1,nx
	      p=alvls(k)+blvls(k)*ps(i,j)
	      x=(p+pbase)*pinci
	      lx=x
	      pi(i,j,k)=tpi(lx)+(x-lx)*(tpi(lx+1)-tpi(lx))
	    end do
	  end do
	end do
c
      elseif(ivcoor.eq.22) then
c
cpb--------------------------------------------------------
c..sigma 0-2.
c..sigma<1: p=pt+sigma*(pb(i,j)-pt)
c..sigma>1: p=pb(i,j)+(sigma-1)*(ps(i,j)-pb(i,j))
c..( sigma=blvls()  pt=alvls() )
c
	do k=k1,k2
	  if(blvls(k).gt.1.) then
	    cpt=0.
	    cpb=1.-(blvls(k)-1.)
	    cps=blvls(k)-1.
	  else
	    cpt=(1.-blvls(k))*alvls(k)
	    cpb=blvls(k)
	    cps=0.
	  end if
	  do j=1,ny
	    do i=1,nx
	      p=cpt+cpb*pb(i,j)+cps*ps(i,j)
	      x=(p+pbase)*pinci
	      lx=x
	      pi(i,j,k)=tpi(lx)+(x-lx)*(tpi(lx+1)-tpi(lx))
	    end do
	  end do
	end do
cpb--------------------------------------------------------
c
      else
c
        write(6,*) '**gwinds** unknown vertical coordinate: ',ivcoor
        stop 117
c
      end if
c
      if(nl.eq.1) then
	do j=1,ny
	  do i=1,nx
	    pi(i,j,1)=pi(i,j,3)
	  end do
	end do
      end if
      if(nl.eq.nlev) then
	do j=1,ny
	  do i=1,nx
	    pi(i,j,5)=pi(i,j,3)
	  end do
	end do
      end if
c
c..compute height at full level
c
      do j=1,ny
	do i=1,nx
c..height at bottom of layer (surface if first layer)
	  zbot=z2(i,j)
c..thickness of layer (from half level to half level)
          dz=th3(i,j)*(pi(i,j,2)-pi(i,j,4))*ginv
c..height at top of layer (half level)
          z2(i,j)=zbot+dz
c..a very simple 'interpolation'
ccc       z3(i,j)=zbot+dz*0.5
c
c..linear interpolation of height is not good (=> T=const. in layer),
c..so we make a first guess of a temperature profile to comp. height
          dthdpi=(th5(i,j)-th1(i,j))/(pi(i,j,5)-pi(i,j,1))
c..get pot.temperature at half level (bottom of layer)
          thbot=th3(i,j)-dthdpi*(pi(i,j,3)-pi(i,j,2))
c..thickness from half level to full level
          dz=(thbot+th3(i,j))*0.5*(pi(i,j,2)-pi(i,j,3))*ginv
c..height at full level
          z3(i,j)=zbot+dz
c
	end do
      end do
c
c..compute geostrophic wind at full level
c..(extrapolation near boundaries)
c
      do j=2,ny-1
        do i=1,nx
          ug(i,j)=-ymd2h(i,j)*(g*(z3(i,j+1)-z3(i,j-1))
     *                   +th3(i,j)*(pi(i,j+1,3)-pi(i,j-1,3)))/f(i,j)
        end do
      end do
      do i=1,nx
        ug(i, 1)=2.*ug(i,   2)-ug(i,   3)
        ug(i,ny)=2.*ug(i,ny-1)-ug(i,ny-2)
      end do
c
      do j=1,ny
        do i=2,nx-1
          vg(i,j)=+xmd2h(i,j)*(g*(z3(i+1,j)-z3(i-1,j))
     *                   +th3(i,j)*(pi(i+1,j,3)-pi(i-1,j,3)))/f(i,j)
        end do
      end do
      do j=1,ny
        vg(1, j)=2.*vg(   2,j)-vg(   3,j)
        vg(nx,j)=2.*vg(nx-1,j)-vg(nx-2,j)
      end do
c
c..prepare for next level (if not last level)
c
      if(nl.ne.nlev) then
	do j=1,ny
	  do i=1,nx
	    th1(i,j)=th3(i,j)
	    th3(i,j)=th5(i,j)
	  end do
	end do
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine tabdef
c
c     defines block tab
c
c     tpi(n) exner-function at pressure pbas+n*pinc
c     te(n) saturation water vapor pressure at temperature tbas+n*tinc
c
      common/tab/tpi(128),pbas,pinc,te(41),tbas,tinc
c
      dimension ewt(41)
c
c..saturation water vapor pressure at:
c..      -100,-95,-90,...,95,100 deg.celsius
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
c..te = eps*ewt (eps=0.622): t=-100 - +100 deg.c.
      eps=0.622
      tinc=5.
      tbas=173.16-tinc
      do l=1,41
        te(l)=ewt(l)*eps
      end do
c
c..exner function, pi: p=0-1270 mb
      cp=1004.
      rcp=287./cp
      cpi1=cp/1000.**rcp
      pinc=10.
      pbas=0.-pinc
      do l=1,128
        p=pbas+l*pinc
        tpi(l)=cpi1*p**rcp
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine potv(u1,v1,th1,u2,v2,th2,u3,v3,th3,ps,pv,
     *                xmd2h,ymd2h,f,nx,ny,alvls,blvls)
c
c       compute potential vorticity.
c
c       sigma/eta levels.
c
c       u,v,th fields: 3 levels (1,2,3 in 'lower to upper' sequence)
c       computing pv in level 2.
c       (for levels k=1 and k=ks: copy fields from the
c        closest level to the level 'k=0' or 'k=ks+1')
c
      integer nx,ny
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
c..1 pvu = 1.e-6 m**2 s**-1 k kg**-1 ....... 1 hPa = 100 Pa
c
      pvu = 1.e-6
      g=9.8
c
      pvscal=-g/pvu/100.
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
c**********************************************************************
c
      subroutine horvort(nx,ny, z1,u1,v1, w2, z3,u3,v3,
     +			 xmd2h,ymd2h, hvx,hvy)
c
c       compute horizontal vorticity components in x and y direction
c
c	hvx = dW/dy - dV/dz
c	hvy = dU/dz - dW/dx
c
c       fields in 3 levels (1,2,3 in 'lower to upper' sequence)
c       computing hvx,hvy in level 2.
c       (for levels k=1 and k=kk: copy fields from the
c        closest level to the level 'k=0' or 'k=kk+1')
c	z:     height(m)
c	u,v,w: vind components(m/s)
c
      integer nx,ny
      real    z1(nx,ny),u1(nx,ny),v1(nx,ny)
      real    w2(nx,ny)
      real    z3(nx,ny),u3(nx,ny),v3(nx,ny)
      real    xmd2h(nx,ny),ymd2h(nx,ny)
      real    hvx(nx,ny),hvy(nx,ny)
c
c###################################################################
c     hvxmin= +1.e+35
c     hvxmax= -1.e+35
c     hvymin= +1.e+35
c     hvymax= -1.e+35
c###################################################################
      do j=2,ny-1
        do i=2,nx-1
c
          dz= z3(i,j)-z1(i,j)
c
          hvx(i,j) = ymd2h(i,j)*(w2(i,j+1)-w2(i,j-1))
     +              - (v3(i,j)-v1(i,j))/dz
c
          hvy(i,j) = (u3(i,j)-u1(i,j))/dz
     +		    - xmd2h(i,j)*(w2(i+1,j)-w2(i-1,j))
c
c###################################################################
c	  if(hvxmin.gt.hvx(i,j)) hvxmin=hvx(i,j)
c	  if(hvxmax.lt.hvx(i,j)) hvxmax=hvx(i,j)
c	  if(hvymin.gt.hvy(i,j)) hvymin=hvy(i,j)
c	  if(hvymax.lt.hvy(i,j)) hvymax=hvy(i,j)
c###################################################################
        end do
      end do
c###################################################################
c     write(6,*) '   hvxmin,hvxmax: ',hvxmin,hvxmax
c     write(6,*) '   hvymin,hvymax: ',hvymin,hvymax
c###################################################################
c
c..lateral boundaries (no values computed)
      do j=2,ny-1
        hvx( 1,j)=hvx(   2,j)
        hvx(nx,j)=hvx(nx-1,j)
        hvy( 1,j)=hvy(   2,j)
        hvy(nx,j)=hvy(nx-1,j)
      end do
      do i=1,nx
        hvx(i, 1)=hvx(i,   2)
        hvx(i,ny)=hvx(i,ny-1)
        hvy(i, 1)=hvy(i,   2)
        hvy(i,ny)=hvy(i,ny-1)
      end do
c
      return
      end
