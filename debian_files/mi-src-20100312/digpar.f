      program digpar
c
c        digitalisert 'omr$de' til felt (parameter-felt for modell)
c
c        leser (b,l)-data fra linje-file.
c        leser felt fra felt-file og legger resultat p$ felt-file.
c
c        grids: polarstereographic, geographic, spherical (rotated)
c
c--------------
c digpar.input:
c======================================================================
c *** digpar.input  ('digpar.input' for the digpar program, digpar.f)
c ***
c *=> Digitized ice and snow to fields, Lam50s
c *=> (run this after sst analysis)
c *=>
c *=> Command format:
c *=> digpar digpar.input 1814 icedig.dat snowdig.dat par1814.dat
c *=>                    <grid> <input>   <input>     <felt, input/output>
c *** .......(#1).........#2...#3.........#4..........#5.........
c ***---------------------------------------------------------------
c **  '#n' = command line arg. no n (n>1)    '$var' = env. var.
c **---------------------------------------------------------------
c *>
c 1000, '#2'           <-- 1000, 'grid_nummer' ... brukes hvis 88,0,...
c 1, '#3'              <-- file_id, 'file' ... input digitalisert ICE
c 2, '#4'              <-- file_id, 'file' ... input digitalisert SNOW
c 3, '#5'              <-- file_id, 'file' ... input/output felt-file
c 0, '*'               <-- file_id, 'file' ... end of file list
c 1                                  <-- input file_id ... ICE
c 3, 88,0000,4,0,2,102,1000,0, 2,0,1 <-- input felt,  kreset,ireset,iold
c 3, 88,0000,4,0,2,102,1000,0        <-- output felt
c 0, 00,0000,0,0,0,000,0000,0        <-- test-felt (for kode=6,7,9,10)
c 2, 1, 1,10, 0,0                    <-- kode, iny, il1,il2, if1,if2
c 2, 0, 0, 0, 1,1                    <-- kode, iny, il1,il2, if1,if2
c 0, 0, 0, 0, 0,0                    <-- kode,... 0 = slutt
c 2                                  <-- input file_id ... SNOW
c 3, 88,0000,4,0,2,104,1000,0, 1,0,0 <-- input-felt,  kreset,ireset,iold
c 3, 88,0000,4,0,2,104,1000,0        <-- output-felt
c 3, 88,0000,4,0,2,102,1000,0        <-- test-felt (for kode=6,7,9,10)
c 5,   0, 0,0, 0,0                   <-- kode, iny, il1,il2, if1,if2
c 7,   0, 0,0, 0,0                   <-- kode, iny, il1,il2, if1,if2
c 7, 100, 0,0, 1,1                   <-- kode, iny, il1,il2, if1,if2
c 0,   0, 0,0, 0,0                   <-- kode,... 0 = slutt
c 1                                  <-- input file_id ... DUMMY (sst)
c 3, 88,0000,4,0,2,103,1000,0, 0,0,0 <-- input-felt,  kreset,ireset,iold
c 3, 88,0000,4,0,2,103,1000,0        <-- output-felt
c 3, 88,0000,4,0,2,102,1000,0        <-- test-felt (for kode=6,7,9,10)
c 9, -15, -32767,-15, 0,0            <-- kode, iny, il1,il2, if1,if2
c 0,   0,      0,  0, 0,0            <-- kode,... 0 = slutt
c 0                                  <-- input file_id ... 0 = end
c======================================================================
c
c              kode = 0: slutt
c                   = 1: markerer med 'iny' innenfor linjene 'il1-il2'
c                   = 2: markerer med 'iny' innenfor linjene 'il1-il2'
c                        hvis felt-verdi er 'if1-if2'
c                   = 3: markerer med 'iny' utenfor linjene 'il1-il2'
c                   = 4: markerer med 'iny' utenfor linjene 'il1-il2'
c                        hvis felt-verdi er 'if1-if2'
c                   = 5: markerer med samme verdi innenfor linjene som
c                        den verdi linjene har, resten av feltet f$r
c                        verdien 'iny' (il1,il2,if1,if2 benyttes ikke)
c                   = 6: markerer med samme verdi innenfor linjene som
c                        den verdi linjene har, resten av feltet f$r
c                        verdien 'iny', gjelder bare punkt i 'test-felt'
c                        med verdi 'if1-if2' (il1,il2 benyttes ikke)
c                   = 7: setter inn verdi 'iny' i punkt hvor 'test-felt'
c                        har verdi 'if1-if2' (il1,il2 benyttes ikke)
c                        (grenselinjene benyttes ikke)
c                   = 8: 5 punkt middel. (if1,if2,il1,il2 benyttes ikke)
c                        (grenselinjene benyttes ikke)
c                   = 9: setter inn verdi 'iny' i punkt hvor verdien er
c                        'il1-il2' og 'test-felt' har verdi 'if1-if2'
c                        (sjekk av min- eller max-verdi i felt)
c                        (grenselinjene benyttes ikke)
c                  = 10: setter inn verdi lik 'test-felt' i punkt hvor verdien
c                        er 'il1-il2' og 'test-felt' har verdi 'if1-if2'
c                        (sjekk av min- eller max-verdi i felt)
c                        (grenselinjene benyttes ikke)
c                  = 11: sorterer linjer etter verdi
c                        (iny,il1,il2,if1,if2 benyttes ikke)
c
c        kreset: 0 = benytt input-felt uforandret
c                1 = sett inn 'ireset' i alle punkt
c                2 = sett inn 'ireset' i punkt med verdi = 'iold'
c                3 = sett inn 'ireset' i punkt med verdi < 'iold'
c                4 = sett inn 'ireset' i punkt med verdi > 'iold'
c
c======================================================================
c
c  DNMI library subroutines: rlunit
c                            mrfelt
c                            mwfelt
c                            daytim
c                            prhelp
c                            rcomnt
c                            getvar
c                            gridpar
c                            xyconvert
c
c----------------------------------------------------------------------
c  DNMI/FoU  xx.xx.1989  Anstein Foss ... ibm
c  DNMI/FoU  25.09.1993  Anstein Foss ... unix
c  DNMI/FoU  23.03.1994  Anstein Foss ... mrfelt/mwfelt
c  DNMI/FoU  16.05.1995  Anstein Foss ... misc. grids
c  DNMI/FoU  14.06.1996  Anstein Foss ... xyconvert
c  DNMI/FoU  11.10.1996  C.Ulstad,A.Foss ... kode=10
c  DNMI/FoU  04.09.1997  Anstein Foss ... 1000, 'grid_no'
c  DNMI/FoU  30.10.2003  Anstein Foss ... SGI+Linux version
c----------------------------------------------------------------------
c
c
      include 'digpar.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c  include file for digpar.f
c
c  maxsiz: max field size (incl. extra geometry identification)
c  maxtst: max no. test specifications
c  maxlin: max no. lines     from line file
c  maxpos: max no. positions from line file
c  maxfil: max no. files
c
ccc   parameter (maxsiz=100000+50)
ccc   parameter (maxtst=100)
ccc   parameter (maxlin=2000,maxpos=50000)
ccc   parameter (maxfil=32)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      common/a/nlin,npos,ilin(3,maxlin),xpos(maxpos),ypos(maxpos)
      common/as/isequence(maxlin)
c
      common/b/ident(20),ifelt(maxsiz),identc(20),ifeltc(maxsiz)
      integer*2 ident,ifelt,identc,ifeltc
c
      common/c/markf(maxsiz),mark(maxsiz)
c
      common/d/ntest,itest(6,maxtst)
c
      real      grid(6),gridc(6)
c
      integer*2 ini(16),ino(16),inc(16)
c
      character*256 filnam(maxfil)
      character*256 finput,cinput
c
c..define a grid matching geographic coordinates (for xyconvert)
      integer igeogrid
      real     geogrid(6)
      data igeogrid/2/
      data  geogrid/1.,1.,1.,1.,0.,0./
c
c
      istop=0
c
      do n=1,maxfil
        filnam(n)='*'
      end do
c
c--------------------------------------------------------------------
c..file unit for 'digpar.input'
      iuinp=9
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: digpar <digpar.input>'
        write(6,*) '      or: digpar <digpar.input> <arguments>'
        write(6,*) '      or: digpar <digpar.input> ?     (to get help)'
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
          write(6,*) 'Help from ''digpar.input'' file:'
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
      nread=0
      nreadc=0
c
   10 continue
c
c-----------------------------------------------------------------------
c        idsl:   file med grense-linjer (punkt gitt som b,l) ... 0=slutt
c        idsi:   input felt-file (0=ikke input, innlest tidligere)
c        ini:    innh.fort. for input-felt
c        kreset: 0 = benytt input-felt uforandret
c                1 = sett inn 'ireset' i alle punkt
c                2 = sett inn 'ireset' i punkt med verdi = 'iold'
c                3 = sett inn 'ireset' i punkt med verdi < 'iold'
c                4 = sett inn 'ireset' i punkt med verdi > 'iold'
c        idso:   output felt-file (evt. samme som input; 0=ikke output)
c        ino:    innh.fort. for output-felt
c        idsc:   input felt-file for 'test-felt' (0=ikke input/lest f@r)
c        inc:    innh.fort. for test-felt (kode=6)
c        itest:  spesifikasjoner: kode, iny, il1,il2, if1,if2
c              kode = 0: slutt
c                   = 1: markerer med 'iny' innenfor linjene 'il1-il2'
c                   = 2: markerer med 'iny' innenfor linjene 'il1-il2'
c                        hvis felt-verdi er 'if1-if2'
c                   = 3: markerer med 'iny' utenfor linjene 'il1-il2'
c                   = 4: markerer med 'iny' utenfor linjene 'il1-il2'
c                        hvis felt-verdi er 'if1-if2'
c                   = 5: markerer med samme verdi innenfor linjene som
c                        den verdi linjene har, resten av feltet f$r
c                        verdien 'iny' (il1,il2,if1,if2 benyttes ikke)
c                   = 6: markerer med samme verdi innenfor linjene som
c                        den verdi linjene har, resten av feltet f$r
c                        verdien 'iny', gjelder bare punkt i 'test-felt'
c                        med verdi 'if1-if2' (il1,il2 benyttes ikke)
c                   = 7: setter inn verdi 'iny' i punkt hvor 'test-felt'
c                        har verdi 'if1-if2' (il1,il2 benyttes ikke)
c                        (grenselinjene benyttes ikke)
c                   = 8: 5 punkt middel. (if1,if2,il1,il2 benyttes ikke)
c                        (grenselinjene benyttes ikke)
c                   = 9: setter inn verdi 'iny' i punkt hvor verdien er
c                        'il1-il2' og 'test-felt' har verdi 'if1-if2'
c                        (sjekk av min- eller max-verdi i felt)
c                        (grenselinjene benyttes ikke)
c                  = 10: setter inn verdi lik 'test-felt' i punkt hvor verdien
c                        er 'il1-il2' og 'test-felt' har verdi 'if1-if2'
c                        (sjekk av min- eller max-verdi i felt)
c                        (grenselinjene benyttes ikke)
c                  = 11: sorterer linjer etter verdi
c                        (iny,il1,il2,if1,if2 benyttes ikke)
c-----------------------------------------------------------------------
c
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) idlin
c
c..end ?
      if(idlin.lt.1) goto 990
