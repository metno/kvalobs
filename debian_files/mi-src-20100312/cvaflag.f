      program cvaflag
c
c        konverter flagg-resultater paa sekvensiell flagg-fil
c        fra analysen til standard flagg-fil.
c
c	 denne versjon kan lese:
c	 1) gamle filer uten interpolerte verdier i gjetnings- og
c	    analyse-felt og med vind som u(x) og v(y), begge med
c	    enhet 1/100 m/s.
c	    grid-parametre er kodet inn for noen grid 1814,1840,1905.
c	 2) nye filer med interpolerte verdier i gjetnings- og
c	    analyse-felt og med vind som dd i enhet 1/10 grad og
c	    ff i enhet 1/10 knop !!???
c
c
c-------------------------------------------------------------------
c      DNMI library subroutines:  rmfile
c                                 rlunit
c                                 daytim
c
c-------------------------------------------------------------------
c  DNMI/FoU  01.02.1994  Anstein Foss
c  DNMI/FoU  27.02.1997  Anstein Foss ... x=y=-32767 & sizes
c-------------------------------------------------------------------
c
c
c.............................................................
c..max size of input file (integer*2 words)
      parameter (maxdat=300000)
c
c..max no. of stations
      parameter (maxsta=15000)
c
c..max no. of observations (data) for one station
      parameter (maxobs=400)
c
c..data types (1=z 2=u 3=v 4=rh 5=dz)
      parameter (maxtyp=5)
c
c..min/max main observation types and instruments allowed
      parameter (minotyp=1,maxotyp=9,mininst=1,maxinst=99)
c
c..no. of sort time intervals
      parameter (nminsort=3)
c
c..max no. of sort cases
      parameter (maxsort=(maxinst-mininst+1)*nminsort)
c
c..length of output sort list element
      parameter(lsorto=20)
c
c..length of index element (latitude,longitude,record_no,word_no)
      parameter(lindx=4)
c
c..length of station header
c..(datalength, obs.type, instrument, latitude, longitude, hour*100+min,
c.. 3 words name (char*6), sort_number, no. of levels, level length)
      parameter(lshead=12)
c
c..max input record length (integer*2)
      parameter (lrecin=2048)
c
c..output record length (integer*2)
      parameter (lrec=1024)
c.............................................................
c
      integer*2 idata(maxdat)
c
      character*6 csta(maxsta)
c
      integer     itimea(5)
c
      integer    minsort(2,nminsort)
      integer    isort(5,maxsort),nsort(18,maxsort)
      integer    njsort(maxsort),jsort(maxtyp,maxsort)
      integer    jsortp(maxtyp,maxsort)
      integer    msort(maxtyp,maxsort)
      integer*2  isorto(lsorto,maxsort)
c
      integer    ipsta(maxsta),konsta(2,maxsta)
      integer    istasort(maxsta)
c
c..iobsp: pressure
c..iobsf: analysis flags
c..iobsv: observed values
c..iobsg: guess field value
c..iobsa: analysis field value
c
      integer*2  iobsp(maxobs)
      integer*2  iobsf(maxtyp,maxobs),iobsv(maxtyp,maxobs)
      integer*2  iobsg(maxtyp,maxobs),iobsa(maxtyp,maxobs)
c
      integer*2  indx(lrec)
      integer*2  iout(lrec*4)
c
      character*256 filnam,filout,flist,flraw
c
      data minsort/-360,-91,  -90,+90,  +91,+360/
c
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
cc fra anao.f (s/r utbrukt):
c....16 ord header:
c     idout( 1)=nrec
c     idout( 2)=lrec
c     idout( 3)=nsta
c     idout( 4)=nobs
c     idout( 5)=lniva
c     idout( 6)=llast
c     idout( 7)=iaar
c     idout( 8)=mnd
c     idout( 9)=idag
c     idout(10)=kl
c     idout(11)=0
c     idout(12)=nrgrid
c     idout(13)=igrid1 ........ -> "xp" ???
c     idout(14)=igrid2 ........ -> "yp" ???
c     idout(15)=igrid3 ........ -> "ds" ???
c     idout(16)=igrid4 ........ -> "fi" ???
c
c....16 ord for hver observasjon
c        ista( 1,n): hovedtype (1-7)
c            ( 2,n): instrumenttype (se filstruktur)
c            ( 3,n): ikode: kode for hvilke data som skrives ut (flagg)
c            ( 4,n): timedifferense ( minutter )
c            ( 5,n): antall obs.data
c            ( 6,n): 0
c            ( 7,n): 0
c            ( 8,n): kjennetegn i karakterformat
c            ( 9,n):          "
c            (10,n):          "
c            (11,n): bredde*100
c            (12,n): lengde*100
c            (13,n): x*100
c            (14,n): y*100
c            (15,n): 0
c            (16,n): 0
c
c....4 ord for hvert p-nivaa ... ('gamle' filer: bare de 4 foerste)
c        iobs( 1,l): p1 (1/10 mb) .......
c            ( 2,l): ikode: kode for hvilke data som skrives ut (flagg)
c            ( 3,l): parameter 1:z, 2:u, 3:v, 4:rh, 5:dz
c            ( 4,l): observert verdi
c            ( 5,l): interpolert verdi i gjetningsfelt
c            ( 6,l): interpolert verdi i analysefelt
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c
c--------------------------------------------
      narg=iargc()
      if(narg.lt.2 .or. narg.gt.4) then
        write(6,*) 'usage: cvaflag analysis_flagfile flagfile'
        write(6,*) '   or: cvaflag analysis_flagfile flagfile listfile'
        write(6,*) '   or: cvaflag analysis_flagfile flagfile listfile',
     *							 ' rawlistfile'
        stop
      endif
      call getarg(1,filnam)
      call getarg(2,filout)
      ilist1=0
      flist='<no_list>'
      if(narg.ge.3) then
        call getarg(3,flist)
	ilist1=1
      end if
      ilist2=0
      flraw='<no_list>'
      if(narg.ge.4) then
        call getarg(4,flraw)
	ilist2=1
      end if
      write(6,*) 'input:  ',filnam
      write(6,*) 'output: ',filout
      write(6,*) 'list:   ',flist
      write(6,*) 'rlist:  ',flraw
