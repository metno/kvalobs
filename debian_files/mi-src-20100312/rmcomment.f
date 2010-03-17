      program rmcomment
c
c        strip ascii files to a maximum number of columns
c        and remove trailing blanks
c        and remove all comment lines
c
c        output filename = input filename + '_no_comment'
c
c------------------------------------------------
c  DNMI/FoU  10.02.1993  Anstein Foss ... fstrip.f
c  DNMI/FoU  02.12.1993  Anstein Foss
c  DNMI/FoU  09.11.1994  Anstein Foss ... rmcomment.f
c------------------------------------------------
c
      character*256 filein,fileot,cmax
      character*512 text
c
c--------------------------------------------
      narg=iargc()
      if(narg.lt.2) then
        write(6,*) '  usage:  rmcomment <max_line_length> <file(s)>'
        stop
      end if
c
      call getarg(1,text)
      k=index(text,' ')
ccc   kcmax=k
ccc   cmax='_'//text(1:k-1)
      kcmax=11
      cmax='_no_comment'
      text(k:k)=char(0)
      read(text,*,iostat=ios) maxcol
cray..................................................
c     ios=0
c     l=index(text,' ')-1
c     if(l.lt.1) l=len(text)
c     izero=ichar('0')
c     maxcol=0
c     do k=1,l
c	inum=ichar(text(k:k))-izero
c	if(inum.ge.0 .and. inum.le.9) then
c	  maxcol=maxcol*10+inum
c	else
c	  ios=9999
c	end if
c     end do
cray..................................................
      if(ios.ne.0) then
        write(6,*) '  usage:  rmcomment <max_line_length> <file>'
        stop 1
      end if
      m=len(text)
      if(maxcol.lt.1 .or. maxcol.gt.m) then
        write(6,*) '  max line length less than 1 or too big'
        write(6,*) '  valid range is  1 to ',m
        stop 1
      end if
c
      do iarg=2,narg
c
      call getarg(iarg,filein)
      m=len(fileot)
      l=index(filein,' ')-1
      if(l.lt.1 .or. l+kcmax.gt.m) then
        write(6,*) '  too long file name:'
	write(6,*) filein
        stop 1
      endif
      fileot=filein(1:l)//cmax(1:kcmax)
      write(6,*) 'input:  ',filein
      write(6,*) 'output: ',fileot
      write(6,*) 'max line length input:  ',maxcol
      kkmax=0
c--------------------------------------------
c
      iuniti=10
      iunito=20
c
      open(iuniti,file=filein,
     *            access='sequential',form='formatted',
     *            status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error.   iostat= ',ios
        write(6,*) 'file: ',filein
        stop 1
      end if
c
      open(iunito,file=fileot,
     *            access='sequential',form='formatted',
     *            status='new',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error.   iostat= ',ios
        write(6,*) 'file: ',fileot
        write(6,*) '(note that this file can''t exist)'
        close(iuniti)
        stop 1
      end if
c
      l=0
      lout=0
c
      do while (.true.)
c
        l=l+1
c
        read(iuniti,fmt='(a)',iostat=ios,err=900,end=500) text
c
	if(text(1:1).ne.'c' .and. text(1:1).ne.'C'
     +                      .and. text(1:1).ne.'*') then
c
        kk=0
        do k=1,maxcol
          if(text(k:k).ne.' ') kk=k
        end do
        kkmax=max0(kkmax,kk)
c
        if(kk.gt.0) then
          write(iunito,fmt='(a)',iostat=ios,err=920) text(1:kk)
ccc     else
ccc       write(iunito,fmt='(a)',iostat=ios,err=920)
        end if
c
	lout=lout+1
	end if
c
      end do
c
  500 l=l-1
      write(6,*) 'max line length output: ',kkmax
      write(6,*) 'no. of lines  input:  ',l
      write(6,*) 'no. of lines output:  ',lout
      close(iuniti)
      close(iunito)
c
      end do
c
      goto 990
c
  900 write(6,*) 'read error.  iostat= ',ios,'    line= ',l
      close(iuniti)
      close(iunito)
      stop 2
c
  920 write(6,*) 'write error.  iostat= ',ios,'    line= ',l
      close(iuniti)
      close(iunito)
      stop 2
c
  990 continue
      end