c
      if(idlin.lt.1 .or. idlin.gt.maxfil) goto 910
      if(filnam(idlin).eq.'*') goto 910
c
      nlines=nlines+1
      read(iuinp,*,err=910,end=910)
     *           idfi,ini(1),ini(2),(ini(i),i=9,14), kreset,ireset,iold
      if(idfi.lt.1 .or. idfi.gt.maxfil) goto 910
      if(filnam(idfi).eq.'*') goto 910
      nlines=nlines+1
      read(iuinp,*,err=910,end=910)
     *           idfo,ino(1),ino(2),(ino(i),i=9,14)
      if(idfo.ne.0) then
        if(idfo.lt.1 .or. idfo.gt.maxfil) goto 910
        if(filnam(idfo).eq.'*') goto 910
      end if
      nlines=nlines+1
      read(iuinp,*,err=910,end=910)
     *           idfc,inc(1),inc(2),(inc(i),i=9,14)
      if(idfc.ne.0) then
        if(idfc.lt.1 .or. idfc.gt.maxfil) goto 910
        if(filnam(idfc).eq.'*') goto 910
      end if
      do n=1,maxtst
        nlines=nlines+1
        read(iuinp,*,err=910,end=910) (itest(i,n),i=1,6)
        if(itest(1,n).eq.0) goto 25
      end do
      n=maxtst+1
      nlines=nlines+1
      read(iuinp,*,err=910,end=910) i
      if(i.eq.0) goto 25
      write(6,*) 'for mange tester'
      stop 117
