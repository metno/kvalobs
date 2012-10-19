      program vcinfo
c
c        this program reads data files with data for one or more 
c        vertical crossections, as created by program vcdata.
c        Mostly created by reverse engineering of vcdata.f and
c        thus using the same include file vcdata.inc :
c        Unashamed raw copy of the sourcecode....
c        Most likely a huge load of unneccesary declarations...
c
c-----------------------------------------------------------------------
c      DNMI library subroutines:  bget*
c
c-----------------------------------------------------------------------
c  met.no    16.12.2005  Arild Burud 
c  met.no    02.01.2006  Anstein Foss ... strip off "!! misc.spec...." in names 
c  met.no    09.12.2008  Arild Burud - Uncompressed data output and new 
c                        option for lat/lon positions instead of x/y 
c-----------------------------------------------------------------------
c
      include 'vcdata.inc'
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..vcdata.inc ..... include file for vcdata.f
c
c  maxij   : max input field size
c  maxd    : max data length (multi  level parameters), one timestep
c  maxd1   : max data length (single level parameters), one timestep
c  maxlev  : max no. of levels
c  maxpar  : max no. of multi  level parameters
c  maxpr1  : max no. of single level parameters
c  maxcrs  : max no. of crossections
c  maxtim  : max no. of timesteps
c  maxinp  : max no. of input positions, all crossections
c  maxpos  : max no. of positions along all crossections
c  maxflt  : max no. of fields for misc. horizontal computations
c  mindat  : max length of read field buffer
c  mparlim : max no. of parameter with min/max limits (after interpolation) 
c
ccc   parameter (maxij=80000)
ccc   parameter (maxd=1000000,maxd1=100000)
ccc   parameter (maxlev=50,maxpar=40,maxpr1=15)
ccc   parameter (maxcrs=80,maxtim=60)
ccc   parameter (maxinp=500)
ccc   parameter (maxpos=9000)
ccc   parameter (maxflt=14)
ccc   parameter (mparlim=50)
c
ccc   parameter (mindat=20+maxij+50)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c  no. of 'compute.xxxxx' parameters
      parameter (mcomp=7)
c
c..parameters added by compute option or always
      parameter (nparx=mcomp,npar1x=8)
c
c..length of second file header
c      parameter (linfout=32)
c
c..buffer and file record length
      parameter (lbuff=1024)
      integer*2 buff(lbuff)
c
c..dimension xxx(max+1) for nice overflow testing
c
      integer ilev(maxlev+1),ipar(maxpar+nparx+1)
      integer iparlev(maxpar+nparx+1)
      integer ipar1(maxpr1+npar1x+1),itim(3,maxtim+1)
      integer ipar1lev(maxpr1+npar1x+1),kpar1(maxpr1+npar1x+1)
      integer numpos(9,maxcrs),itime(5,maxtim)
      integer khermit(2,maxpos,2),ihermit(2,maxpos)
      integer lname(maxcrs),iwhere(2,maxcrs,maxtim)
      integer igsize(4),igsiz2(4),iout(maxcrs)
      integer iparlim(2,mparlim+1)
      real    posinp(3,maxinp),posmap(2,maxpos),grid(6),grid2(6)
      real    geopos(maxpos,2),gxypos(maxpos,2),pardat(maxpos,4)
      real    outpos(maxpos,npar1x)
      real    dhermit(2,maxpos)
      real    vrange(2,maxcrs)
      real    alevel(maxlev),blevel(maxlev)
      real    parlim(2,mparlim+1)
      character*256 cname(maxcrs)
c
c..arrays for all crossections:
c..       cdat(npos_total,npar,nlev)  cdat1(npos_total,npar1)
c
c..output for each crossection:
c..       cdat(npos,nlev,npar)  cdat1(npos,npar1)
c
      real    cdat(maxd),cdat1(maxd1)
c
      integer isdat(maxpar*maxlev),isdat1(maxpr1+npar1x)
     +       ,isgrid(6)
      real    sgrid(6)
c
      real      f(maxij,maxflt),fmap(maxij,3)
      integer*2 indat(mindat)
c
      integer   icomp(mcomp)
      integer   ilevot(maxlev)
      integer   info(12),idato(4),idfout(4),infout(32)
c
      integer*2 idfile(32)
