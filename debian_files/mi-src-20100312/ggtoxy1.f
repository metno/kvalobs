      program ggtoxy1
c
c        fra geografisk grid (lengde,bredde)
c        til polarstereografisk grid (x,y).
c        benytter bessel-interpolasjon i geografisk grid.
c
c        det geografiske gridet kan best$ av en eller flere sektorer.
c        (sektorene sl$s sammen til ett grid for $ gi best mulig
c         interpolasjon)
c
c        nb| det kreves kjennskap til sektorenes 'posisjoner' ved
c            tilberedelse av input-'stack'.
c
c        nb| se p$ resultatet for $ sjekke at resultatet er o.k.
c            (ikke kontroll av 'sektor'-identifikasjon ved input)
c
c-----------------------------------------------------------------------
c     kodeio = 0: skalar-felt, uforandret
c            = 1: input dd (vind-retning), output u (vind-komp. x-retn.)
c            =-1: input ff (vind-styrke),  output v (vind-komp. y-retn.)
c                 (nb| '-1' m$ v#re f@rste parameter etter '1')
c            = 2: input mslp (redusert bakketrykk), output z 1000 mb.
c                 (1 mb = 8 m)
c            = 3: input abs. temp. i gr. kelvin, output pot.temp.(gr.k.)
c                 (nb| benytter p=1000 mb hvis t(mslp) input)
c            = 4: input u (e/w), output u (vind-komp. x-retn.)
c            =-4: input v (n/s), output v (vind-komp. y-retn.)
c                 (nb| '-4' m$ v#re f@rste parameter etter '4')
c            = 5: input t  (k),           output t  (uforandret, k)
c            =-5: input td (duggpunkt,k), output rh (rel. fuktighet, %)
c                 (nb| '-5' m$ v#re f@rste parameter etter '5')
c            = 6: input abs. temp. i gr. kelvin, output temp.(gr.c.)
c            = 7: b@lge-retning, dd:
c                 input:  retning som bºlgene kommer fra (wmo standard)
c                 output: retning som bºlgene g}r mot (oceanografisk
c                         standard, som dnmi's modeller og programmer
c                         benytter).
c-----------------------------------------------------------------------
c
c=====================================================================
c     # .... compile
c     f77 -o ggtoxy1 -o2 ggtoxy1.f -lmi
c     # .... run
c     ggtoxy1 ggtoxy1.input <arguments>
c=====================================================================
c
c---------------------------------------------------------------------
c  DNMI library routines:  RLUNIT
c			   RCOMNT
c			   PRHELP
c			   GETVAR
c			   RFTURBO
c			   MWFTURBO
c---------------------------------------------------------------------
c
c---------------------------------------------------------------------
c  DNMI/FoU  198x - 1992  Anstein Foss ... ibm
c  DNMI/FoU   10.08.1992  Anstein Foss ... unix
c  DNMI/FoU   26.08.1992  Anstein Foss
c  DNMI/FoU   26.08.1992  Anstein Foss
c  DNMI/FoU   21.09.1993  Anstein Foss
c  DNMI/FoU   24.01.1994  Anstein Foss
c  DNMI/FoU   21.03.1994  Anstein Foss
c  DNMI/FoU   24.10.2003  Anstein Foss ... SGI+Linux version, auto swap
c---------------------------------------------------------------------
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for ggtoxy1.f
ccc   parameter (maxij=32746,maxgi=32746,maxlon=365,maxlat=91)
ccc   parameter (maxtim=40,maxlev=30,maxpar=15)
ccc   parameter (maxflt=6,maxgrd=8,maxsec=20,maxsin=8,maxcom=4)
ccc   parameter (maxfil=8)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'ggtoxy1.inc'
c
      common/a1/itimi(2,maxtim),itimo(2,maxtim),
     *          ilevi(2,maxlev),ilevo(2,maxlev),
     *          ipari(2,maxpar),iparo(2,maxpar),
     *          kodeio(maxpar),iskalo(maxpar),interp(maxpar),
     *          iflim(maxpar),flim(2,maxpar),
     *          nfdef(3,maxflt),ffdef(4,maxflt),
     *          ngdef(6,maxgrd),ggdef(4,maxgrd),
     *          nsect(5,maxsec),isect(6,2,maxsec),
     *          cfelt(maxflt),cgrid(maxgrd)
      character*16 cfelt,cgrid
c
      common/g1/ggrid1(maxlon,maxlat),ggrid2(maxlon,maxlat)
c
      common/g2/identg(20),igridg(maxgi)
      integer*2 identg,igridg
c
      common/g3/coslon(maxlon,maxcom),sinlon(maxlon,maxcom)
c
      common/f1/xlon(maxij,maxcom),ylat(maxij,maxcom),
     *          intxy(maxij,maxcom),nintxy(3,maxcom)
c
      common/f2/felt1(maxij),felt2(maxij)
c
      common/f3/ident(20),ifelt(maxij)
      integer*2 ident,ifelt
c
      dimension ierr(3),ihelpr(6,maxsec)
      integer*2 idrec1r(1024),idrec2r(1024),innhr(16*64,maxsec)
      integer*2 in(16),ino(16)
      integer*2 idateg(12),idatef(12)
      dimension icgrid(maxcom),grid(4),kread(maxsec,maxpar),icount(5,2)
      character*16 cfelti,cgridi(maxcom*2)
      character*80 tekst
      integer      ifiles(maxfil),iucomb(maxgrd)
      character*256 files(maxfil),filein,fileot
      character*256 finput,timlim,prtfil,cinput
c
      logical swapfile
c
c..record length in unit bytes (machine dependant)
      call rlunit(lrunit)
c
      undef=+1.e+35
c
      limitg=20+maxgi
c
      do ns=1,maxsec
        do i=1,6
          ihelpr(i,ns)=0
	end do
      end do
c
c
      lfilei=0
      lfileo=0
      icfelt=0
      do 14 nc=1,maxcom
   14 icgrid(nc)=0
c
      kwintb=0
c
      iuinp=8
      line=0
      iprhlp=0
c
c-----------------------------------------------------------------
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) ' usage: ggtoxy1 <ggtoxy1.input>'
        write(6,*) '    or: ggtoxy1 <ggtoxy1.input> <arguments>'
        write(6,*) '    or: ggtoxy1 <ggtoxy1.input> \?    (to get help)'
        write(6,*)
        stop 1
      end if
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error: ',finput(1:lenstr(finput,1))
        stop 1
      end if
c
      if(narg.eq.2) then
        call getarg(2,cinput)
        if(cinput.eq.'?') then
          call prhelp(iuinp,'*=>')
          close(iuinp)
          stop 1
        end if
      end if
c
c..read comment lines
      call rcomnt(iuinp,'*>','*',line,ierror)
      if(ierror.ne.0) then
        write(6,*) 'error reading comment lines.'
	goto 155
      end if
c
c..file_units and file_names
      do n=1,maxfil
        line=line+1
        read(iuinp,*,err=155) ifiles(n),files(n)
        if(ifiles(n).le.0) goto 20
      end do
      n=maxfil+1
      read(iuinp,*,err=155) i
      if(i.gt.0) then
        write(6,*) 'too many files.  max ("maxfil"): ',maxfil
        stop 117
      endif
   20 nfiles=n-1
c
c..read 1 comment lines
      line=line+1
      read(iuinp,*,err=155)
c
c..'min,max' forecast length in hours
      line=line+1
      read(iuinp,*,err=155) timlim
c
c..read 1 comment lines
      line=line+1
      read(iuinp,*,err=155)
c
c..info_print, 'file_name'
      line=line+1
      read(iuinp,*,err=155) info,prtfil
c
c
c..check if input as environment variables or command line arguments
c
      iprhlp=0
c
      do n=1,nfiles
	if(iprhlp.eq.0) then
          call getvar(1,files(n),1,1,1,ierror)
          if(ierror.ne.0) iprhlp=1
	end if
      end do
c
      if(iprhlp.eq.0) then
        call getvar(1,timlim,1,1,1,ierror)
        if(ierror.ne.0) iprhlp=1
      end if
c
      if(iprhlp.eq.0) then
        call getvar(1,prtfil,1,1,1,ierror)
        if(ierror.ne.0) iprhlp=1
      end if
c
      if(iprhlp.eq.1) then
        write(6,*) 'help from ''ggtoxy1.input'' file:'
        call prhelp(iuinp,'*=>')
	close(iuinp)
	stop 1
      end if
c
c..get the min and max forecast length in hours
      read(timlim,*,iostat=ios,err=40) itim1,itim2
      if(itim1.gt.itim2) goto 40
      goto 45
   40 write(6,*) '*** not correct min and max forecast length (hours):'
      write(6,*) '*** ',timlim
      stop 117
   45 continue
c
      if(info.ne.1) then
        iunit=6
      elseif(prtfil(1:1).eq.'*') then
        iunit=6
      else
        iunit=9
        open(iunit,file=prtfil,
     *             access='sequential',form='formatted',
     *                      status='unknown',iostat=ios)
        if(ios.ne.0) iunit=6
      endif
c
      if(info.lt.0) info=0
      if(info.gt.1) info=1
      if(iunit.lt.1 .or. iunit.gt.99) iunit=6
c
c        definisjoner for input (og sektor-kombinasjon)
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) nngrid
c
      if(nngrid.lt.1 .or. nngrid.gt.maxgrd) then
      write(6,*) '********** siste input-linje:',line
      write(6,*) '*** ikke o.k.    antall ''sektor-kombinasjoner'''
      write(6,*) '*** antall input: ',nngrid
      write(6,*) '***      max o.k: ',maxgrd
      nngrid=1
      write(6,*) '***      min o.k: ',nngrid
      write(6,*) '*** evt. forandring i programmet:  maxgrd ***'
      stop 1001
      endif