c--------------------------------------------
c
c..input and output file unit
      iunit=20
c
      open(iunit,file=filnam,
     *		 access='sequential',form='unformatted',
     *		 status='old',iostat=ios)
c
      if(ios.ne.0) then
	write(6,*) 'open error: ',filnam
	stop 1
      end if
c
      if(ilist1.eq.1) then
        open(9,file=flist,
     *         access='sequential',form='formatted',
     *	       status='unknown',iostat=ios)
      end if
c
      if(ilist2.eq.1) then
        open(8,file=flraw,
     *         access='sequential',form='formatted',
     *	       status='unknown',iostat=ios)
      end if
c
c..read one record
      read(iunit,iostat=ios) (idata(i),i=1,lrecin)
c
      if(ios.ne.0) then
	write(6,*) 'read error. iostat= ',ios
	stop 2
      end if
c
      write(6,*) 'header(16):'
      write(6,fmt='(1x,10i6)') (idata(i),i=1,16)
c
      if(ilist1.eq.1) then
	write(9,*) 'flag file: ',filnam
	write(9,*) 'header(16):'
	write(9,fmt='(1x,10i6)') (idata(i),i=1,16)
      end if
c
      nrec=idata(1)
      lreci=idata(2)
      nsta=idata(3)
      lobs=idata(5)
      llast=idata(6)
      itimea(1)=idata(7)
      itimea(2)=idata(8)
      itimea(3)=idata(9)
      itimea(4)=idata(10)
      itimea(5)=0
      ngrid=idata(12)
c
      lendat=nrec*lreci-lreci+llast
c
      if(lendat.gt.maxdat) then
        write(6,*) '**** input data too long for program'
	write(6,*) '**** lendat,maxdat: ',lendat,maxdat
        stop 2
      end if
c
      if(nsta.gt.maxsta) then
        write(6,*) '**** too many stations for program'
	write(6,*) '**** nsta,maxsta: ',nsta,maxsta
        stop 2
      end if
c
      ih=idata(10)/100
      im=idata(10)-ih*100
      minobs=ih*60+im
      minday=24*60
c
c-old.file---------------------------------------------
      ioldflag=1
      if(lobs.eq.6) then
	ioldflag=0
      elseif(ngrid.eq.1814) then
        idata(13)= 5900
        idata(14)=10900
        idata(15)=  500
        idata(16)=   -5
      elseif(ngrid.eq.1840) then
        idata(13)= 4300
        idata(14)=12100
        idata(15)=  500
        idata(16)=  -32
      elseif(ngrid.eq.1905) then
        idata(13)=10900
        idata(14)=13600
        idata(15)=  500
        idata(16)=    0
      else
	write(6,*) 'ERROR. Unknown grid: ',ngrid
	stop 3
      end if
      if(ioldflag.eq.1) write(6,*) 'Old version flag file'
c-old.file---------------------------------------------
c
c-old.file---------------------------------------------
      if(ioldflag.eq.1) then
c..make grid independant wind components
        if(idata(15).gt.0) then
          xp=idata(13)*0.01
          yp=idata(14)*0.01
c.."mi standard"
          an=79.*150./(idata(15)*0.1)
          fi=idata(16)
        else
          xp=idata(13)
          yp=idata(14)
c.."mi standard"
          an=79.*150./(-idata(15)*0.1)
          fi=idata(16)
        end if
        grad=180./3.141592654
        firot=270.-fi
c..from m/s to knots
        ffscale=3600./1852.
      end if
