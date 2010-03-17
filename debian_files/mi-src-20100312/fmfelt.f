      program fmfelt
c
c        input from sequential model i/o file,
c        standard unix fortran 77 format, use 'assign -f f77' on cray.
c        output to dnmi standard or archive felt file.
c
c        field identification and field data written in to separate
c        records.
c
c     program return codes (stop n):
c        stop 1 : error in command line arguments
c        stop 2 : open/read/write error, input or output file
c        stop 3 : date/time error
c        stop 4 : too few fields written
c        stop 5 : not ok file termination (possibly a transfer error)
c
c
c-----------------------------------------------------------------------
c      DNMI library subroutines:  wfturbo (turbo version of wfelt)
c                                 rlunit
c                                 cmdarg
c                                 daytim
c                                 wcfelt
c
c-----------------------------------------------------------------------
c  DNMI/FoU  1989 - 1992  Anstein Foss ... ibm
c  DNMI/FoU   29.10.1992  Anstein Foss ... unix
c  DNMI/FoU   29.09.1993  Anstein Foss ... wfelt (big fields), wcfelt
c  DNMI/FoU   12.03.1994  Anstein Foss ... wfturbo
c  DNMI/FoU   17.06.1994  Anstein Foss ... -f option
c  DNMI/FoU   24.03.1995  Anstein Foss ... -e option
c  DNMI/FoU   09.06.1995  Anstein Foss ... extra geomettry identification
c  DNMI/FoU   29.10.1997  Anstein Foss ... bad field size used for -t
c  DNMI/FoU   08.10.2003  Anstein Foss ... automatic byteswap, feltfile
c  DNMI/FoU   12.08.2004  Anstein Foss ... longer filenames
c-----------------------------------------------------------------------
c
ccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for fmfelt.f
c
c  maxsiz : max field size
c
ccc   parameter (maxsiz=1000000)
ccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'fmfelt.inc'
c
      common/a/ident(20),ifelt(maxsiz+3)
      integer*2 ident,ifelt
c
      integer   ierr(3)
      integer*2 idrec1(1024),idrec2(1024),idsave(32)
      integer*2 in(16),lident(20)
c.wfelt.............................................................
cc    integer   ihelpw(6)
c.wfelt.............................................................
c.wfturbo...........................................................
      integer   ihelpw(10)
      integer*2 innhw(16,64)
c.wfturbo...........................................................
c
      double precision sum
c
      character*256 filein,fileot
c
      logical swap,swapfile
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=10)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=2)
      integer       nargs
      character*256 cargs(margs)
      integer       mispec
      parameter    (mispec=3)
      integer       ispec(mispec)
      integer       mrspec
      parameter    (mrspec=1)
      real          rspec(mrspec)
      integer       mcspec
      parameter    (mcspec=1)
      character*1   cspec(mcspec)
      integer       ierror
      integer       nerror
c..cmdarg................................................
c
c..cmdarg................................................
      data copt/'a','b','d','m','o','s','t','f','e','z'/
      data iopt/ 0 , 0 , 0 , 1 , 0 , 0 , 0 , 1 , 0 , 0 /
c..cmdarg........1...2...3...4...5...6...7...8...9..10...
c
      data in/16*0/
c.wfelt.............................................................
cc    data ihelpw/6*0/
c.wfelt.............................................................
c.wfturbo...........................................................
      data ihelpw/10*0/
c.wfturbo...........................................................
c
c
c..cmdarg................................................
c..get command line arguments
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c..cmdarg................................................
c
      if(iopts(1,8).ne.0 .and. iopts(1,8).ne.2) ierror=-1
c
      if(ierror.eq.0 .and. nargs.lt.2) ierror=-1