c
   25 ntest=n-1
c
      lininp=0
      do n=1,ntest
        if(itest(1,n).le.6) lininp=1
      end do
c
      if(ini(2).eq.0) ini(2)=igrid
      if(ino(2).eq.0) ino(2)=igrid
      if(inc(2).eq.0) inc(2)=igrid
c
c
c..leser digitalisert 'grense' gitt i b,l-koordinater
      if(lininp.ne.0) then
	call getdat(iunit,filnam(idlin))
	do l=1,nlin
	  isequence(l)=l
	end do
      end if
c
c..felt fra felt-file
      if(idfi.gt.0) then
        call feltin(1,iunit,filnam(idfi),ini,nx,ny,igtype,grid)
        ino(15)=ini(15)
        nread=nread+1
      elseif(nread.lt.1) then
        write(6,*) 'felt m$ leses f@rste gang'
        stop 117
      end if
c
c..test-felt fra felt-file
      if(idfc.gt.0) then
        call feltin(2,iunit,filnam(idfc),inc,nxc,nyc,igtypc,gridc)
        if(nxc.ne.nx .or. nyc.ne.ny .or. igtypc.ne.igtype) then
          write(6,*) 'felt.      grid,type,nx,ny: ',
     +                           ident(2),igtype,nx,ny
          write(6,*) 'test-felt. grid,type,nx,ny: ',
     +                           identc(2),igtypc,nxc,nyc
          stop 117
        end if
        nreadc=nreadc+1
      end if
