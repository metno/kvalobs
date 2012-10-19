      program rfinh
c
c       read felt file and print list of contents.
c
c---------------------------------------------------------------
c
c      DNMI library subroutines:  mrfelt
c                                 cmdarg
c                                 qfelt
c
c------------------------------------------------
c  DNMI/FoU  xx.xx.198x  Anstein Foss ... ibm
c  DNMI/FoU  01.09.1992  Anstein Foss ... unix
c  DNMI/FoU  30.09.1993  Anstein Foss ... using qfelt
c  DNMI/FoU  02.02.1994  Anstein Foss
c  DNMI/FoU  04.08.1994  Anstein Foss
c  DNMI/FoU  29.05.1996  Anstein Foss ... -m option
c  DNMI/FoU  25.01.2001  Anstein Foss ... mrfelt/qfelt autoswap
c  DNMI/FoU  01.04.2003  Anstein Foss ... bug fix (for -0)
c  DNMI/IT   24.04.2007  Rebecca Rudsar ... more output
c------------------------------------------------
c
c
      parameter (lrec=1024)
      parameter (nin=64*4)
c
      integer   ifound(nin),ibad(nin)
      integer*2 in(16,nin),idrec1(1024)
      integer*2 iprod,igrid,itime
c
      character*64 filnam,opt,optval
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=15)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=1)
      integer       nargs
      character*60  cargs(margs)
      integer       mispec
      parameter    (mispec=300)
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
      integer inpos(nopt),iloop(3,nopt)
c
c..cmdarg...............................................................
      data copt/'n','g','d','t','v','p','l','a','0','1','2','3','4','5',
     +		'm'/
      data iopt/ 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 , 0 , 0 , 0 , 0 ,
     +		 0 /
c..cmdarg........1...2...3...4...5...6...7...8...9..10..11..12..13..14..
c...............15
c
      data inpos/1,  2,  9, 10, 11, 12, 13,  0,  0,  0,  0,  0,  0 , 0 ,
     +		 0 /
c
c
c..cmdarg................................................
c..get command line arguments
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c..cmdarg................................................
c
      if(ierror.eq.0 .and. nargs.lt.1) ierror=-1
c
      if(ierror.ne.0) then
        write(6,*) ' usage:  rfinh felt_file'
        write(6,*) '    or:  rfinh felt_file [options]'
        write(6,*) '   options:'
        write(6,*) '      -n <producer_no>   : producer (1-99)'
        write(6,*) '      -g <grid_no>       : grid'
        write(6,*) '      -d <data_type>     : data type'
        write(6,*) '      -t <forecast_hour> : forecast time'
        write(6,*) '      -v <v.coord._no>   : vertical coordinate'
        write(6,*) '      -p <parameter_no>  : parameter no.'
        write(6,*) '      -l <level>         : level (no.)'
        write(6,*) '      -a                 : also when missing data'
        write(6,*) '      -m                 : only when missing data'
        write(6,*) '      -0                 : print only number found'
        write(6,*) '      -1                 : print format 1 (default)'
        write(6,*) '      -2                 : print format 2'
        write(6,*) '      -3                 : print format 3'
        write(6,*) '      -4                 : print format 4'
        write(6,*) '      -5                 : print format 5'
        write(6,*) '   (e.g. -t24 -v 1 -p1,2,3 -l 500,1000)'
        write(6,*)
        stop
      endif
c
c--------------------------------------------
c
      iprod =-32767
      igrid =-32767
      idatyp=-32767
      itime =-32767
      ivcoor=-32767
      iparam=-32767
      ilevel=-32767
      iexist=1
      iprint=1
c
      if(iopts(1, 1).eq.1) iprod =ispec(iopts(2,1))
      if(iopts(1, 2).eq.1) igrid =ispec(iopts(2,2))
      if(iopts(1, 3).eq.1) idatyp=ispec(iopts(2,3))
      if(iopts(1, 4).eq.1) itime =ispec(iopts(2,4))
      if(iopts(1, 5).eq.1) ivcoor=ispec(iopts(2,5))
      if(iopts(1, 6).eq.1) iparam=ispec(iopts(2,6))
      if(iopts(1, 7).eq.1) ilevel=ispec(iopts(2,7))
      if(iopts(1, 8).gt.0) iexist=0
      if(iopts(1, 9).gt.0) iprint=0
      if(iopts(1,10).gt.0) iprint=1
      if(iopts(1,11).gt.0) iprint=2
      if(iopts(1,12).gt.0) iprint=3
      if(iopts(1,13).gt.0) iprint=4
      if(iopts(1,14).gt.0) iprint=5
      if(iopts(1,15).gt.0) iexist=2
c
      filnam=cargs(1)
c
      if(iprint.eq.1 .or. iprint.eq.2 .or.
     +   iprint.eq.4 .or. iprint.eq.5) then
        write(6,*) 'Felt file:'
        write(6,*) filnam(1:lenstr(filnam,1))
      end if
c
      iunitf=20
c
      mode=1
      if(iprint.eq.0) mode=11
      call mrfelt(mode,filnam,iunitf,in,0,1,0.,0.,
     +            1024,idrec1,ierror)
      if(ierror.ne.0) goto 950
