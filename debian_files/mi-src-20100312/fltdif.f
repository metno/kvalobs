      program fltdif
c
c        find min and max difference between fields on two Felt files.
c
c----------------------------------------------------------
c
c      DNMI library subroutines:  cmdarg
c                                 qfelt
c                                 mrfelt
c                                 mrfturbo (turbo version of mrfelt)
c
c------------------------------------------------
c  DNMI/FoU  xx.xx.198x  Anstein Foss ... ibm
c  DNMI/FoU  01.09.1992  Anstein Foss ... unix
c  DNMI/FoU  12.03.1994  Anstein Foss ... using qfelt
c  DNMI/FoU  23.06.1994  Anstein Foss
c  DNMI/FoU  04.08.1994  Anstein Foss
c  DNMI/FoU  08.06.2001  Anstein Foss ... mrfelt/mrfturbo/qfelt autoswap
c------------------------------------------------
c
      include 'fltdif.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for fltdif.f
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
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      common/f/ident1(20),ifelt1(maxsiz)
     *        ,ident2(20),ifelt2(maxsiz)
      integer*2 ident1,ifelt1,ident2,ifelt2
c
      integer   ifound(maxinq),ibad(maxinq)
      integer*2 in(16),inh(16,maxinq)
      integer*2 imin,imax
      integer   istat(6,maxpar)
c
      integer   num1(maxsiz),num2(maxsiz)
c
      character*256 filnam1,filnam2
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
c..cmdarg...............................................
      data copt/'n','g','d','t','v','p','l','e','x','c'/
      data iopt/ 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 0 , 0 /
c..cmdarg........1...2...3...4...5...6...7...8...9...10.
c
      data inpos/1,  2,  9, 10, 11, 12, 13,  0 , 0 , 0 /
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
      if(ierror.eq.0 .and. nargs.lt.2) ierror=-1
c
      if(ierror.ne.0) then
        write(6,*) ' usage:  fltdif felt_file_1 felt_file_2'
        write(6,*) '    or:  fltdif felt_file_1 felt_file_2 [options]'
        write(6,*) '   options:'
        write(6,*) '      -n <producer_no>   : producer (1-99)'
        write(6,*) '      -g <grid_no>       : grid'
        write(6,*) '      -d <data_type>     : data type'
        write(6,*) '      -t <forecast_hour> : forecast time'
        write(6,*) '      -v <v.coord._no>   : vertical coordinate'
        write(6,*) '      -p <parameter_no>  : parameter no.'
        write(6,*) '      -l <level>         : level (no.)'
        write(6,*) '      -e                 : not print if equal',
     *                                         ' fields'
        write(6,*) '      -x                 : print positions with',
     *                                         ' most diff.'
        write(6,*) '      -c                 : not check date/time'
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
      ieq   =1
      ipdiff=0
      ktime =1
c
      if(iopts(1, 1).eq.1) iprod =ispec(iopts(2,1))
      if(iopts(1, 2).eq.1) igrid =ispec(iopts(2,2))
      if(iopts(1, 3).eq.1) idatyp=ispec(iopts(2,3))
      if(iopts(1, 4).eq.1) itime =ispec(iopts(2,4))
      if(iopts(1, 5).eq.1) ivcoor=ispec(iopts(2,5))
      if(iopts(1, 6).eq.1) iparam=ispec(iopts(2,6))
      if(iopts(1, 7).eq.1) ilevel=ispec(iopts(2,7))
      if(iopts(1, 8).gt.0) ieq   =0
      if(iopts(1, 9).gt.0) ipdiff=1
      if(iopts(1,10).gt.0) ktime=0
c
      filnam1=cargs(1)
      filnam2=cargs(2)
c
      lfilnam1=lenstr(filnam1,1)
      lfilnam2=lenstr(filnam2,1)
c
      iunit1=21
      iunit2=22
c
c..a trick to handle two felt files (and at least one fast):
c..using mrfturbo to read one file and mrfelt the other.
c
      call mrfturbo(1,filnam1,iunit1,in,0,lfield,field,1.0,
     +              32,ident1,ierror)
      if (ierror.ne.0) stop 1
c
      call mrfelt(1,filnam2,iunit2,in,0,lfield,field,1.0,
     +            32,ident2,ierror)
      if (ierror.ne.0) stop 1
c
      write(6,*)
      write(6,*) 'Felt file 1: ',filnam1(1:lfilnam1)
      write(6,*) 'word 1-12,   record 1:'
      write(6,fmt='(2x,12i6)') (ident1(i),i=1,12)