c
      nnsec=0
c
      do 100 n=1,nngrid
      line=line+1
      read(iuinp,*,err=155)
c        'gnavn',nsec, nlon,nlat, glon1,glat1,dlon,dlat, igloni,iglati
      line=line+1
      read(iuinp,*,err=155) cgrid(n),nsec,nlon,nlat,(ggdef(i,n),i=1,4),
     *                      (ngdef(i,n),i=3,4)
c
      if(nlon.lt.2 .or. nlon.gt.maxlon .or.
     *   nlat.lt.2 .or. nlat.gt.maxlat) then
      write(6,*) '********** siste input-linje:',line
      write(6,*) '*** ikke o.k:  ',cgrid(n)
      write(6,*) '*** nlon,nlat: ',nlon,nlat
      write(6,*) '***   max o.k: ',maxlon,maxlat
      nlon=2
      nlat=2
      write(6,*) '***   min o.k: ',nlon,nlat
      write(6,*) '*** evt. forandring i programmet:  maxlon,maxlat ***'
      stop 1001
      endif
c
      ngdef(1,n)=nlon
      ngdef(2,n)=nlat
      ngdef(5,n)=nnsec+1
      ngdef(6,n)=nnsec+nsec
c
      if(nsec.lt.1 .or. nnsec.gt.maxsec) then
      write(6,*) '********** siste input-linje:',line
      write(6,*) '*** ikke o.k.    antall sektorer input'
      write(6,*) '*** antall input:   ',nsec
      write(6,*) '*** totalt antall:  ',nnsec+nsec
      write(6,*) '*** totalt max o.k: ',maxsec
      nsec=1
      write(6,*) '***        min o.k: ',nsec
      write(6,*) '*** evt. forandring i programmet:  maxsec ***'
      stop 1001
      endif
c
      line=line+1
      read(iuinp,*,err=155)
      do 110 ns=nnsec+1,nnsec+nsec
      line=line+1
      read(iuinp,*,err=155) nsect(1,ns),((isect(i,j,ns),i=1,6),j=1,2),
     *                     (nsect(i,ns),i=3,5)
      nsect(2,ns)=1
      if(isect(1,2,ns).gt.0) nsect(2,ns)=2
  110 continue
      nnsec=nnsec+nsec
  100 continue
c
c  test if 'sector' combinations don't fill the defined geographic grid
c
      do n=1,nngrid
	nlon=ngdef(1,n)
	nlat=ngdef(2,n)
	do ilat=1,nlat
	  do ilon=1,nlon
	    ggrid1(ilon,ilat)=1.
	  end do
	end do
	nsec1=ngdef(5,n)
	nsec2=ngdef(6,n)
	do ns=ngdef(5,n),ngdef(6,n)
	  do nsp=1,nsect(2,ns)
	    iglon=isect(1,nsp,ns)-1
	    iglat=isect(2,nsp,ns)-1
	    ilon1=isect(3,nsp,ns)
	    ilon2=isect(4,nsp,ns)
	    ilat1=isect(5,nsp,ns)
	    ilat2=isect(6,nsp,ns)
	    ilons=+1
	    if(ilon1.gt.ilon2) ilons=-1
	    ilats=+1
	    if(ilat1.gt.ilat2) ilats=-1
	    lat=iglat
	    do ilat=ilat1,ilat2,ilats
	      lat=lat+1
	      lon=iglon
	      do ilon=ilon1,ilon2,ilons
		lon=lon+1
		ggrid1(lon,lat)=0.
	      end do
	    end do
	  end do
	end do
	usum=0.
	do ilat=1,nlat
	  do ilon=1,nlon
	    usum=usum+ggrid1(ilon,ilat)
	  end do
	end do
	iucomb(n)=0
	if(usum.gt.0.) iucomb(n)=1
      end do
c
c        definisjoner for output
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) nnfelt
c
      if(nnfelt.lt.1 .or. nnfelt.gt.maxflt) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** ikke o.k.    antall output grid'
        write(6,*) '*** antall input: ',nnfelt
        write(6,*) '***      max o.k: ',maxflt
        nnfelt=1
        write(6,*) '***      min o.k: ',nnfelt
        write(6,*) '*** evt. forandring i programmet:  maxflt ***'
        stop 1001
      end if
c
      line=line+1
      read(iuinp,*,err=155)
      do 120 n=1,nnfelt
        line=line+1
        read(iuinp,*,err=155) cfelt(n),nogrid,ii,jj,(ffdef(i,n),i=1,4)
        nfdef(1,n)=nogrid
        nfdef(2,n)=ii
        nfdef(3,n)=jj
c
        if(ii*jj.gt.maxij) then
          iijj=ii*jj
          write(6,*) '********** siste input-linje:',line
          write(6,*) '*** ikke o.k.    antall punkt i output grid'
          write(6,*) '***  nogrid,ii,jj: ',nogrid,ii,jj
          write(6,*) '***         ii*jj: ',iijj
          write(6,*) '***       max o.k: ',maxij
          write(6,*) '*** evt. forandring i programmet:  maxij ***'
          stop 1001
        end if
  120 continue
c
  140 continue
c
c        loop start
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) tekst
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) noprod,ifilei,ifileo,kdate
c
c        loop end ?
c
      if(noprod.lt.1 .or. ifilei.lt.1 .or. ifileo.lt.1) then
        noprod=0
        ifilei=0
        ifileo=0
        goto 160
      endif
c
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) ncomb,(cgridi(i),i=1,ncomb)
c
      if(ncomb.gt.maxcom) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** ikke o.k.    antall ''sektor-kombinasjoner'' ',
     *                               'for et felt'
        write(6,*) '*** antall input: ',ncomb
        write(6,*) '***      max o.k: ',maxcom
        write(6,*) '*** evt. forandring i programmet:  maxcom ***'
        noprod=0
        ifilei=0
        ifileo=0
        goto 160
      endif
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) cfelti
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) ntim,((itimi(i,n),i=1,2),n=1,ntim)
      line=line+1
      read(iuinp,*,err=155)      ((itimo(i,n),i=1,2),n=1,ntim)
c
      if(ntim.gt.maxtim) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** ikke o.k.    antall prognose-tider'
        write(6,*) '*** antall input: ',ntim
        write(6,*) '***      max o.k: ',maxtim
        write(6,*) '*** evt. forandring i programmet:  maxtim ***'
        noprod=0
        ifilei=0
        ifileo=0
        goto 160
      end if
c
      k=0
      do 145 n=1,ntim
        if(itimi(2,n).ne.itimo(2,n)) k=1
  145 continue
      if(k.eq.1) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** advarsel.  spesifikasjon av prognose-tider'
        do 146 n=1,ntim
          write(6,*) '*** input,output: ',itimi(2,n),itimo(2,n)
  146   continue
      end if
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) nlev,((ilevi(i,n),i=1,2),n=1,nlev)
      line=line+1
      read(iuinp,*,err=155)      ((ilevo(i,n),i=1,2),n=1,nlev)
c
      if(nlev.gt.maxlev) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** ikke o.k.    antall niv$'
        write(6,*) '*** antall input: ',nlev
        write(6,*) '***      max o.k: ',maxlev
        write(6,*) '*** evt. forandring i programmet:  maxlev ***'
        noprod=0
        ifilei=0
        ifileo=0
        goto 160
      end if