c
      if(ierror.ne.0) then
        write(6,*)
        write(6,*) '  usage:  fmfelt input_file output_file'
        write(6,*) '     or:  fmfelt <options> input_file output_file'
        write(6,*)
        write(6,*) '     input  file: sequential model i/o file'
        write(6,*) '     output file: felt file'
        write(6,*)
        write(6,*) '     options:'
        write(6,*) '        -a : ascii format input'
        write(6,*) '        -b : break execution at first failure'
        write(6,*) '        -d : turn off date/time check'
        write(6,*) '        -e : skip fields with incorrect date/time'
        write(6,*) '        -f <first_forecast_hour,',
     *				'last_forecast_hour>'
        write(6,*) '        -m <min_no_of_fields> : otherwise error'
        write(6,*) '        -o : not overwrite existing fields'
        write(6,*) '        -s : swap bytes (dnmi model to dec/vax)'
        write(6,*) '        -t : test and print for each field'
        write(6,*) '        -z : skip extra geometry',
     +				  ' identification after field data'
	write(6,*) '             (possibly bad ''grid type'')'								
        write(6,*)
        goto 981
      end if
c
c
      iformt=0
      ibreak=0
      itestd=1
      nstop =0
      ioverw=1
      itestf=0
      iswapb=0
      minfc=-32767
      maxfc=+32767
      igskip=0
c
c..cmdarg................................................
c     data copt/'a','b','d','m','o','s','t','f','e','z'/
c     data iopt/ 0 , 0 , 0 , 1 , 0 , 0 , 0 , 1 , 0 , 0 /
c..cmdarg........1...2...3...4...5...6...7...8...9..10...
c
      if(iopts(1,1).gt.0) iformt=1
      if(iopts(1,2).gt.0) ibreak=1
      if(iopts(1,3).gt.0) itestd=0
      if(iopts(1,4).gt.0) nstop =ispec(iopts(2,4))
      if(iopts(1,5).gt.0) ioverw=0
      if(iopts(1,6).gt.0) iswapb=1
      if(iopts(1,7).gt.0) itestf=1
      if(iopts(1,8).eq.2) then
	minfc=ispec(iopts(2,8))
	maxfc=ispec(iopts(2,8)+1)
      end if
      if(iopts(1, 9).gt.0) itestd=3
      if(iopts(1,10).gt.0) igskip=1
c
      filein=cargs(1)
      fileot=cargs(2)
c
      write(6,*) 'Input:  ',filein(1:lenstr(filein,1))
      write(6,*) 'Output: ',fileot(1:lenstr(fileot,1))
c
      if(iformt.ne.0) iswapb=0
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
      iuniti=10
      iunito=20
c
      if(iformt.eq.0) then
c..binary format (unformatted)
        open(iuniti,file=filein,
     *              form='unformatted',access='sequential',
     *              status='old',iostat=ios,err=950)
      else
c..ascii format
        open(iuniti,file=filein,
     *              form='formatted',access='sequential',
     *              status='old',iostat=ios,err=950)
      end if
c
c..check update flag and state of output felt file
c
      call wcfelt(fileot,iunito,iretur)
c
c..open output felt file
      open(iunito,file=fileot,
     *            form='unformatted',access='direct',
     *            recl=2048/lrunit,
     *            status='old',iostat=ios,err=900)
c
      swap= swapfile(-iunito)
c
c..read record 1 and 2
      if(swap) then
	call bswap2(1024,idrec1)
	call bswap2(1024,idrec2)
      end if
      read(iunito,rec=1,err=910,iostat=ios) idrec1
      read(iunito,rec=2,err=910,iostat=ios) idrec2
      if(swap) then
	call bswap2(1024,idrec1)
	call bswap2(1024,idrec2)
      end if
c
      do i=1,32
        idsave(i)=idrec1(i)
      end do
c
c..set update flag (written at first file update)
      idrec1(15)=1
      iwuflag=0
c
c..allow all types of felt files (standard, archive and cyclic_archive)
c..take care when calling wfelt/wfturbo
c
      kwrite=0
      if(idrec1(1).eq.998) then
c..archive felt file
        write(6,*) 'Archive Felt file.'
        kwrite=999
        itestd=2
      elseif(idrec1(1).eq.997) then
c..cyclic_archive felt file ...... not making a new date/time.
        write(6,*) 'Cyclic Archive Felt file.'
        kwrite=997
        itestd=2
      end if
c
c       write termin time
      month=idrec1(6)/100
      iday=idrec1(6)-month*100
      write(6,1010) iday,month,idrec1(5),idrec1(7)
 1010 format(' Time: ',2i4,2i6,' utc')
