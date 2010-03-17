      program flttst
c
c        find min,max,mean and no. of undefined/missing values (-32767)
c        in fields on one Felt file.
c
c----------------------------------------------------------
c
c      DNMI library subroutines:  cmdarg
c                                 mrfturbo
c                                 qfelt
c
c------------------------------------------------
c  DNMI/FoU  xx.xx.198x  Anstein Foss ... ibm
c  DNMI/FoU  01.09.1992  Anstein Foss ... unix
c  DNMI/FoU  22.10.1993  Anstein Foss ... using qfelt
c  DNMI/FoU  12.03.1994  Anstein Foss
c  DNMI/FoU  10.05.1994  Anstein Foss
c  DNMI/FoU  25.01.2001  Anstein Foss ... mrfturbo/qfelt autoswap
c------------------------------------------------
c
      include 'flttst.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for flttst.f
c
c  maxsiz : max field size
c  maxpar : max no. of parameters (for summary)
c
ccc   parameter (maxsiz=100000)
ccc   parameter (maxpar=300)
c
c..don't change the following:
c
ccc   parameter (limit=20+maxsiz)
ccc   parameter (maxinq=256)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      common/f/ident(20),ifelt(maxsiz)
      integer*2 ident,ifelt
c
      integer   ifound(maxinq),ibad(maxinq)
      integer*2 in(16),inh(16,maxinq)
      integer*2 imin,imax
      integer   istat(6,maxpar)
c
      double precision fstat(maxpar),fmean
c
      character*256 filnam
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=8)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=1)
      integer       nargs
      character*256 cargs(margs)
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
c..cmdarg.......................................
      data copt/'n','g','d','t','v','p','l','s'/
      data iopt/ 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 /
c..cmdarg........1...2...3...4...5...6...7...8..
c
      data inpos/1,  2,  9, 10, 11, 12, 13,  0 /
c
      lfield=0
      field=0.
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
        write(6,*) ' usage:  flttst felt_file'
        write(6,*) '    or:  flttst felt_file [options]'
        write(6,*) '   options:'
        write(6,*) '      -n <producer_no>   : producer (1-99)'
        write(6,*) '      -g <grid_no>       : grid'
        write(6,*) '      -d <data_type>     : data type'
        write(6,*) '      -t <forecast_hour> : forecast time'
        write(6,*) '      -v <v.coord._no>   : vertical coordinate'
        write(6,*) '      -p <parameter_no>  : parameter no.'
        write(6,*) '      -l <level>         : level (no.)'
        write(6,*) '      -s                 : only print summary'
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
      isumma=0
c
      if(iopts(1, 1).eq.1) iprod =ispec(iopts(2,1))
      if(iopts(1, 2).eq.1) igrid =ispec(iopts(2,2))
      if(iopts(1, 3).eq.1) idatyp=ispec(iopts(2,3))
      if(iopts(1, 4).eq.1) itime =ispec(iopts(2,4))
      if(iopts(1, 5).eq.1) ivcoor=ispec(iopts(2,5))
      if(iopts(1, 6).eq.1) iparam=ispec(iopts(2,6))
      if(iopts(1, 7).eq.1) ilevel=ispec(iopts(2,7))
      if(iopts(1, 8).gt.0) isumma=1
c
      filnam=cargs(1)
      lfilnam=lenstr(filnam,1)
c
      write(6,*) 'Felt file: ',filnam(1:lfilnam)
c
      iunit=20
c
      call mrfturbo(1,filnam,iunit,in,0,lfield,field,1.0,
     +              32,ident,ierror)
      if (ierror.ne.0) stop 1
c
      write(6,*)
      write(6,*) 'word 1-12,   record 1:'
      write(6,fmt='(2x,12i6)') (ident(i),i=1,12)
      write(6,*)
      if(isumma.eq.0) then
        write(6,*) 'in(1,2,9-14),ident(10,11,15-18,20),',
     *             'min,max,mean,nundef:'
        write(6,*)
      end if
c
      iarchive=0
      if(ident(1).eq.998 .or. ident(1).eq.997) iarchive=1
c
      npar=0
      nparx=0
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
        do n=1,maxinq
          ibad(n)=0
        end do
      end if
c
c..qfelt: return all 'innh.fort' with existing field data:
c
      ntotal=0
      iexist=1
      ierror=0
      iend=0
      ireq=1
c
      do while (iend.eq.0 .and. ierror.eq.0)
c
        do i=1,16
          inh(i,1)=-32767
        end do
        inh( 1,1)=iprod
        inh( 2,1)=igrid
        inh( 9,1)=idatyp
        inh(10,1)=itime
        inh(11,1)=ivcoor
        inh(12,1)=iparam
        inh(13,1)=ilevel