c
      line=line+1
      read(iuinp,*,err=155)
      line=line+1
      read(iuinp,*,err=155) npar
      line=line+1
      read(iuinp,*,err=155)
c
      if(npar.gt.maxpar) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** ikke o.k.    antall parametre'
        write(6,*) '*** antall input: ',npar
        write(6,*) '***      max o.k: ',maxpar
        write(6,*) '*** evt. forandring i programmet:  maxpar ***'
        noprod=0
        ifilei=0
        ifileo=0
        goto 160
      end if
c
c        kwin=1:  dd,ff -> u,v
      kwin=0
c
      do 150 n=1,npar
      line=line+1
      read(iuinp,*,err=155) kodeio(n),(ipari(i,n),i=1,2),
     *			              (iparo(i,n),i=1,2),iskalo(n),
     *                      interp(n),iflim(n),(flim(i,n),i=1,2)
      if(kodeio(n).eq.1) kwin=1
      if(kodeio(n).eq.7) kwin=1
  150 continue
c-----------------------------------------------------------------
c
      goto 160
c
c
  155 continue
      write(6,*) '********** siste input-linje:',line
      write(6,*) '*** read error'
      noprod=0
      ifilei=0
      ifileo=0
c
c
  160 if(ifilei.ne.lfilei .and. lfilei.gt.0) then
        close(lfilei)
        lfilei=0
      endif
c
      if(ifileo.ne.lfileo .and. lfileo.gt.0) then
        call mwfturbo(3,fileot,lfileo,0,1,1.,1.,1,ident,ierror)
        lfileo=0
      endif
c
      if(noprod.lt.1) goto 900
c
c
      if(ifilei.ne.lfilei) then
        nfil=0
        do n=1,nfiles
          if(ifiles(n).eq.ifilei .and. nfil.eq.0) nfil=n
        end do
        if(nfil.eq.0) then
          write(6,*) '*** no file name for input file unit ',ifilei
          stop 117
        endif
	filein=files(nfil)
        write(6,*) 'Input file: ',filein(1:lenstr(filein,1))
        open(ifilei,file=filein,
     *              access='direct',form='unformatted',
     *              recl=2048/lrunit,
     *              status='old',iostat=ios,err=910)
        read(ifilei,rec=1,iostat=ios,err=912) idateg
	if(swapfile(-ifilei)) call bswap2(12,idateg)
	idater=0
ccc	if(idateg(1).eq.998 .or. idateg(1).eq.997) idater=999
        do ns=1,maxsec
          do i=1,6
            ihelpr(i,ns)=0
	  end do
        end do
      endif
c
      if(ifileo.ne.lfileo) then
        nfil=0
        do n=1,nfiles
          if(ifiles(n).eq.ifileo .and. nfil.eq.0) nfil=n
        end do
        if(nfil.eq.0) then
          write(6,*) '*** no file name for output file unit ',ifileo
          stop 117
        endif
	fileot=files(nfil)
        write(6,*) 'Output file:'
	write(6,*) fileot
        call mwfturbo(1,fileot,ifileo,0,1,1.,1.,12,idatef,ierror)
	if(ierror.ne.0) goto 920
      endif
c
      if(ifilei.ne.lfilei .or. ifileo.ne.lfileo) then
        write(6,1010) ifilei,(idateg(i),i=5,7)
 1010   format('  input grid-file: ',i3,'   dato/termin: ',3i6)
        write(6,1012) ifileo,(idatef(i),i=5,7)
 1012   format(' output felt-file: ',i3,'   dato/termin: ',3i6)
        k=0
        if(idateg(5).ne.idatef(5)) k=1
        if(idateg(6).ne.idatef(6)) k=1
        if(idateg(7).ne.idatef(7)) k=1
        if(k.eq.1) write(6,*) '   ****** ikke samme dato/termin *****'
        if(k.eq.1 .and. kdate.ne.0) stop 1001
      endif
c
      lfilei=ifilei
      lfileo=ifileo
c
c
      kf=0
      do 170 i=1,nnfelt
      if(cfelti.eq.cfelt(i)) goto 172
  170 continue
      write(6,*) '********** siste input-linje:',line
      write(6,*) '*** ikke definert output: ',cfelti
      stop 1001
  172 if(i.ne.icfelt) then
        kf=1
        nogrid=nfdef(1,i)
        ii=nfdef(2,i)
        jj=nfdef(3,i)
        xp=ffdef(1,i)
        yp=ffdef(2,i)
        an=ffdef(3,i)
        fi=ffdef(4,i)
        grid(1)=xp
        grid(2)=yp
        grid(3)=an
        grid(4)=fi
c
        ident15=nint(xp*100.)
        ident16=nint(yp*100.)
c.."mi" standard (150km grid: an=79.)
        ident17=nint((150.*79./an)*10.)
        ident18=nint(fi)
        if(xp.lt.-327. .or. xp.gt.+327. .or.
     +     yp.lt.-327. .or. yp.gt.+327.) then
          ident15= nint(xp)
          ident16= nint(yp)
          ident17=-nint((150.*79./an)*10.)
          ident18= nint(fi)
        end if
c
        icfelt=i
      end if
c
      nsin=0
      do 180 nc=1,ncomb
      kg=0
      do 181 ng=1,nngrid
      if(cgridi(nc).eq.cgrid(ng)) goto 182
  181 continue
      write(6,*) '*** ikke definert input: ',cgridi(nc)
      stop 1001
  182 if(ng.ne.icgrid(nc)) then
      icgrid(nc)=ng
      kg=1
      endif
ccc   if(kg.eq.1 .or. kf.eq.1) write(6,*) '-- nytt felt og/eller grid -'
      if(kg.eq.1 .or. kf.eq.1) call geopos(icfelt,icgrid(nc),nc)
      ns1=ngdef(5,ng)
      ns2=ngdef(6,ng)
      nsin=nsin+(ns2-ns1+1)
  180 continue
c
      if(nsin.gt.maxsin) then
        write(6,*) '********** siste input-linje:',line
        write(6,*) '*** ikke o.k.    antall input sektorer for et felt'
        write(6,*) '*** antall input: ',nsin
        write(6,*) '***      max o.k: ',maxsin
        write(6,*) '*** evt. forandring i programmet:  maxsin ***'
        noprod=0
        ifilei=0
        ifileo=0
        goto 160
      end if
c
c
      if(kwintb.eq.0 .and. kwin.eq.1) then
        call wintab
        kwintb=1
      end if
c
      iijj=ii*jj
c
      write(6,*) tekst
      write(6,6060)
      if(info.eq.1 .and. iunit.ne.6) then
        write(iunit,*) tekst
        write(iunit,6060)
      end if
 6060 format(' antall.  sektorer: benyttet,forkastet,ikke funnet.',
     *               '  felt: ºnsket,laget.')
c
c        icount(1,n): antall sektorer lest
c        icount(2,n): antall sektorer forkastet
c        icount(3,n): antall sektorer ikke funnet
c        icount(4,n): antall felt @nsket
c        icount(5,n): antall felt laget
c                 n=1: en prognose-tid
c                 n=2: alle prognose-tider (en 'loop')
c
      icount(1,2)=0
      icount(2,2)=0
      icount(3,2)=0
      icount(4,2)=0
      icount(5,2)=0
c
      in(1)=noprod
c
      ino(1)=noprod
      ino(2)=nogrid
      ino(15)=1
      ino(16)=0
c
c        time
      do 200 nt=1,ntim
      if(itimi(2,nt).lt.itim1 .or. itimi(2,nt).gt.itim2) goto 200
      ino( 9)=itimo(1,nt)
      ino(10)=itimo(2,nt)
c
      icount(1,1)=0
      icount(2,1)=0
      icount(3,1)=0
      icount(4,1)=0
      icount(5,1)=0
c
c        level
      do 210 nl=1,nlev
      in(13)=ilevi(1,nl)
      in(14)=ilevi(2,nl)
      ino(13)=ilevo(1,nl)
      ino(14)=ilevo(2,nl)
c
c        parameter (output loop)
      do 220 np=1,npar
      icount(4,1)=icount(4,1)+1
      if(kodeio(np).lt.0) goto 220
      np1=np
      np2=np
      do 225 i=1,iijj
  225 felt1(i)=undef