c
      if(kreset.eq.1) then
        do i=1,nx*ny
          ifelt(i)=ireset
        end do
        ident(19)=0
        ident(20)=0
      elseif(kreset.eq.2) then
        do i=1,nx*ny
          if(ifelt(i).eq.iold) ifelt(i)=ireset
        end do
      elseif(kreset.eq.3) then
        do i=1,nx*ny
          if(ifelt(i).lt.iold) ifelt(i)=ireset
        end do
      elseif(kreset.eq.4) then
        do i=1,nx*ny
          if(ifelt(i).gt.iold) ifelt(i)=ireset
        end do
      end if
c
      if(lininp.ne.0) then
c..punkt fra (l,b) til (x,y)
        call xyconvert(npos,xpos,ypos,
     +                 igeogrid,geogrid,igtype,grid,ierror)
        if(ierror.ne.0) write(6,*) 'XYCONVERT ERROR: ',ierror
        if(ierror.ne.0) stop 117
      end if
c
c
      do 100 nt=1,ntest
c
      kode=itest(1,nt)
      iny= itest(2,nt)
      il1= itest(3,nt)
      il2= itest(4,nt)
      if1= itest(5,nt)
      if2= itest(6,nt)
c
      if(kode.eq.11) then
c..sorter linjer etter verdi
	do l=1,nlin
	  is=l
	  il=isequence(l)
	  iv=ilin(3,il)
	  do ll=l+1,nlin
	    jl=isequence(ll)
	    jv=ilin(3,jl)
	    if(jv.lt.iv) then
	      is=ll
	      iv=jv
	    end if
	  end do
	  jl=isequence(l)
	  isequence(l)=isequence(is)
	  isequence(is)=jl
	end do
	goto 100
      end if