c-old.file---------------------------------------------
c
      i2=lreci
      do irec=2,nrec
        i1=i2+1
        i2=i2+lreci
        if(i2.gt.lendat) i2=lendat
c
c..read one record
	read(iunit,iostat=ios) (idata(i),i=i1,i2)
c
        if(ios.ne.0) then
	  write(6,*) 'read error. iostat= ',ios
	  write(6,*) 'file: ',filnam
	  stop 2
        end if
c
      end do
c
      close(iunit)
c
c..find pointer to each station (the word before each beginning)
      nobs=0
      ip=16
      do n=1,nsta
	ipsta(n)=ip
        nd=idata(ip+5)
	nobs=nobs+nd
c..x,y-position not used
	idata(ip+13)=-32767
	idata(ip+14)=-32767
        ip=ip+16+nd*lobs
      end do
c
      do n=1,nsta
	ip=ipsta(n)
        id1=idata(ip+ 8)
        id2=idata(ip+ 9)
        id3=idata(ip+10)
        id11=iand(ishft(id1,-8),255)
        id12=iand(id1,255)
        id21=iand(ishft(id2,-8),255)
        id22=iand(id2,255)
        id31=iand(ishft(id3,-8),255)
        id32=iand(id3,255)
        csta(n)=char(id11)//char(id12)//char(id21)//char(id22)
     *                                //char(id31)//char(id32)
      end do
c
c..sort stations by name for final output
      do n=1,nsta
	konsta(1,n)=0
      end do
      do n=1,nsta
	ns=1
	do while (konsta(1,ns).eq.1)
	  ns=ns+1
	end do
	nmin=ns
	do i=ns+1,nsta
	  if(konsta(1,i).eq.0 .and. csta(i).lt.csta(nmin)) nmin=i
	end do
	istasort(n)=nmin
	konsta(1,nmin)=1
      end do
c
      if(ilist2.eq.1) then
c..raw print of analysis flag file
	write(8,*) 'flag file: ',filnam
	write(8,*) 'nsta,nobs: ',nsta,nobs
	write(8,*) 'header(16):'
	write(8,fmt='(1x,10i6)') (idata(i),i=1,16)
	do n=1,nsta
	  konsta(1,n)=0
	end do
	do n=1,nsta
	if(konsta(1,n).eq.0) then
	  ip=ipsta(n)
	  write(8,*)
	  write(8,fmt='(1x,a6,1x,16i6)') csta(n),(idata(i),i=ip+1,ip+16)
	  ipn=ip
	  nnn=0
	  do m=n,nsta
	    ip=ipsta(m)
	    k=1
	    if(idata(ip+1).ne.idata(ipn+1)) k=0
	    if(idata(ip+2).ne.idata(ipn+2)) k=0
	    if(idata(ip+4).ne.idata(ipn+4)) k=0
	    do i=8,14
	      if(idata(ip+i).ne.idata(ipn+i)) k=0
	    end do
	    if(k.eq.1) then
	      nnn=nnn+1
	      konsta(1,n)=nnn
	      nd=idata(ip+5)
	      ip=ip+16
	      do j=1,nd
	        write(8,fmt='(6x,i3,''+ '',10i6)')
     *			      nnn,(idata(i),i=ip+1,ip+lobs)
	        ip=ip+lobs
	      end do
	    end if
	  end do
	end if
        end do
	close(8)
      end if
c
c..count the total no. of stations in the index
      nindx=0
c
c..no. of sort cases
      numsort=0
c
      nlost=0
c
      do ns=1,nsta
	konsta(1,ns)=0
	konsta(2,ns)=0
	ip=ipsta(ns)
	iotyp=idata(ip+1)
	instr=idata(ip+2)
	minut=idata(ip+4)
	nt=0
	do i=1,nminsort
	  if(minut.ge.minsort(1,i) .and. minut.le.minsort(2,i)) nt=i
	end do
	if(iotyp.ge.minotyp .and. iotyp.le.maxotyp .and.
     *	   instr.ge.mininst .and. instr.le.maxinst .and. nt.gt.0) then
	  nso=0
	  do i=1,numsort
	    if(iotyp.eq.isort(1,i) .and. instr.eq.isort(2,i)
     *				   .and.    nt.eq.isort(3,i)) nso=i
	  end do
	  if(nso.eq.0 .and. numsort.lt.maxsort) then
	    numsort=numsort+1
	    isort(1,numsort)=iotyp
	    isort(2,numsort)=instr
	    isort(3,numsort)=nt
	    isort(4,numsort)=0
	    isort(5,numsort)=numsort
	    nso=numsort
	  end if
	  konsta(1,ns)=nso
	end if
	if(konsta(1,ns).eq.0) nlost=nlost+1
      end do
