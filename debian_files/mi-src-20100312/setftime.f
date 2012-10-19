      program setftime
c
c  Purpose:  Set date and time in a binary data file.
c            Made for "Year 2000" testing.
c            Input data files:
c             -  Standard FELT files
c             -  Observation files
c             -  Sequential model input/output file with fields
c             -  Sequential model input file with observations
c             -  Archive  FELT files
c             -  Line files
c
c  Usage: setftime -t year,month,day,hour,minute <option> file_in
c         Options: -f  : Standard FELT file
c                  -o  : Observation file
c                  -s  : Sequential model input/output file with fields
c                  -z  : Sequential analysis obs. file
c                  -y  : Sequential analysis satellite obs. file
c                  -a  : Archive FELT file
c                  -l  : Line file
c
c  Examples: setftime -t 2000,1,1,0,0 -f felt.dat
c            setftime -t 2000,1,1,0,0 -s modelout.dat
c            setftime -m setftime.list
c
c-----------------------------------------------------------------------
c
c      DNMI library routines:  cmdarg
c                              rlunit
c                              lenstr
c                              hrdiff
c                              vtime
c
c-----------------------------------------------------------------------
c  DNMI/FoU  02.04.1998  Anstein Foss ... for Year 2000 testing
c  DNMI/FoU  03.05.1998  Anstein Foss ... list file option and more
c  DNMI/FoU  19.08.1998  Anstein Foss ... -bug (felt)
c  DNMI/FoU  03.09.1998  Anstein Foss ... -y option (sat.obs.)
c  DNMI/FoU  09.03.1999  Anstein Foss ... corrected obs.file date setting
c-----------------------------------------------------------------------
c
c
c.cmdarg...................................................
      parameter (nopt=20)
      parameter (margs=1000)
      parameter (mispec=10,mrspec=1,mcspec=1)
c
      integer      iopt(nopt)
      integer      iopts(2,nopt)
      integer      ispec(mispec)
      real         rspec(mrspec)
      character*4  cspec(mcspec)
      character*1  copt(nopt)
      character*256 cargs(margs)
c.cmdarg...................................................
c
      parameter (mfiles=10)
c
      integer       itset(17,mfiles),ilist(7,margs),kpart(2,3)
      character*256 filename(mfiles)
      character*128 text,text2
c
c.cmdarg...................................................
c................1...2...3...4...5...6...7...8...9..10..
      data copt/'f','a','o','s','z','y','l','t','m','x',
     +          '0','1','2','3','4','5','6','7','8','9'/
      data iopt/ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 1 , 0 , 0 ,
     +           0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 /
c.cmdarg...................................................
c
c
      nopt0=0
      nopt9=0
      do n=1,nopt
	if(copt(n).eq.'0') nopt0=n
	if(copt(n).eq.'9') nopt9=n
      end do
      if(nopt0.eq.0 .or. nopt9.eq.0 .or. nopt9-nopt0.ne.9) then
	write(6,*) 'FATAL PROGRAM ERROR'
	write(6,*) '      nopt0,nopt9: ',nopt0,nopt9
	call exit(255)
      end if
c
      ierror=0
      nargs =0
c
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c
      if(nargs.lt.1) ierror=1
c
      iftype=0
      iflist=0
      nfext=-1
      iupdate=0
      if(ierror.eq.0) then
        do n=1,7
          if(iopts(1,n).gt.0) then
            if(iftype.ne.0) ierror=1
            iftype=n
          end if
        end do
        do n=nopt0,nopt9
          if(iopts(1,n).gt.0) then
            if(nfext.ne.-1) ierror=1
            nfext=n-nopt0
          end if
        end do
        if(nfext.eq.-1) nfext=0
        if(iopts(1, 9).ne.0) iflist=1
        if(iopts(1,10).ne.0) iupdate=1
        if(iftype.gt.0 .and. iopts(1,8).ne.5) ierror=1
        if(iflist.gt.0 .and. iopts(1,8).ne.0) ierror=1
        if(iftype.eq.0 .and. iflist.eq.0) ierror=1
        if(iftype.gt.0 .and. iflist.gt.0) ierror=1
        if(iftype.gt.0 .and. nargs.eq.0) ierror=1
        if(iflist.gt.0 .and. nargs.ne.1) ierror=1
      end if
c
      if(ierror.ne.0) then
        write(6,*) ' usage: setftime -t year,month,day,hour,minute',
     +                                ' <options> file(s)'
        write(6,*) '    or: setftime -m <list file>'
        write(6,*)
        write(6,*) '   option: -f : Standard FELT file'
        write(6,*) '           -o : Observation   file'
        write(6,*) '           -s : Sequential model i/o file (fields)'
        write(6,*) '           -z : Sequential analysis obs. file'
        write(6,*) '           -y : Sequential analysis satellite',
     +							' obs. file'
        write(6,*) '           -a : Archive FELT file'
        write(6,*) '           -l : Line file'
        write(6,*) '           -1 : also handling "-1" files'
        write(6,*) '           -2 : also handling "-1" and "-2" files'
        write(6,*) '           -3 ... -9 : "-1" ... "-N" files'
        write(6,*) '           -0 : handling only specified file(s)',
     +                                                  ' (default)'
        write(6,*) '           -x : execute date/time update always'
        write(6,*)
        write(6,*) '    "-1" files are at day-1 "-2" at day-2 ...'
        write(6,*) '    -f2 = -f -2'
        write(6,*) '    when using -f2 etc. and not -x, files are'
        write(6,*) '    moved from "file" to "file-1" etc. when',
     +                                  ' possible'
        write(6,*)
        write(6,*) '   List file format:   max no. of lines: ',margs
        write(6,*) '   yyyy,mm,dd,hh,mn <option> file'
        write(6,*) '   2000,01,01,00,00 -o2 syno00.dat'
        write(6,*) '   (options -t and -x not allowed in list file,'
        write(6,*) '    option formats -o2 -f4 etc. must be used)'
        stop
      end if
c
c
      iunitl=10
      iunit =20
      iunit2=30
c
c
      if(iftype.gt.0) then
c
        i=iopts(2,8)
        itset(1,1)=ispec(i)
        itset(2,1)=ispec(i+1)
        itset(3,1)=ispec(i+2)
        itset(4,1)=ispec(i+3)
        itset(5,1)=ispec(i+4)
