      program obslist
c
c        leser standard dnmi observasjons-filer.
c        skriver ut stasjons-nr, posisjon og hoyde paa obs1.lst .
c        skriver ut innholdsfortegnelsen           paa obs2.lst .
c
c--------------------------------------------------------------------
c  DNMI/FoU  xx.xx.199x  Anstein Foss
c  DNMI/FoU  14.12.1994  Anstein Foss
c  DNMI/FoU  27.01.2001  Anstein Foss ... automatic swap
c  DNMI/FoU  13.01.2003  Anstein Foss   type 12=tide
c--------------------------------------------------------------------
c
      parameter (maxrec=1024)
      parameter (maxdat=maxrec*2)
c
      parameter (ninmax=maxrec/8)
c
      common  nbo(maxrec),inh(8,ninmax),idata(maxdat)
      integer*2 nbo,inh,idata
c
      character*6 cid
c
      character*256 fname,option,ftype
c
      logical swapfile,swap
c
c--------------------------------------------
      inord=0
      nfile=0
      ierror=0
c
      narg=iargc()
      do n=1,narg
        call getarg(n,option)
        if(option(1:1).ne.'-') then
          fname=option
          nfile=nfile+1
        elseif(option.eq.'-n') then
          inord=1
        else
          ierror=ierror+1
        end if
      end do
      if(nfile.ne.1) ierror=ierror+1
c
      if(ierror.ne.0) then
        write(6,*) '  usage:  obslist obs_file'
        write(6,*) '     or:  obslist obs_file -n  (nord format file)'
        write(6,*) ' (warning: output to obs1.lst and obs2.lst)'
        stop
      endif
c
      lenrec=maxrec
      if(inord.eq.1) lenrec=256
      ninrec=lenrec/8
c--------------------------------------------
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c..output file
      open(10,file='obs1.lst',access='sequential',form='formatted',
     *                                    status='unknown',err=940)
      open(11,file='obs2.lst',access='sequential',form='formatted',
     *                                    status='unknown',err=940)
c
      ifil=20
c
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      write( 6,*) 'File: ',fname
      write(10,*) '========= OBSLIST ==========='
      write(10,*) 'File: ',fname
      write(11,*) '========= OBSLIST ==========='
      write(11,*) 'File: ',fname
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      open(ifil,file=fname,
     *          access='direct',form='unformatted',
     *          recl=lenrec*2/lrunit,
     *          status='old',iostat=ios,err=900)
c
      swap= swapfile(-ifil)
c
      irec=1
      read(ifil,rec=irec,iostat=ios,err=910) (nbo(i),i=1,lenrec)
      if (swap) call bswap2(lenrec,nbo)
c
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      write( 6,*) ' record 1, word 1-12:'
      write( 6,fmt='(3x,12i6)') (nbo(i),i=1,12)
      write(10,*) ' record 1, word 1-12:'
      write(10,fmt='(3x,12i6)') (nbo(i),i=1,12)
      write(11,*) ' record 1, word 1-12:'
      write(11,fmt='(3x,12i6)') (nbo(i),i=1,12)
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      ihtype=nbo(1)
      iterm=nbo(7)
      nreci1=3
      nreci2=nbo(9)+inord
c
      if(ihtype.eq.1) then
        ftype=' ***** synop *****'
      elseif(ihtype.eq.2) then
        ftype=' ***** aireps *****'
      elseif(ihtype.eq.3) then
        ftype=' ***** satob *****'
      elseif(ihtype.eq.4) then
        ftype=' ***** dribu *****'
      elseif(ihtype.eq.5) then
        ftype=' ***** temp *****'
      elseif(ihtype.eq.6) then
        ftype=' ***** pilot *****'
      elseif(ihtype.eq.7) then
        ftype=' ***** satem *****'
      elseif(ihtype.eq.8) then
        ftype=' ***** metar *****'
      elseif(ihtype.eq.9) then
        ftype=' ***** ocean *****'
      elseif(ihtype.eq.12) then
        ftype=' ***** tide *****'
      else
        write(ftype,*)  ' ***** Unknown obs. type:',ihtype
      endif
c
      write(6, *) ftype
      write(10,*) ftype
      write(11,*) ftype
c
      write(10,*)
      write(11,*)
c
c        sone, stasjons-nr, bredde, lengde, h@yde, utc, 'ident'
c
      write(10,1050)
ccc...........  01 001    -90.00   -180.00   -32767  1200  01001  no dat
 1050 format(' station  latitude longitude   height   utc  ident')
 1051 format(2x,i2.2,1x,i3.3,4x,f6.2,3x,f7.2,3x,i6,2x,i4.4,2x,a6)
 1052 format(2x,i2.2,1x,i3.3,4x,f6.2,3x,f7.2,25x,'no data')
 1053 format(2x,i2.2,1x,i3.3,4x,f6.2,3x,f7.2,25x,'error')