c
      do nso=1,numsort
	n=nso
	do j=nso+1,numsort
	  igt=0
	  do i=1,3
	    if(igt.eq.0 .and. isort(i,j).lt.isort(i,n)) igt=-1
	    if(igt.eq.0 .and. isort(i,j).gt.isort(i,n)) igt=+1
	  end do
	  if(igt.eq.-1) n=j
	end do
	if(n.ne.nso) then
	  do i=1,5
	    ihelp=isort(i,nso)
	    isort(i,nso)=isort(i,n)
	    isort(i,n)=ihelp
	  end do
	end if
      end do
c
      do n=1,numsort
	nt=isort(3,n)
	isort(3,n)=minsort(1,nt)
	isort(4,n)=minsort(2,nt)
      end do
c
      do n=1,numsort
	do m=1,maxtyp
	  msort(m,n)=0
	end do
      end do
c
      do ns=1,nsta
	if(konsta(1,ns).gt.0) then
	  nso=konsta(1,ns)
	  nlvl=0
	  ip1=ipsta(ns)
	  ls=0
	  do is=ns,nsta
	    if(konsta(1,is).eq.nso) then
	      iok=1
	      ip=ipsta(is)
	      if(idata(ip+1).ne.idata(ip1+1)) iok=0
	      if(idata(ip+2).ne.idata(ip1+2)) iok=0
	      if(idata(ip+4).ne.idata(ip1+4)) iok=0
	      do i=8,14
		if(idata(ip+i).ne.idata(ip1+i)) iok=0
	      end do
	      if(iok.eq.1) then
		nlvls=nlvl
	        nd=idata(ip+5)
	        ip=ip+17
	        do id=1,nd
	          l=0
	          do n=1,nlvl
		    if(iobsp(n).eq.idata(ip)) l=n
	          end do
	          if(l.eq.0 .and. nlvl.lt.maxobs) then
		    nlvl=nlvl+1
		    l=nlvl
		    iobsp(l)=idata(ip)
		    do i=1,maxtyp
		      iobsv(i,l)=-32767
		    end do
	          end if
	          if(l.gt.0) then
		    ityp=idata(ip+2)
	            if(ityp.ge.1 .and. ityp.le.maxtyp) then
c..problem: usually aireps with several reports at same time
		      if(iobsv(ityp,l).ne.-32767) iok=0
c..observation value
		      iobsv(ityp,l)=idata(ip+3)
c..count observations of each type
	              msort(ityp,nso)=msort(ityp,nso)+1
		    end if
	          end if
	          ip=ip+lobs
	        end do
		if(ls.eq.0) then
		  ls=is
		elseif(iok.eq.1) then
		  konsta(2,ls)=is
		  konsta(1,is)=-konsta(1,is)
		  ls=is
		else
		  nlvl=nlvls
		end if
	      end if
	    end if
	  end do
c
	  nindx=nindx+1
c
	end if
c
      end do
c
c..output parameter ordering
      do nums=1,numsort
        nso=isort(5,nums)
	do m=1,maxtyp
	  jsort(m,nums)=0
	  jsortp(m,nums)=maxtyp
	end do
	ntyp=0
	do m=1,maxtyp
	  if(msort(m,nso).gt.0) then
	    ntyp=ntyp+1
	    jsort(ntyp,nums)=m
	    jsortp(m,nums)=ntyp
	  end if
	end do
	njsort(nums)=ntyp
      end do
c
      write(6,*) 'numsort,maxsort: ',numsort,maxsort
      write(6,*) 'nindx,nlost:     ',nindx,nlost
c
c
      call rmfile(filout,0,ierror)
c
      call rlunit(lrunit)
c
      open(iunit,file=filout,
     *		 access='direct',form='unformatted',
     *		 recl=lrec*2/lrunit,
     *		 status='unknown',iostat=ios)
c
      if(ios.ne.0) then
	write(6,*) 'open error: ',filout
	stop 1
      end if
c
      do i=1,lrec
	iout(i)=0
      end do
c
      write(iunit,rec=1,iostat=ios,err=910) (iout(i),i=1,lrec)
c
      do n=1,numsort
	nsort(1,n)=0
	nsort(2,n)=+32767
	nsort(3,n)=-32767
	nsort(4,n)=+32767
	nsort(5,n)=-32767
	nsort(6,n)=+32767
	nsort(7,n)=-32767
	nsort(8,n)=+32767
	nsort(9,n)=-32767
	nsort(10,n)=0
	nsort(11,n)=0
	nsort(12,n)=0
	nsort(13,n)=0
	nsort(14,n)=0
	nsort(15,n)=-32767
	nsort(16,n)=-32767
	nsort(17,n)=-32767
	nsort(18,n)=-32767
      end do
c
      do n=1,numsort
	do m=1,maxtyp
	  msort(m,n)=0
	end do
      end do
c
c..length of file header (reserve a whole number of records)
      nwrd=64+numsort*lsorto
      nrech=(nwrd+lrec-1)/lrec