c
        write(6,1001) itset(3,1),itset(2,1),itset(1,1),
     +                itset(4,1),itset(5,1)
 1001   format(' Set time: ',2(i2.2,'.'),i4.4,1x,i2.2,':',i2.2)
c
c..check legal date/time
        minset=itset(5,1)
        itset(5,1)=0
        call vtime(itset(1,1),ierror)
        itset(5,1)=minset
        if(minset.lt.00 .or. minset.gt.59) ierror=1
        if(ierror.ne.0) then
          write(6,*) 'ILLEGAL DATE/TIME !!!!!!!!!'
          call exit(2)
        end if
c
        nfiles=nfext+1
        do n=2,nfiles
          itset(1,n)=itset(1,n-1)
          itset(2,n)=itset(2,n-1)
          itset(3,n)=itset(3,n-1)
          itset(4,n)=itset(4,n-1)
          itset(5,n)=-24
          call vtime(itset(1,n),ierror)
          if(ierror.ne.0) write(6,*) 'PROGRAM ERROR : VTIME (1)'
          if(ierror.ne.0) call exit(255)
          itset(5,n)=itset(5,n-1)
        end do
c
      else
c
        write(6,*) 'List file: ',cargs(1)(1:lenstr(cargs(1),1))
        open(iunitl,file=cargs(1),
     +              access='sequential',form='formatted',
     +              status='old',iostat=ios)
        if(ios.ne.0) then
          write(6,*) 'Open error. List file. Iostat: ',ios
          call exit(2)
        end if
        ierr=0
        nargs=0
        do while (ios.eq.0 .and. nargs.lt.margs)
          nargs=nargs+1
          read(iunitl,fmt='(a)',iostat=ios) text
          if(ios.eq.0) then
	    lt=len(text)
	    k=0
	    do n=1,3
	      k1=0
	      k2=0
	      k3=0
	      do while (k3.eq.0 .and. k.lt.lt)
		k=k+1
		if(text(k:k).ne.' ') then
		  if(k1.eq.0) k1=k
		  k2=k
	        elseif(text(k:k).eq.' ' .and. k1.gt.0) then
		  k3=1
		end if
	      end do
	      kpart(1,n)=k1
	      kpart(2,n)=k2
	      if(k2.eq.0) ios=999
	    end do
	  end if
	  if(ios.eq.0) then
	    k1=kpart(1,1)
	    k2=kpart(2,1)
	    text2=text(k1:k2)//char(0)
            read(text2,*,iostat=ios) (ilist(i,nargs),i=1,5)
            if(ios.eq.0) then
              minset=ilist(5,nargs)
              ilist(5,nargs)=0
              call vtime(ilist(1,nargs),ierror)
              ilist(5,nargs)=minset
              if(minset.lt.00 .or. minset.gt.59) ierror=1
              if(ierror.ne.0) then
                write(6,*) 'ILLEGAL DATE/TIME !!!!!!!!!'
                ios=999
              end if
            end if
            if(ios.eq.0) then
	      k1=kpart(1,2)
	      k2=kpart(2,2)
	      text2=text(k1:k2)
              iftype=0
              nfext=-1
	      if(text(k1:k1).ne.'-') ios=999
	      k1=k1+1
	      do k=k1,k2
                kf=0
                do n=1,7
                  if(text(k:k).eq.copt(n)) then
                    if(iftype.ne.0) ios=999
                    iftype=n
                    kf=1
                  end if
                end do
                do n=nopt0,nopt9
                  if(text(k:k).eq.copt(n)) then
                    if(nfext.ne.-1) ios=999
                    nfext=n-nopt0
                    kf=1
                  end if
                end do
                if(kf.eq.0) ios=999
              end do
              if(iftype.eq.0) ios=999
              if(nfext.eq.-1) nfext=0
              ilist(6,nargs)=iftype
              ilist(7,nargs)=nfext
            end if
	    if(ios.eq.0) then
	      k1=kpart(1,3)
	      k2=kpart(2,3)
	      cargs(nargs)=text(k1:k2)
            else
              write(6,*) 'Error in line ',nargs,' :'
              write(7,*) text(1:lenstr(text,1))
              ierr=ierr+1
              ios=0
            end if
          end if
        end do
c
        close(iunitl)
        nargs=nargs-1
        write(6,*) 'No. of lines read: ',nargs
        if(ierr.gt.0 .or. nargs.lt.1) call exit(2)
c
      end if
c
c
      do iarg = 1,nargs
c
        filename(1)=cargs(iarg)
c
        if(iflist.eq.1) then
          itset(1,1)=ilist(1,iarg)
          itset(2,1)=ilist(2,iarg)
          itset(3,1)=ilist(3,iarg)
          itset(4,1)=ilist(4,iarg)
          itset(5,1)=ilist(5,iarg)
          iftype    =ilist(6,iarg)
          nfiles    =ilist(7,iarg)+1
          do n=2,nfiles
            itset(1,n)=itset(1,n-1)
            itset(2,n)=itset(2,n-1)
            itset(3,n)=itset(3,n-1)
            itset(4,n)=itset(4,n-1)
            itset(5,n)=-24
            call vtime(itset(1,n),ierror)
            if(ierror.ne.0) write(6,*) 'PROGRAM ERROR : VTIME (2)'
            if(ierror.ne.0) call exit(255)
            itset(5,n)=itset(5,n-1)
          end do
        end if
c
        do n=1,nfiles
          do i=6,17
            itset(i,n)=0
          end do
        end do
c
        k=lenstr(filename(1),1)
        if(nfiles.gt.1) then
          do n=2,nfiles
            filename(n)=filename(1)(1:k)//'-'//copt(nopt0+n-1)
          end do
        end if
c
        write(6,*) 'Input file: ',filename(1)(1:k)
c
        if(iftype.eq.1) then
c
          call stdfelt(nfiles,filename,itset,iunit,iupdate)
c
        elseif(iftype.eq.2) then
c
          call arcfelt(nfiles,filename,itset,iunit,iupdate)
c
        elseif(iftype.eq.3) then
c
          call obsfile(nfiles,filename,itset,iunit,iupdate)
