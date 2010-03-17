      program host_bigendian
c
c-----------------------------------------------------------------------
c  met.no/FoU  16.06.2003  Anstein Foss
c-----------------------------------------------------------------------
c
      implicit none
c
      integer iexit
c
      logical bigendian
c
      if(bigendian()) then
	iexit=1
      else
	iexit=0
      end if
c
      write(6,fmt='(i1)') iexit
      call exit(iexit)
c
      end