c
c..length of station index (reserve a whole number of records)
      nwrd=nindx*lindx
      nreci=(nwrd+lrec-1)/lrec
c
c..station index pointer (record,word)
      ireci1=nrech+1
      iwrdi1=1
      ireci=ireci1-1
      iwrdi=iwrdi1-1
c
c..data pointer (record,word)
      irecd1=nrech+nreci+1
      iwrdd1=1
      irec=irecd1-1
      iwrd=iwrdd1-1
c
      nsta1=0
      nsta2=0
      nobso=0
c
      do nums=1,numsort
c
c..pointer to station index
	nsort(15,nums)=ireci+1
	nsort(16,nums)=iwrdi+1
c..pointer to data
	nsort(17,nums)=irec+1
	nsort(18,nums)=iwrd+1
c
	nso=isort(5,nums)
	ntyp=njsort(nums)
c
	if(ilist1.eq.1) then
	  write(9,*)
	  write(9,*) '==== nums,nso:      ',nums,nso
	  write(9,*) '==== iotyp,instr:   ',(isort(i,nums),i=1,2)
	  write(9,*) '==== minut1,minut2: ',(isort(i,nums),i=3,4)
	  write(9,*) '==== ntyp: ',ntyp
	  write(9,*) '==== ityp: ',(jsort(i,nums),i=1,ntyp)
	  write(9,*)
	end if
c
c-old.file---------------------------------------------
	ioldwind=0
	if(ioldflag.eq.1) then
c
c..make grid independant wind components
c..input:  ityp=2  u(x)          in unit 1/100 m/s
c..        ityp=3  v(y)          in unit 1/100 m/s
c..output: ityp=2  dd(direction) in unit 1/10  degree
c..        ityp=3  ff(speed)     in unit 1/10  knots
c
	  iu=0
	  iv=0
	  do m=1,ntyp
	    if(jsort(m,nums).eq.2) iu=m
	    if(jsort(m,nums).eq.3) iv=m
	  end do
	  if(iu.gt.0 .and. iv.gt.0) ioldwind=1
	end if
c-old.file---------------------------------------------
c
	do nsout=1,nsta
c
	  ns=istasort(nsout)
c
	  if(konsta(1,ns).eq.nso) then
	    ip=ipsta(ns)
	    nlvl=0
	    ndat1=0
ccc         iminut=minobs+idata(ip+4)
ccc         if(iminut.lt.0)      iminut=iminut+minday
ccc         if(iminut.ge.minday) iminut=iminut-minday
ccc         ih=iminut/60
ccc         im=iminut-ih*60
ccc         ihhmm=ih*100+im
	    ip1=ip
	    nsta1=nsta1+1
	    nsta2=nsta2-1
	    is=ns
c
	    do while (is.gt.0)
c
	      nsta2=nsta2+1
	      ip=ipsta(is)
	      nd=idata(ip+5)
	      ip=ip+17
	      do id=1,nd
	        l=0
	        do n=1,nlvl
		  if(iobsp(n).eq.idata(ip)) l=n
	        end do
	        if(l.eq.0 .and. nlvl.lt.maxobs) then
		  nlvl=nlvl+1
		  l=nlvl
		  iobsp(l)=idata(ip)
		  do i=1,maxtyp
		    iobsf(i,l)=-32767
		    iobsv(i,l)=-32767
		    iobsg(i,l)=-32767
		    iobsa(i,l)=-32767
		  end do
	        end if
	        if(l.gt.0) then
		  ityp=idata(ip+2)
	          if(ityp.ge.1 .and. ityp.le.maxtyp) then
		    m=jsortp(ityp,nums)
c..observation flag and value
		    iobsf(m,l)=idata(ip+1)
		    iobsv(m,l)=idata(ip+3)
c..interpolated values
c-old		    iobsg(m,l)=idata(ip+4)
c-old		    iobsa(m,l)=idata(ip+5)
c-old.file---------------------------------------------
		    if(ioldflag.eq.0) then
		      iobsg(m,l)=idata(ip+4)
		      iobsa(m,l)=idata(ip+5)
		    else
		      iobsg(m,l)=-32767
		      iobsa(m,l)=-32767
		    end if
c-old.file---------------------------------------------
		  end if
	        end if
	        ip=ip+lobs
	      end do
	      ndat1=ndat1+nd
c
	      is=konsta(2,is)
c
	    end do 