c
        elseif(iftype.eq.4) then
c
          call seqfelt(nfiles,filename,itset,iunit,iunit2,iupdate)
c
        elseif(iftype.eq.5) then
c
          call anaobs(nfiles,filename,itset,iunit,iunit2,iupdate)
c
        elseif(iftype.eq.6) then
c
          call anasat(nfiles,filename,itset,iunit,iunit2,iupdate)
c
        elseif(iftype.eq.7) then
c
          call linefile(nfiles,filename,itset,iunit,iupdate)
c
        else
c
          write(6,*) 'PROGRAM ERROR:  iftype = ',iftype
          call exit(255)
c
        end if
c
      end do
c
      end
c
c***********************************************************************
c
      subroutine stdfelt(nfiles,filename,itset,iunit,iupdate)
c
c       standard felt file (type 999 only)
c
      integer       nfiles,iunit,iupdate
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      parameter (lrec=1024)
      parameter (ninhr=64)
c
      integer*2 idata(lrec*2),inh(16,ninhr),idrec1(lrec)
      integer*2 itim1,itim2,itim3
c
      call rlunit(lrunit)
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='direct',form='unformatted',
     +           recl=2048/lrunit,
     +           status='old',iostat=ios)
      irec=1
      if(ios.eq.0) read(iunit,rec=irec,iostat=ios) idrec1
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
      if(idrec1(1).ne.999) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Not a standard felt file !!!!'
        close(iunit)
        return
      end if
c
      itset( 6,nf)=idrec1(5)
      itset( 7,nf)=idrec1(6)/100
      itset( 8,nf)=idrec1(6)-(idrec1(6)/100)*100
      itset( 9,nf)=idrec1(7)/100
      itset(10,nf)=idrec1(7)-(idrec1(7)/100)*100
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
      itim1=itset(1,nf)
      itim2=itset(2,nf)*100+itset(3,nf)
      itim3=itset(4,nf)*100+itset(5,nf)
c
      ireci1=3
      ireci2=idrec1(9)
      ninh  =idrec1(11)
      ni2   =0
c
      do ireci=ireci1,ireci2
c
        irec=ireci
        read(iunit,rec=irec,iostat=ios,err=400) inh
c
        ni1=ni2+1
        ni2=min(ni2+ninhr,ninh)
        nin=ni2-ni1+1
c
        do ni=1,nin
c
          inh(3,ni)=itim1
          inh(4,ni)=itim2
          inh(5,ni)=itim3
c
          if(inh(6,ni).gt.0 .and. inh(8,ni).gt.0) then
c
            inh06=inh( 6,ni)
            inh07=inh( 7,ni)
            inh08=inh( 8,ni)
            inh16=inh(16,ni)
            if(inh16.lt.0) inh16=0
            if(inh07.lt.1 .or. inh07.gt.lrec) inh07=1
            irecd1=inh06
            iword =inh07
            ldata1=inh08
            ldata2=inh16/100
            irecd2=inh16-ldata2*100
            irec1=irecd1+irecd2*32767
            ldata=ldata1+ldata2*32767
            iw=iword-1
            irec=irec1
            read(iunit,rec=irec,iostat=ios,err=400) (idata(i),i=1,1024)
            if(iw+14.gt.1024) then
              irec=irec+1
              read(iunit,rec=irec,iostat=ios,err=400)
     +                                  (idata(i),i=1025,2048)
            end if
            idata(iw+12)=itim1
            idata(iw+13)=itim2
            idata(iw+14)=itim3
            irec=irec1
            write(iunit,rec=irec,iostat=ios,err=450) (idata(i),i=1,1024)
            if(iw+14.gt.1024) then
              irec=irec+1
              write(iunit,rec=irec,iostat=ios,err=450)
     +                                  (idata(i),i=1025,2048)
            end if
c
          end if
c
        end do
c
        irec=ireci
        write(iunit,rec=irec,iostat=ios,err=450) inh
c
      end do
c
      idrec1(5)=itim1
      idrec1(6)=itim2
      idrec1(7)=itim3
c
c..machine/system time
      call daytim(iyear,month,iday,ihour,minute,isecnd)
      idrec1(2)=iyear
      idrec1(3)=month*100+iday
      idrec1(4)=ihour*100+minute
c
      irec=1
      write(iunit,rec=irec,iostat=ios,err=450) idrec1
c
      goto 500
c
  400 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'read error.  iostat,record= ',ios,irec
      goto 500
c
  450 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'write error.  iostat,record= ',ios,irec
      goto 500
c
  500 close(iunit)
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine arcfelt(nfiles,filename,itset,iunit,iupdate)
c
c       archive (or cyclic archive) felt file (type 998,997)
c
      integer       nfiles,iunit,iupdate
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      parameter (lrec=1024)
      parameter (ninhr=64)
c
      integer   itime(5)
      integer*2 idata(lrec*2),inh(16,ninhr),idrec1(lrec)
      integer*2 ltim1,ltim2,ltim3,itim1,itim2,itim3
c
      call rlunit(lrunit)
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='direct',form='unformatted',
     +           recl=2048/lrunit,
     +           status='old',iostat=ios)
      irec=1
      if(ios.eq.0) read(iunit,rec=irec,iostat=ios) idrec1
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
      if(idrec1(1).ne.998 .and. idrec1(1).ne.997) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Not an archive felt file !!!!'
        close(iunit)
        return
      end if
c
      itset( 6,nf)=idrec1(5)
      itset( 7,nf)=idrec1(6)/100
      itset( 8,nf)=idrec1(6)-(idrec1(6)/100)*100
      itset( 9,nf)=idrec1(7)/100
      itset(10,nf)=idrec1(7)-(idrec1(7)/100)*100
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
      itim1=itset(1,nf)
      itim2=itset(2,nf)*100+itset(3,nf)
      itim3=itset(4,nf)*100+itset(5,nf)
      ltim1=idrec1(5)
      ltim2=idrec1(6)
      ltim3=idrec1(7)
c
      itime(1)=idrec1(5)
      itime(2)=idrec1(6)/100
      itime(3)=idrec1(6)-(idrec1(6)/100)*100
      itime(4)=idrec1(7)/100
      itime(5)=idrec1(7)-(idrec1(7)/100)*100