c
      if(kode.gt.4 .and. kode.lt.9) then
        il1=-999999
        il2=+999999
      end if
c
      if((kode.eq.6 .or. kode.eq.7 .or. kode.eq.9 .or. kode.eq.10)
     *                                          .and. nreadc.eq.0) then
        write(6,*) ' *** test-felt ikke innlest.  kode=',kode
        stop 117
      end if
c
      if(kode.eq.7) then
c..benytter ikke 'linjer'
        do i=1,nx*ny
          if(ifeltc(i).ge.if1 .and. ifeltc(i).le.if2) ifelt(i)=iny
        end do
        goto 100
      end if
c
      if(kode.eq.8) then
c..benytter ikke 'linjer'
        call midl5(ifelt,nx,ny,mark)
        goto 100
      end if
c
      if(kode.eq.9) then
c..benytter ikke 'linjer'
        do i=1,nx*ny
          if(ifelt(i) .ge.il1 .and. ifelt(i) .le.il2 .and.
     *       ifeltc(i).ge.if1 .and. ifeltc(i).le.if2) ifelt(i)=iny
        end do
        goto 100
      end if
c
      if(kode.eq.10) then
c..benytter ikke 'linjer'
        do i=1,nx*ny
          if(ifelt(i) .ge.il1 .and. ifelt(i) .le.il2 .and.
     *      ifeltc(i).ge.if1 .and. ifeltc(i).le.if2) ifelt(i)=ifeltc(i)
        end do
        goto 100
      end if
c
      if(lininp.eq.0) stop 2222
c
c..markerer omr$der iflg. innleste grenser
      call area1(markf,mark,nx,ny,il1,il2,kode,iny)
c
c..markerer @nskede punkt i input/output-felt
      call area2(ifelt,markf,nx,ny,kode,iny,if1,if2,ifeltc)
c
  100 continue
c
c
c..felt til felt-file
      if(idfo.gt.0) call feltut(iunit,filnam(idfo),ino)
c
      goto 10
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
c**********************************************************************
c
      subroutine area1(markf,mark,nx,ny,ilv1,ilv2,kode,iny)
c
c       forandrer feltet iflg. innleste linjer.
c
      include 'digpar.inc'
c
      common/a/nlin,npos,ilin(3,maxlin),xpos(maxpos),ypos(maxpos)
      common/as/isequence(maxlin)
c
      integer   markf(nx,ny),mark(nx,ny)
c
c
      ityp1=ilv1
      ityp2=ilv2
      kod=kode
      new=iny
c
      if(kod.lt.5) then
        do j=1,ny
          do i=1,nx
            markf(i,j)=0
          end do
        end do
      else
        do j=1,ny
          do i=1,nx
            markf(i,j)=new
          end do
        end do
      end if
c
      do 30 ls=1,nlin
c
      l=isequence(ls)
c
c..sjekk om denne linjen skal benyttes
      ityp=ilin(3,l)
      if(ityp.lt.ityp1 .or. ityp.gt.ityp2) goto 30
c
      np1=ilin(1,l)
      np2=ilin(2,l)
c
c..behandler hver linje separat.
c
      do j=1,ny
        do i=1,nx
          mark(i,j)=0
        end do
      end do
c
c..nb| trekker linje-stykke mellom siste og f@rste punkt
c
      x2=xpos(np2)
      y2=ypos(np2)
      j2=y2
c
      do np=np1,np2
c
        x1=x2
        y1=y2
        j1=j2
        x2=xpos(np)
        y2=ypos(np)
        j2=y2
        if(y1.lt.y2) then
          ja=j1+1
          jb=j2
        else
          ja=j2+1
          jb=j1
        end if
        if(ja.lt. 1) ja=1
        if(jb.gt.ny) jb=ny