c
c..sort the levels (from high to low pressure)
	    do n=1,nlvl-1
	      iprmax=iobsp(n)
	      nmax=n
	      do i=n+1,nlvl
	        if(iobsp(i).gt.iprmax) then
		  iprmax=iobsp(i)
		  nmax=i
	        end if
	      end do
	      if(nmax.ne.n) then
		ihelp=iobsp(nmax)
		iobsp(nmax)=iobsp(n)
		iobsp(n)=ihelp
		do i=1,ntyp
		  ihelp=iobsf(i,nmax)
		  iobsf(i,nmax)=iobsf(i,n)
		  iobsf(i,n)=ihelp
		  ihelp=iobsv(i,nmax)
		  iobsv(i,nmax)=iobsv(i,n)
		  iobsv(i,n)=ihelp
		  ihelp=iobsg(i,nmax)
		  iobsg(i,nmax)=iobsg(i,n)
		  iobsg(i,n)=ihelp
		  ihelp=iobsa(i,nmax)
		  iobsa(i,nmax)=iobsa(i,n)
		  iobsa(i,n)=ihelp
		end do
	      end if
	    end do
c
	    nsort(1,nums)=nsort(1,nums)+1
	    ip=ipsta(ns)
	    iotype=idata(ip+1)
	    instru=idata(ip+2)
	    minute=idata(ip+4)
	    if(iotype.lt.nsort(2,nums)) nsort(2,nums)=iotype
	    if(iotype.gt.nsort(3,nums)) nsort(3,nums)=iotype
	    if(instru.lt.nsort(4,nums)) nsort(4,nums)=instru
	    if(instru.gt.nsort(5,nums)) nsort(5,nums)=instru
	    if(minute.lt.nsort(6,nums)) nsort(6,nums)=minute
	    if(minute.gt.nsort(7,nums)) nsort(7,nums)=minute
	    do n=1,nlvl
	      if(iobsp(n).lt.nsort(8,nums)) nsort(8,nums)=iobsp(n)
	      if(iobsp(n).gt.nsort(9,nums)) nsort(9,nums)=iobsp(n)
	    end do
	    nsort(10,nums)=nsort(10,nums)+ndat1
	    nsort(11,nums)=nsort(11,nums)+nlvl
	    nobso=nobso+nlvl
	    do n=1,nlvl
	      do m=1,maxtyp
	        if(iobsv(m,n).ne.-32767) msort(m,nums)=msort(m,nums)+1
	      end do
	    end do
c
c-old.file---------------------------------------------
	    if(ioldwind.eq.1) then
c
c..make grid independant wind components
c..input:  ityp=2  u(x)          in unit 1/100 m/s
c..        ityp=3  v(y)          in unit 1/100 m/s
c..output: ityp=2  dd(direction) in unit 1/10  degree
c..        ityp=3  ff(speed)     in unit 1/10  knot
c
	      glong=idata(ip+12)*0.01
	      do n=1,nlvl
c..observed values
		if(iobsv(iu,n).ne.-32767 .and.
     *		   iobsv(iv,n).ne.-32767) then
		  u=iobsv(iu,n)*0.01
		  v=iobsv(iv,n)*0.01
		  ff=sqrt(u*u+v*v)*ffscale
		  iff=nint(ff*10.)
		  if(iff.gt.0) then
		    dd=firot+glong-atan2(v,u)*grad
		    idd=nint(dd*10.)
		    if(idd.gt.3600) idd=idd-3600
		    if(idd.le.  00) idd=idd+3600
		  else
		    idd=0
		  end if
		  iobsv(iu,n)=idd
		  iobsv(iv,n)=iff
		end if
c..guesss field values
		if(iobsg(iu,n).ne.-32767 .and.
     *		   iobsg(iv,n).ne.-32767) then
		  u=iobsg(iu,n)*0.01
		  v=iobsg(iv,n)*0.01
		  ff=sqrt(u*u+v*v)*ffscale
		  iff=nint(ff*10.)
		  if(iff.gt.0) then
		    dd=firot+glong-atan2(v,u)*grad
		    idd=nint(dd*10.)
		    if(idd.gt.3600) idd=idd-3600
		    if(idd.le.  00) idd=idd+3600
		  else
		    idd=0
		  end if
		  iobsg(iu,n)=idd
		  iobsg(iv,n)=iff
		end if
c..analysis field values
		if(iobsa(iu,n).ne.-32767 .and.
     *		   iobsa(iv,n).ne.-32767) then
		  u=iobsa(iu,n)*0.01
		  v=iobsa(iv,n)*0.01
		  ff=sqrt(u*u+v*v)*ffscale
		  iff=nint(ff*10.)
		  if(iff.gt.0) then
		    dd=firot+glong-atan2(v,u)*grad
		    idd=nint(dd*10.)
		    if(idd.gt.3600) idd=idd-3600
		    if(idd.le.  00) idd=idd+3600
		  else
		    idd=0
		  end if
		  iobsa(iu,n)=idd
		  iobsa(iv,n)=iff
		end if
	      end do
	    end if
c-old.file---------------------------------------------
c
c..station index (latitude,longitude,record,word)
	    indx(iwrdi+1)=idata(ip+11)
	    indx(iwrdi+2)=idata(ip+12)
	    indx(iwrdi+3)=irec+1
	    indx(iwrdi+4)=iwrd+1
	    iwrdi=iwrdi+4