c
      if(iprint.eq.1 .or. iprint.eq.2 .or.
     +   iprint.eq.4 .or. iprint.eq.5) then
        write(6,*)
        write(6,*) ' word 1-11 in record 1:'
        write(6,fmt='(2x,11i6)') (idrec1(i),i=1,11)
        write(6,*)
        write(6,*)
      end if
c
        if(iprint.eq.1) then
          write(6,*) 'in(1-5),in(9-15),ilen,irec,iwrd:'
        elseif(iprint.eq.2) then
          write(6,*) 'in(1-16):'
        elseif(iprint.eq.4) then
          write(6,*) 'in(1-5),in(9-15),iex:'
        elseif(iprint.eq.5) then
          write(6,*) 'in(1-2),in(9-15),iex:'
        end if
c
c..search loops if more than one (or none) value specified
c..check if same value is specified more than once
c..(would destroy the search loop system below)
      nloop=0
      do n=1,nopt
        if(inpos(n).gt.0 .and. iopts(1,n).gt.1) then
          i1=iopts(2,n)
          i2=iopts(2,n)+iopts(1,n)-1
          io=i1
          do i=i1+1,i2
            idone=0
            do j=i1,i-1
              if(ispec(j).eq.ispec(i)) idone=1
            end do
            if(idone.eq.0) then
              io=io+1
              ispec(io)=ispec(i)
            end if
          end do
          nloop=nloop+1
          iloop(1,nloop)=inpos(n)
          iloop(2,nloop)=i1
          iloop(3,nloop)=io
        end if
      end do
c
      if(nloop.eq.0) then
        do n=1,nin
          ibad(n)=0
        end do
      end if
c
      ntotal=0
      ierror=0
      iend=0
      ireq=1
c
      do while (iend.eq.0 .and. ierror.eq.0)
c
        do i=1,16
          in(i,1)=-32767
        end do
        in( 1,1)=iprod
        in( 2,1)=igrid
        in( 9,1)=idatyp
        in(10,1)=itime
        in(11,1)=ivcoor
        in(12,1)=iparam
        in(13,1)=ilevel
c
        call qfelt(iunitf,ireq,iexist,nin,in,ifound,nfound,
     +             iend,ierror,ioerr)
c
        if(ierror.ne.0) write(6,*) 'QFELT error.'
c
        ngood=nfound
c
        if(nloop.gt.0) then
c
          do n=1,nfound
            ibad(n)=nloop
          end do
          do l=1,nloop
            ni=iloop(1,l)
            do i=iloop(2,l),iloop(3,l)
              do n=1,nfound
                if(in(ni,n).eq.ispec(i)) ibad(n)=ibad(n)-1
              end do
            end do
          end do
          ngood=0
          do n=1,nfound
            if(ibad(n).eq.0) ngood=ngood+1
          end do
c
        end if
c
        ntotal=ntotal+ngood
c
        if(iprint.eq.1 .or. iprint.eq.3) then
          do n=1,nfound
            if(ibad(n).eq.0) then
              ilen=-1
              irec=-1
              iwrd=-1
              if(ifound(n).gt.0) then
                ilen=ifound(n)
                iwrd=in(7,n)
                if(iwrd.lt.1) iwrd=1
                ir1=in( 6,n)
                ir2=in(16,n)-(in(16,n)/100)*100
                irec=ir1+ir2*32767
              end if
              write(6,fmt='(1x,i3,11i5,2i8,i5)')
     +               (in(i,n),i=1,5),(in(i,n),i=9,15),ilen,irec,iwrd
            end if
          end do
        elseif(iprint.eq.2) then
          do n=1,nfound
            if(ibad(n).eq.0) then
              write(6,fmt='(1x,16i6)') (in(i,n),i=1,16)
            end if
          end do
        elseif(iprint.eq.4) then
          do n=1,nfound
            if(ibad(n).eq.0) then
	      iex=0
	      if(ifound(n).gt.0) iex=1
		write(6,fmt='(1x,12i6,i3)')
     +                      (in(i,n),i=1,5),(in(i,n),i=9,15),iex
	    end if
          end do
        elseif(iprint.eq.5) then
          do n=1,nfound
            if(ibad(n).eq.0) then
	      iex=0
	      if(ifound(n).gt.0) iex=1
		write(6,fmt='(1x,9i6,i3)')
     +                      (in(i,n),i=1,2),(in(i,n),i=9,15),iex
	    end if
          end do
        end if
c
c.....end do while (iend.eq.0 .and. ierror.eq.0)
      end do
c
      mode=3
      if(iprint.eq.0) mode=13
      call mrfelt(mode,filnam,iunitf,in,0,1,0.,0.,
     +            1,idrec1,ierr)
c
      if(ierror.ne.0) ntotal=-1
c
      if(iprint.eq.0) then
        write(6,*) ntotal
      elseif(iprint.eq.3) then
        continue
      elseif(iexist.eq.1) then
        write(6,*) 'no. of existing fields: ',ntotal
      elseif(iexist.eq.2) then
        write(6,*) 'no. of missing fields: ',ntotal
      else
        write(6,*) 'no. of possible fields: ',ntotal
      end if
c
      goto 990
c
  950 if(iprint.eq.0) then
        ntotal=-1
        write(6,*) ntotal
      end if
      stop
c
  990 continue
      end
