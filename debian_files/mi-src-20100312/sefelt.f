      program sefelt
c
c	list felt (eller del av felt) til skjerm eller file
c
c----------------------------------------------------------------
c  DNMI/FoU  xx.xx.198x  Anstein Foss
c  DNMI/FoU  28.07.1993  Anstein Foss
c  DNMI/FoU  30.09.1993  Anstein Foss
c  DNMI/FoU  15.11.1994  Anstein Foss
c  DNMI/FoU  10.06.1995  Anstein Foss ... extra geometry identification
c  DNMI/FoU  04.11.2003  Anstein Foss ... SGI+Linux version
c  DNMI/FoU  10.11.2003  Anstein Foss ... bugfix
c----------------------------------------------------------------
c
      include 'sefelt.inc'
c
      parameter (limit=20+maxsiz)
c
      common/a/na(20),ia(maxsiz)
      integer*2 na,ia
c
      integer*2 in(16)
c
      character*256 fname
      character*256 sname
c
c
c--------------------------------------------
      narg=iargc()
      if(narg.ne.1) then
        write(6,*) '  usage:  sefelt felt_file'
        stop
      endif
      call getarg(1,fname)
      write(6,*) 'Felt file: ',fname(1:lenstr(fname,1))
c--------------------------------------------
c
      iunit=20
c
      call mrfelt(1,fname,iunit,in,0,0,0.,0.,20,na,ierror)
c
      if(ierror.ne.0) goto 900
c
      write(6,1001) (na(i),i=5,7),(na(i),i=2,4)
 1001 format(/,' Dato/termin:    ',3i6,
     *       /,' Sist oppdatert: ',3i6,/)
c
      write(6,*) 'Velg output:  1=terminal  2=file  3=terminal+file'
      read(5,*) iut
c
c------------------------------------------------------------------
      if(iut.eq.2 .or. iut.eq.3) then
        lfname=lenstr(fname,0)
        k1=1
        do k=1,lfname
          if(fname(k:k).eq.'/') k1=k+1
        end do
        sname='sefelt_'//fname(k1:lfname)
        write(6,*) 'output print file:'
        write(6,*) sname
        open(10,file=sname,
     *          form='formatted',access='sequential',
     *                       status='unknown')
        write(10,*) 'Felt file:'
        write(10,*) fname(1:lenstr(fname,1))
        write(10,1001) (na(i),i=5,7),(na(i),i=2,4)
      end if
c------------------------------------------------------------------
c
   10 continue
      write(6,*)
      write(6,*) ' velg: 0 = ferdig'
      write(6,*) '       1 = skriver ut hele feltet'
      write(6,*) '       2 = skriver ut en "terminal-rute",',
     *                          ' valg av nedre venstre hjorne'
      write(6,*) '       3 = skriver ut et bestemt omraade'
      write(6,*) '       4 = velger omraade for hvert felt'
      read(5,*) ivalg1
      ijstep=1
      if(ivalg1.eq.0) goto 900
      if(ivalg1.lt.1 .or. ivalg1.gt.4) goto 10
      if(ivalg1.eq.2) then
        write(6,*) ' gi nedre venstre hjorne:  i,j'
        read(5,*,err=10) i1,j1
        i2=i1+11
        j2=j1+24
      elseif(ivalg1.eq.3) then
        write(6,*) ' gi omraadet og "step":  i1,i2, j1,j2, ijstep'
        read(5,*,err=10) i1,i2, j1,j2, ijstep
        if(ijstep.lt.1) ijstep=1
      endif
c
      write(6,*)
      if(iut.eq.1) then
   12   continue
        write(6,*) ' velg: 0 = ikke skriv ut min/max'
        write(6,*) '       1 = skriv ut min/max'
        read(5,*,err=12) ivalg2
        if(ivalg2.lt.0 .or. ivalg2.gt.1) goto 12
      else
   13   continue
        write(6,*) ' velg: 0 = ikke skriv ut min/max'
        write(6,*) '       1 = skriv ut min/max paa terminal'
        write(6,*) '       2 = skriv ut min/max paa file'
        write(6,*) '       3 = skriv ut min/max paa terminal og file'
        read(5,*,err=13) ivalg2
        if(ivalg2.lt.0 .or. ivalg2.gt.3) goto 13
      endif
c
      if(iut.eq.1) then
   14   continue
        write(6,*) ' velg: 0 = ikke skriv ut "20 ord" identifikasjon'
        write(6,*) '       1 = skriv ut identifikasjon'
        read(5,*,err=14) ivalg3
        if(ivalg3.lt.0 .or. ivalg3.gt.1) goto 14
      else
   15   continue
        write(6,*) ' velg: 0 = ikke skriv ut "20 ord" identifikasjon'
        write(6,*) '       1 = skriv ut paa terminal'
        write(6,*) '       2 = skriv ut paa file'
        write(6,*) '       3 = skriv ut paa terminal og file'
        read(5,*,err=15) ivalg3
        if(ivalg3.lt.0 .or. ivalg3.gt.3) goto 15
      endif
c
   50 continue
      do i=1,16
	in(i)=-32767
      end do
      write(6,*)
      write(6,*) ' ord 1,2,9-14 i innh.-fort. (0,,,,,,,, => slutt)'
      read(5,*,err=10) in(1),in(2),(in(i),i=9,14)
      write(6,*)
c
      if(in(1).lt.1) goto 10
c
      call mrfelt(2,fname,iunit,in,0,0,0.,0.,limit,na,ierror)
c
      if(ierror.ne.0) goto 50
c
      ii=na(10)
      jj=na(11)
