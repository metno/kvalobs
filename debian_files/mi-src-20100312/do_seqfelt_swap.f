      program do_seqfelt_swap
c
c  Testing fortran sequential unformatted (binary) DNMI sequential FIELD file.
c  Return (exit) with: 0=not swapped (or could not be determined!)
c                      1=swapped file
c     The code (0 or 1) printed and retuned as exit code.
c
c  Sequential file: 4 bytes header + contents + 4 bytes trailer
c                   Header and trailer has the same value, number bytes in contents.
c		    The first header in DNMI sequential FIELD file is always 40 bytes.
c		    NOTE: the header and trailer is really an "unsigned integer*4" (2**32 - 1),
c			  but this program only handles max "signed integer*4" (2**31 - 1)!
c
c-----------------------------------------------------------------------
c  met.no/FoU  27.06.2003  Anstein Foss
c  met.no/FoU  26.10.2003  Anstein Foss
c-----------------------------------------------------------------------
c
      implicit none
c
      integer narg,iargc,iexit,iunit,lrunit,ios
      integer idseq
c
      character*256 filename
c
      narg=iargc()
c
      if(narg.ne.1) then
	write(6,*) ' usage: do_seqfelt_swap <filename>'
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
     +     recl=48/lrunit,status='old',iostat=ios)
      if(ios.eq.0) then
	read(iunit,rec=1,iostat=ios) idseq
	if(ios.eq.0) then
	  if(idseq.eq.40) then
	    iexit=0
	  else
	    call bswap4(1,idseq)
	    if(idseq.eq.40) iexit=1
	  end if
	end if
	close(iunit)
      end if
c
      if(iexit.eq.255) iexit=0
c
      write(6,fmt='(i1)') iexit
      call exit(iexit)
c
      end