c
c..subr. hrdiff and vtime can't handle minutes, so that's done here
c..(but not necessary to handle this for any known archive felt files)
      minset=itset(5,nf)
      mintim=itime(5)
      itset(5,nf)=0
      itime(5)=0
      call hrdiff(0,0,itime,itset(1,nf),ihdiff,ierr1,ierr2)
      itset(5,nf)=minset
      itime(5)=mintim
      if(ierr1.ne.0 .or. ierr2.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'BAD date/time.'
        write(6,*) '    Data: ',itime
        write(6,*) '    Set : ',itset
        close(iunit)
        return
      end if
      mndiff=minset-mintim
      if(mndiff.lt.00) then
        ihdiff=ihdiff-1
        mndiff=mndiff+60
      elseif(mndiff.gt.59) then
        ihdiff=ihdiff+1
        mndiff=mndiff-60
      end if
c
      ireci1=3
      ireci2=idrec1(9)
      ninh  =idrec1(11)
      ni2   =0
c
      do ireci=ireci1,ireci2
c
        irec=ireci
        read(iunit,rec=irec,iostat=ios,err=400) inh
c
        ni1=ni2+1
        ni2=min(ni2+ninhr,ninh)
        nin=ni2-ni1+1
c
        do ni=1,nin
c
          if(inh(3,ni).ne.ltim1 .or. inh(4,ni).ne.ltim2
     +                          .or. inh(5,ni).ne.ltim3) then
            ltim1=inh(3,ni)
            ltim2=inh(4,ni)
            ltim3=inh(5,ni)
            itime(1)=inh(3,ni)
            itime(2)=inh(4,ni)/100
            itime(3)=inh(4,ni)-(inh(4,ni)/100)*100
            itime(4)=inh(5,ni)/100
            itime(5)=inh(5,ni)-(inh(5,ni)/100)*100
            mintim=itime(5)
            minute=mintim+mndiff
            ihd=ihdiff
            if(minute.lt.00) then
              ihd=ihd-1
              minute=minute+60
            elseif(minute.gt.59) then
              ihd=ihd+1
              minute=minute-60
            end if
            itime(5)=ihd
            call vtime(itime,ierr)
            if(ierr.ne.0) then
              write(6,*) filename(nf)(1:lenstr(filename(nf),1))
              write(6,*) 'BAD TIME: ',itime
              write(6,*) 'FILE DESTROYED'
              call exit(3)
            end if
            itim1=itime(1)
            itim2=itime(2)*100+itime(3)
            itim3=itime(4)*100+minute
          end if
          inh(3,ni)=itim1
          inh(4,ni)=itim2
          inh(5,ni)=itim3
c
          if(inh(6,ni).gt.0 .and. inh(8,ni).gt.0) then
c
            inh06=inh( 6,ni)
            inh07=inh( 7,ni)
            inh08=inh( 8,ni)
            inh16=inh(16,ni)
            if(inh16.lt.0) inh16=0
            if(inh07.lt.1 .or. inh07.gt.lrec) inh07=1
            irecd1=inh06
            iword =inh07
            ldata1=inh08
            ldata2=inh16/100
            irecd2=inh16-ldata2*100
            irec1=irecd1+irecd2*32767
            ldata=ldata1+ldata2*32767
            iw=iword-1
            irec=irec1
            read(iunit,rec=irec,iostat=ios,err=400) (idata(i),i=1,1024)
            if(iw+14.gt.1024) then
              irec=irec+1
              read(iunit,rec=irec,iostat=ios,err=400)
     +                                  (idata(i),i=1025,2048)
            end if
            idata(iw+12)=itim1
            idata(iw+13)=itim2
            idata(iw+14)=itim3
            irec=irec1
            write(iunit,rec=irec,iostat=ios,err=450) (idata(i),i=1,1024)
            if(iw+14.gt.1024) then
              irec=irec+1
              write(iunit,rec=irec,iostat=ios,err=450)
     +                                  (idata(i),i=1025,2048)
            end if
c
          end if
c
        end do
c
        irec=ireci
        write(iunit,rec=irec,iostat=ios,err=450) inh
c
      end do
c
      idrec1(5)=itset(1,nf)
      idrec1(6)=itset(2,nf)*100+itset(3,nf)
      idrec1(7)=itset(4,nf)*100+itset(5,nf)
c
      do nt=1,2
        iw=16+3*nt
        itime(1)=idrec1(iw+1)
        itime(2)=idrec1(iw+2)/100
        itime(3)=idrec1(iw+2)-(idrec1(iw+2)/100)*100
        itime(4)=idrec1(iw+3)/100
        itime(5)=idrec1(iw+3)-(idrec1(iw+3)/100)*100
        mintim=itime(5)
        minute=mintim+mndiff
        ihd=ihdiff
        if(minute.lt.00) then
          ihd=ihd-1
          minute=minute+60
        elseif(minute.gt.59) then
          ihd=ihd+1
          minute=minute-60
        end if
        itime(5)=ihd
        call vtime(itime,ierr)
        if(ierr.ne.0) then
          write(6,*) filename(nf)(1:lenstr(filename(nf),1))
          write(6,*) 'BAD TIME: ',itime
          write(6,*) 'FILE DESTROYED'
          call exit(3)
        end if
        idrec1(iw+1)=itime(1)
        idrec1(iw+2)=itime(2)*100+itime(3)
        idrec1(iw+3)=itime(4)*100+minute
      end do
c
c..machine/system time
      call daytim(iyear,month,iday,ihour,minute,isecnd)
      idrec1(2)=iyear
      idrec1(3)=month*100+iday
      idrec1(4)=ihour*100+minute
c
      irec=1
      write(iunit,rec=irec,iostat=ios,err=450) idrec1
c
      goto 500
c
  400 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'read error.  iostat,record= ',ios,irec
      goto 500
c
  450 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'write error.  iostat,record= ',ios,irec
      goto 500
c
  500 close(iunit)
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine seqfelt(nfiles,filename,itset,iunit,iunit2,iupdate)
c
c       sequential model i/o file with fields
c
      integer       nfiles,iunit,iunit2,iupdate
      integer       itime(5)
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      parameter (ldata=100000+50)
c
      integer*2 ident(20),idata(ldata)
      integer*2 itim1,itim2,itim3,ltim1,ltim2,ltim3
      character*256 tmpfile
      character*512 syscmd
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='sequential',form='unformatted',
     +           status='old',iostat=ios)
      if(ios.eq.0) read(iunit,iostat=ios) ident
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
c
      itset( 6,nf)=ident(12)
      itset( 7,nf)=ident(13)/100
      itset( 8,nf)=ident(13)-(ident(13)/100)*100
      itset( 9,nf)=ident(14)/100
      itset(10,nf)=ident(14)-(ident(14)/100)*100
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
      rewind(iunit)
