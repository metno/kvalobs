      program ftnsplit
c
c  a "fsplit" program...
c
c------------------------------------------------
c  DNMI/FoU  09.06.2001  Anstein Foss
c------------------------------------------------
c
      character*256 filein,fileot,cmax
      character*512 textinp,text
c
      maxcol=len(text)
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*) '  usage:  ftnsplit <file(s)>'
        stop
      end if
c
      lr=0
      lw=0
      nf=0
      kkmax=0
c
      do iarg=1,narg
c
      call getarg(iarg,filein)
c
      iuniti=10
      iunito=20
      write(6,*) filein(1:lenstr(filein,1))
      open(iuniti,file=filein,
     *            access='sequential',form='formatted',
     *            status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error.   iostat= ',ios
        write(6,*) 'file: ',filein
        stop 1
      end if
c
      iwrite=0
c
      do while (.true.)
c
        lr=lr+1
	textinp=' '
c
        read(iuniti,fmt='(a)',iostat=ios,err=900,end=500) textinp
	text=textinp
c
        kf=0
        kk=0
        do k=1,maxcol
          if(text(k:k).ne.' ') then
	    if(kf.eq.0) kf=k
	    kk=k
          end if
        end do
	if(kkmax.lt.kk) kkmax=kk
c
	if(iwrite.eq.1) then
c
          if(kf.ge.7 .and. kk.eq.kf+2) then
	    call chcase(1,1,text)
	    if(text(kf:kf+2).eq.'end') iwrite=-1
	  end if
c
	else
c
          if(kf.ge.7 .and. kk.ge.kf+8) then
	    if(text(kf:kf).eq.'p' .or. text(kf:kf).eq.'P' .or.
     +	       text(kf:kf).eq.'s' .or. text(kf:kf).eq.'S' .or.
     +	       text(kf:kf).eq.'f' .or. text(kf:kf).eq.'F') then
	      call chcase(1,1,text)
	      ks=0
	      if(text(kf:kf+ 7).eq.'program ')    ks=kf+8
	      if(text(kf:kf+10).eq.'subroutine ') ks=kf+11
	      if(text(kf:kf+ 8).eq.'function ')   ks=kf+9
	      if(ks.gt.0 .and. ks.le.kk) then
	        do while (ks.le.kk .and. text(ks:ks).eq.' ')
		  ks=ks+1
	        end do
	        if(ks.gt.0 .and. ks.le.kk) then
	          ke=ks
	          k=0
	          do while (k.eq.0 .and. ke.lt.kk)
	            ke=ke+1
		    if(text(ke:ke).eq.' ' .or. text(ke:ke).eq.'(') k=1
	          end do
	          if(k.eq.1) ke=ke-1
	          if(ks.le.ke) then
		    fileot=text(ks:ke)//'.f'
		    write(6,*) '     ',fileot(1:lenstr(fileot,1))
                    open(iunito,file=fileot,
     +                          access='sequential',form='formatted',
     +                          status='new',iostat=ios)
                    if(ios.ne.0) then
                      write(6,*) 'open error.   iostat= ',ios
                      write(6,*) 'file: ',fileot
                      write(6,*) '(note that this file can''t exist)'
                      close(iuniti)
                      stop 1
                    end if
		    nf=nf+1
		    iwrite=1
		  end if
		end if
	      end if
	    end if
	  end if
c
	end if
c
	if(iwrite.ne.0) then
          if(kk.gt.0) then
            write(iunito,fmt='(a)',iostat=ios,err=920) textinp(1:kk)
          else
            write(iunito,fmt='(a)',iostat=ios,err=920)
          end if
	  lw=lw+1
	  if(iwrite.eq.-1) then
	    close(iunito)
	    iwrite=0
	  end if
	end if
c
      end do
c
  500 lr=lr-1
      close(iuniti)
      if(iwrite.ne.0) close(iunito)
c
      end do
c
      write(6,*) 'max line length:     ',kkmax
      write(6,*) 'no. of lines input:  ',lr
      write(6,*) 'no. of files output: ',nf
      write(6,*) 'no. of lines output: ',lw
c
      goto 990
c
  900 write(6,*) 'read error.  iostat= ',ios,'    line= ',lr
      close(iuniti)
      close(iunito)
      stop 2
c
  920 write(6,*) 'write error.  iostat= ',ios,'    line= ',lr
      close(iuniti)
      close(iunito)
      stop 2
c
  990 continue
      end