c
        ninh=maxinq
        call qfelt(iunit,ireq,iexist,ninh,inh,ifound,nfound,
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
                if(inh(ni,n).eq.ispec(i)) ibad(n)=ibad(n)-1
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
        do nf=1,nfound
c
          if(ibad(nf).eq.0) then
c
            do i=1,16
              in(i)=inh(i,nf)
            end do
c
            call mrfturbo(2,filnam,iunit,in,0,lfield,field,1.0,
     +                    limit,ident,ierror)
c
            if (ierror.eq.0) then
c
            k=1
            if(ident( 1).ne.in( 1)) k=0
            if(ident( 2).ne.in( 2)) k=0
            if(ident( 3).ne.in( 9)) k=0
            if(ident( 4).ne.in(10)) k=0
            if(ident( 5).ne.in(11)) k=0
            if(ident( 6).ne.in(12)) k=0
            if(ident( 7).ne.in(13)) k=0
            if(ident( 8).ne.in(14)) k=0
            if(ident( 9).ne.in(15)) k=0
            if(ident(12).ne.in( 3)) k=0
            if(ident(13).ne.in( 4)) k=0
            if(ident(14).ne.in( 5)) k=0
c
            if(k.eq.0) then
              write(6,1030) in(1),in(2),(in(i),i=9,15),(in(i),i=3,5)
 1030         format(1x,i3,i5,i2,i4,i2,i4,i5,i2,' * ',i2,' * ',3i5,
     *               ' <-in...ERROR')
              write(6,1031) (ident(i),i=1,9),(ident(i),i=12,14)
 1031         format(1x,i3,i5,i2,i4,i2,i4,i5,i2,' * ',i2,' * ',3i5,
     *               ' <-ident.....')
            end if
c
            nx=ident(10)
            ny=ident(11)
            isize=nx*ny
c
            imin=+32767
            imax=-32767
            fmean=0.d0
            nundef=0
            do i=1,isize
              if(ifelt(i).ne.-32767) then
                if(imin.gt.ifelt(i)) imin=ifelt(i)
                if(imax.lt.ifelt(i)) imax=ifelt(i)
                fmean=fmean+ifelt(i)
              else
                nundef=nundef+1
              end if
            end do
            if(nundef.lt.isize) then
	      fmean=fmean/(isize-nundef)
	    else
	      imin=0
	      imax=0
	    end if
c
            if(isumma.eq.0) then
              imean=nint(fmean)
	      if(iarchive.eq.0) then
                write(6,1050) in(1),in(2),(in(i),i=9,14),
     *                        (ident(i),i=10,11),(ident(i),i=15,18),
     *                        ident(20),imin,imax,imean,nundef
 1050           format(1x,i3,i5,i2,i4,i2,i4,i5,i2,2i4,4i6,i3,4i7)
	      else
                write(6,1051) (in(i),i=1,5),(in(i),i=9,14),
     *                        (ident(i),i=10,11),(ident(i),i=15,18),
     *                        ident(20),imin,imax,imean,nundef
 1051           format(1x,i3,i5,3i5,i2,i4,i2,i4,i5,i2,2i4,4i6,i3,4i7)
	      end if
            end if
c
            if(nundef.lt.isize) then
              ivco=ident(5)
              ipar=ident(6)
              iscl=ident(20)
              np=0
              n =0
              do while (np.eq.0 .and. n.lt.npar)
                n=n+1
                if(istat(1,n).eq.ivco .and. istat(2,n).eq.ipar
     *                                .and. istat(3,n).eq.iscl) np=n
              end do
              if(np.eq.0) then
                if(npar.lt.maxpar) then
                  npar=npar+1
                  istat(1,npar)=ivco
                  istat(2,npar)=ipar
                  istat(3,npar)=iscl
                  istat(4,npar)=0
                  istat(5,npar)=+32767
                  istat(6,npar)=-32767
                  fstat(npar)=0.d0
                  np=npar
                else
                  nparx=nparx+1
                end if
              end if
              if(np.gt.0) then
                istat(4,np)=istat(4,np)+1
                if(istat(5,np).gt.imin) istat(5,np)=imin
                if(istat(6,np).lt.imax) istat(6,np)=imax
                fstat(np)=fstat(np)+fmean
              end if
            end if
c
            end if
c
          end if
c
c.......end do nf=1,nfound
        end do
c
c.....end do while (iend.eq.0 .and. ierror.eq.0)
      end do
c
      call mrfturbo(3,filnam,iunit,in,0,lfield,field,1.0,
     +              1,ident,ierror)
c
      if(npar.gt.0) then
        write(6,*)
        write(6,*) 'Summary:'
        write(6,*)
        do n=1,npar
          fstat(n)=fstat(n)/istat(4,n)
          imean=nint(fstat(n))
          write(6,fmt='('' v.c.,param,scale,no.: '',2i6,i3,i6,4x,
     *                    '' min,max,mean: '',3i7)')
     *                         (istat(i,n),i=1,6),imean
        end do
        if(nparx.gt.0) then
          write(6,*)
          write(6,*) 'table overflow.'
          write(6,*) 'no. of fields not in summary: ',nparx
        end if
      end if
c
      write(6,*)
c
      end