c
      isize=0
      iabort=0
c
      nfelt1=0
      nfelt2=0
      nfelt3=0
      nfskip=0
      lident( 4)=-9999
      lident(12)=-9999
      lident(13)=-9999
      lident(14)=-9999
      iskip=0
c
c
      do while (.true.)
c
	lskip=iskip
	iskip=0
c
        if(iformt.eq.0) then
          read(iuniti,iostat=ios,err=500,end=510) (ident(i),i=1,20)
        else
          read(iuniti,*,iostat=ios,err=500,end=510) (ident(i),i=1,20)
        end if
c
        if(iswapb.eq.1) then
c..swap bytes
          do i=1,20
            iswap=ident(i)
            iswap=ior(iand(ishft(iswap,-8),255),
     *                ishft(iand(iswap,255),+8))
            ident(i)=iswap
          end do
        end if
c
        if(itestd.eq.1) then
          if(ident(12).ne.idrec1(5) .or. ident(13).ne.idrec1(6)
     *                              .or. ident(14).ne.idrec1(7)) then
            month=ident(13)/100
            iday =ident(13)-month*100
            write(6,1011) iday,month,ident(12),ident(14)
 1011       format('  Data.         Time: ',2i4,2i6,' utc',
     *           /,'       not as felt file date/time.')
            iabort=1
            goto 510
          end if
        end if
c
        if(ident( 4).ne.lident( 4) .or.
     *     ident(12).ne.lident(12) .or.
     *     ident(13).ne.lident(13) .or.
     *     ident(14).ne.lident(14)) then
          month=ident(13)/100
          iday =ident(13)-month*100
          ihour=ident(14)/100
          write(6,1012) nfelt1+1,iday,month,ident(12),ihour,ident(4)
 1012     format(' field no.',i5,'   time:',2i3,i5,i3,' utc ',sp,i6,ss)
          lident( 4)=ident( 4)
          lident(12)=ident(12)
          lident(13)=ident(13)
          lident(14)=ident(14)
	  lskip=0
        end if
c
	if(itestd.eq.2) then
c..archive or cyclic_archive file
c..check first and last date/time on file
	  igt1=0
	  igt2=0
	  do i=1,3
	    if(igt1.eq.0 .and. ident(11+i).lt.idrec1(19+i)) igt1=-1
	    if(igt1.eq.0 .and. ident(11+i).gt.idrec1(19+i)) igt1=+1
	    if(igt2.eq.0 .and. ident(11+i).lt.idrec1(22+i)) igt2=-1
	    if(igt2.eq.0 .and. ident(11+i).gt.idrec1(22+i)) igt2=+1
	  end do
	  if(igt1.eq.-1 .or. igt2.eq.+1) then
	    if(lskip.eq.0)
     *	      write(6,fmt='(''    skip date/time, not on felt file'')')
	    iskip=1
	  end if
        end if
c
	if(itestd.eq.3) then
c..skip fields with incorrect date/time (standard felt file output)
	  if(ident(12).ne.idrec1(5) .or.
     +	     ident(13).ne.idrec1(6) .or.
     +	     ident(14).ne.idrec1(7)) then
	    if(lskip.eq.0)
     *	      write(6,fmt='(''    skip date/time, not on felt file'')')
	    iskip=1
	  end if
	end if
c
	if(ident(4).lt.minfc .or. ident(4).gt.maxfc) then
	  if(iskip.eq.0 .and. lskip.eq.0)
     *	     write(6,fmt='(''    skip forecast'')')
	  iskip=1
	end if
c
        nx=ident(10)
        ny=ident(11)
        isize=nx*ny
	igtype=ident(9)
	lxid=0
	if(igtype.gt.999 .and. igskip.eq.0) then
c..extra geometry identification stored after the field data (lxid words)
	  i=igtype
	  igtype=igtype/1000
	  lxid=i-igtype*1000
	  isize=isize+lxid
	elseif(igtype.gt.999 .and. igskip.ne.0) then
	  ident(9)=igtype/1000
	end if
        if(isize.gt.maxsiz) goto 400