c
      character*256 fileot,filein(maxtim),text
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*256 cinput,cipart
      character*256 finput
      character*1   tchar
      integer   igopt,ivopt,np1d
c
      logical   swapfile,swap
c
      integer  igeogrid
      real      geogrid(6)
c
c..geographic coordinates with a grid description (for xyconvert)
      data igeogrid/2/
      data  geogrid/1.,1.,1.,1.,0.,0./
c
      data undef/+1.e+35/
c
c
      iprhlp=0
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c..termination character for free format read (machine dependant)
      call termchar(tchar)
c
c..file unit for  data file
      iunito=30
c
c--------------------------------------------------------------------
c
      narg=iargc()
      if(narg.lt.1 .or. narg.gt.3) then
        call phelp()
      end if
c      Get filename for study
      call getarg(1,finput)
      fileot=finput
c      Want verbose output?
      ivopt=0
c      Want lat/lon positions?
      igopt=0
      do i=2,narg
         call getarg(i,cinput)
         if (cinput.eq.'-v') then 
           ivopt=1
         elseif (cinput.eq.'-g') then 
           igopt=1
         else 
           write(6,*) 'Error, option not recognized: ',cinput
           call phelp()
         endif
      end do
c
c..open output file
      open(iunito,file=finput,
     *              access='direct',form='unformatted',
     *              recl=(lbuff*2)/lrunit,
     *              status='old',iostat=ios,err=910)
      if(ios.ne.0) then
        write(6,*) 'open error: ',finput(1:lenstr(finput,1))
        call phelp()
      end if
c
c
      if (ivopt.ne.0) write(6,*) 
     +'reading file: ',finput(1:lenstr(finput,1))
c
      nlines = 0
c
      iformt =-1
      nrprod = 0
      nrgrid = 0
      ivcoor = 0
      levspec= 0
      nlev   = 0
      ilevpn = 0
      ntim   = 0
      nfilin = 0
      npar   = 0
      npar1  = 0
      inter  = 0
      nparlim= 0
c
      do i=1,mcomp
        icomp(i) = 0
      end do
c
      isline=-1
c
c      fileot ='*'
c
      iprmin = -32767
      iprmax = +32767
c
      iend=0
c
c-------------------------------------------------------------------

      ireco=0
      ibuff=0
c
c..file identification
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            idfout(1),4,ierror)
c      if(ierror.ne.0) goto 920
c      idfout( 1)=121
c      idfout( 2)=1
c      idfout( 3)=lbuff*2
      if (ivopt.ne.0) then 
        write(*,*) 'file identification:'
        write(*,*)  idfout( 1), idfout( 2), idfout( 3), idfout( 4)
      endif
 
      if ( (idfout( 1).ne.121) .or.
     +     (idfout( 2).ne.1 .and. idfout(2).ne.2) .or.
     +     (idfout( 3) .ne. (lbuff*2)) ) goto 920

      linfout = idfout( 4)

      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            infout(1),linfout,ierror)

      if(ierror.ne.0) goto 920

c..file identification
c
      ivcoor   =infout( 1)
      mpos     =infout( 2)
      ncross   =infout( 3)
      nlev     =infout( 4)
      npar     =infout( 5)
      nlev1    =infout( 6)
      nnpar1   =infout( 7)
      ntime    =infout( 9)
      ntim     =infout( 8)+ntime
      nlvlid   =infout(10)
      nrprod   =infout(11)
      nrgrid   =infout(12)
      ltext    =infout(13)
      mlname   =infout(14)
      npmap    =infout(15)
      igtype   =infout(16)
      igsize(1)=infout(17)
      igsize(2)=infout(18)
      igsize(3)=infout(19)
      igsize(4)=infout(20)
      igtyp2   =infout(21)
      igsiz2(1)=infout(22)
      igsiz2(2)=infout(23)
      igsiz2(3)=infout(24)
      igsiz2(4)=infout(25)