c
      itim1=itset(1,nf)
      itim2=itset(2,nf)*100+itset(3,nf)
      itim3=itset(4,nf)*100+itset(5,nf)
      ltim1=ident(12)
      ltim2=ident(13)
      ltim3=ident(14)
c
      itime(1)=ident(12)
      itime(2)=ident(13)/100
      itime(3)=ident(13)-(ident(13)/100)*100
      itime(4)=ident(14)/100
      itime(5)=ident(14)-(ident(14)/100)*100
c
c..subr. hrdiff and vtime can't handle minutes, so that's done here
c..(but not necessary to handle this for any known archive felt files)
      minset=itset(5,nf)
      mintim=itime(5)
      itset(5,nf)=0
      itime(5)=0
      call hrdiff(0,0,itime,itset(1,nf),ihdiff,ierr1,ierr2)
      itset(5,nf)=minset
      itime(5)=mintim
      if(ierr1.ne.0 .or. ierr2.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'BAD date/time.'
        write(6,*) '    Data: ',itime
        write(6,*) '    Set : ',itset
        close(iunit)
        return
      end if
      mndiff=minset-mintim
      if(mndiff.lt.00) then
        ihdiff=ihdiff-1
        mndiff=mndiff+60
      elseif(mndiff.gt.59) then
        ihdiff=ihdiff+1
        mndiff=mndiff-60
      end if
c
      tmpfile=filename(nf)(1:lenstr(filename(nf),1))//'.TMP'
      open(iunit2,file=tmpfile,
     +            access='sequential',form='unformatted',
     +            status='unknown',iostat=ios)
      if(ios.ne.0) then
        write(6,*) tmpfile(1:lenstr(tmpfile,1))
        write(6,*) 'Open error. Tmp file. Iostat: ',ios
        close(iunit)
        return
      end if
c
      do while (.true.)
c
        read(iunit,iostat=ios,end=500,err=400) ident
c
        nx=ident(10)
        ny=ident(11)
        igtype=ident(9)
        lgeom=0
        if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
c
        ld=nx*ny+lgeom
        if(ld.gt.ldata) goto 410
c
        read(iunit,iostat=ios,err=400) (idata(i),i=1,ld)
c
	if(ident(12).ne.ltim1 .or.
     +     ident(13).ne.ltim2 .or.
     +     ident(14).ne.ltim3) then
          ltim1=ident(12)
          ltim2=ident(13)
          ltim3=ident(14)
          itime(1)=ident(12)
          itime(2)=ident(13)/100
          itime(3)=ident(13)-(ident(13)/100)*100
          itime(4)=ident(14)/100
          itime(5)=ident(14)-(ident(14)/100)*100
          mintim=itime(5)
          minute=mintim+mndiff
          ihd=ihdiff
          if(minute.lt.00) then
            ihd=ihd-1
            minute=minute+60
          elseif(minute.gt.59) then
            ihd=ihd+1
            minute=minute-60
          end if
          itime(5)=ihd
          call vtime(itime,ierr)
          if(ierr.ne.0) then
            write(6,*) filename(nf)(1:lenstr(filename(nf),1))
            write(6,*) 'BAD TIME: ',itime
            write(6,*) 'FILE DESTROYED'
            call exit(3)
          end if
          itim1=itime(1)
          itim2=itime(2)*100+itime(3)
          itim3=itime(4)*100+minute
        end if
c
        ident(12)=itim1
        ident(13)=itim2
        ident(14)=itim3
c
        write(iunit2,iostat=ios,err=450) ident
        write(iunit2,iostat=ios,err=450) (idata(i),i=1,ld)
c
      end do
c
      goto 500
c
  400 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'read error.  iostat= ',ios
      goto 500
c
  410 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'too big field.  ld,ldata: ',ld,ldata
      write(6,*) '             nx,ny,lgeom: ',nx,ny,lgeom
      goto 500
c
  450 write(6,*) tmpfile(1:lenstr(tmpfile,1))
      write(6,*) 'write error.  iostat= ',ios
      goto 500
c
  500 close(iunit)
      close(iunit2)
      if(loop.lt.nloop) goto 550
c
      syscmd=' '
      syscmd='mv -f '//tmpfile(1:lenstr(tmpfile,1))//' '
     +               //filename(nf)(1:lenstr(filename(nf),1))//char(0)
c##################################################################
      write(6,*) '>> ',syscmd(1:lenstr(syscmd,1))
c##################################################################
      call system(syscmd)
c
  550 continue
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine obsfile(nfiles,filename,itset,iunit,iupdate)
c
c       standard dnmi observation files
c       (syno,arep,sato,drau,temp,pilo,sate,meta,"ers-files")
c
      integer       nfiles,iunit,iupdate
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      parameter (ninhr=128)
c
      integer*2 idata(2048),inh(8,ninhr),idrec1(1024)
      integer*2 itim1,itim2,itim3
      integer*2 itim1m,itim2m,itim1p,itim2p
      integer*2 itimo1,itimo2,itimo1m,itimo2m,itimo1p,itimo2p
      integer   itime(5)
c
      call rlunit(lrunit)
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='direct',form='unformatted',
     +           recl=2048/lrunit,
     +           status='old',iostat=ios)
      irec=1
      if(ios.eq.0) read(iunit,rec=irec,iostat=ios) idrec1
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
      if(idrec1(1).lt.1 .or. idrec1(1).gt.9) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Not a standard observation file !!!!'
        close(iunit)
        return
      end if