c
        if(iformt.eq.0 .and. iskip.ne.1) then
          read(iuniti,iostat=ios,err=495,end=500) (ifelt(i),i=1,isize)
	elseif(iformt.eq.0) then
          read(iuniti,iostat=ios,err=500,end=500)
        else
          read(iuniti,*,iostat=ios,err=495,end=500) (ifelt(i),i=1,isize)
        end if
c
        nfelt1=nfelt1+1
c
	if(iskip.eq.1) then
	  nfskip=nfskip+1
	  goto 300
	end if
c
        if(iswapb.eq.1) then
c..swap bytes
          do i=1,isize
            iswap=ifelt(i)
            iswap=ior(iand(ishft(iswap,-8),255),
     *                ishft(iand(iswap,255),+8))
            ifelt(i)=iswap
          end do
        end if
c
        if(itestf.eq.1) then
          write(6,*) 'field no. ',nfelt1
          write(6,1022) ident
 1022     format('  ident: ',11i6,/,'         ',9i6)
          nudef=0
          imin=+32767
          imax=-32767
          sum=0.
          do i=1,nx*ny
            if(ifelt(i).eq.-32767) then
              nudef=nudef+1
            else
              sum=sum+ifelt(i)
              if(imin.gt.ifelt(i)) imin=ifelt(i)
              if(imax.lt.ifelt(i)) imax=ifelt(i)
            end if
          end do
          if(nx*ny.gt.nudef) then
            sum=sum/(nx*ny-nudef)
            write(6,1024) imin,imax,sum,nudef
 1024       format('  min,max,mean,nundef: ',2i7,f12.4,i7)
          else
            write(6,1025) nudef
 1025       format('               nundef: ',26x,i7)
          end if
        end if
c
        in( 1)=ident( 1)
        in( 2)=ident( 2)
        in( 3)=ident(12)
        in( 4)=ident(13)
        in( 5)=ident(14)
        in( 9)=ident( 3)
        in(10)=ident( 4)
        in(11)=ident( 5)
        in(12)=ident( 6)
        in(13)=ident( 7)
        in(14)=ident( 8)
c.old   in(15)=ident( 9)
c
        ierr(1)=ioverw
        ierr(2)=kwrite
        ierr(3)=0
        ldata=20+isize
c.wfelt................................................................
cc      call wfelt(iunito,ip,in,idrec1,idrec2,ldata,ident,ierr,ihelpw)
c.wfelt................................................................
c.wfturbo..............................................................
        call wfturbo(iunito,ip,in,idrec1,idrec2,ldata,ident,
     *               ierr,ihelpw,innhw)
c.wfturbo..............................................................
        if(ip.eq.1) then
          if(iwuflag.eq.0) then
