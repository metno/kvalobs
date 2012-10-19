      program wspecinfo
c
cI    "wspecinfo" tries to extract time/date and position info from 
cI    wavespectre files (from wave models).
CI    Written by Arild Burud, 01.12.2005
cI    Based on the program cvwamspec by Anstein Foss
c
c      All declarations from original program - lots of obsoletes here..
c
c-----------------------------------------------------------------------
c  met.no    16.12.2005  Arild Burud 
c-----------------------------------------------------------------------
c
      parameter (maxpos=400,maxtim=30)
      parameter (maxfre=60,maxang=60)
      parameter (mdata=maxfre*maxang)
c
c..output buffer and file record length
      parameter (lbuff=1024)
      integer*2 buff(lbuff)
c
      parameter (lhead=4,linfo=6,lextra=8)
      integer*2 ihead(lhead),info(linfo)
      integer   iextra(lextra)
      real      sextra(lextra)
c
      integer*2 ident(20),idummy,iprog,idata(mdata)
      integer*2 latpos(maxpos),lonpos(maxpos)
      integer   intime(6,maxtim),numpos(maxtim)
      integer   lposname(maxpos),iadress(2*maxpos*maxtim)
      real      spec(mdata),fr(maxfre),dfim(maxfre),dir(maxang)
      real      fr2(maxfre),efreq(maxfre),eang(0:maxang+1)
      real      outfr(maxfre),outdir(maxang)
      character*16 posname(maxpos)
      character*8 clat,clon
      character*1 cns,cew
c
      character*256 inpfile,outfile,text

      integer i,n
      real    latscale,lonscale
c
      narg=iargc()

      if(narg.ne.1) then
        write(6,*) ' usage: wspecinfo <input_file>'
        write(6,*) '    eg: wspecinfo wamspec12.dat'
        write(6,*)
     + ' Program prints information about wave spectra '
        write(6,*)
     + ' as an index of lat/lon positions and date/time. '

        if(narg.eq.0) stop
        call exit(1)
      end if
c
c      Open the desired file 
c
      call getarg(1,inpfile)
c
c      write(6,*) 'input:  ',inpfile(1:lenstr(inpfile,1))
c
      iuout=20

      call rlunit(lrunit)
c
      open(iuout,file=inpfile,
     +           access='direct',form='unformatted',
     +           recl=(lbuff*2)/lrunit,
     +           status='unknown',iostat=ios)
c
      if(ios.ne.0) then
        write(6,*) 'ERROR OPEN ',inpfile(1:lenstr(inpfile,1))
        call exit(2)
      end if
c
      irec=0
      ibuff=0
c
c..header
c
c  ihead(1) = 251   : ?
c  ihead(2) = 1     : ?
c  ihead(3) = lbuff : 1024 = lenght of each binary record 
c  ihead(4) = linfo : 6    = length of info()
c
      call bgeti2(iuout,irec,buff,lbuff,ibuff,ihead,lhead,ierr)
c      do i=1,lhead,1
c         write(*,*) 'ihead(',i,'):',ihead(i)
c      end do
      
      if(ierr.ne.0) call exit(2)
c
c..misc. info
c
c  info(1) = npos  : Number of lat/lon positions in the file
c  info(2) = ntim  : Number of date/times stored
c  info(3) = nang  : Number of spectrum directions
c  info(4) = nfre  : Number of spectrum frequences
c  info(5) = lextra: Number of extra parameters w/scaling (?)
c  info(2) = ltext : Length of text description ('text' below)
c
      call bgeti2(iuout,irec,buff,lbuff,ibuff,info,linfo,ierr)
      if(ierr.ne.0) call exit(2)
      npos=info(1)
      ntim=info(2)
      nang=info(3)
      nfre=info(4)
c      lextra=info(5)
      ltext=info(6)
c
c..text - A descriptive title for these model data
c
      call bgetch(iuout,irec,buff,lbuff,ibuff,
     +            text,1,ltext,ierr)
      if(ierr.ne.0) call exit(2)
      write(*,*) 'Model description: ',text(1:ltext)
      write(*,*) 'Number of lat/lon positions: ',npos
c
c..geographic latitude
c     Get scalefactor for integer to real representation
c     Usually iscale = -2
      call bgeti4(iuout,irec,buff,lbuff,ibuff,iscale,1,ierr)
      if(ierr.ne.0) call exit(2)
      latscale = 10.**iscale
      call bgeti2(iuout,irec,buff,lbuff,ibuff,latpos,npos,ierr)
      if(ierr.ne.0) call exit(2)
c
c..geographic longitude
c     Get scalefactor for integer to real representation
c     Usually iscale = -2
      call bgeti4(iuout,irec,buff,lbuff,ibuff,iscale,1,ierr)
      if(ierr.ne.0) call exit(2)
      lonscale = 10.**iscale
      call bgeti2(iuout,irec,buff,lbuff,ibuff,lonpos,npos,ierr)
      if(ierr.ne.0) call exit(2)
c
c..position names, first get length of each name posname(1:npos) 
c      
      call bgeti4(iuout,irec,buff,lbuff,ibuff,lposname,npos,ierr)
      if(ierr.ne.0) call exit(2)
c..position names, now get each posname(1:npos)
      do n=1,npos,1
        call bgetch(iuout,irec,buff,lbuff,ibuff,
     +              posname(n),1,lposname(n),ierr)
        write(*,1011) real(latpos(n))*latscale,
     +                real(lonpos(n))*lonscale,
     +                posname(n)(1:lposname(n))
        if(ierr.ne.0) call exit(2)
      end do
 1011 format(1x,f8.3,1x,f8.3,' : ',a)
c
      write(*,*) 'Number of dates/times: ',ntim
c
c..date/time, valid time and forecast length in hours
      call bgeti4(iuout,irec,buff,lbuff,ibuff,intime,ntim*6,ierr)
      if(ierr.ne.0) call exit(2)
      do n=1,ntim
         write(*,1012) (intime(i,n),i=1,6)
      end do
 1012 format(1x,i4.4,3i3.2,':',i2.2,1x,i4)

      end