c
      nsta1=0
      nsta2=0
      nsta3=0
c
      lrecd2=0
      lword2=0
c
      do 100 ireci=nreci1,nreci2
      irec=ireci
      read(ifil,rec=irec,iostat=ios) ((inh(i,n),i=1,8),n=1,ninrec)
      if (swap) call bswap2(ninrec*8,inh)
      if(ios.ne.0) then
        write(6, *) '*** read error.  record,iostat: ',irec,ios
        write(11,*) '*** read error.  record,iostat: ',irec,ios
        goto 100
      endif
c
      do 110 n=1,ninrec
c
      write(11,fmt='('' inh: '',8i6)') (inh(i,n),i=1,8)
c
c        dummy (bakerst i innholds-fortegnelsen)
      if(inh(1,n).eq.-1 .and. inh(2,n).eq.-1 .and.
     *   inh(3,n).eq.-1 .and. inh(4,n).eq.-1) goto 110
c
      glat=inh(3,n)*0.01
      glon=inh(4,n)*0.01
c
      if(inh(6,n).lt.1 .or. inh(8,n).lt.20) then
        write(10,1052) inh(1,n),inh(2,n),glat,glon
        nsta1=nsta1+1
        goto 110
      endif
c
      irecd1=inh(6,n)+inord
      iw0=   inh(7,n)-1
      iw2=   inh(8,n)+iw0
      if(iw2.gt.maxdat) iw2=maxdat
      irecd2=irecd1-1+(iw2+lenrec-1)/lenrec
c
      l2=0
      if(irecd1.eq.lrecd2) then
        if(lword2.gt.0) then
          do 130 l=1,lenrec
  130       idata(l)=idata(lword2+l)
        endif
        irecd1=irecd1+1
        l2=lenrec
      endif
      do 140 irec=irecd1,irecd2
        l1=l2+1
        l2=l2+lenrec
        read(ifil,rec=irec,iostat=ios) (idata(l),l=l1,l2)
        if(ios.ne.0) then
          write(6, *) '*** read error.  record,iostat: ',irec,ios
          write(11,*) '*** read error.  record,iostat: ',irec,ios
          nsta2=nsta2+1
          lrecd2=0
          lword2=0
          goto 110
        endif
  140 continue
      lrecd2=irecd2
      lword2=l2-lenrec
      if (swap) call bswap2(iw2-iw0,idata(iw0+1))
c
      if(idata(iw0+7).ne.inh(3,n) .or.
     *   idata(iw0+8).ne.inh(4,n)) then
        write( 6,fmt='('' feil.  inh(1-8): '',8i6)') (inh(i,n),i=1,8)
        write( 6,fmt='(''       data(1-8): '',8i6)') (idata(i),i=1,8)
        write(11,fmt='('' feil.  inh(1-8): '',8i6)') (inh(i,n),i=1,8)
        write(11,fmt='(''       data(1-8): '',8i6)') (idata(i),i=1,8)
        write(10,1053) inh(1,n),inh(2,n),glat,glon
        nsta2=nsta2+1
        goto 110
      endif
c
      nsta3=nsta3+1
c
      id1=idata(iw0+4)
      id2=idata(iw0+5)
      id3=idata(iw0+6)
      id11=iand(ishft(id1,-8),255)
      id12=iand(id1,255)
      id21=iand(ishft(id2,-8),255)
      id22=iand(id2,255)
      id31=iand(ishft(id3,-8),255)
      id32=iand(id3,255)
      cid=char(id11)//char(id12)//char(id21)//char(id22)//
     *                            char(id31)//char(id32)
c
      iutc=idata(iw0+14)
      ihs= idata(iw0+15)
c
      write(10,1051) inh(1,n),inh(2,n),glat,glon,ihs,iutc,cid
c
  110 continue
c
  100 continue
c
      write( 6,*) ' number of stations with data:    ',nsta3
      write( 6,*) ' number of stations with error:   ',nsta2
      write( 6,*) ' number of stations without data: ',nsta1
c
      write(10,*)
      write(10,*) ' number of stations with data:    ',nsta3
      write(10,*) ' number of stations with error:   ',nsta2
      write(10,*) ' number of stations without data: ',nsta1
c
      write(11,*)
      write(11,*) ' number of stations with data:    ',nsta3
      write(11,*) ' number of stations with error:   ',nsta2
      write(11,*) ' number of stations without data: ',nsta1
c
  190 close(ifil)
c
      goto 950
c
  900 write(6,*) '*** open error.'
      write(6,*) '***   file:   ',fname
      write(6,*) '***   iostat: ',ios
      goto 950
  910 write(6,*) '*** read error.'
      write(6,*) '***   file:   ',fname
      write(6,*) '***   record: ',irec
      write(6,*) '***   iostat: ',ios
      goto 950
c
  940 write(6,*) ' **** open error. output print file'
      goto 950
c
  950 continue
      close(10)
      close(11)
      end
