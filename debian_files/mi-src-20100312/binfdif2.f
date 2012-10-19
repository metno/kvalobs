      program binfdif2
c
c        binary data, integer*2 - differences between two files.
c
c----------------------------------------------------------------------
C
C      DNMI library subroutines:  RLUNIT
C
C------------------------------------------------
C  DNMI/FoU  04.11.1992  Anstein Foss ... Unix
C------------------------------------------------
c
c
      parameter (lrec=2048)
c
      integer*2 i2d1(lrec),i2d2(lrec)
c
      character*256 carg,file1,file2
c
c
      narg=iargc()
      if(narg.eq.3) then
        call getarg(1,file1)
        call getarg(2,file2)
        call getarg(3,carg)
        read(carg,*,iostat=ios) nbytes
        if(ios.ne.0) narg=0
      end if
      if(narg.ne.3) then
        write(6,*) '  usage:  binfdif2 file_1 file_2 no_of_bytes'
        stop
      end if
      write(6,*) 'file 1: ',file1
      write(6,*) 'file 2: ',file2
      write(6,*) 'bytes:  ',nbytes
c
      nword=nbytes/2
      if(nword*2.ne.nbytes) write(6,*) 'Can''t handle the last byte'
c
c..get record length unit in bytes for RECL= in OPEN statements
c..(machine dependant)
      call rlunit(lrunit)
c
      open(11,file=file1,
     *	      access='direct',form='unformatted',
     *        recl=(lrec*2)/lrunit,
     *	      status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'Open error: ',file1
        stop
      end if
c
      open(12,file=file2,
     *	      access='direct',form='unformatted',
     *        recl=(lrec*2)/lrunit,
     *        status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'Open error: ',file2
        stop
      end if
c
      irec=0
      lword=0
      ndiff=0
c
      do iword=1,nword,lrec
	lw=min0(iword+lrec-1,nword)
	lread=lw-iword+1
	irec=irec+1
        read(11,rec=irec,iostat=ios,err=91) (i2d1(i),i=1,lread)
        read(12,rec=irec,iostat=ios,err=92) (i2d2(i),i=1,lread)
        do i=1,lread
          if(i2d1(i).ne.i2d2(i)) then
            n=iword-1+i
            idiff=i2d1(i)-i2d2(i)
            write(6,fmt='(2x,i8,'' : '',2i8,2x,i8)')
     *                       n,i2d1(i),i2d2(i),idiff
            ndiff=ndiff+1
          end if
        end do
	lword=lw
      end do
c
      goto 99
c
   91 write(6,*) 'Read error: ',file1
      goto 99
   92 write(6,*) 'Read error: ',file2
      goto 99
c
   99 continue
      write(6,*) 'Words read:  ',lword
      write(6,*) 'Differences: ',ndiff
      close(11)
      close(12)
      end
