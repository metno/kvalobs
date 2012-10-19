      program getftime
c
c  Purpose:  Read date and time from a binary data file,
c            then possibly print this.
c            Input data files:
c             -  Standard FELT files (return 'analysis' date/time)
c             -  Observation files   (return 'main' obs. date/time)
c             -  Sequential model input/output file with fields
c                (read identification of the first field,
c                 and return 'analysis' date/time)
c             -  Archive  FELT files (return first and last
c                                     'analysis' time)
c             -  Line files (digitized lines etc.)
c
c  Usage: getftime <option> file_in print_file
c         Options: -f  : Standard FELT file (default)
c                  -o  : Observation file (note that -f or no option
c                        also works with observation files as thees
c                        currently have the same 'header' structure)
c                  -s  : Sequential model input/output file with fields
c                  -z  : Sequential model input/output file with
c			 obervations
c                  -y  : Sequential model input/output file with
c			 satellite obervations
c                  -a  : Archive FELT file, first and last date/time
c                  -l  : Line file (minute is always 00)
c                  -u  : Also return file update date/time
c                        (u option only allowed together with f and o,
c                         use -fu to get last and update date/time
c                         of an Archive FELT file)
c                  -p  : Input file is a PC (or byte swapped) file
c                        (may work with the s option)
c                  -d  : Disable auto detection of swapped field/obs files
c
c  Examples: getftime felt.dat
c            getftime felt.dat felt.time
c            getftime -fup feltpc.dat
c            getftime -fup feltpc.dat feltpc.time
c            getftime -s modelout.dat modelout.time
c
c  Print formats (more spaces are printed than shown here):
c         -f  :  1993 93 02 25 12 00
c         -o  :  1993 93 02 25 12 00
c         -s  :  1993 93 02 25 12 00
c         -z  :  1993 93 02 25 12 00
c         -y  :  1993 93 02 25 12 00
c         -fu :  1993 93 02 25 12 00 1993 93 02 25 14 55
c         -ou :  1993 93 02 25 12 00 1993 93 02 25 14 55
c         -a  :  1993 93 02 18 12 00 1993 93 02 25 12 00
c         -l  :  1993 93 02 25 12 00
c
c        Format: year   (4 digits)
c                year   (2 digits)
c                month  (2 digits)
c                day    (2 digits)
c                hour   (2 digits)
c                minute (2 digits)
c        If open or read error: -32767 will be printed (all places)
c
c-----------------------------------------------------------------------
c
c      DNMI library routines:  CMDARG
c                              RLUNIT
c
c-----------------------------------------------------------------------
c  DNMI/FoU  25.09.1992  Anstein Foss
c  DNMI/FoU  25.03.1993  Anstein Foss
c  DNMI/FoU  07.09.1994  Anstein Foss  ... added -l linefile
c  DNMI/FoU  03.09.1998  Anstein Foss  ... added -y sat.obs.file
c  DNMI/FoU  18.11.2000  Anstein Foss  ... auto detection of swapped field/obs
c-----------------------------------------------------------------------
c
c
      integer*2    idfile(32),itime(12)
c
      real    satgrid(5)
      integer sattime(4)
c
      logical swapfile,swap
c
c.cmdarg...................................................
      parameter (nopt=10,margs=2)
      parameter (mispec=1,mrspec=1,mcspec=1)
c
      integer      iopt(nopt)
      integer      iopts(2,nopt)
      integer      ispec(mispec)
      real         rspec(mrspec)
      character*4  cspec(mcspec)
      character*1  copt(nopt)
      character*256 cargs(margs)
c
c................1...2...3...4...5...6...7...8...9..10.....
      data copt/'f','a','o','s','z','y','l','u','p','d'/
      data iopt/ 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 /
c.cmdarg...................................................
c
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
      if(ierror.eq.0) then
c............................1...2...3...4...5...6...7...8...9..10..
c.................data copt/'f','a','o','s','z','y','l','u','p','d'/
        iftype=0
        iupdat=0
        iswapb=0
        iprint=0
	iautoswap=1
        do n=1,7
          if(iopts(1,n).gt.0) then
            if(iftype.ne.0) ierror=1
            iftype=n
          end if
        end do
        if(iftype.eq.0)     iftype=1
        if(iopts(1,8).gt.0) iupdat=1
        if(iopts(1,9).gt.0) iswapb=1
        if(iopts(1,10).gt.0) iautoswap=0
        if(nargs.gt.1)      iprint=1
        if(iupdat.eq.1 .and. iftype.ne.1 .and. iftype.ne.3) ierror=1
      end if
c
      if(ierror.ne.0) then
        write(6,*) '  usage:   getftime <options> file_in'
        write(6,*) '     or:   getftime <options> file_in print_file'
        write(6,*) '  options: -f  : Standard FELT file (default)'
        write(6,*) '           -o  : Observation   file'
        write(6,*) '           -s  : Sequential model i/o file (fields)'
        write(6,*) '           -z  : Sequential analysis observation',
     +							' file'
        write(6,*) '           -y  : Sequential analysis satellite',
     +							' obs. file'
        write(6,*) '           -fu : Standard FELT file (+ update time)'
        write(6,*) '           -ou : Observation   file (+ update time)'
        write(6,*) '           -a  : Archive  FELT file (first + last)'
        write(6,*) '           -l  : Line file'
        write(6,*) '           -p  : PC or byte swapped input file'
        write(6,*) '           -d  : Disable auto detection of swapped',
     +                                               ' field/obs files'
        stop
      end if