c
      if(kodeio(np).eq.1 .or. kodeio(np).eq.4
     *                   .or. kodeio(np).eq.5) then
        if(np.eq.npar .or. kodeio(np+1).ne.-kodeio(np)) then
          write(6,*) '********** siste input-linje:',line
          write(6,*) '* feil spesifikasjon av ''kode'' / parameter'
          noprod=0
          ifilei=0
          ifileo=0
          goto 160
        endif
        np2=np+1
        do 226 i=1,iijj
  226   felt2(i)=undef
      endif
c
      nfound=0
c
      do 228 nc=1,ncomb
      ng=icgrid(nc)
      ns1=ngdef(5,ng)
      ns2=ngdef(6,ng)
      do 228 npi=np1,np2
      do 228 ns=ns1,ns2
  228 kread(ns,npi)=0
c
c        grid combinations
      nsii=0
      do 230 nc=1,ncomb
      ng=icgrid(nc)
      nlon=ngdef(1,ng)
      nlat=ngdef(2,ng)
      glon1=ggdef(1,ng)
      glat1=ggdef(2,ng)
      dlon=ggdef(3,ng)
      dlat=ggdef(4,ng)
      ns1=ngdef(5,ng)
      ns2=ngdef(6,ng)
      nsi0=nsii
      nsii=nsii+(ns2-ns1+1)
c
c        parameter (input loop)
      do 240 npi=np1,np2
      in(11)=ipari(1,npi)
      in(12)=ipari(2,npi)
c
c        fyller med 'udefinert'
      do 245 lat=1,nlat
      do 245 lon=1,nlon
  245 ggrid1(lon,lat)=undef
c
c        *100: som i '20 ord' identifikasjon .... evt. sjekk av input
ccc   iglon1=glon1*100.+sign(0.5,glon1)
ccc   iglat1=glat1*100.+sign(0.5,glat1)
ccc   idlon=dlon*100.+sign(0.5,dlon)
ccc   idlat=dlat*100.+sign(0.5,dlat)
c
c
      ingrid=0
      iundef=iucomb(ng)
c
c        input grids
      nsi=nsi0
      do 250 ns=ns1,ns2
      nsi=nsi+1
      in(2)=nsect(1,ns)
c
      in( 9)=itimi(1,nt)
      in(10)=itimi(2,nt)
c
      ierr(1)=0
      ierr(2)=idater
      ierr(3)=0
      call rfturbo(ifilei,ip,in,identg,limitg,ierr,ihelpr(1,ns),
     *             idrec1r,idrec2r,innhr(1,ns))
      if(ip.ne.1 .and. in(9).ne.-32767 .and. in(10).eq.0) then
	in(9)=-32767
        ierr(1)=0
        ierr(2)=idater
        ierr(3)=0
        call rfturbo(ifilei,ip,in,identg,limitg,ierr,ihelpr(1,ns),
     *               idrec1r,idrec2r,innhr(1,ns))
      end if
      if(ip.ne.1) then
      iundef=1
      if(info.eq.1)
     *   write(iunit,fmt='('' ikke funnet: '',8i7)') in(1),in(2),
     *                                               (in(i),i=9,14)
      if(ip.ne.0 .or. ierr(1).ne.0 .or. ierr(2).ne.0 .or. ierr(3).ne.0)
     *   write(iunit,fmt='(''    ip='',i4,''   ierr:'',3i9)') ip,ierr
      icount(3,1)=icount(3,1)+1
      goto 250
      endif
c
c        benytter evt. ikke sektoren (sjekker ord 19)
      if(nsect(3,ns).eq.1 .and. nsect(4,ns).eq.identg(19)) then
      if(info.eq.1)
     *   write(iunit,fmt='('' forkastet:   '',8i7)') in(1),in(2),
     *                                               (in(i),i=9,14)
      iundef=1
      icount(2,1)=icount(2,1)+1
      goto 250
      endif
c
      icount(1,1)=icount(1,1)+1
      ingrid=ingrid+1
      kread(ns,npi)=1
c
ccc   klon=identg(10)
ccc   klat=identg(11)
ccc   kglon1=identg(16)
ccc   kglat1=identg(15)
ccc   kdlon=identg(18)
ccc   kdlat=identg(17)
c
ccc   if(kdlon.ne.idlon .or. kdlat.ne.idlat) then
ccc   write(6,*) 'idlon,idlat: ',idlon,idlat
ccc   write(6,*) 'kdlon,kdlat: ',kdlon,kdlat
ccc   stop 117
ccc   endif
c
      nlonin=identg(10)
      nlatin=identg(11)
      iskal=identg(20)
      skal=10.**iskal
c
      nin=nsect(2,ns)
      do 260 n=1,nin
      iglon=isect(1,n,ns)-1
      iglat=isect(2,n,ns)-1
      ilon1=isect(3,n,ns)
      ilon2=isect(4,n,ns)
      ilat1=isect(5,n,ns)
      ilat2=isect(6,n,ns)
      ilons=+1
      if(ilon1.gt.ilon2) ilons=-1
      ilats=+1
      if(ilat1.gt.ilat2) ilats=-1
      lat=iglat
      do 265 ilat=ilat1,ilat2,ilats
      lat=lat+1
      lon=iglon
      ll=(ilat-1)*nlonin
      do 265 ilon=ilon1,ilon2,ilons
      lon=lon+1
      if(igridg(ll+ilon).ne.-32767) then
      ggrid1(lon,lat)=skal*igridg(ll+ilon)
      else
      iundef=1
      endif
  265 continue
  260 continue
c
c        end of grid input loop ...... index: ns
  250 continue
c
      nfound=nfound+ingrid
c
      if(kodeio(npi).eq.1 .or. kodeio(npi).eq.4
     *                    .or. kodeio(npi).eq.5) then
        iundf2=iundef
        ingrd2=ingrid
c          (1) - ggrid1 = dd
c          (4) - ggrid1 = u(e/w)
c          (5) - ggrid1 = t (k)
        do 300 lat=1,nlat
        do 300 lon=1,nlon
  300   ggrid2(lon,lat)=ggrid1(lon,lat)
c          (1) - ggrid2 = dd
c          (4) - ggrid2 = u(e/w)
c          (5) - ggrid2 = t (k)
        goto 240
      endif
c
      if(kodeio(npi).eq.-1 .and. ingrd2.lt.1) ingrid=0
      if(kodeio(npi).eq.-4 .and. ingrd2.lt.1) ingrid=0
      if(kodeio(npi).eq.-5 .and. ingrd2.lt.1) ingrid=0
c
      if(ingrid.lt.1) goto 240
c
      if(kodeio(npi).eq.-1) then
        iundef=iundef+iundf2
c          ggrid1 = ff    ggrid2 = dd
        call ddffuv(nlon,nlat,glon1,glat1,dlon,dlat,fi,iundef,undef)
c          ggrid1 = u     ggrid2 = v
c
      elseif(kodeio(npi).eq.2) then
c          mslp -> z 1000 mb
        if(iundef.eq.0) then
          do 310 lat=1,nlat
          do 310 lon=1,nlon
  310     ggrid1(lon,lat)=8.*(ggrid1(lon,lat)-1000.)
        else
          tundef=0.9*undef
          do 315 lat=1,nlat
          do 315 lon=1,nlon
  315     if(ggrid1(lon,lat).lt.tundef)
     *       ggrid1(lon,lat)=8.*(ggrid1(lon,lat)-1000.)
        endif
c
      elseif(kodeio(npi).eq.3) then
c          abs.temp.(gr.kelvin) -> pot.temp. (gr.kelvin)
        p=ilevo(1,nl)
        c=(1000./p)**0.28585
        if(iundef.eq.0) then
          do 320 lat=1,nlat
          do 320 lon=1,nlon
  320     ggrid1(lon,lat)=c*ggrid1(lon,lat)
        else
          tundef=0.9*undef
          do 325 lat=1,nlat
          do 325 lon=1,nlon
  325     if(ggrid1(lon,lat).lt.tundef)
     *       ggrid1(lon,lat)=c*ggrid1(lon,lat)
        endif
c
      elseif(kodeio(npi).eq.-4) then
        iundef=iundef+iundf2
c          ggrid1 = v(n/s)   ggrid2 = u(e/w)
        call uvgguv(nlon,nlat,nc,glon1,glat1,dlon,dlat,iundef,undef)
c          ggrid1 = u        ggrid2 = v
c
      elseif(kodeio(npi).eq.-5) then
        iundef=iundef+iundf2
c          ggrid1 = td (k)   ggrid2 = t (k)
        call tktdrh(nlon,nlat,iundef,undef)