c
	    ip=ipsta(ns)
	    iwrd1=iwrd+1
	    iout(iwrd+ 1)=0
	    iout(iwrd+ 2)=idata(ip+ 1)
	    iout(iwrd+ 3)=idata(ip+ 2)
	    iout(iwrd+ 4)=idata(ip+11)
	    iout(iwrd+ 5)=idata(ip+12)
ccc	    iout(iwrd+ 6)=ihhmm
	    iout(iwrd+ 6)=idata(ip+ 4)
	    iout(iwrd+ 7)=idata(ip+ 8)
	    iout(iwrd+ 8)=idata(ip+ 9)
	    iout(iwrd+ 9)=idata(ip+10)
	    iout(iwrd+10)=nums
	    iout(iwrd+11)=nlvl
	    iout(iwrd+12)=1+ntyp*4
	    iwrd=iwrd+lshead
	    do n=1,nlvl
	      iwrd=iwrd+1
	      iout(iwrd)=iobsp(n)
	      do m=1,ntyp
		iout(iwrd+m)=iobsf(m,n)
	      end do
	      iwrd=iwrd+ntyp
	      do m=1,ntyp
		iout(iwrd+m)=iobsv(m,n)
	      end do
	      iwrd=iwrd+ntyp
	      do m=1,ntyp
		iout(iwrd+m)=iobsg(m,n)
	      end do
	      iwrd=iwrd+ntyp
	      do m=1,ntyp
		iout(iwrd+m)=iobsa(m,n)
	      end do
	      iwrd=iwrd+ntyp
	    end do
	    iout(iwrd1)=iwrd-iwrd1+1
c
	    if(ilist1.eq.1) then
	      write(9,*)
	      write(9,fmt='(1x,a6,1x,16i6)')
     *			   csta(ns),(iout(i),i=iwrd1,iwrd1+lshead-1)
	      lenlvl=1+ntyp*4
	      iw=iwrd1+lshead
	      do n=1,nlvl
	        write(9,fmt='(5x,21i6)')
     *			     (iout(i),i=iw,iw+lenlvl-1)
		iw=iw+lenlvl
	      end do
	    end if
c
	    if(iwrd.ge.lrec) then
c..write data
	      do iw1=1,iwrd-lrec+1,lrec
	        iw2=iw1+lrec-1
	        irec=irec+1
	        write(iunit,rec=irec,iostat=ios,err=910)
     *			             (iout(i),i=iw1,iw2)
	      end do
	      iwrd=iwrd-iw2
	      do i=1,iwrd
		iout(i)=iout(iw2+i)
	      end do
	    end if
c
	    if(iwrdi.ge.lrec) then
c..write station index (NOTE: lindx*N=lrec)
	      ireci=ireci+1
	      write(iunit,rec=ireci,iostat=ios,err=910)
     *			             (indx(i),i=1,lrec)
	      iwrdi=0
	    end if
c
	  end if
c
c.......end do ns=1,nsta
	end do
c
c.....end do nums=1,numsort
      end do
c
      if(iwrd.gt.0) then
c..write last part of data
	do i=iwrd+1,lrec
	  iout(i)=-32767
	end do
	irec=irec+1
	write(iunit,rec=irec,iostat=ios,err=910) (iout(i),i=1,lrec)
      end if
c
      if(iwrdi.gt.0) then
c..write last part of station index
	do i=iwrdi+1,lrec
	  indx(i)=-1
	end do
	ireci=ireci+1
	write(iunit,rec=ireci,iostat=ios,err=910) (indx(i),i=1,lrec)
      end if
c
c..make output sort list
c.. 1: observation main type
c.. 2: instrument
c.. 3: min time difference in minutes 
c.. 4: max time difference in minutes 
c.. 5: min pressure in 1/10 hPa
c.. 6: max pressure in 1/10 hPa
c.. 7: no. of stations
c.. 8: pointer to index: record
c.. 9: pointer to index: word
c..10: pointer to  data: record
c..11: pointer to  data: word
c..12: no. of data types stored (no. of nonzero elements below)
c..13: data type 1 (1=z 2=u 3=v 4=rh 5=dz)
c..14: data type 2 (1=z 2=u 3=v 4=rh 5=dz) or 0
c..15: data type 3 (1=z 2=u 3=v 4=rh 5=dz) or 0
c..16: data type 4 (1=z 2=u 3=v 4=rh 5=dz) or 0
c..17: data type 5 (1=z 2=u 3=v 4=rh 5=dz) or 0
c..18: data type 6 (1=z 2=u 3=v 4=rh 5=dz) or 0
c..19: data type 7 (1=z 2=u 3=v 4=rh 5=dz) or 0
c..20: data type 8 (1=z 2=u 3=v 4=rh 5=dz) or 0
c
      do n=1,numsort
	isorto( 1,n)=isort(1,n)
	isorto( 2,n)=isort(2,n)
	isorto( 3,n)=nsort(6,n)
	isorto( 4,n)=nsort(7,n)
	isorto( 5,n)=nsort(8,n)
	isorto( 6,n)=nsort(9,n)
	isorto( 7,n)=nsort(1,n)
	isorto( 8,n)=nsort(15,n)
	isorto( 9,n)=nsort(16,n)
	isorto(10,n)=nsort(17,n)
	isorto(11,n)=nsort(18,n)
	isorto(12,n)=njsort(n)
	i=12
	do j=1,njsort(n)
	  i=i+1
	  isorto(i,n)=jsort(j,n)
	end do
	do j=i+1,lsorto
	  isorto(j,n)=0
	end do
      end do