c
      write(6,*) 'Felt file 2: ',filnam2(1:lfilnam2)
      write(6,*) 'word 1-12,   record 1:'
      write(6,fmt='(2x,12i6)') (ident2(i),i=1,12)
c
      if(ident1(1).eq.999 .and. ident2(1).eq.999 .and. ktime.eq.1) then
	if(ident1(5).ne.ident2(5) .or.
     +     ident1(6).ne.ident2(6) .or.
     +     ident1(7).ne.ident2(7)) then
	  write(6,*)
	  write(6,*) 'The two files have different date/time!'
	  stop 1
	end if
      end if
c
      if(ieq.eq.0) then
        write(6,*)
        write(6,*) 'no print if equal fields'
      end if
      write(6,*)
      write(6,*) 'min and max difference  file_1 - file_2'
      write(6,*)
c
c..always check/use time if archive files
      if(ident1(1).ne.999 .or. ident2(1).ne.999) ktime=1
c
      do i=1,maxsiz
        num1(i)=0
        num2(i)=0
      end do
      iprod1=+32767
      iprod2=-32767
      igrid1=+32767
      igrid2=-32767
      nxdim1=+32767
      nxdim2=-32767
      nydim1=+32767
      nydim2=-32767
c
      neq=0
      noteq=0
      notdim=0
      notscl=0
      ntest=0
      noton2=0
      nread1=0
      nread2=0
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
        call qfelt(iunit1,ireq,iexist,ninh,inh,ifound,nfound,
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
            call mrfturbo(12,filnam1,iunit1,in,0,lfield,field,1.0,
     +                    limit,ident1,ierror)
c
            if(ierror.eq.0) then
c
	      nread1=nread1+1
c
	      if (ktime.eq.0) then
		in(3)=-32767
		in(4)=-32767
		in(5)=-32767
	      end if
c
              call mrfelt(12,filnam2,iunit2,in,0,lfield,field,1.0,
     +                    limit,ident2,ierror)
c
	      if(ierror.eq.0) then
c
		nread2=nread2+1
c
c..check equal size and scaling before comparing the fields
                if(ident1(10).ne.ident2(10) .or.
     +             ident1(11).ne.ident2(11)) then
                  notdim=notdim+1
                elseif(ident1(20).ne.ident2(20)) then
                  notscl=notscl+1
                else
                  nx=ident1(10)
                  ny=ident1(11)
                  call differ(ident1,ident2,ifelt1,ifelt2,nx,ny,
     +                        ieq,neq,noteq,mindif,maxdif,num1,num2)
                  ivco=ident1(5)
                  ipar=ident1(6)
                  iscl=ident1(20)
                  np=0
                  do n=1,npar
                    if(istat(1,n).eq.ivco .and.
     +		       istat(2,n).eq.ipar .and.
     +                 istat(3,n).eq.iscl) np=n
                  end do
                  if(np.eq.0) then
                    if(npar.lt.maxpar) then
                      npar=npar+1
                      istat(1,npar)=ivco
                      istat(2,npar)=ipar
                      istat(3,npar)=iscl
                      istat(4,npar)=1
                      istat(5,npar)=mindif
                      istat(6,npar)=maxdif
                    else
                      nparx=nparx+1
                    end if
                  else
                    istat(4,np)=istat(4,np)+1
                    if(istat(5,np).gt.mindif) istat(5,np)=mindif
                    if(istat(6,np).lt.maxdif) istat(6,np)=maxdif
                  end if
                  if(iprod1.gt.ident1( 1)) iprod1=ident1( 1)
                  if(iprod2.lt.ident1( 1)) iprod2=ident1( 1)
                  if(igrid1.gt.ident1( 2)) igrid1=ident1( 2)
                  if(igrid2.lt.ident1( 2)) igrid2=ident1( 2)
                  if(nxdim1.gt.ident1(10)) nxdim1=ident1(10)
                  if(nxdim2.lt.ident1(10)) nxdim2=ident1(10)
                  if(nydim1.gt.ident1(11)) nydim1=ident1(11)
                  if(nydim2.lt.ident1(11)) nydim2=ident1(11)
                end if
c
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
      call mrfturbo(3,filnam1,iunit1,in,0,lfield,field,1.0,
     +              1,ident1,ierror)
c
      call mrfelt(3,filnam2,iunit2,in,0,lfield,field,1.0,
     +            1,ident2,ierror)
c
      if(nparx.gt.0) then
        write(6,*)
        write(6,*) 'table overflow.'
        write(6,*) 'no. of fields not in summary: ',nparx
      end if
c
      if(npar.gt.0) then
        write(6,*)
        do n=1,npar
         write(6,fmt='('' v.c.,param,scale,no.: '',2i6,i3,i6,3x,
     *                    '' min_dif,max_dif: '',2i7)')
     *                         (istat(i,n),i=1,6)
        end do
      end if
c
      if(ipdiff.eq.0) then
        continue
      elseif(igrid1.ne.igrid2 .or. iprod1.ne.iprod2 .and.
     *       nxdim1.ne.nxdim2 .or. nydim1.ne.nydim2) then
        write(6,*)
        write(6,*) 'will not print positions with most differences'
        write(6,*) '     min,max grid     no: ',igrid1,igrid2
        write(6,*) '     min,max producer no: ',iprod1,iprod2
        write(6,*) '     min,max x dimension: ',nxdim1,nxdim2
        write(6,*) '     min,max y dimension: ',nydim1,nydim2
        write(6,*)
      else
c..punkt med flest registrerte min/max forskjeller
        nx=nxdim1
        ny=nydim1
        isize=nx*ny
        nmax=20
        write(6,*)
        nn=1
        n=0
        do while (nn.gt.0 .and. n.lt.nmax)
          n=n+1
          nn=0
          ni=0
          do i=1,isize
            if(num1(i).gt.nn) then
              ni=i
              nn=num1(i)
              num1(i)=-num1(i)
            end if
          end do
          if(nn.gt.0) then
            j=(ni+nx-1)/nx
            i=ni-(j-1)*nx
            write(6,*) '     i,j, no. min/max dif.: ',i,j,nn
          end if
        end do
c..punkt med flest registrerte forskjeller (=/= 0)
        write(6,*)
        nn=1
        n=0
        do while (nn.gt.0 .and. n.lt.nmax)
          n=n+1
          nn=0
          ni=0
          do i=1,isize
            if(num2(i).gt.nn) then
              ni=i
              nn=num2(i)
              num2(i)=-num2(i)
            end if
          end do
          if(nn.gt.0) then
            j=(ni+nx-1)/nx
            i=ni-(j-1)*nx
            write(6,*) '     i,j, no. =/= 0:  ',i,j,nn
          end if
        end do
      end if
c
      noton2=nread1-nread2
      write(6,*)
      write(6,*) ' no. of equal fields:               ',neq
      write(6,*) ' no. of unequal fields:             ',noteq
      write(6,*) ' no. of fields with unequal size:   ',notdim
      write(6,*) ' no. of fields with unequal scaling:',notscl
      write(6,*) ' no. of fields not found on file 2: ',noton2
      write(6,*)
c
      end
c
c***********************************************************************
c
      subroutine differ(ident1,ident2,ifelt1,ifelt2,nx,ny,
     *                  ieq,neq,noteq,mindif,maxdif,num1,num2)
c
      integer*2 ident1(20),ident2(20)
      integer*2 ifelt1(nx,ny),ifelt2(nx,ny)
      integer   num1(nx,ny),num2(nx,ny)
c
      n1=+99999
      i1=0
      j1=0
      n2=-99999
      i2=0
      j2=0
c
      do j=1,ny
        do i=1,nx
          iv1=ifelt1(i,j)
          iv2=ifelt2(i,j)
          if(iv1.ne.-32767 .and. iv2.ne.-32767) then
            idiff=iv1-iv2
            if(idiff.lt.n1) then
              n1=idiff
              i1=i
              j1=j
            end if
            if(idiff.gt.n2) then
              n2=idiff
              i2=i
              j2=j
            end if
            if(idiff.ne.0) num2(i,j)=num2(i,j)+1
          end if
        end do
      end do
c
      if(n1.gt.n2) then
c..not found one gridpoint where both values are defined
	n1=0
	n2=0
      elseif(n1.eq.0) then
	i1=0
	j1=0
      elseif(n2.eq.0) then
	i2=0
	j2=0
      end if
c
      if(n1.ne.0 .or. n2.ne.0) then
        noteq=noteq+1
        write(6,1010) (ident1(i),i=1,8),ident1(20),n1,i1,j1,n2,i2,j2
 1010   format(1x,i2,i5,i2,i4,i2,i4,i5,i2,i3,' min,i,j:',i6,2i4,
     *                                       ' max,i,j:',i6,2i4)
      elseif(ieq.ne.0) then
        neq=neq+1
        write(6,1020) (ident1(i),i=1,8),ident1(20)
 1020   format(1x,i2,i5,i2,i4,i2,i4,i5,i2,i3,'      min = max = 0')
      else
        neq=neq+1
      endif
c
      mindif=n1
      maxdif=n2
      if(n1.ne.0) num1(i1,j1)=num1(i1,j1)+1
      if(n2.ne.0) num1(i2,j2)=num1(i2,j2)+1
c
      return
      end