c
      if (ivopt.ne.0) then 
       write(*,*) 'ivcoor, mpos, ncross, nlev, npar, nlev1, nnpar1'
       write(*,*) ivcoor, mpos, ncross, nlev, npar, nlev1, nnpar1
       write(*,*) 'ntime, ntim, nlvlid, nrprod, nrgrid, ltext, mlname'
       write(*,*) ntime, ntim, nlvlid, nrprod, nrgrid, ltext, mlname
       write(*,*) 'npmap, igtype, igtyp2'
       write(*,*) npmap, igtype, igtyp2
       write(*,*) 'igsize(1), igsize(2), igsize(3), igsize(4)'
       write(*,*) igsize(1), igsize(2), igsize(3), igsize(4)
       write(*,*) 'igsiz2(1), igsiz2(2), igsiz2(3), igsiz2(4)'
       write(*,*) igsiz2(1), igsiz2(2), igsiz2(3), igsiz2(4)
      endif
      write(*,*) 'Producer, Grid: ',nrprod, nrgrid
      if(ierror.ne.0) goto 920
c..text
      call bgetch(iunito,ireco,buff,lbuff,ibuff,
     +            text,1,ltext,ierror)
      write(*,*) 'Description: ',text(1:ltext)
      if(ierror.ne.0) goto 920
c
c..input grid parameters
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            isgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
c
      call bgetr4(iunito,ireco,buff,lbuff,ibuff,
     +            sgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
c      construct the original array
      do i=1,6
        call asr4i2(-1,1,sgrid(i),0,iundef,undef,isgrid(i),nud)
        grid(i)=sgrid(i)
      end do
      if (ivopt.ne.0) then 
       write(*,*) 'input grid parameters:'
       write(*,*) grid(1),grid(2),grid(3),grid(4),
     +            grid(5),grid(6)
      endif
      if(ierror.ne.0) goto 920
c
c..presentation grid parameters
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            isgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
c
      call bgetr4(iunito,ireco,buff,lbuff,ibuff,
     +            sgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
c      construct the original array
      do i=1,6
        call asr4i2(-1,1,sgrid(i),0,iundef,undef,isgrid(i),nud)
        grid2(i)=sgrid(i)
      end do
      if (ivopt.ne.0) then 
       write(*,*) 'presentation grid parameters:'
       write(*,*) grid2(1),grid2(2),grid2(3),grid2(4),
     +            grid2(5),grid2(6)
      endif
c
c..vertical range
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            isrange,1,ierror)
      if(ierror.ne.0) goto 920
      call bgetr4(iunito,ireco,buff,lbuff,ibuff,
     +            vrange,2*ncross,ierror)
      if(ierror.ne.0) goto 920
      call asr4i2(-1,2*ncross,vrange(1,1),0,iundef,undef,isrange,nud)
      if (ivopt.ne.0) then 
       write(*,*) 'vertical range:'
       do i=1,ncross
        write(*,'(2(f11.3,1x))') vrange(1,i),vrange(2,i)
       end do
      endif
c
c..parameter no. (multilevel and single level)
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            ipar(1),npar,ierror)
      if (ivopt.ne.0) then 
       write(*,*) 'parameter no. (multilevel and single level):'
       write(*,*) (ipar(i),i=1,npar)
      endif
      if(ierror.ne.0) goto 920
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            ipar1(1),nnpar1,ierror)
      if (ivopt.ne.0) then 
       write(*,*) (ipar1(i),i=1,nnpar1)
      endif
c      Do this tricky lookout for number of 1d parameters
      np1d = 0
      do i=1,nnpar1
        if (ipar1(i).le.0) then
          np1d = i-1
          goto 30
        endif
      end do
 30   continue
      write (*,*) 'Number of parameters: ',npar+np1d
      do i=1,npar
        write (*,*) ipar(i)
      end do
      do i=1,np1d
        write (*,*) ipar1(i)
      end do
      if(ierror.ne.0) goto 920
c
c..no. of positions in each crossection
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            iout,ncross,ierror)
      if (ivopt.ne.0) then 
       write(*,*) 'no. of positions in each crossection:'
       write(*,*) (iout(i),i=1,ncross)
      endif
      if(ierror.ne.0) goto 920
c
c..crossection names
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            lname,ncross,ierror)
      if (ivopt.ne.0) then 
       write(*,*) 'crossection names length:'
       write(*,*) (lname(i),i=1,ncross)
      endif
      if(ierror.ne.0) goto 920
      write(*,*) 'Number of cross-sections: ',ncross