c
c..not changing hour and minutes (file header, index or data)
      itim1=itset(1,nf)
      itim2=itset(2,nf)*100+itset(3,nf)
      itim3=itset(4,nf)*100+itset(5,nf)
      if(itim3.ne.idrec1(7)) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'BAD observation termin (not changing hour,minutes):'
        write(6,*) '    File: ',(idrec1(i),i=5,7)
        write(6,*) '    Set:  ',itim1,itim2,itim3
        close(iunit)
        return
      end if
c
      itset( 6,nf)=idrec1(5)
      itset( 7,nf)=idrec1(6)/100
      itset( 8,nf)=idrec1(6)-(idrec1(6)/100)*100
      itset( 9,nf)=idrec1(7)/100
      itset(10,nf)=idrec1(7)-(idrec1(7)/100)*100
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
c..data date may be the day before or after the main (file) date  
c..old day
      itimo1=idrec1(5)
      itimo2=idrec1(6)
c..old day before
      do i=1,4
	itime(i)=itset(5+i,nf)
      end do
      itime(5)=-24
      call vtime(itime,ierror)
      if(ierror.eq.0) then
	itimo1m=itime(1)
	itimo2m=itime(2)*100+itime(3)
      else
	itimo1m=9999
	itimo2m=9999
      end if
c..old day after
      do i=1,4
	itime(i)=itset(5+i,nf)
      end do
      itime(5)=+24
      call vtime(itime,ierror)
      if(ierror.eq.0) then
	itimo1p=itime(1)
	itimo2p=itime(2)*100+itime(3)
      else
	itimo1p=9999
	itimo2p=9999
      end if
c
c..new day
      itim1=itset(1,nf)
      itim2=itset(2,nf)*100+itset(3,nf)
      itim3=itset(4,nf)*100+itset(5,nf)
c..new day before
      do i=1,4
	itime(i)=itset(i,nf)
      end do
      itime(5)=-24
      call vtime(itime,ierror)
      if(ierror.eq.0) then
	itim1m=itime(1)
	itim2m=itime(2)*100+itime(3)
      else
	itim1m=9999
	itim2m=9999
      end if
c..new day after
      do i=1,4
	itime(i)=itset(i,nf)
      end do
      itime(5)=+24
      call vtime(itime,ierror)
      if(ierror.eq.0) then
	itim1p=itime(1)
	itim2p=itime(2)*100+itime(3)
      else
	itim1p=9999
	itim2p=9999
      end if
c
      ireci1=3
      ireci2=idrec1(9)
      ninh  =idrec1(11)+idrec1(12)
      ni2   =0
c
      do ireci=ireci1,ireci2
c
        irec=ireci
        read(iunit,rec=irec,iostat=ios,err=400) inh
c
        ni1=ni2+1
        ni2=min(ni2+ninhr,ninh)
        nin=ni2-ni1+1
c
        do ni=1,nin
c
c..only hour,minutes in index (not updated)
ccc       inh(5,ni)=itim3
c
          if(inh(6,ni).gt.0 .and. inh(7,ni).gt.0
     +                      .and. inh(8,ni).gt.0) then
c
            irec1=inh(6,ni)
            iword=inh(7,ni)
            ldata=inh(8,ni)
            iw=iword-1
            irec=irec1
            read(iunit,rec=irec,iostat=ios,err=400) (idata(i),i=1,1024)
            if(iw+14.gt.1024) then
              irec=irec+1
              read(iunit,rec=irec,iostat=ios,err=400)
     +                                  (idata(i),i=1025,2048)
            end if
	    if(idata(iw+12).eq.itimo1 .and.
     +	       idata(iw+13).eq.itimo2) then
              idata(iw+12)=itim1
              idata(iw+13)=itim2
c..hour,minutes not updated
ccc           idata(iw+14)=itim3
	    elseif(idata(iw+12).eq.itimo1m .and.
     +	           idata(iw+13).eq.itimo2m) then
              idata(iw+12)=itim1m
              idata(iw+13)=itim2m
	    elseif(idata(iw+12).eq.itimo1p .and.
     +	           idata(iw+13).eq.itimo2p) then
              idata(iw+12)=itim1p
              idata(iw+13)=itim2p
	    else
	      write(6,*) '     Bad date not changed: ',
     +				idata(iw+12),idata(iw+13)
	    end if
            irec=irec1
            write(iunit,rec=irec,iostat=ios,err=450) (idata(i),i=1,1024)
            if(iw+14.gt.1024) then
              irec=irec+1
              write(iunit,rec=irec,iostat=ios,err=450)
     +                                  (idata(i),i=1025,2048)
            end if
c
          end if
c
        end do
c
        irec=ireci
        write(iunit,rec=irec,iostat=ios,err=450) inh
c
      end do
c
      idrec1(5)=itim1
      idrec1(6)=itim2
c..hour,minutes not updated
ccc   idrec1(7)=itim3
c
c..machine/system time
      call daytim(iyear,month,iday,ihour,minute,isecnd)
      idrec1(2)=iyear
      idrec1(3)=month*100+iday
      idrec1(4)=ihour*100+minute
c
      irec=1
      write(iunit,rec=irec,iostat=ios,err=450) idrec1
c
      goto 500
c
  400 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'read error.  iostat,record= ',ios,irec
      goto 500
c
  450 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'write error.  iostat,record= ',ios,irec
      goto 500
c
  500 close(iunit)
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine anaobs(nfiles,filename,itset,iunit,iunit2,iupdate)
c
c       sequential file with observations for analysis
c
      integer       nfiles,iunit,iunit2,iupdate
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      parameter (lrec=1024)
c
      integer*2 idata(lrec)
      character*256 tmpfile
      character*512 syscmd
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='sequential',form='unformatted',
     +           status='old',iostat=ios)
      if(ios.eq.0) read(iunit,iostat=ios) idata
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
c
      itset( 6,nf)=idata( 7)
      itset( 7,nf)=idata( 8)
      itset( 8,nf)=idata( 9)
      itset( 9,nf)=idata(10)
      itset(10,nf)=00
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
      if(itset(5,nf).ne.0) write(6,*) 'NOT MINUTES in ana obs files !!!'
c
      tmpfile=filename(nf)(1:lenstr(filename(nf),1))//'.TMP'
      open(iunit2,file=tmpfile,
     +            access='sequential',form='unformatted',
     +            status='unknown',iostat=ios)
      if(ios.ne.0) then
        write(6,*) tmpfile(1:lenstr(tmpfile,1))
        write(6,*) 'Open error. Tmp file. Iostat: ',ios
        close(iunit)
        return
      end if
