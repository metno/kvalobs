      program do_seqfile_swap
c
c  Testing fortran sequential unformatted (binary) file.
c  Return (exit) with: 0=not swapped (or could not be determined!)
c                      1=swapped file
c     The code (0 or 1) printed and retuned as exit code.
c
c  Sequential file: 4 bytes header + contents + 4 bytes trailer
c                   Header and trailer has the same value, number bytes in contents.
c		    NOTE: the header and trailer is really an "unsigned integer*4" (2**32 - 1),
c			  but this program only handles max "signed integer*4" (2**31 - 1)!
c
c-----------------------------------------------------------------------
c  met.no/FoU  26.10.2003  Anstein Foss
c-----------------------------------------------------------------------
c
      implicit none
c
      integer narg,iargc,iexit,iunit,lrunit,ios,n,i,lrec,nbytes
      integer idseq,idseqs(2),idseqr(2),iswap(2)
c
      integer*1 ibytes(8)
c
      character*256 filename
c
      narg=iargc()
c
      if(narg.ne.1) then
	write(6,*) ' usage: do_seqfile_swap <filename>'
	stop
      end if
c
      call getarg(1,filename)
c
      iexit=255
c
      iunit=20
c
      call rlunit(lrunit)
c
      open(iunit,file=filename,access='direct',form='unformatted',
     +     recl=8/lrunit,status='old',iostat=ios)
      if(ios.eq.0) then
	read(iunit,rec=1,iostat=ios) idseq
	if(ios.eq.0) then
	  idseqs(1)=idseq
	  call bswap4(1,idseq)
	  idseqs(2)=idseq
	else
	  iexit=0
	end if
	close(iunit)
      else
	iexit=0
      end if
c
      if(iexit.eq.255) then
c
        if(idseqs(1).eq.0) then
	  iexit=0
        elseif(idseqs(1).gt.0 .and. idseqs(2).lt.0) then
	  iexit=0
        elseif(idseqs(1).lt.0 .and. idseqs(2).gt.0) then
	  iexit=1
        elseif(idseqs(1).lt.idseqs(2)) then
	  idseqr(1)=idseqs(1)
	  idseqr(2)=idseqs(2)
	  iswap(1)=0
	  iswap(2)=1
        else
	  idseqr(1)=idseqs(1)
	  idseqr(2)=idseqs(2)
	  idseqs(1)=idseqr(2)
	  idseqs(2)=idseqr(1)
	  iswap(1)=1
	  iswap(2)=0
        end if
c
	n=0
	do while (iexit.eq.255 .and. n.lt.2)
c
	  n=n+1
	  lrec=4+idseqs(n)
	  nbytes=mod(lrec,lrunit)
c
	  if(lrec/lrunit.gt.1 .or. nbytes.eq.0) then
c
	    open(iunit,file=filename,access='direct',form='unformatted',
     +           recl=lrec/lrunit,status='old',iostat=ios)
	    if(ios.eq.0) then
	      if (nbytes.eq.0) then
	        read(iunit,rec=2,iostat=ios) idseq
	      else
	        read(iunit,rec=2,iostat=ios) (ibytes(i),i=1,nbytes),idseq
	      end if
	      if(ios.eq.0 .and. idseq.eq.idseqr(n)) iexit=iswap(n)
	    end if
c
	  else
c
	    lrec=4+idseqs(n)+4
	    nbytes=mod(lrec,lrunit)
	    open(iunit,file=filename,access='direct',form='unformatted',
     +           recl=(lrec+lrunit-1)/lrunit,status='old',iostat=ios)
c
	    read(iunit,rec=1,iostat=ios) i,(ibytes(i),i=1,nbytes),idseq
	    if(ios.eq.0 .and. idseq.eq.idseqr(n)) iexit=iswap(n)
c
	  end if
c
	end do
c
	close(iunit)
c
      end if
c
      if(iexit.eq.255) iexit=0
c
      write(6,fmt='(i1)') iexit
      call exit(iexit)
c
      end