c          ggrid1 = t (k)    ggrid2 = rh (%)
c
      elseif(kodeio(npi).eq.6) then
c          abs.temp.(gr.kelvin) -> temp. (gr.celsius)
        if(iundef.eq.0) then
          do 330 lat=1,nlat
          do 330 lon=1,nlon
  330     ggrid1(lon,lat)=ggrid1(lon,lat)-273.15
        else
          tundef=0.9*undef
          do 331 lat=1,nlat
          do 331 lon=1,nlon
  331     if(ggrid1(lon,lat).lt.tundef)
     *       ggrid1(lon,lat)=ggrid1(lon,lat)-273.15
        endif
c
      elseif(kodeio(npi).eq.7) then
c..wave direction (using dummy ff)
        do 332 lat=1,nlat
        do 332 lon=1,nlon
        ggrid2(lon,lat)=ggrid1(lon,lat)
  332   ggrid1(lon,lat)=1.
c          ggrid1 = ff    ggrid2 = dd
        call ddffuv(nlon,nlat,glon1,glat1,dlon,dlat,fi,iundef,undef)
c          ggrid1 = u     ggrid2 = v
      endif
c
c        interpolation
c+++++++++++++++++++++++++++
c     call test2(nlon,nlat,undef,fmin,fmax,fmid,nundef)
c     write(6,6002) fmin,fmax,fmid,nundef
c6002 format(' ggrid1.',5x,3f12.4,i6)
c+++++++++++++++++++++++++++
      call georek(ii,jj,nc,interp(npi),iundef,undef)
c
      if(kodeio(npi).eq.-1 .or. kodeio(npi).eq.-4
     *                     .or. kodeio(npi).eq.-5
     *                     .or. kodeio(npi).eq. 7) then
        do 340 i=1,iijj
        feltx=felt1(i)
        felt1(i)=felt2(i)
  340   felt2(i)=feltx
        do 345 lat=1,nlat
        do 345 lon=1,nlon
  345   ggrid1(lon,lat)=ggrid2(lon,lat)
c+++++++++++++++++++++++++++
c       call test2(nlon,nlat,undef,fmin,fmax,fmid,nundef)
c       write(6,6002) fmin,fmax,fmid,nundef
c+++++++++++++++++++++++++++
        call georek(ii,jj,nc,interp(npi),iundef,undef)
        do 350 i=1,iijj
        feltx=felt1(i)
        felt1(i)=felt2(i)
  350   felt2(i)=feltx
      endif
c
c
c        end of parameter loop (input) ....... index: npi
  240 continue
c
c        end of grid combination loop ........ index: nc ...(grid: ng)
  230 continue
c
      if(kodeio(np).eq.1 .or. kodeio(np).eq.4
     *                   .or. kodeio(np).eq.5) then
c        nb| to felt n@dvendig for $ f$ resultat
      nfound=0
      do 354 nc=1,ncomb
      ng=icgrid(nc)
      ns1=ngdef(5,ng)
      ns2=ngdef(6,ng)
      do 354 ns=ns1,ns2
      if(kread(ns,np).eq.0) kread(ns,np+1)=0
      if(kread(ns,np+1).eq.0) kread(ns,np)=0
  354 nfound=nfound+kread(ns,np)
      endif
c
      if(nfound.lt.1) goto 220
c
      if(kodeio(np).eq.7) then
c..wave direction
c..input:  felt1=u  felt2=v
        ffscal=1.
        kundef=1
c..idirec: 0 = wmo definition of direction: 180 degrees is from south
c          1 = oceanographic definition of direction:
c                                           180 degrees is towards south
c..set idirec as used by dnmi models and programs.
        idirec=1
        call uvddff(ii,jj,felt1,felt2,grid,ffscal,kundef,undef,idirec)
c..output: felt1=dd  (felt2=ff)
      endif
c
      iw19=0
      do 355 nc=1,ncomb
      ng=icgrid(nc)
      ns1=ngdef(5,ng)
      ns2=ngdef(6,ng)
      do 355 ns=ns1,ns2
  355 iw19=iw19+nsect(5,ns)*kread(ns,np)
c
      npout1=np
      npout2=np
      if(kodeio(np).eq.1 .or. kodeio(np).eq.4
     *                   .or. kodeio(np).eq.5) npout2=np+1
c
      do npout=npout1,npout2
c
        ino(11)=iparo(1,npout)
        ino(12)=iparo(2,npout)
c
	if(npout.eq.npout1) then
	  continue
	else
          do i=1,iijj
            felt1(i)=felt2(i)
	  end do
	end if
c
        if(info.eq.1) then
          call test1(ii,jj,undef,fmin,fmax,fmid,nundef)
          write(iunit,6001) ino(1),ino(2),(ino(i),i=9,14),
     *                      fmin,fmax,fmid,nundef
 6001     format(' ut:',8i5,3x,3f8.0,i5)
        end if
c
        tundef=0.9*undef
        if(iflim(npout).eq.1 .or. iflim(npout).eq.3) then
	  fmin=flim(1,npout)
          do i=1,iijj
            if(felt1(i).lt.fmin) felt1(i)=fmin
	  end do
	end if
        if(iflim(npout).eq.2 .or. iflim(npout).eq.3) then
	  fmax=flim(2,npout)
          do i=1,iijj
            if(felt1(i).lt.tundef .and.
     *	       felt1(i).gt.fmax) felt1(i)=fmax
	  end do
	end if
c
        if(kodeio(npout).eq.7) then
c..wave direction field, replace 0 with 360 degrees
	  tundef=0.9*undef
          skal=10.**(-iskalo(npout))
          do i=1,iijj
	    if(felt1(i).lt.tundef) then
	      iddout=nint(felt1(i)*skal)
	      if(iddout.eq.0) felt1(i)=felt1(i)+360.
	    end if
          end do
        end if
c
	ident( 1)=ino( 1)
	ident( 2)=ino( 2)
	ident( 3)=ino( 9)
	ident( 4)=ino(10)
	ident( 5)=ino(11)
	ident( 6)=ino(12)
	ident( 7)=ino(13)
	ident( 8)=ino(14)
	ident( 9)=ino(15)
        ident(10)=ii
        ident(11)=jj
        ident(12)=identg(12)
        ident(13)=identg(13)
        ident(14)=identg(14)
        ident(15)=ident15
        ident(16)=ident16
        ident(17)=ident17
        ident(18)=ident18
        ident(19)=iw19
        ident(20)=iskalo(npout)
        lfelt=iijj
	ldata=20+maxij
c
        call mwfturbo(2,fileot,ifileo,2,lfelt,felt1,1.,
     *                  ldata,ident,ierror)
        icount(5,1)=icount(5,1)+1
c
      end do
c
c        end of parameter loop (output) ...... index: np
  220 continue
c
c        end of level loop ................... index: nl
  210 continue
c
      icount(1,2)=icount(1,2)+icount(1,1)
      icount(2,2)=icount(2,2)+icount(2,1)
      icount(3,2)=icount(3,2)+icount(3,1)
      icount(4,2)=icount(4,2)+icount(4,1)
      icount(5,2)=icount(5,2)+icount(5,1)
c
      write(6,6065) itimo(2,nt),(icount(i,1),i=1,5)
      if(info.eq.1 .and. iunit.ne.6)
     *   write(iunit,6065) itimo(2,nt),(icount(i,1),i=1,5)
 6065 format(' tid:',i4,'   sektorer:',3i5,'   felt:',2i5)
c
c        end of time loop .................... index: nt
  200 continue
c
      write(6,6066) (icount(i,2),i=1,5)
      if(info.eq.1 .and. iunit.ne.6)
     *   write(iunit,6066) (icount(i,2),i=1,5)
 6066 format(' total.  ','   sektorer:',3i5,'   felt:',2i5)
c
      goto 140
c
  900 continue
      close(iuinp)
      goto 990
c
  910 write(6,*) 'open error. input grid-file:',ifilei
      write(6,*) '                     iostat:',ios
      stop 1001
  912 write(6,*) 'read error. input grid-file:',ifilei
      write(6,*) '                     iostat:',ios
      stop 1001
  920 write(6,*) 'open error. output felt-file:',ifileo
      write(6,*) '                      iostat:',ios
      stop 1001
  922 write(6,*) 'read error. output felt-file:',ifileo
      write(6,*) '                      iostat:',ios
      stop 1001
c
  990 continue
      end
c
c**********************************************************************
c
      subroutine geopos(ifelt,igrid,icomb)