c
      nrec=idata(1)
      llast=idata(6)
c
c..the only date/time in file (only time difference in data)
      idata( 7)=itset(1,nf)
      idata( 8)=itset(2,nf)
      idata( 9)=itset(3,nf)
      idata(10)=itset(4,nf)
c
      ld=lrec
      write(iunit2,iostat=ios,err=450) (idata(i),i=1,ld)
c
      do irec=2,nrec
c
        ld=lrec
        if(irec.eq.nrec) ld=llast
        read(iunit,iostat=ios,end=500,err=400) (idata(i),i=1,ld)
c
        write(iunit2,iostat=ios,err=450) (idata(i),i=1,ld)
c
      end do
c
      goto 500
c
  400 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'read error.  iostat= ',ios
      goto 500
c
  450 write(6,*) tmpfile(1:lenstr(tmpfile,1))
      write(6,*) 'write error.  iostat= ',ios
      goto 500
c
  500 close(iunit)
      close(iunit2)
      if(loop.lt.nloop) goto 550
c
      syscmd=' '
      syscmd='mv -f '//tmpfile(1:lenstr(tmpfile,1))//' '
     +               //filename(nf)(1:lenstr(filename(nf),1))//char(0)
c##################################################################
      write(6,*) '>> ',syscmd(1:lenstr(syscmd,1))
c##################################################################
      call system(syscmd)
c
  550 continue
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine anasat(nfiles,filename,itset,iunit,iunit2,iupdate)
c
c       sequential file with satellite observations for analysis
c
      integer       nfiles,iunit,iunit2,iupdate
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      REAL scatt_obs(4),               ! obs-vector  - scatt obs
     &     scatt_pos(4),               ! positions, lat/lon, x/y
     &     DistCone(2),                ! distance to cone for scatt
     &                                 ! wind solution 1 and 2
     &     alt_obs,                    ! obs-vector, satellite windspeed
     &     alt_pos(4)                  ! positions, lat/lon, x/y
      INTEGER
     &     anatime(4),                 ! analysis time
     &     scatt_kldif,                ! timedifference in minuttes between
     &                                 ! observation and analysis
     &     alt_kldif,
     &     idfs(3), idfa(3)            ! identification
      real grdnb,xpinp,ypinp,aninp,fiinp
c
      character*256 tmpfile
      character*512 syscmd
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='sequential',form='unformatted',
     +           status='old',iostat=ios)
c read grid-information:
      if(ios.eq.0) read(iunit,iostat=ios) grdnb,xpinp,ypinp,aninp,fiinp
c read time-information:
      if(ios.eq.0) read(iunit,iostat=ios) (anatime(il), il=1,4)
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
c
      itset( 6,nf)=anatime(1)
      itset( 7,nf)=anatime(2)
      itset( 8,nf)=anatime(3)
      itset( 9,nf)=anatime(4)
      itset(10,nf)=00
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
      if(itset(5,nf).ne.0)
     +   write(6,*) 'NOT MINUTES in ana sat.obs files !!!'
c
      tmpfile=filename(nf)(1:lenstr(filename(nf),1))//'.TMP'
      open(iunit2,file=tmpfile,
     +            access='sequential',form='unformatted',
     +            status='unknown',iostat=ios)
      if(ios.ne.0) then
        write(6,*) tmpfile(1:lenstr(tmpfile,1))
        write(6,*) 'Open error. Tmp file. Iostat: ',ios
        close(iunit)
        return
      end if
c
c..the only date/time in file (only time difference in data)
      anatime(1)=itset(1,nf)
      anatime(2)=itset(2,nf)
      anatime(3)=itset(3,nf)
      anatime(4)=itset(4,nf)
c
c read grid-information:
      write(iunit2,iostat=ios,err=450) grdnb,xpinp,ypinp,aninp,fiinp
c read time-information:
      write(iunit2,iostat=ios,err=450) (anatime(il), il=1,4)
c
C Read the number of scatterometer obsrvations :
      read( iunit, iostat=ios, err=400 ) nbscatt
      write(iunit2,iostat=ios,err=450)   nbscatt
c read identification
      read( iunit, iostat=ios, err=400 ) (idfs(il), il=1,3)
      write(iunit2,iostat=ios,err=450)   (idfs(il), il=1,3)

c read scatterometer observations:
      do iut = 1, nbscatt
        read( iunit, iostat=ios, err=400)
     &      (scatt_obs(il), il=1,4),
     &      (scatt_pos(ik), ik=1,4),
     &      (DistCone(im), im=1,2),
     &       scatt_kldif
        write(iunit2,iostat=ios,err=450)
     &      (scatt_obs(il), il=1,4),
     &      (scatt_pos(ik), ik=1,4),
     &      (DistCone(im), im=1,2),
     &       scatt_kldif
      enddo

C Read the number of satellite windspeed obsrvations :
      read( iunit, iostat=ios, err=400) nbalt
      write(iunit2,iostat=ios,err=450)  nbalt
c read identification
      read( iunit, iostat=ios, err=400)
     &      (idfa(il), il=1,3)
      write(iunit2,iostat=ios,err=450)
     &      (idfa(il), il=1,3)

c read satellite windspeed observations:
      do iut = 1, nbalt
        read( iunit, iostat=ios, err=400)
     &       alt_obs,
     &       (alt_pos(ik), ik=1,4),
     &       alt_kldif
        write(iunit2,iostat=ios,err=450)
     &       alt_obs,
     &       (alt_pos(ik), ik=1,4),
     &       alt_kldif
      enddo
c
      goto 500
c
  400 write(6,*) filename(nf)(1:lenstr(filename(nf),1))
      write(6,*) 'read error.  iostat= ',ios
      goto 500
c
  450 write(6,*) tmpfile(1:lenstr(tmpfile,1))
      write(6,*) 'write error.  iostat= ',ios
      goto 500
c
  500 close(iunit)
      close(iunit2)
      if(loop.lt.nloop) goto 550
c
      syscmd=' '
      syscmd='mv -f '//tmpfile(1:lenstr(tmpfile,1))//' '
     +               //filename(nf)(1:lenstr(filename(nf),1))//char(0)