c
        do j=ja,jb
          y=j
          i=((y2-y)*x1+(y-y1)*x2)/(y2-y1)
          if(i.lt.0) then
            mark(1,j)=mark(1,j)+1
          elseif(i.lt.nx) then
            mark(i+1,j)=mark(i+1,j)+1
          end if
        end do
c
      end do
c
c..fyller med 0 eller 1 (0=utenfor  1=innenfor)
      do j=1,ny
        k=0
        do i=1,nx
          k=k+mark(i,j)
          k=k-k/2*2
          mark(i,j)=k
        end do
      end do
c
c..setter inn i output-grid
      if(kod.lt.5) then
        do j=1,ny
          do i=1,nx
            if(mark(i,j).eq.1) markf(i,j)=1
          end do
        end do
      else
        do j=1,ny
          do i=1,nx
            if(mark(i,j).eq.1) markf(i,j)=ityp
          end do
        end do
      end if
c
   30 continue
c
      return
      end
c
c**********************************************************************
c
      subroutine area2(ifelt,markf,nx,ny,kode,iny,if1,if2,ifeltc)
c
c       forandrer feltet iflg. spesifikasjoner
c
      integer*2 ifelt(nx,ny),ifeltc(nx,ny)
      integer   markf(nx,ny)
c
c
      kod=kode
      new=iny
      ifv1=if1
      ifv2=if2
c
      if(kod.eq.1) then
        do j=1,ny
          do i=1,nx
            if(markf(i,j).eq.1) ifelt(i,j)=new
          end do
        end do
      elseif(kod.eq.2) then
        do j=1,ny
          do i=1,nx
            if(markf(i,j).eq.1 .and. ifelt(i,j).ge.ifv1
     *                         .and. ifelt(i,j).le.ifv2) ifelt(i,j)=new
          end do
        end do
      elseif(kod.eq.3) then
        do j=1,ny
          do i=1,nx
            if(markf(i,j).eq.0) ifelt(i,j)=new
          end do
        end do
      elseif(kod.eq.4) then
        do j=1,ny
          do i=1,nx
            if(markf(i,j).eq.0 .and. ifelt(i,j).ge.ifv1
     *                         .and. ifelt(i,j).le.ifv2) ifelt(i,j)=new
          end do
        end do
      elseif(kod.eq.5) then
        do j=1,ny
          do i=1,nx
            ifelt(i,j)=markf(i,j)
          end do
        end do
      elseif(kod.eq.6) then
        do j=1,ny
          do i=1,nx
            if(ifeltc(i,j).ge.ifv1 .and. ifeltc(i,j).le.ifv2)
     *                                            ifelt(i,j)=markf(i,j)
          end do
        end do
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine feltin(nfelt,iunit,filnam,in,nx,ny,igtype,grid)
c
c
      include 'digpar.inc'
c
      common/b/ident(20),ifelt(maxsiz),identc(20),ifeltc(maxsiz)
      integer*2 ident,ifelt,identc,ifeltc
c
      integer*2 in(16)
      real      grid(6)
      character*(*) filnam
c
      integer*2 idfile(32)
      double precision sum
c
      write(6,*) ' -----  feltin  -----'
c
      call mrfelt(1,filnam,iunit,in,0,1,1.,1.,32,idfile,ierror)
      if(ierror.ne.0) stop 117
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
      if(nfelt.ne.2) then
c
        in(3)=-32767
        in(4)=-32767
        in(5)=-32767
        ldata=20+maxsiz
        call mrfelt(2,filnam,iunit,in,0,1,1.,1.,
     *                ldata,ident,ierror)
        if(ierror.ne.0) stop 117
c
        call gridpar(+1,ldata,ident,igtype,nx,ny,grid,ierror)
        if(ierror.ne.0) write(6,*) 'GRIDPAR ERROR: ',ierror
        if(ierror.ne.0) stop 200