c
c        beregner l,b (geografiske koord.) i hvert punkt i et x,y-grid,
c        og deretter 'x,y'-posisjon i geografisk grid
c
c
      include 'ggtoxy1.inc'
c
      common/a1/itimi(2,maxtim),itimo(2,maxtim),
     *          ilevi(2,maxlev),ilevo(2,maxlev),
     *          ipari(2,maxpar),iparo(2,maxpar),
     *          kodeio(maxpar),iskalo(maxpar),interp(maxpar),
     *          iflim(maxpar),flim(2,maxpar),
     *          nfdef(3,maxflt),ffdef(4,maxflt),
     *          ngdef(6,maxgrd),ggdef(4,maxgrd),
     *          nsect(5,maxsec),isect(6,2,maxsec),
     *          cfelt(maxflt),cgrid(maxgrd)
      character*16 cfelt,cgrid
c
      common/f1/xlon(maxij,maxcom),ylat(maxij,maxcom),
     *          intxy(maxij,maxcom),nintxy(3,maxcom)
c
      common/g3/coslon(maxlon,maxcom),sinlon(maxlon,maxcom)
c
c
      nf=ifelt
      ng=igrid
      nc=icomb
c
      ii=nfdef(2,nf)
      jj=nfdef(3,nf)
      xp=ffdef(1,nf)
      yp=ffdef(2,nf)
      an=ffdef(3,nf)
      fi=ffdef(4,nf)
c
      nlon=ngdef(1,ng)
      nlat=ngdef(2,ng)
      glon1=ggdef(1,ng)
      glat1=ggdef(2,ng)
      dlon=ggdef(3,ng)
      dlat=ggdef(4,ng)
      igloni=ngdef(3,ng)
      iglati=ngdef(4,ng)
c
      glon2=glon1+(igloni-1)*dlon
      glon3=glon2+360.
c
      om=180./3.1415927
      om2=om*2.
c
      n=-ii
      do 20 j=1,jj
      n=n+ii
      dy=yp-j
      dy2=dy*dy
      do 20 i=1,ii
      dx=i-xp
      rp=sqrt(dx*dx+dy2)
      glat=90.-om2*atan(rp/an)
      glon=0.
      if(rp.gt.1.e-10) glon=fi+om*atan2(dx,dy)
      if(glon.lt.glon2) glon=glon+360.
      if(glon.gt.glon3) glon=glon-360.
      xlon(n+i,nc)=glon
      ylat(n+i,nc)=glat
   20 continue
c
      iijj=ii*jj
      nlonm1=nlon-1
      nlonm2=nlon-2
      nlatm1=nlat-1
      nlatm2=nlat-2
c
      glon0=-(glon1-dlon)
      glat0=-(glat1-dlat)
c
      nintxy(1,nc)=0
      nintxy(2,nc)=0
      nintxy(3,nc)=0
c
c        passer p$ sm$ un@yaktigheter ved beregning av posisjon
      dxy=0.01
      xl1=1-dxy
      xl2=nlon+dxy
      yl1=1-dxy
      yl2=nlat+dxy
c
      do 40 n=1,iijj
      x=(glon0+xlon(n,nc))/dlon
      y=(glat0+ylat(n,nc))/dlat
      i=x
      j=y
      k=1
      if(i.lt.2 .or. i.gt.nlonm2 .or. j.lt.2 .or. j.gt.nlatm2) then
      if(i.lt.1 .and. x.gt.xl1) x=x+dxy
      if(i.gt.nlonm1 .and. x.lt.xl2) x=x-dxy
      if(j.lt.1 .and. y.gt.yl1) y=y+dxy
      if(j.gt.nlatm1 .and. y.lt.yl2) y=y-dxy
      i=x
      j=y
      k=2
      if(i.lt.1 .or. i.gt.nlonm1 .or. j.lt.1 .or. j.gt.nlatm1) k=3
      endif
      xlon(n,nc)=x
      ylat(n,nc)=y
      intxy(n,nc)=k
      nintxy(k,nc)=nintxy(k,nc)+1
   40 continue
c
c..cos(lon),sin(lon)  for  u(e/w),v(n/s) -> u(x),v(y)
      rad=3.1415927/180.
      rot0=rad*(glon1-dlon-fi)
      drot=rad*dlon
      do 60 i=1,nlon
        rot=rot0+i*drot
        coslon(i,nc)=cos(rot)
        sinlon(i,nc)=sin(rot)
   60 continue
c
      return
      end
c
c*********************************************************************
c
      subroutine georek(ii,jj,icomb,interp,iundef,undef)
c
c        interpolasjon.
c
c        interp = 1: bessel-interpolasjon (16 punkts), (bilin. v. kant)
c                 2: biline#r interpolasjon (4 punkts)
c
c        iundef = 0: ingen 'udefinerte' punkt i geografisk grid
c                 1: det finnes 'udefinerte' punkt
c
c
      include 'ggtoxy1.inc'
c
      common/g1/ggrid1(maxlon,maxlat),ggrid2(maxlon,maxlat)
c
      common/f1/xlon(maxij,maxcom),ylat(maxij,maxcom),
     *          intxy(maxij,maxcom),nintxy(3,maxcom)
c
      common/f2/felt1(maxij),felt2(maxij)
c
      iijj=ii*jj
      nc=icomb
c
      if(nintxy(1,nc)+nintxy(2,nc).eq.0) return
c
      if(iundef.gt.0) goto 50
c
      if(interp.eq.2 .or. nintxy(1,nc).eq.0) goto 30
c
      do 10 n=1,iijj
c
      if(intxy(n,nc).eq.1) then
c        bessel-interpolasjon
      x=xlon(n,nc)
      y=ylat(n,nc)
      j=y
      y1=y-j
      i=x
      x1=x-i
      y2=1.-y1
      y3=-0.25*y1*y2
      y4=-0.1666667*y1*y2*(y1-0.5)
      c1=y3-y4
      c2=y2-y3+3.*y4
      c3=y1-y3-3.*y4
      c4=y3+y4
      t1= c1*ggrid1(i-1,j-1)+c2*ggrid1(i-1,j)
     *   +c3*ggrid1(i-1,j+1)+c4*ggrid1(i-1,j+2)
      t2= c1*ggrid1(i,  j-1)+c2*ggrid1(i,  j)
     *   +c3*ggrid1(i,  j+1)+c4*ggrid1(i,  j+2)
      t3= c1*ggrid1(i+1,j-1)+c2*ggrid1(i+1,j)
     *   +c3*ggrid1(i+1,j+1)+c4*ggrid1(i+1,j+2)
      t4= c1*ggrid1(i+2,j-1)+c2*ggrid1(i+2,j)
     *   +c3*ggrid1(i+2,j+1)+c4*ggrid1(i+2,j+2)
      x2=1.-x1
      felt1(n)= x2*t2+x1*t3
     *         -0.25*x2*x1*(t4-t3-t2+t1)
     *         -0.1666667*x2*x1*(x1-0.5)*(t4-3.*t3+3.*t2-t1)
c
      elseif(intxy(n,nc).eq.2) then
c        bilineaer interpolasjon
      x=xlon(n,nc)
      y=ylat(n,nc)
      j=y
      y1=y-j
      i=x
      x1=x-i
      felt1(n)= ggrid1(i,j)+y1*(ggrid1(i,j+1)-ggrid1(i,j))
     *                     +x1*(ggrid1(i+1,j)-ggrid1(i,j))
     *         +x1*y1*(ggrid1(i+1,j+1)-ggrid1(i+1,j)
     *                -ggrid1(i,j+1)+ggrid1(i,j))
      endif
c
   10 continue
c
      return
c
   30 do 40 n=1,iijj
c
      if(intxy(n,nc).ne.3) then
c        bilineaer interpolasjon
      x=xlon(n,nc)
      y=ylat(n,nc)
      j=y
      y1=y-j
      i=x
      x1=x-i
      felt1(n)= ggrid1(i,j)+y1*(ggrid1(i,j+1)-ggrid1(i,j))
     *                     +x1*(ggrid1(i+1,j)-ggrid1(i,j))
     *         +x1*y1*(ggrid1(i+1,j+1)-ggrid1(i+1,j)
     *                -ggrid1(i,j+1)+ggrid1(i,j))
      endif
c
   40 continue
c
      return
c
c        sjekker 'udefinerte' punkt
c
   50 tudef=0.9*undef
c
      if(interp.eq.2 .or. nintxy(1,nc).eq.0) goto 80
c
      do 60 n=1,iijj