c..set update flag after the first write
c..(not touching the file if it's not necessary)
	    if(swap) call bswap2(1024,idrec1)
            write(iunito,rec=1,iostat=ios,err=940) idrec1
	    if(swap) call bswap2(1024,idrec1)
            iwuflag=1
          end if
          nfelt2=nfelt2+1
        else
          write(6,1015) ip,ierr,in(1),in(2),(in(i),i=9,15)
 1015     format(' Field not written.  ip=',i4,'  ierr=',3i9,
     *         /,'       in(1,2,9-15): ',9i6)
          nfelt3=nfelt3+1
          if(ibreak.eq.1) then
            iabort=3
            goto 510
          end if
        end if
c
  300   continue
c
c.....end do while (.true.)
      end do
c
  400 if(lxid.eq.0) then
        write(6,*) 'too big field.  nx,ny,nx*ny: ',nx,ny,isize
        write(6,*) '      ("maxsiz")  max nx*ny: ',maxsiz
      else
        write(6,*) 'too big field.  nx,ny,nx*ny:    ',nx,ny,isize
	write(6,*) ' gridtype,extra_identification: ',igtype,lxid
        write(6,*) '    ("maxsiz")  max nx*ny+lxid: ',maxsiz
      end if
      iabort=3
      goto 510
c
  495 if(lxid.gt.0) then
	write(6,*) 'WARNING: tried to read field data pluss '
	write(6,*) '  ',lxid,'words of extra geometry identification,'
	write(6,*) '  possibly due to wrong ''grid type'' =',igtype
	write(6,*) '  (try the -z option)'
      end if
c
  500 write(6,*) '*** read error ... input file'
      write(6,*) '***   file: ',filein(1:lenstr(filein,1))
      write(6,*) '*** iostat: ',ios
      iabort=2
c
  510 write(6,*) '  no. of fields read:        ',nfelt1
      write(6,*) '  no. of fields written:     ',nfelt2
      write(6,*) '  no. of fields not written: ',nfelt3
      write(6,*) '  no. of fields skipped:     ',nfskip
c
      if(nfelt2.eq.0) then
        write(6,*) '       no new fields on felt file'
        goto 550
      end if
c
c.wfturbo..............................................................
      if(ihelpw(1).gt.0) then
c..write last innh back to file (if it is updated)
        ihelpw(7)=1
        call wfturbo(iunito,ip,in,idrec1,idrec2,ldata,ident,
     *               ierr,ihelpw,innhw)
        if(ip.ne.1) then
          write(6,1035) ip,ierr
 1035     format(' file update error. ip=',i4,'  ierr=',3i9)
        end if
      end if
c.wfturbo..............................................................
c
      write(6,*) 'record 1, word 1 - 11, at start and after update:'
      write(6,fmt='(5x,11i6)') (idsave(i),i=1,11)
c
      call daytim(iyear,month,iday,ihour,minut,isecn)
      idrec1(2)=iyear
      idrec1(3)=month*100+iday
      idrec1(4)=ihour*100+minut
c
c..reset update flag
      idrec1(15)=0
c
      if(swap) call bswap2(1024,idrec1)
      write(iunito,rec=1,iostat=ios,err=940) idrec1
      if(swap) call bswap2(1024,idrec1)
c
      write(6,fmt='(5x,11i6)') (idrec1(i),i=1,11)
c
      if(idrec1(1).eq.998 .or. idrec1(1).eq.997) then
        if(idrec1(1).eq.998) write(6,*) 'archive felt file.'
        if(idrec1(1).eq.997) write(6,*) 'cyclic archive felt file.'
        write(6,*) 'record 1, word 20 - 28 :'
        write(6,fmt='(2x,11i6)') (idrec1(i),i=20,28)
      end if
c
  550 close(iuniti)
      close(iunito)
c
      if(iabort.eq.1) goto 983
      if(iabort.eq.2) goto 985
      if(iabort.eq.3) goto 984
c
      if(nfelt2.lt.nstop) goto 984
c
      goto 990
c
c
  900 write(6,*) '*** open error ... output felt file'
      write(6,*) '***   file: ',fileot(1:lenstr(fileot,1))
      write(6,*) '*** iostat: ',ios
      goto 982
c
  910 write(6,*) '*** read error record 1 or 2 ... output felt file'
      write(6,*) '***   file: ',fileot(1:lenstr(fileot,1))
      write(6,*) '*** iostat: ',ios
      goto 982
c
  940 write(6,*) '*** error write record 1 ... output felt file'
      write(6,*) '***   file: ',fileot(1:lenstr(fileot,1))
      write(6,*) '*** iostat: ',ios
      goto 982
c
  950 write(6,*) '*** open error ... input file'
      write(6,*) '***   file: ',filein(1:lenstr(filein,1))
      write(6,*) '*** iostat: ',ios
      goto 982
c
  981 continue
ccc   stop 1
      write(6,*) 'fmfelt ***** stop 1 *****'
      call exit(1)
c
  982 continue
ccc   stop 2
      write(6,*) 'fmfelt ***** stop 2 *****'
      call exit(2)
c
  983 continue
ccc   stop 3
      write(6,*) 'fmfelt ***** stop 3 *****'
      call exit(3)
c
  984 continue
ccc   stop 4
      write(6,*) 'fmfelt ***** stop 4 *****'
      call exit(4)
c
  985 continue
ccc   stop 5
      write(6,*) 'fmfelt ***** stop 5 *****'
      call exit(5)
c
  990 continue
      end