c
        sum=0.
        imin=ifelt(1)
        imax=ifelt(1)
        do i=1,nx*ny
          if(ifelt(i).lt.imin) imin=ifelt(i)
          if(ifelt(i).gt.imax) imax=ifelt(i)
          sum=sum+ifelt(i)
        end do
c
        fmid=sum/(nx*ny)
        write(6,*) ' felt.  min,max,middel: ',imin,imax,fmid
c
      elseif(nfelt.eq.2) then
c
        in(3)=-32767
        in(4)=-32767
        in(5)=-32767
        ldata=20+maxsiz
        call mrfelt(2,filnam,iunit,in,0,1,1.,1.,
     *                ldata,identc,ierror)
        if(ierror.ne.0) stop 117
c
        call gridpar(+1,ldata,identc,igtype,nx,ny,grid,ierror)
        if(ierror.ne.0) write(6,*) 'GRIDPAR ERROR: ',ierror
        if(ierror.ne.0) stop 200
c
      end if
c
      call mrfelt(3,filnam,iunit,in,0,1,1.,1.,1,idfile,ierror)
      if(ierror.ne.0) stop 117
c
      return
      end
c
c***********************************************************************
c
      subroutine feltut(iunit,filnam,in)
c
c
      include 'digpar.inc'
c
      common/b/ident(20),ifelt(maxsiz),identc(20),ifeltc(maxsiz)
      integer*2 ident,ifelt,identc,ifeltc
c
      integer*2 in(16)
      character*(*) filnam
c
      double precision sum
c
      write(6,*) ' -----  feltut  -----'
c
      nx=ident(10)
      ny=ident(11)
      sum=0.
      imin=ifelt(1)
      imax=ifelt(1)
      do i=1,nx*ny
        if(ifelt(i).lt.imin) imin=ifelt(i)
        if(ifelt(i).gt.imax) imax=ifelt(i)
        sum=sum+ifelt(i)
      end do
c
      fmid=sum/(nx*ny)
      write(6,*) ' felt.  min,max,middel: ',imin,imax,fmid
c
      ident(1)=in( 1)
      ident(2)=in( 2)
      ident(3)=in( 9)
      ident(4)=in(10)
      ident(5)=in(11)
      ident(6)=in(12)
      ident(7)=in(13)
      ident(8)=in(14)
c
      ldata=20+maxsiz
      call mwfelt(0,filnam,iunit,0,1,1.,1.,
     *              ldata,ident,ierror)
      if(ierror.ne.0) stop 117
c
      return
      end
c
c**********************************************************************
c
      subroutine getdat(iunit,filnam)
c
c        input fra file 'filnam'
c
c        leser digitalisert 'grense' gitt i b,l-koordinater
c
c        output:        nlin: antall linjer/omr$der/polygon
c                  ilin(1,n): ) peker til f@rste og siste punkt p$
c                      (2,n): ) linje 'n' i 'xpos,ypos'
c                      (3,n): verdi eller type
c                       npos: totalt antall punkt
c                    xpos(i): lengde ) for punkt 'i'
c                    ypos(i): bredde )
c
      include 'digpar.inc'
c
      common/a/nlin,npos,ilin(3,maxlin),xpos(maxpos),ypos(maxpos)
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
ccccc ivscal=  iout(2,5) .... antar type/verdi er uskalert
      nptxt=   iout(1,6)
      lntxt=   iout(2,6)
c
      gscal=10.**igscal
ccccc vscal=10.**ivscal
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
      if(nlin.gt.maxlin .or. npos.gt.maxpos) goto 920
c
      np=6+nptxt
      ip=0
c
      do 100 l=1,nlin
c
      if(np.eq.maxout) then
      irec=irec+1
      read(iunit,rec=irec,iostat=ios,err=910) iout
      np=0
      end if
      np=np+1
      ilin(1,l)=ip+1
      ilin(2,l)=ip+iout(1,np)
      ilin(3,l)=iout(2,np)
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
c..NOTE: xpos()=longitude  ypos()=latitude
      ypos(ip+i)=gscal*iout(1,np+i)
  120 xpos(ip+i)=gscal*iout(2,np+i)
      ip=ip+ni
      np=np+ni
  110 continue