c
      if(intxy(n,nc).eq.1) then
c        bessel-interpolasjon
      x=xlon(n,nc)
      y=ylat(n,nc)
      j=y
      i=x
      if(amax1(ggrid1(i,j),ggrid1(i+1,j),ggrid1(i,j+1),ggrid1(i+1,j+1))
     *                                                  .lt.tudef) then
      if(amax1(ggrid1(i-1,j-1),ggrid1(i,j-1),ggrid1(i+1,j-1),
     *         ggrid1(i+2,j-1),ggrid1(i-1,j),ggrid1(i+2,j),
     *         ggrid1(i-1,j+1),ggrid1(i+2,j+1),ggrid1(i-1,j+2),
     *         ggrid1(i,j+2),ggrid1(i+1,j+2),ggrid1(i+2,j+2))
     *                                                  .lt.tudef) then
      y1=y-j
      x1=x-i
      y2=1.-y1
      y3=-0.25*y1*y2
      y4=-0.1666667*y1*y2*(y1-0.5)
      c1=y3-y4
      c2=y2-y3+3.*y4
      c3=y1-y3-3.*y4
      c4=y3+y4
      t1= c1*ggrid1(i-1,j-1)+c2*ggrid1(i-1,j)
     *   +c3*ggrid1(i-1,j+1)+c4*ggrid1(i-1,j+2)
      t2= c1*ggrid1(i,  j-1)+c2*ggrid1(i,  j)
     *   +c3*ggrid1(i,  j+1)+c4*ggrid1(i,  j+2)
      t3= c1*ggrid1(i+1,j-1)+c2*ggrid1(i+1,j)
     *   +c3*ggrid1(i+1,j+1)+c4*ggrid1(i+1,j+2)
      t4= c1*ggrid1(i+2,j-1)+c2*ggrid1(i+2,j)
     *   +c3*ggrid1(i+2,j+1)+c4*ggrid1(i+2,j+2)
      x2=1.-x1
      felt1(n)= x2*t2+x1*t3
     *         -0.25*x2*x1*(t4-t3-t2+t1)
     *         -0.1666667*x2*x1*(x1-0.5)*(t4-3.*t3+3.*t2-t1)
      else
c        bilineaer interpolasjon
      y1=y-j
      x1=x-i
      felt1(n)= ggrid1(i,j)+y1*(ggrid1(i,j+1)-ggrid1(i,j))
     *                     +x1*(ggrid1(i+1,j)-ggrid1(i,j))
     *         +x1*y1*(ggrid1(i+1,j+1)-ggrid1(i+1,j)
     *                -ggrid1(i,j+1)+ggrid1(i,j))
      endif
      endif
c
      elseif(intxy(n,nc).eq.2) then
      x=xlon(n,nc)
      y=ylat(n,nc)
      j=y
      i=x
      if(amax1(ggrid1(i,j),ggrid1(i+1,j),ggrid1(i,j+1),ggrid1(i+1,j+1))
     *                                                  .lt.tudef) then
c        bilineaer interpolasjon
      y1=y-j
      x1=x-i
      felt1(n)= ggrid1(i,j)+y1*(ggrid1(i,j+1)-ggrid1(i,j))
     *                     +x1*(ggrid1(i+1,j)-ggrid1(i,j))
     *         +x1*y1*(ggrid1(i+1,j+1)-ggrid1(i+1,j)
     *                -ggrid1(i,j+1)+ggrid1(i,j))
      endif
      endif
c
   60 continue
c
      return
c
   80 do 90 n=1,iijj
c
      if(intxy(n,nc).ne.3) then
      x=xlon(n,nc)
      y=ylat(n,nc)
      j=y
      i=x
      if(amax1(ggrid1(i,j),ggrid1(i+1,j),ggrid1(i,j+1),ggrid1(i+1,j+1))
     *                                                  .lt.tudef) then
c        bilineaer interpolasjon
      y1=y-j
      x1=x-i
      felt1(n)= ggrid1(i,j)+y1*(ggrid1(i,j+1)-ggrid1(i,j))
     *                     +x1*(ggrid1(i+1,j)-ggrid1(i,j))
     *         +x1*y1*(ggrid1(i+1,j+1)-ggrid1(i+1,j)
     *                -ggrid1(i,j+1)+ggrid1(i,j))
      endif
      endif
c
   90 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine wintab
c
      common/t1/costab(360),sintab(360)
c
      rad=3.1415927/180.
c
      do 10 n=1,360
      alfa=rad*(n-1)
      costab(n)=cos(alfa)
      sintab(n)=sin(alfa)
   10 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine ddffuv(nlong,nlati,glong1,glati1,dlong,dlati,fifelt,
     *                                                  iundef,undef)
c
c        input:  ggrid1 = ff (knots)
c                ggrid2 = dd (0-360 deg.)
c
c        output: ggrid1 = u (m/s)
c                ggrid2 = v (m/s)
c
      include 'ggtoxy1.inc'
c
      common/g1/ggrid1(maxlon,maxlat),ggrid2(maxlon,maxlat)
c
      common/t1/costab(360),sintab(360)
c
      nlon=nlong
      nlat=nlati
      dlon=dlong
      vxr=-(90.+fifelt)+glong1-dlon+4.*360.+0.5
c
ccc   rad=3.1415927/180.
      cfms=1852./3600.
c
      if(iundef.gt.0) goto 30
c
c----------------------------------------------
c        the north pole problem ... (uk '201')
      lat=0
      glat=glati1
      if(glat.gt.90.-0.1*dlati .and. glat.lt.90.+0.1*dlati) lat=1
      glat=glati1+(nlat-1)*dlati
      if(glat.gt.90.-0.1*dlati .and. glat.lt.90.+0.1*dlati) lat=nlat
c
      if(lat.ne.0 .and. iundef.eq.0) then
      do 10 lon=1,nlon
   10 ggrid2(lon,lat)=ggrid2(lon,lat)+180.
c
      elseif(lat.ne.0) then
      udef=undef
      tudef=0.9*udef
      do 15 lon=1,nlon
   15 if(ggrid2(lon,lat).lt.tudef) ggrid2(lon,lat)=ggrid2(lon,lat)+180.
      endif
c----------------------------------------------
c
      do 20 lat=1,nlat
      do 20 lon=1,nlon
      ideg=vxr+lon*dlon-ggrid2(lon,lat)
      ideg=ideg-(ideg/360)*360+1
      ff=cfms*ggrid1(lon,lat)
      ggrid1(lon,lat)=ff*costab(ideg)
      ggrid2(lon,lat)=ff*sintab(ideg)
   20 continue
c
      return
c
   30 udef=undef
      tudef=0.9*udef
c
      do 40 lat=1,nlat
      do 40 lon=1,nlon
      if(ggrid1(lon,lat).lt.tudef .and. ggrid2(lon,lat).lt.tudef) then
      ideg=vxr+lon*dlon-ggrid2(lon,lat)
      ideg=ideg-(ideg/360)*360+1
      ff=cfms*ggrid1(lon,lat)
      ggrid1(lon,lat)=ff*costab(ideg)
      ggrid2(lon,lat)=ff*sintab(ideg)
      else
      ggrid1(lon,lat)=udef
      ggrid2(lon,lat)=udef
      endif
   40 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine uvgguv(nlong,nlati,icomb,glong1,glati1,dlong,dlati,
     *                                                 iundef,undef)
c
c        input:  ggrid1 = v (n/s) (m/s)
c                ggrid2 = u (e/w) (m/s)
c
c        output: ggrid1 = u (x_felt) (m/s)
c                ggrid2 = v (y_felt) (m/s)
c
      include 'ggtoxy1.inc'
c
      common/g1/ggrid1(maxlon,maxlat),ggrid2(maxlon,maxlat)
c
      common/g3/coslon(maxlon,maxcom),sinlon(maxlon,maxcom)
c
      nlon=nlong
      nlat=nlati
      nc=icomb
c
      if(iundef.gt.0) goto 30
c
      do 20 lat=1,nlat
      do 20 lon=1,nlon
        vns=ggrid1(lon,lat)
        uew=ggrid2(lon,lat)
        ggrid1(lon,lat)=uew*coslon(lon,nc)-vns*sinlon(lon,nc)
        ggrid2(lon,lat)=uew*sinlon(lon,nc)+vns*coslon(lon,nc)
   20 continue
c
      return
c
   30 udef=undef
      tudef=0.9*udef