c
c..update file header (possibly more than one record)
c
      lout=lrec*nrech
      do i=1,lout
	iout(i)=0
      end do
c
      call daytim(iyear,imonth,iday,ihour,iminut,isecn)
c
      iout( 1)=50
      iout( 2)=iyear
      iout( 3)=imonth*100+iday
      iout( 4)=ihour*100+iminut
      iout( 5)=itimea(1)
      iout( 6)=itimea(2)*100+itimea(3)
      iout( 7)=itimea(4)
      iout( 8)=irec
      iout( 9)=nrech+nreci
      iout(10)=nindx
      iout(11)=nindx
      iout(12)=ngrid
      iout(13)=iwrd
      iout(14)=nrech
c..version no.
      iout(15)=0
c
c..length of file haeder (sort list follows after this)
      iout(16)=64
      iout(17)=maxtyp
      iout(18)=numsort
      iout(19)=lsorto
      iout(20)=lindx
      iout(21)=lshead
c..pointer to index
      iout(22)=ireci1
      iout(23)=iwrdi1
c..pointer to data
      iout(24)=irecd1
      iout(25)=iwrdd1
c
c..output data scaling (z(m),dd(degrees),ff(knots),rh(%),dz(m))
      iout(31)= 0
      iout(32)=-1
      iout(33)=-1
      iout(34)=-1
      iout(35)= 0
c
c..list of sorted data
      iw=64
      do n=1,numsort
 	do i=1,lsorto
	  iout(iw+i)=isorto(i,n)
	end do
	iw=iw+lsorto
      end do
c
c..update first record as late as possible
      do irech=nrech,1,-1
	iw1=(irech-1)*lrec+1
	iw2=irech*lrec
        write(iunit,rec=irech,iostat=ios,err=910) (iout(i),i=iw1,iw2)
      end do
c
      close(iunit)
c
      write(6,*) 'irec,iwrd,lrec: ',irec,iwrd,lrec
c
      if(ilist1.eq.1) then
	write(9,*)
	write(9,*) 'numsort,maxsort: ',numsort,maxsort
	write(9,*) 'nindx,nlost:     ',nindx,nlost
	write(9,*)
	write(9,*) 'nsta,nsta1,nsta2: ',nsta,nsta1,nsta2
	write(9,*) 'nobs,nobso:       ',nobs,nobso
	write(9,*)
	write(9,*) 'n,isort(1:5,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,(isort(i,n),i=1,5)
	end do
	write(9,*)
	write(9,*) 'n,njsort(n),jsort(1:maxtyp,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,njsort(n),
     *	 				    (jsort(i,n),i=1,maxtyp)
	end do
	write(9,*)
	write(9,*) 'n,nsort(1:11,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,(nsort(i,n),i=1,11)
	end do
	write(9,*)
	write(9,*) 'n,nsort(12:18,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,(nsort(i,n),i=12,18)
	end do
	write(9,*)
	write(9,*) 'n,msort(1:maxtyp,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,(msort(i,n),i=1,maxtyp)
	end do
	write(9,*)
	write(9,*) 'n1,n2,fileheader(n1:n2):'
	nstep=10
	do n1=1,64,nstep
	  n2=min0(n1+nstep-1,64)
	  write(9,fmt='(1x,2i4,'' : '',10i6)') n1,n2,(iout(n),n=n1,n2)
	end do
	write(9,*)
	write(9,*) 'n,isorto(1:11,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,(isorto(i,n),i=1,11)
	end do
	write(9,*)
	write(9,*) 'n,isorto(12:lsorto,n):'
	do n=1,numsort
	  write(9,fmt='(1x,i2,'':'',12i6)') n,(isorto(i,n),i=12,lsorto)
	end do
	close(9)
      end if
c
      goto 990
c
  910 write(6,*) 'write error.  iostat= ',ios
      write(6,*) 'file: ',filout
      stop 5
c
  920 write(6,*) 'read error.  iostat= ',ios
      write(6,*) 'file: ',filout
      stop 5
c
  990 continue
c
      end