c
  100 continue
c
c        sjekk input
c
      if(irec.ne.nrec) goto 930
      if(npos.lt.32767 .and. ip.ne.npos) goto 930
      if(npos.eq.32767 .and. ip.lt.32767) goto 930
c
      npos=ip
c
      write(6,*)
      write(6,*) 'input fra file:'
      write(6,*) '      antall linjer:    ',nlin
      write(6,*) '      antall posisjoner:',npos
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
      write(6,*) '      no. of lines:     ',nlin
      write(6,*) '      no. of positions: ',npos
      write(6,*) 'program:'
      write(6,*) '  max no. of lines:     ',maxlin
      write(6,*) '  max no. of positions: ',maxpos
      goto 980
c
  930 write(6,*) 'error'
      write(6,*) 'file: ',filnam
      write(6,*) 'record 1:  no. of records:   ',nrec
      write(6,*) '           no. of positions: ',npos
      write(6,*) 'read:      no. of records:   ',irec
      write(6,*) '           no. of positions: ',ip
      goto 980
c
  980 stop 117
c
      end
c
c***********************************************************************
c
      subroutine midl5(ifelt,nx,ny,mark)
c
      integer*2 ifelt(nx,ny)
      integer   mark(nx,ny)
c
      do j=1,ny
        do i=1,nx
           mark(i,j)=ifelt(i,j)
        end do
      end do
c
      do j=2,ny-1
        do i=2,nx-1
          n=mark(i,j-1)+mark(i-1,j)+mark(i,j)+mark(i+1,j)+mark(i,j+1)
          if(n.gt.0) then
            n=(n+2)/5
          else
            n=(n-2)/5
          end if
          ifelt(i,j)=n
        end do
      end do
c
      do i=2,nx-1
        j=1
        n=mark(i-1,j)+mark(i,j)+mark(i+1,j)+mark(i,j+1)
        if(n.gt.0) then
          n=(n+2)/4
        else
          n=(n-2)/4
        end if
        ifelt(i,j)=n
        j=ny
        n=mark(i,j-1)+mark(i-1,j)+mark(i,j)+mark(i+1,j)
        if(n.gt.0) then
          n=(n+2)/4
        else
          n=(n-2)/4
        end if
        ifelt(i,j)=n
      end do
c
      do j=2,ny-1
        i=1
        n=mark(i,j-1)+mark(i,j)+mark(i+1,j)+mark(i,j+1)
        if(n.gt.0) then
          n=(n+2)/4
        else
          n=(n-2)/4
        end if
        ifelt(i,j)=n
        i=nx
        n=mark(i,j-1)+mark(i-1,j)+mark(i,j)+mark(i,j+1)
        if(n.gt.0) then
          n=(n+2)/4
        else
          n=(n-2)/4
        end if
        ifelt(i,j)=n
      end do
c
        i=1
        j=1
        n=mark(i,j)+mark(i+1,j)+mark(i,j+1)
        if(n.gt.0) then
          n=(n+1)/3
        else
          n=(n-1)/3
        end if
        ifelt(i,j)=n
        i=nx
        j=1
        n=mark(i-1,j)+mark(i,j)+mark(i,j+1)
        if(n.gt.0) then
          n=(n+1)/3
        else
          n=(n-1)/3
        end if
        ifelt(i,j)=n
        i=1
        j=ny
        n=mark(i,j-1)+mark(i,j)+mark(i+1,j)
        if(n.gt.0) then
          n=(n+1)/3
        else
          n=(n-1)/3
        end if
        ifelt(i,j)=n
        i=nx
        j=ny
        n=mark(i,j-1)+mark(i-1,j)+mark(i,j)
        if(n.gt.0) then
          n=(n+1)/3
        else
          n=(n-1)/3
        end if
        ifelt(i,j)=n
c
        return
        end