c
ccc   write(6,*) ' GETFTIME'
      write(6,*) ' Input file: ',cargs(1)(1:60)
      if(iprint.eq.1) write(6,*) ' Print file: ',cargs(2)(1:60)
c
      ltime=12
      do i=1,ltime
        itime(i)=-32767
      end do
c
      iunit=20
      istop=0
c
      n1=0
      n2=0
c
      if(iftype.le.3 .or. iftype.eq.7) then
c
c..record length in bytes
	lrecb=2048
	if(iftype.eq.7) lrecb=1024
c
c..get record length unit in bytes for RECL= in OPEN statements
c..(machine dependant)
        call rlunit(lrunit)
c
        open(iunit,file=cargs(1),
     *             access='direct',form='unformatted',
     *		   recl=lrecb/lrunit,
     *             status='old',iostat=ios,err=110)
c
        idlen=32
        read(iunit,rec=1,iostat=ios,err=120) (idfile(i),i=1,idlen)
c
        if(iftype.eq.2) then
          n1=20
          n2=23
        elseif(iftype.eq.7) then
          n1=-3
        else
          n1=5
          if(iupdat.eq.1) n2=2
        end if
c
      else
c
        open(iunit,file=cargs(1),
     *             access='sequential',form='unformatted',
     *             status='old',iostat=ios,err=110)
c
	if(iftype.eq.6) then
c
	  read(iunit,iostat=ios,err=120) (satgrid(i),i=1,5)
	  read(iunit,iostat=ios,err=120) (sattime(i),i=1,4)
	  idfile(1)=sattime(1)
	  idfile(2)=sattime(2)
	  idfile(3)=sattime(3)
	  idfile(4)=sattime(4)
	  n1=-1
c
	else
c
          idlen=20
          read(iunit,iostat=ios,err=120) (idfile(i),i=1,idlen)
c
          if(iftype.eq.4) n1=12
          if(iftype.eq.5) n1=-7
c
	end if
c
      end if
c
      swap=.false.
      if(iftype.le.3 .and. iswapb.ne.1
     +		     .and. iautoswap.eq.1) swap= swapfile(-iunit)
c
      close(iunit)
c
c..swap bytes
      if(iswapb.eq.1 .or. swap) call bswap2(idlen,idfile)
c
      if(n1.gt.0) then
        itime( 1)=idfile(n1)
        itime( 2)=idfile(n1)-(idfile(n1)/100)*100
        itime( 3)=idfile(n1+1)/100
        itime( 4)=idfile(n1+1)-(idfile(n1+1)/100)*100
        itime( 5)=idfile(n1+2)/100
        itime( 6)=idfile(n1+2)-(idfile(n1+2)/100)*100
        ltime= 6
      else
        n1=-n1
        itime( 1)=idfile(n1)
        itime( 2)=idfile(n1)-(idfile(n1)/100)*100
        itime( 3)=idfile(n1+1)
        itime( 4)=idfile(n1+2)
        itime( 5)=idfile(n1+3)
        itime( 6)=0
        ltime= 6
      end if
      if(n2.gt.0) then
        itime( 7)=idfile(n2)
        itime( 8)=idfile(n2)-(idfile(n2)/100)*100
        itime( 9)=idfile(n2+1)/100
        itime(10)=idfile(n2+1)-(idfile(n2+1)/100)*100
        itime(11)=idfile(n2+2)/100
        itime(12)=idfile(n2+2)-(idfile(n2+2)/100)*100
        ltime=12
      end if
c
      goto 150
c
  110 write(6,*) ' Open error. Input data file. Iostat: ',ios
      istop=1
      goto 150
c
  120 write(6,*) ' Read error. Input data file. Iostat: ',ios
      close(iunit)
      istop=1
cc    goto 150
c
  150 continue
c
      write(6,fmt='(''   year   year  month    day   hour minute'')')
      write(6,fmt='(1x,i6.4,5(1x,i6.2))') (itime(i),i=1,6)
      if(ltime.gt.6) then
        write(6,fmt='(1x,i6.4,5(1x,i6.2))') (itime(i),i=7,ltime)
      end if
c
      if(iprint.eq.1) then
c
        open(iunit,file=cargs(2),
     *             access='sequential',form='formatted',
     *             status='unknown',iostat=ios,err=180)
c
        write(iunit,fmt='(2(1x,i6.4,5(1x,i6.2)))',iostat=ios,err=190)
     *                                           (itime(i),i=1,ltime)
c
        close(iunit)
c
      end if
c
      if(istop.eq.1) call exit(1)
      stop
c
  180 write(6,*) ' Open error. Output print file. Iostat: ',ios
      call exit(1)
c
  190 write(6,*) ' Write error. Output print file. Iostat: ',ios
      close(iunit)
      call exit(1)
c
      end