c##################################################################
      write(6,*) '>> ',syscmd(1:lenstr(syscmd,1))
c##################################################################
      call system(syscmd)
c
  550 continue
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine linefile(nfiles,filename,itset,iunit,iupdate)
c
c       linefile (digitized etc.)
c
      integer       nfiles,iunit,iupdate
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      integer*2 idata(2,256)
c
      call rlunit(lrunit)
c
      nloop=1
      if(nfiles.gt.1 .and. iupdate.eq.0) nloop=2
c
      do loop=1,nloop
c
      do nf=nfiles,1,-1
c
      open(iunit,file=filename(nf),
     +           access='direct',form='unformatted',
     +           recl=1024/lrunit,
     +           status='old',iostat=ios)
      irec=1
      if(ios.eq.0) read(iunit,rec=irec,iostat=ios) idata
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) 'Open/read error. Iostat: ',ios
        close(iunit)
        return
      end if
c
      itset( 6,nf)=idata(1,2)
      itset( 7,nf)=idata(2,2)
      itset( 8,nf)=idata(1,3)
      itset( 9,nf)=idata(1,4)
      itset(10,nf)=00
c
      if(loop.lt.nloop) goto 500
c
      if(iupdate.eq.0) then
        iup=0
        do i=1,5
          if(itset(i,nf).ne.itset(5+i,nf)) iup=1
        end do
        if(iup.eq.0) then
          write(6,*) 'OK  time: ',
     +               filename(nf)(1:lenstr(filename(nf),1))
          goto 500
        end if
      end if
      write(6,*) 'NEW time: ',filename(nf)(1:lenstr(filename(nf),1))
c
      if(itset(5,nf).ne.0) write(6,*) 'NOT MINUTES in linefiles !!!'
c
      idata(1,2)=itset(1,nf)
      idata(2,2)=itset(2,nf)
      idata(1,3)=itset(3,nf)
      idata(2,3)=itset(4,nf)
c
      irec=1
      write(iunit,rec=irec,iostat=ios) idata
      if(ios.ne.0) then
        write(6,*) filename(nf)(1:lenstr(filename(nf),1))
        write(6,*) ' Write error. Iostat: ',ios
      end if
c
  500 close(iunit)
c
c.....end do nf=nfiles,1,-1
      end do
c
      if(loop.lt.nloop) call mvfile(nfiles,filename,itset)
c
c.....end do loop=1,nloop
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine mvfile(nfiles,filename,itset)
c
c       move files from file.dat to file.dat-1 etc.
c
c       itset(1:5,n)   : year,month,day,hour,minute to set
c       itset(6:10,n)  : current year,month,day,hour,minute
c       itset(11,n)    : work
c       itset(12,n)    : work
c       itset(13:17,n) : year,month,day,hour,minute during move
c
      integer       nfiles
      integer       itset(17,nfiles)
      character*(*) filename(nfiles)
c
      character*512 syscmd
c
      do n=1,nfiles
        k=n
        do i=1,5
          if(itset(i,n).ne.itset(5+i,n)) k=0
        end do
        itset(11,n)=k
        itset(12,n)=k
      end do
c
      do n=1,nfiles
        if(itset(11,n).eq.0) then
          k=0
          m=0
          do while (k.eq.0 .and. m.lt.nfiles)
            m=m+1
            k=m
            do i=1,5
              if(itset(i,n).ne.itset(5+i,m)) k=0
            end do
          end do
          itset(11,n)=k
          if(k.gt.0) itset(12,k)=n
        end if
      end do
c##################################################################
c     do n=1,nfiles
c       write(6,1001) n,(itset(i,n),i=1,12),filename(n)
c1001   format(1x,i2,':',2(2x,i4.4,4i3.2),2i4,3x,a4)
c     end do
c##################################################################
c
      do n=1,nfiles
        if(itset(11,n).eq.0) then
          k=0
          m=0
          do while (k.eq.0 .and. m.lt.nfiles)
            m=m+1
            if(itset(12,m).lt.1) k=m
          end do
          if(k.lt.1) write(6,*) 'PROGRAM ERROR. MVFILE.'
          if(k.lt.1) call exit(255)
          itset(11,n)=k
          itset(12,k)=n
        end if
      end do
c##################################################################
c     do n=1,nfiles
c       write(6,1001) n,(itset(i,n),i=1,12),filename(n)
c     end do
c##################################################################
c
      do n=nfiles,1,-1
        if(itset(11,n).ne.n) then
          k=itset(11,n)
          m=itset(12,n)
          if(m.lt.n) then
            lfn=lenstr(filename(n),0)
            syscmd=' '
            syscmd='mv -f '//filename(n)(1:lfn)//' '
     +                     //filename(n)(1:lfn)//'.TMP'//char(0)
c##################################################################
            write(6,*) '>> ',syscmd(1:lenstr(syscmd,1))
c##################################################################
            call system(syscmd)
            do i=1,5
              itset(12+i,n)=itset(5+i,n)
            end do
          end if
          if(k.lt.n) then
            lfk=lenstr(filename(k),0)
            lfn=lenstr(filename(n),0)
            syscmd=' '
            syscmd='mv -f '//filename(k)(1:lfk)//' '
     +                     //filename(n)(1:lfn)//char(0)
c##################################################################
            write(6,*) '>> ',syscmd(1:lenstr(syscmd,1))
c##################################################################
            call system(syscmd)
            do i=1,5
              itset(5+i,n)=itset(5+i,k)
            end do
          else
            lfk=lenstr(filename(k),0)
            lfn=lenstr(filename(n),0)
            syscmd=' '
            syscmd='mv -f '//filename(k)(1:lfk)//'.TMP'//' '
     +                     //filename(n)(1:lfn)//char(0)
c##################################################################
            write(6,*) '>> ',syscmd(1:lenstr(syscmd,1))
c##################################################################
            call system(syscmd)
            do i=1,5
              itset(5+i,n)=itset(12+i,k)
            end do
          end if
        end if
      end do
c##################################################################
c     do n=1,nfiles
c       write(6,1001) n,(itset(i,n),i=1,12),filename(n)
c     end do
c##################################################################
c
      return
      end