c
      do 40 lat=1,nlat
      do 40 lon=1,nlon
        if(ggrid1(lon,lat).lt.tudef.and.ggrid2(lon,lat).lt.tudef) then
          vns=ggrid1(lon,lat)
          uew=ggrid2(lon,lat)
          ggrid1(lon,lat)=uew*coslon(lon,nc)-vns*sinlon(lon,nc)
          ggrid2(lon,lat)=uew*sinlon(lon,nc)+vns*coslon(lon,nc)
        else
          ggrid1(lon,lat)=udef
          ggrid2(lon,lat)=udef
        endif
   40 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine uvddff(ii,jj,felt1,felt2,grid,ffscal,iundef,undef,
     *                                                      idirec)
c
c       from u(x),v(y) to dd,ff  in polarstereographic grid.
c
c       input:   felt1(ii,jj) : u(x)
c                felt2(ii,jj) : v(x)
c
c       output:  felt1(ii,jj) : dd (direction, 0 - 360 degrees)
c                felt2(ii,jj) : ff (speed)
c
c       idirec: 0 = meteorological (wmo) definition of direction:
c                   180 degrees is from south
c               1 = oceanographic definition of direction:
c                   180 degrees is towards south
c
c
      integer ii,jj
      real    felt1(ii,jj),felt2(ii,jj),grid(4)
c
      ffs=ffscal
c
      xp=grid(1)
      yp=grid(2)
      an=grid(3)
      fi=grid(4)
      om=180./3.1415927
      om2=om*2.
c
      grad=180./3.1415927
      fix=270.-fi
c
c..compute directions according to meteorological definition
c
      if(iundef.eq.0) then
        do 10 j=1,jj
        do 10 i=1,ii
c..from (x,y) to (latitude,longitude)
          x=i
          y=j
          dx=x-xp
          dy=yp-y
          rr=sqrt(dx*dx+dy*dy)
ccc       glat=90.-om2*atan(rr/an)
          glon=0.
          if(rr.gt.1.e-20) glon=fi+om*atan2(dx,dy)
ccc       if(glon.le.-180.) glon=360.+glon
ccc       if(glon.gt. 180.) glon=glon-360.
c..from (u,v) to (dd,ff)
          u=felt1(i,j)
          v=felt2(i,j)
          ff=sqrt(u*u+v*v)
          if(ff.gt.1.e-20) then
            dd=fix+glon-atan2(v,u)*grad
            if(dd.gt.360.) dd=dd-360.
            if(dd.le.  0.) dd=dd+360.
ccc         idd=dd+0.5
ccc         if(idd.eq.0) idd=360
          else
            ff=0.
            dd=0.
ccc         idd=0
          endif
cc        iff=ff+0.5
          felt1(i,j)=dd
          felt2(i,j)=ff*ffs
   10   continue
      else
        udef=undef
        tudef=0.9*udef
        do 20 j=1,jj
        do 20 i=1,ii
          if(felt1(i,j).lt.tudef .and. felt2(i,j).lt.tudef) then
c..from (x,y) to (latitude,longitude)
            x=i
            y=j
            dx=x-xp
            dy=yp-y
            rr=sqrt(dx*dx+dy*dy)
ccc         glat=90.-om2*atan(rr/an)
            glon=0.
            if(rr.gt.1.e-20) glon=fi+om*atan2(dx,dy)
ccc         if(glon.le.-180.) glon=360.+glon
ccc         if(glon.gt. 180.) glon=glon-360.
c..from (u,v) to (dd,ff)
            u=felt1(i,j)
            v=felt2(i,j)
            ff=sqrt(u*u+v*v)
            if(ff.gt.1.e-20) then
              dd=fix+glon-atan2(v,u)*grad
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
ccc           idd=dd+0.5
ccc           if(idd.eq.0) idd=360
            else
              ff=0.
              dd=0.
ccc           idd=0
            endif
cc          iff=ff+0.5
            felt1(i,j)=dd
            felt2(i,j)=ff*ffs
          else
            felt1(i,j)=udef
            felt2(i,j)=udef
          endif
   20   continue
      endif
c
c..possibly correct directions to oceanographic definition
c
      if(idirec.eq.1) then
        if(iundef.eq.0) then
          do 30 j=1,jj
          do 30 i=1,ii
            if(felt1(i,j).gt.0.) then
              felt1(i,j)=felt1(i,j)+180.
              if(felt1(i,j).gt.360.) felt1(i,j)=felt1(i,j)-360.
            endif
   30     continue
        else
          do 40 j=1,jj
          do 40 i=1,ii
            if(felt1(i,j).gt.0. .and. felt1(i,j).lt.tudef) then
              felt1(i,j)=felt1(i,j)+180.
              if(felt1(i,j).gt.360.) felt1(i,j)=felt1(i,j)-360.
            endif
   40     continue
        endif
      endif
c
      return
      end
c
c***********************************************************************
c
      subroutine tktdrh(nlong,nlati,iundef,undef)
c
c        input:  ggrid1 = td (k) ... duggpunkt
c                ggrid2 = t  (k) ... teperatur
c
c        output: ggrid1 = t  (k) ... teperatur (uforandret, men flyttet)
c                ggrid2 = rh (%) ... relativ fuktighet
c
      include 'ggtoxy1.inc'
c
      common/g1/ggrid1(maxlon,maxlat),ggrid2(maxlon,maxlat)
c
      dimension ewt(41)
c        metningstrykk for t=-100,-95,-90,...+100 gr.c.
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
      nlon=nlong
      nlat=nlati
      udef=undef
c
c        tmin/tmax-test tar seg ogs$ av 'udefinerte' punkt
      tmin=-100.+273.15
      tmax=+100.+273.15
c
      help=-273.15+105.
c
      do 20 lat=1,nlat
      do 20 lon=1,nlon
        if(ggrid1(lon,lat).gt.tmin .and. ggrid1(lon,lat).lt.tmax .and.
     *     ggrid2(lon,lat).gt.tmin .and. ggrid2(lon,lat).lt.tmax) then
          x=(ggrid1(lon,lat)+help)*0.2
          l=x
          etd=ewt(l)+(ewt(l+1)-ewt(l))*(x-l)
          x=(ggrid2(lon,lat)+help)*0.2
          l=x
          et=ewt(l)+(ewt(l+1)-ewt(l))*(x-l)
          ggrid1(lon,lat)=ggrid2(lon,lat)
          ggrid2(lon,lat)=100.*etd/et
        else
          ggrid1(lon,lat)=ggrid2(lon,lat)
          ggrid2(lon,lat)=udef
        endif
   20 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine test1(ii,jj,undef,fmin,fmax,fmid,nundef)
c
c
      include 'ggtoxy1.inc'
c
      common/f2/felt1(maxij),felt2(maxij)
c
      double precision zsum
c
      iijj=ii*jj
      tudef=0.9*undef
      nudef=0
      zmin=+1.e+35
      zmax=-1.e+35
      zsum=0.
c
      do 10 i=1,iijj
      if(felt1(i).lt.tudef) then
      if(zmin.gt.felt1(i)) zmin=felt1(i)
      if(zmax.lt.felt1(i)) zmax=felt1(i)
      zsum=zsum+felt1(i)
      else
      nudef=nudef+1
      endif
   10 continue
c
      fmin=zmin
      fmax=zmax
      fmid=+1.e+35
      i=iijj-nudef
      if(i.gt.0) fmid=zsum/i
      nundef=nudef
c
      return
      end
c
c***********************************************************************
c
      subroutine test2(nlong,nlati,undef,fmin,fmax,fmid,nundef)
c
c
      include 'ggtoxy1.inc'
c
      common/g1/ggrid1(maxlon,maxlat),ggrid2(maxlon,maxlat)
c
      double precision zsum
c
      nlon=nlong
      nlat=nlati
      tudef=0.9*undef
      nudef=0
      zmin=+1.e+35
      zmax=-1.e+35
      zsum=0.
c
      do 10 lat=1,nlat
      do 10 lon=1,nlon
      if(ggrid1(lon,lat).lt.tudef) then
      if(zmin.gt.ggrid1(lon,lat)) zmin=ggrid1(lon,lat)
      if(zmax.lt.ggrid1(lon,lat)) zmax=ggrid1(lon,lat)
      zsum=zsum+ggrid1(lon,lat)
      else
      nudef=nudef+1
      endif
   10 continue
c
      fmin=zmin
      fmax=zmax
      fmid=+1.e+35
      i=nlon*nlat-nudef
      if(i.gt.0) fmid=zsum/i
      nundef=nudef
c
      return
      end