c      write(*,*) 'Crossection names:'
      do i=1,ncross
        cname(i)=' '
        call bgetch(iunito,ireco,buff,lbuff,ibuff,
     +              cname(i),1,lname(i),ierror)    
c..strip off "!! positions specs........" in each name
	k=index(cname(i),'!!')
	if (k.gt.0) then
	  k=k-1
	  do while (k.gt.1 .and. cname(i)(k:k).eq.' ')
	    k=k-1
	  end do
	  kk=k+1
	  do k=kk,lname(i)
	    cname(i)(k:k)=' '
	  end do
	  lname(i)=(kk+1)/2*2
	end if
        write(*,*) cname(i)(1:lname(i))
        if(ierror.ne.0) goto 920
      end do
c
c..date/time
      write(*,*) 'Number of dates/times: ',ntime
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            itime,5*ntime,ierror)
      do j=1,ntime
        write(*,*) (itime(i,j),i=1,5)
      end do
      if(ierror.ne.0) goto 920
c
c..no. of map positions in each crossection
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            iout,ncross,ierror)
c      if (ivopt.ne.0) then
       write(*,*) 'Number of map positions in each crossection:'
       do i=1,ncross
        write(*,*) iout(i)
       end do
c      endif
      if(ierror.ne.0) goto 920
c
c..map positions
      call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +            ispmap,1,ierror)
      if(ierror.ne.0) goto 920
c..map x
c..map y
      call bgetr4(iunito,ireco,buff,lbuff,ibuff,
     +            outpos(1,1),npmap,ierror)
      if(ierror.ne.0) goto 920
      call bgetr4(iunito,ireco,buff,lbuff,ibuff,
     +            outpos(1,2),npmap,ierror)
      if(ierror.ne.0) goto 920
      do i=1,npmap
        posmap(1,i) = outpos(i,1)
        posmap(2,i) = outpos(i,2)
      end do
      call asr4i2(-1,2*npmap,posmap(1,1),0,iundef,undef,ispmap,nud)
      if (igopt.eq.0) then
        write(*,*) 'Map X,Y positions:'
      else
        write(*,*) 'Map Lon,Lat positions:'
      endif
      do i=1,npmap
c     --- Try to convert back to Lat/Lon
        gxypos(i,1)=posmap(1,i)
        gxypos(i,2)=posmap(2,i)
        call xyconvert(1,gxypos(i,1),gxypos(i,2),
     +               igtype,grid,igeogrid,geogrid,ierror)
        if (igopt.eq.0)
     +   write(*,'(2(f11.3,1x))') 
     +        posmap(1,i), posmap(2,i)
        if (igopt.eq.1)
     +   write(*,'(2(f11.3,1x))') 
     +        gxypos(i,1), gxypos(i,2)
        if(ierror.ne.0) goto 920
c     +           outpos(i,1)*10.**ispmap,
c     +           outpos(i,2)*10.**ispmap,
      end do
      if(ierror.ne.0) goto 920
c
c..pointer to data for each crossection and time
      if (ivopt.ne.0) 
     + write(*,*) 'pointer to data for each crossection and time:'
      do it=1,ntime
        if (idfout(2).eq.1) then
          call bgeti4(iunito,ireco,buff,lbuff,ibuff,
     +                iwhere(1,1,it),2*ncross,ierror)
        else
          call bgeti4d(iunito,ireco,buff,lbuff,ibuff,
     +                 iwhere(1,1,it),2*ncross,ierror)
        endif
        if (ivopt.ne.0) write(*,*) (iwhere(1,i,it),i=1,2*ncross)
        if(ierror.ne.0) goto 920
      end do
c
      call exit(0)
      close(iunito)
c
      goto 990
c
  910 write(6,*) 'open error. iostat=',ios,' input file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      call exit(3)
c
  920 write(6,*) 'read error. (',ierror,') input file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      call exit(3)
c
  990 continue
c
      end

      subroutine phelp
        write(6,*)
        write(6,*) '   usage: vcinfo data.vc [-g] [-v] '
        write(6,*) '   This program print information from '
        write(6,*) '   a vertical crosssection data file such as'
        write(6,*) '   made from the program vcdata.'
        write(6,*) '   Option -g gives lat/lon map positions, default',
     +             ' is x/y positions.'
        write(6,*) '   Option -v gives very verbose output.'
        write(6,*)
        call exit(1)
      end