c
      if(ivalg3.eq.1 .or. ivalg3.eq.3) write( 6,1010) na
      if(ivalg3.eq.2 .or. ivalg3.eq.3) write(10,1011) na
 1010 format(  '  ident: ',11i6,/,21x,9i6)
 1011 format(/,'  ident: ',11i6,/,21x,9i6)
c
      lgeom=0
      if(na(9).gt.999) lgeom=na(9)-(na(9)/1000)*1000
      if(lgeom.gt.0) then
	ig1=ii*jj+1
	ig2=ii*jj+lgeom
        if(ivalg3.eq.1 .or. ivalg3.eq.3) write( 6,1020)
     +					       (ia(i),i=ig1,ig2)
        if(ivalg3.eq.2 .or. ivalg3.eq.3) write(10,1021)
     +					       (ia(i),i=ig1,ig2)
 1020   format(  '  extra ident: ',10i6,3(/,15x,10i6))
 1021   format(/,'  extra ident: ',10i6,3(/,15x,10i6))
      end if
c
      if(ivalg2.ne.0) call minmax(ia,ii,jj,ivalg2)
c
      if(ivalg1.eq.1) then
      i1=1
      i2=ii
      j1=1
      j2=jj
      elseif(ivalg1.eq.4) then
      write(6,*) ' gi omraadet:  i1,i2, j1,j2'
      read(5,*) i1,i2, j1,j2
      endif
c
      call rute(na,ia,ii,jj,i1,i2,j1,j2,iut,ijstep)
c
      goto 50
c
  900 continue
      call mrfelt(3,fname,iunit,in,0,0,0.,0.,1,na,ierror)
c
      end
c
c**********************************************************************
c
      subroutine minmax(ia,iii,jjj,iut)
c
      integer*2 ia(iii,jjj)
c
      ii=iii
      jj=jjj
c
      minval=ia(1,1)
      maxval=ia(1,1)
      in=1
      jn=1
      ix=1
      jx=1
c
      do 10 j=1,jj
      do 10 i=1,ii
      if(ia(i,j).lt.minval) then
        minval=ia(i,j)
        in=i
        jn=j
      endif
      if(ia(i,j).gt.maxval) then
        maxval=ia(i,j)
        ix=i
        jx=j
      endif
   10 continue
c
      if(iut.eq.1 .or. iut.eq.3)
     +		write(6,1001) minval,in,jn, maxval,ix,jx
      if(iut.eq.2 .or. iut.eq.3)
     +		write(10,1001) minval,in,jn, maxval,ix,jx
 1001 format(/,'  min,i,j: ',i6,i5,i4,'    max,i,j: ',i6,i5,i4)
c
      return
      end
c
c********************************************************************
c
      subroutine rute(na,ia,ii,jj,i11,i22,j11,j22,iut,ijstep)
c
      integer*2 na(20),ia(ii,jj)
c
      integer nr(50),long(50)
c
      i1=i11
      i2=i22
      j1=j22
      j2=j11
      is=+ijstep
      js=-ijstep
c
c..geografisk grid (ec) som kommer i form nord(j=1) soer(j=nj)
      if(na(9).eq.2 .and. na(17).lt.0) then
        j1=j11
        j2=j22
        js=-js
      end if
c
c..geografisk grid
      igeo=0
      latstp=0
      lonstp=0
      latbas=0
      lonbas=0
      if(na(9).eq.2) then
	igeo=1
	latbas=na(15)-na(17)
	lonbas=na(16)-na(18)
	latstp=na(17)
	lonstp=na(18)
      end if
c
      nu1=1
      nu2=2
      if(iut.eq.1) nu2=1
      if(iut.eq.2) nu1=2
c
      do nu=nu1,nu2
c
	if(nu.eq.1) then
	  iu=6
	  ntt=12
	else
	  iu=10
	  ntt=21
	end if
	ntt=ntt-igeo
c
        i=(i2-i1)/is
        ni=(i+ntt)/ntt
        i=(j2-j1)/js
        j2=j1+i*js
        i3=i1-ntt*is
        do ki=1,ni
          i3=i3+ntt*is
          i4=i3+ntt*is-1
          if(i4.gt.i2) i4=i2
c
	  if(igeo.eq.0) then
c
            inr=0
            do i=i3,i4,is
	      inr=inr+1
	      nr(inr)=i
            end do
            write(iu,*) ' '
            write(iu,1001) (nr(i),i=1,inr)
 1001       format(6x,21(i5,'+'),:)
            do j=j1,j2,js
              write(iu,1002) j,(ia(i,j),i=i3,i4,is)
 1002         format(1x,i3,'+ ',21i6,:)
            end do
            write(iu,1001) (nr(i),i=1,inr)
c
	  else
c
            inr=0
            do i=i3,i4,is
	      inr=inr+1
	      nr(inr)=i
	      long(inr)=lonbas+i*lonstp
	      if(long(inr).gt.+18000) long(inr)=long(inr)-36000
	      if(long(inr).le.-18000) long(inr)=long(inr)+36000
            end do
            write(iu,*) ' '
	    write(iu,1003) (long(i),i=1,inr)
 1003       format(12x,20i6,:)
            write(iu,1004) (nr(i),i=1,inr)
 1004       format(12x,20(i5,'+'),:)
            do j=j1,j2,js
	      lat=latbas+j*latstp
              write(iu,1005) lat,j,(ia(i,j),i=i3,i4,is)
 1005         format(1x,i5,1x,i3,'+ ',20i6,:)
            end do
            write(iu,1004) (nr(i),i=1,inr)
	    write(iu,1003) (long(i),i=1,inr)
c
	  end if
c
	end do
c
        if(nu.eq.1) then
          write(iu,1006)
 1006     format(' -----',12('------'))
        else
          write(iu,1007)
 1007     format(' -----',21('------'))
        end if
c
      end do
c
      return
      end
