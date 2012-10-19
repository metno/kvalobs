      program funpack
c
c     Field UNPACK
c
c     ******* unix version *******
c
c     input:  en eller flere filer med pakkede felt (f.eks. fra
c             programmet copflt, pakket med subr. fpack1/fpack2)
c             hvis dataene har forskjellig termin-tid
c             (dvs. anayse-tidspunkt), blir bare data for den siste
c             (nyeste) termintiden benyttet.
c
c     output: en standard felt-file
c	      eller en sekvensiell file med felt (ident,felt,ident,...)
c
c     Denne versjonen kan ogsaa benytte input-filer i PC-format (-pc),
c     spesiell versjon for DOS uten padding av siste record i filen.
c
c     Automatisk byte-swapping av input hvis nodvendig.
c
c---------------------------------------------------------------------
c
c      DNMI library subroutines:  lenstr
c                                 rlunit
c                                 rmfile
c                                 bswap2
c
c---------------------------------------------------------------------
c  DNMI/FoU  31.10.1989  Anstein Foss
c  DNMI/FoU  02.08.1990  Anstein Foss
c  DNMI/FoU  27.12.1990  Anstein Foss
c  DNMI/FoU  13.05.1993  Anstein Foss
c  DNMI/FoU  11.09.1997  Anstein Foss ... packed.field>32767 words ++
c  DNMI/FoU  17.10.1997  Anstein Foss ... -s = sequential output files
c  DNMI/FoU  22.03.2003  Anstein Foss ... automatic byte swap (input)
c  DNMI/FoU  27.03.2008  Rebecca Rudasr ... endret maxij fra 500000 til 1000000
c---------------------------------------------------------------------
c
c..mfilin: max antall input data filer med felt
c
      parameter (mfilin=100)
c
c..maxij:  max st@rrelse (ii*jj) for felt.
      parameter (maxij=1000000)
c
      parameter (lenrec=1024)
c
      parameter (maxpak=maxij)
c
      parameter (ninrec=lenrec/16)
c
      common/a/ireco1(lenrec),ireco2(lenrec),innho(16,ninrec),
     *         ident(20),ifelt(maxij),
     *         identp(24),ipack(maxpak)
      integer*2 ireco1,ireco2,innho,ident,ifelt
      integer*2 identp,ipack
c
      common/b/itimeo(4),
     *         ninf(mfilin),itime(4,mfilin),iprod(mfilin),
     *         nprod(2,mfilin),kpack(3,mfilin),nread(mfilin),
     *         nfread(mfilin)
c
      integer*2 ihead(16)
      integer*2 ileft(lenrec)
c
      character*256 filin(mfilin),filout
c
c-pack-----------------------------------------------------------------
c        ihead( 1)= 101 ...... file identification
c             ( 2)= year........................)
c             ( 3)= month*100+day ..............) file generation
c             ( 4)= hour*100+minute (utc) ......)
c             ( 5)= year.....................)
c             ( 6)= month*100+day ...........) date/time of fields
c             ( 7)= hour*100+minute (utc) ...)
c             ( 8)= max. no. of fields
c             ( 9)= no. of producers
c             (10)= first producer
c             (11)= packing method:  1=pc  2=unix
c             (12)= 20/24, length of field-identification
c             (13)= max. field length (unpacked, 16 bit words) NOT USED
c             (14)= ipunit, length unit for identp(24), in words
c             (15)= iupgap, 0=unpack with data gaps  1=without data gaps
c             (16)= 0  (not used)
c-pack-----------------------------------------------------------------
c
c-----------------------------------------------------------------
      narg=iargc()
      if(narg.lt.2) then
        write(6,*) '  usage:  funpack [options]',
     +			      ' file_in_1 file_in_2 ... file_out'
        write(6,*) '     options:'
        write(6,*) '             -pc : input files in PC format'
        write(6,*) '             -s  : sequential output file'
ccc     stop 1
        call exit(1)
      elseif(narg.gt.mfilin+1) then
        write(6,*) '  too many input files.  max. no. is ',mfilin
ccc     stop 1
        call exit(1)
      end if
c
      nfilin=narg-1
      do n=1,nfilin
        call getarg(n,filin(n))
      end do
      call getarg(narg,filout)
c
      ipcfile=0
      iseqout=0
      n=0
      m=2
      i=0
      do while (i.lt.m)
	i=i+1
        if(filin(i).eq.'-pc') then
	  ipcfile=1
	  n=i
        elseif(filin(i).eq.'-spc' .or. filin(i).eq.'-pcs') then
	  ipcfile=1
	  iseqout=1
	  n=i
        elseif(filin(i).eq.'-s') then
	  iseqout=1
	  n=i
	else
	  m=i
	end if
      end do
c
      if(n.gt.0) then
	nfilin=nfilin-n
	if(nfilin.lt.1) then
	  write(6,*) '   no input files'
ccc       stop 1
          call exit(1)
	end if
	do i=1,nfilin
	  filin(i)=filin(i+n)
	end do
      end if
c
      n=0
      do i=1,nfilin
	if(filin(i).eq.filout) n=1
      end do
      if(n.ne.0) then
	write(6,*) '  error: input filename = output filename'
ccc     stop 1
        call exit(1)
      end if
c-----------------------------------------------------------------
c
      ifilin=10
      ifilot=20
c
      nogap=1
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c---------------------------------------------------------------------
c
c        finn termin-tider og 'minste' produsent nr. paa aktuelle
c        input-filer
c
      nopen=0
      do 50 n=1,nfilin
        ninf(n)=0
        open(ifilin,file=filin(n),
     +		    access='direct',form='unformatted',
     +              recl=2048/lrunit,
     +              status='old',iostat=ios)
        if(ios.ne.0) then
          write(6,*) ' ** open error. input file: ',
     +			filin(n)(1:lenstr(filin(n),1))
          goto 50
        end if
        nxread=0
        call rdata(ifilin,ipcfile,ihead,16,nxread,ios,ierr)
        if(ierr.ne.0) then
          write(6,*) ' ** read error. input file: ',
     +			filin(n)(1:lenstr(filin(n),1))
          close(ifilin)
          goto 50
        end if
        itime(1,n)=ihead(5)
        itime(2,n)=ihead(6)/100
        itime(3,n)=ihead(6)-ihead(6)/100*100
        itime(4,n)=ihead(7)
        ninf(n)=ihead(8)
        nprod(1,n)=ihead(9)
        iprod(n)=ihead(10)
        kpack(1,n)=ihead(11)
	if(ipcfile.eq.1 .and. kpack(1,n).eq.1) kpack(1,n)=2
        kpack(2,n)=ihead(12)
        kpack(3,n)=ihead(14)
	if(kpack(3,n).lt.1) kpack(3,n)=1
	if(ihead(15).ne.1) nogap=0
        close(ifilin)
        nopen=nopen+1
        if(nopen.eq.1) then
          igt=1
        else
          igt=0
          do i=1,4
            if(igt.eq.0 .and. itime(i,n).lt.itimeo(i)) igt=-1
            if(igt.eq.0 .and. itime(i,n).gt.itimeo(i)) igt=+1
	  end do
        end if
        if(igt.eq.1) then
          itimeo(1)=itime(1,n)
          itimeo(2)=itime(2,n)
          itimeo(3)=itime(3,n)
          itimeo(4)=itime(4,n)
        end if
   50 continue
c
      if(nopen.lt.1) then
        write(6,*) ' ** no input files.  no output file.'
ccc     stop 2
        call exit(2)
      end if
c
c..marker input-filer med feil dato/termin i listen
c
      do n=1,nfilin
        if(ninf(n).eq.0) then
          continue
        elseif(itime(1,n).ne.itimeo(1) .or.
     *         itime(2,n).ne.itimeo(2) .or.
     *         itime(3,n).ne.itimeo(3) .or.
     *         itime(4,n).ne.itimeo(4)) then
          ninf(n)=-1
        end if
      end do
c
c..remove output file if it exists
      call rmfile(filout,0,ierror)
c
      write(6,*) 'output felt file: ',filout(1:lenstr(filout,1))
      write(6,1020) itimeo(3),itimeo(2),itimeo(1),itimeo(4)
 1020 format(2x,'termin:  ',i2.2,':',i2.2,':',i4.4,1x,i4.4,' utc')
c
      if(iseqout.eq.0) then
        open(ifilot,file=filout,
     +		    access='direct',form='unformatted',
     +              recl=2*lenrec/lrunit,
     +              status='unknown',iostat=ios,err=410)
      else
        open(ifilot,file=filout,
     +		    access='sequential',form='unformatted',
     +              status='unknown',iostat=ios,err=410)
      end if
c
      do n=1,nfilin
        if(ninf(n).gt.0) then
          write(6,*) 'input file ok:'
          write(6,*) filin(n)(1:lenstr(filin(n),1))
          write(6,1020) itime(3,n),itime(2,n),itime(1,n),itime(4,n)
        elseif(ninf(n).lt.0) then
          write(6,*) 'input file has wrong date/time:'
          write(6,*) filin(n)(1:lenstr(filin(n),1))
          write(6,1020) itime(3,n),itime(2,n),itime(1,n),itime(4,n)
        else
          write(6,*) 'input file not used, open/read error:'
          write(6,*) filin(n)(1:lenstr(filin(n),1))
        end if
      end do
c
      ninnh=0
      ifpmax=0
      do n=1,nfilin
        if(ninf(n).gt.0) then
          ninnh=ninnh+ninf(n)
          ifpmax=ifpmax+nprod(1,n)
        else
          ifpmax=ifpmax+1
        end if
        nfread(n)=0
        nread(n)=0
        nprod(2,n)=0
      end do
c
      do i=1,lenrec
        ireco1(i)=0
        ireco2(i)=0
      end do
      ireco1(1)=999
      ireco1(5)=itimeo(1)
      ireco1(6)=itimeo(2)*100+itimeo(3)
      ireco1(7)=itimeo(4)
c
      if(iseqout.eq.0) then
c..record 1 og 2 oppdateres til slutt
        irec=1
        write(ifilot,rec=irec,iostat=ios,err=420) ireco1
        irec=2
        write(ifilot,rec=irec,iostat=ios,err=420) ireco2
      end if
c
      nreci=(ninnh+ninrec-1)/ninrec
      ireco1( 8)=2+nreci
      ireco1( 9)=2
      ireco1(13)=lenrec
      ireco1(14)=nogap
      ninnht=ninnh
      ninnh=0
      nino=0
c
      nrecd=2+nreci
      lword=lenrec
      nleft=0
      nwrite=0
c
c..nb! 'produsenter' skal lagres i stigende rekkefoelge i innh.
c
      lprodu=0
c
      do 110 ifp=1,ifpmax
c
        iprodu=9999
        nf=0
        do n=1,nfilin
          if(ninf(n).gt.0 .and. iprodu.gt.iprod(n)) then
            iprodu=iprod(n)
            nf=n
          end if
	end do
        if(nf.eq.0) goto 400
        nprod(2,nf)=nprod(2,nf)+1
	ipunit=kpack(3,nf)
c
        write(6,*) 'input file:      producer: ',iprodu
        write(6,*)  filin(nf)(1:lenstr(filin(nf),1))
c
        open(ifilin,file=filin(nf),
     +		    access='direct',form='unformatted',
     +              recl=2048/lrunit,
     +              status='old',iostat=ios,err=190)
c
        if(nread(nf).eq.0) then
          call rdata(ifilin,ipcfile,ihead,16,nread(nf),ios,ierr)
          if(ierr.ne.0) goto 192
c..tid for siste oppdatering
          igt=0
          do i=2,4
            if(igt.eq.0 .and. ihead(i).lt.ireco1(i)) igt=-1
            if(igt.eq.0 .and. ihead(i).gt.ireco1(i)) igt=+1
	  end do
          if(igt.eq.1) then
            ireco1(2)=ihead(2)
            ireco1(3)=ihead(3)
            ireco1(4)=ihead(4)
          end if
        elseif(kpack(1,nf).eq.0) then
c..hopp over records som allerede er lest (annen produsent)
          nnread=nfread(nf)
          nfread(nf)=0
          nread(nf)=0
          call rdata(ifilin,ipcfile,ihead,16,nread(nf),ios,ierr)
          if(ierr.ne.0) goto 192
          do iread=1,nnread
            nfread(nf)=nfread(nf)+1
            call rdata(ifilin,ipcfile,ident,20,nread(nf),ios,ierr)
c..ident(1-20)=0 => 'end_of_file' (dummy bakerst)
            if(ierr.eq.0) then
              n0=0
              do i=1,20
                if(ident(i).eq.0) n0=n0+1
	      end do
              if(n0.eq.20) ierr=2
            end if
            if(ierr.ne.0) goto 192
c..evt. ekstra geometri-identifikasjon etter felt-data
	    lgeom=0
	    igtype=ident(9)
	    if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
	    nx=ident(10)
	    ny=ident(11)
            ldata=nx*ny+lgeom
            call rdata(ifilin,ipcfile,ifelt,ldata,nread(nf),ios,ierr)
            if(ierr.ne.0) goto 192
	  end do
        elseif(kpack(1,nf).eq.2) then
          nnread=nfread(nf)
          nfread(nf)=0
          call rdata(ifilin,ipcfile,ihead,16,nread(nf),ios,ierr)
          if(ierr.ne.0) goto 192
          do iread=1,nnread
            nfread(nf)=nfread(nf)+1
            call rdata(ifilin,ipcfile,identp,24,nread(nf),ios,ierr)
c..identp(1-24)=0 => 'end_of_file' (dummy bakerst)
            if(ierr.eq.0) then
              n0=0
              do i=1,24
                if(identp(i).eq.0) n0=n0+1
	      end do
              if(n0.eq.24) ierr=2
            end if
            if(ierr.ne.0) goto 192
            ldata=identp(24)*ipunit
            call rdata(ifilin,ipcfile,ipack,ldata,nread(nf),ios,ierr)
            if(ierr.ne.0) goto 192
	  end do
        else
          write(6,*) ' *** ukjent pakke-metode: ',kpack(1,nf)
          goto 193
        end if
c
        if(iprodu.ne.lprodu) then
          ireco2(2*iprodu-1)=ireco1(9)+1
          ireco2(2*iprodu)=nino*16+1
          lprodu=iprodu
        end if
c
        nfound=0
c
        do 140 inp=1,ninf(nf)
c
          if(kpack(1,nf).eq.0) then
            nfread(nf)=nfread(nf)+1
            nxread=nread(nf)
            call rdata(ifilin,ipcfile,ident,20,nread(nf),ios,ierr)
c.....ident(1-20)=0 => 'end_of_file' (dummy bakerst)
            if(ierr.eq.0) then
              n0=0
              do i=1,20
                if(ident(i).eq.0) n0=n0+1
	      end do
              if(n0.eq.20) ierr=2
            end if
            if(ierr.eq.2) goto 193
            if(ierr.ne.0) goto 192
            if(ident(1).ne.iprodu) then
              nfread(nf)=nfread(nf)-1
              nread(nf)=nxread
              iprod(nf)=ident(1)
              goto 180
            end if
c..evt. ekstra geometri-identifikasjon etter felt-data
	    lgeom=0
	    igtype=ident(9)
	    if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
	    nx=ident(10)
	    ny=ident(11)
            ldata=nx*ny+lgeom
            call rdata(ifilin,ipcfile,ifelt,ldata,nread(nf),ios,ierr)
            if(ierr.ne.0) goto 192
          elseif(kpack(1,nf).eq.2) then
            nfread(nf)=nfread(nf)+1
            nxread=nread(nf)
            call rdata(ifilin,ipcfile,identp,24,nread(nf),ios,ierr)
c.....identp(1-24)=0 => 'end_of_file' (dummy bakerst)
            if(ierr.eq.0) then
              n0=0
              do i=1,24
                if(identp(i).eq.0) n0=n0+1
	      end do
              if(n0.eq.24) ierr=2
            end if
            if(ierr.eq.2) goto 193
            if(ierr.ne.0) goto 192
            if(identp(1).ne.iprodu) then
              nfread(nf)=nfread(nf)-1
              nread(nf)=nxread
              iprod(nf)=identp(1)
              goto 180
            end if
            ldata=identp(24)*ipunit
            call rdata(ifilin,ipcfile,ipack,ldata,nread(nf),ios,ierr)
            if(ierr.ne.0) goto 192
            lfmax=maxij
            call funpk2(ldata,identp,ipack,ipunit,
     +			lfmax,ident,ifelt,iok)
            if(iok.ne.1) goto 140
          else
            write(6,*) ' *** ukjent pakke-metode: ',kpack(1,nf)
            goto 140
          end if
c
          nfound=nfound+1
c
c..evt. ekstra geometri-identifikasjon etter felt-data
	  lgeom=0
	  igtype=ident(9)
	  if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
c
	  nx=ident(10)
	  ny=ident(11)
          length=20+nx*ny+lgeom
c
	  if(iseqout.ne.0) then
c..sequential output file
	    write(ifilot,iostat=ios,err=420) (ident(i),i=1,20)
	    write(ifilot,iostat=ios,err=420) (ident(i),i=21,length)
	    nwrite=nwrite+1
	    goto 140
	  end if
c
          nino=nino+1
          innho( 1,nino)=ident( 1)
          innho( 2,nino)=ident( 2)
          innho( 3,nino)=ident(12)
          innho( 4,nino)=ident(13)
          innho( 5,nino)=ident(14)
          innho( 6,nino)=-32767
          innho( 7,nino)=-32767
          innho( 8,nino)=-32767
          innho( 9,nino)=ident( 3)
          innho(10,nino)=ident( 4)
          innho(11,nino)=ident( 5)
          innho(12,nino)=ident( 6)
          innho(13,nino)=ident( 7)
          innho(14,nino)=ident( 8)
          innho(15,nino)=ident( 9)
          innho(16,nino)=0
c
	  if(nogap.eq.0) then
c..standard felt file,
c..write with data gaps (each field starts in first word of a record)
	    irec1=nrecd+1
	    iword=1
            i2=0
            do i1=1,length,lenrec
              i2=min(i1+lenrec-1,length)
              nrecd=nrecd+1
              write(ifilot,rec=nrecd,iostat=ios,err=420)
     +                                      (ident(i),i=i1,i2)
	    end do
	  else
c..standard felt file,
c..write without data gaps
	    iword=lword+1
	    irec1=nrecd+1
	    if(iword.gt.lenrec) iword=1
	    i2=0
	    if(nleft.gt.0 .and. nleft+length.ge.lenrec) then
	      i2=lenrec-nleft
	      nrecd=nrecd+1
              write(ifilot,rec=nrecd,iostat=ios,err=420)
     +                     (ileft(i),i=1,nleft),(ident(i),i=1,i2)
	      nleft=0
	    end if
	    do while (length-i2.ge.lenrec)
	      i1=i2+1
	      i2=i2+lenrec
	      nrecd=nrecd+1
              write(ifilot,rec=nrecd,iostat=ios,err=420)
     +                     		    (ident(i),i=i1,i2)
	    end do
	    lword=lenrec
	    if(i2.lt.length) then
	      nleft=length-i2
	      do i=1,nleft
		ileft(i)=ident(i2+i)
	      end do
	      lword=nleft
	    end if
	  end if
c
	  nwrite=nwrite+1
c
	  irecd2=(irec1-1)/32767
	  irecd1=irec1-irecd2*32767
	  lfelt2=(length-1)/32767
	  lfelt1=length-lfelt2*32767
          innho( 6,nino)=irecd1
          innho( 7,nino)=iword
          innho( 8,nino)=lfelt1
          innho(16,nino)=lfelt2*100+irecd2
c
          ireco1(100+iprodu)=ireco1(100+iprodu)+1
c
          if(nino.eq.ninrec) then
            irec=ireco1(9)+1
            write(ifilot,rec=irec,iostat=ios,err=420) innho
            ireco1(9)=irec
            ninnh=ninnh+nino
            ireco1(11)=ninnh
            nino=0
          end if
c
  140   continue
c
c
  180   ninf(nf)=ninf(nf)-nfound
        close(ifilin)
	if(iseqout.ne.0) goto 110
        goto 195
c
  190   write(6,*) ' ** open error. input file.  iostat: ',ios
        write(6,*) ' ** ',filin(nf)(1:lenstr(filin(nf),1))
ccc     close(ifilin)
        ninf(nf)=0
        goto 110
c
  192   write(6,*) ' ** read error. input file.  iostat: ',ios
        close(ifilin)
        ninf(nf)=0
        goto 195
c
c..end_of_file
  193   close(ifilin)
        ninf(nf)=0
ccc     goto 195
c
  195   continue
c
	if(nogap.ne.0 .and. nleft.gt.0) then
	  nrecd=nrecd+1
          write(ifilot,rec=nrecd,iostat=ios,err=420)
     +                                      (ileft(i),i=1,nleft)
        end if
        lreci=ireco1(9)
        linnh=ninnh
        if(nino.gt.0) then
          do n=nino+1,ninrec
            do i=1,16
	      innho(i,n)=-1
	    end do
	  end do
          irec=ireco1(9)+1
          write(ifilot,rec=irec,iostat=ios,err=420) innho
          ireco1(9)=irec
          ninnh=ninnh+nino
          ireco1(11)=ninnh
        end if
c
	nrecd2=(nrecd-1)/32767
	nrecd1=nrecd-nrecd2*32767
	ireco1( 8)=nrecd1
	ireco1(10)=nwrite
	ireco1(12)=nrecd2
	ireco1(13)=lword
c
c..oppdaterer record 2 og 1 (1 sist er sikrest)
        irec=2
        write(ifilot,rec=irec,iostat=ios,err=420) ireco2
        irec=1
        write(ifilot,rec=irec,iostat=ios,err=420) ireco1
c
        llreci=ireco1(9)
c
c..reset tellere (evt. midlertidig oppdatert over)
        ireco1(9)=lreci
        ninnh=linnh
	if(nogap.ne.0 .and. nleft.gt.0) nrecd=nrecd-1
c
  110 continue
c
  400 continue
      if(iseqout.ne.0) then
c++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        write(6,*)
        write(6,*) 'output file: ',filout(1:lenstr(filout,1))
        write(6,*) 'antall felt: ',nwrite
c++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	goto 990
      end if
c
      ireco1(9)=llreci
c
c++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      write(6,*)
      write(6,*) 'output file: ',filout(1:lenstr(filout,1))
      write(6,1054) itimeo(3),itimeo(2),itimeo(1),itimeo(4)
      write(6,1055) ireco1(10)
      write(6,1056) (ireco1(i),i=1,11)
 1054 format(1x,'termin:       ',i2.2,':',i2.2,':',i4.4,1x,i4.4,' utc')
 1055 format(1x,'antall felt:  ',i5)
 1056 format(1x,'record 1: ',11i6)
      write(6,*)
c++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      goto 990
c
  410 write(6,*) ' ** open error. output felt file.  iostat= ',ios
      write(6,*) ' ** ',filout(1:lenstr(filout,1))
ccc   stop 3
      call exit(3)
c
  420 write(6,*) ' ** write error. output felt file.  iostat= ',ios
      write(6,*) ' ** ',filout(1:lenstr(filout,1))
      close(ifilot)
ccc   stop 3
      call exit(3)
c
  990 continue
      close(ifilot)
c
      end
c
c**********************************************************************
c
      subroutine funpk2(ldata,identp,ipack,ipunit,
     +                  lfmax,ident,ifelt,iok)
c
c        utpakking av felt-data (type 10,11,12,13,14,15,16,17)
c
c-----------------------------------------------------------------------
c  DNMI/FoU   1989-1996  Anstein Foss
c  DNMI/FoU  15.09.1997  Anstein Foss ... type 15,16,17 (and updates)
c-----------------------------------------------------------------------
c
      integer   ldata,ipunit,lfmax,iok
      integer*2 identp(24),ipack(ldata)
      integer*2 ident(20),ifelt(lfmax)
c
      common/cpack2/inclim(16),konpk1(15),konpk2(4,32,15),konpk3(16,15)
cpc   integer*2 inclim,konpk1,konpk2,konpk3
      integer*4 inclim,konpk1,konpk2,konpk3
c
cpc   integer*2 iundef,iuzero,imult,nshft,nand
      integer*4 iundef,iuzero,imult,nshft,nand,ifelt4,ipack4
      integer*2 iunpk(16)
c
      data ncall/0/
c
c-----------------------------------------------------------------------
c     pakking (fpack1/funpk1, type 10,11,12,13,14):
c     pakking (fpack2/funpk2, type 10,11,12,13,14,15,16,17):
c     ------------------------------------------------------
c
c-10: 20 ord felt-identifikasjon
c-    10
c-     0
c-     0
c-    lengde data (16 bits ord)
c     nx*ny ord upakket
c
c-11: 20 ord felt-identifikasjon
c-    11
c-     0
c-     0
c-    lengde data (16 bits ord)
c     nbits ....................................)
c     incbas ...................................)
c     ifirst ...................................) * ny
c     ((nx-1)*nbits+15)/16 ord pakket data .....)
c
c-12: 20 ord felt-identifikasjon
c-    12
c-     0
c-    iundef
c-    lengde data (16 bits ord)
c     nbits (iudef=2**nbits-1) .................)
c     incbas ...................................)
c     ifirst ...................................) * ny
c     (nx*nbits+15)/16 ord pakket data .........)
c
c-13: 20 ord felt-identifikasjon
c-    13
c-    imult
c-     0
c-    lengde data (16 bits ord)
c     nbits ....................................)
c     incbas ...................................)
c     ifirst ...................................) * ny
c     ((nx-1)*nbits+15)/16 ord pakket data .....)
c
c-14: 20 ord felt-identifikasjon
c-    14
c-    imult
c-    iundef
c-    lengde data (16 bits ord)
c     nbits (iudef=2**nbits-1) .................)
c     incbas ...................................)
c     ifirst ...................................) * ny
c     (nx*nbits+15)/16 ord pakket data .........)
c
c-15: 20 ord felt-identifikasjon
c-    15 (oppsett som type 10)
c-     0
c-    iundef
c-    lengde data og bitmatrise (16 bits ord)
c     (nx*ny+15)/16 ord bitmatrise (0=udefinert verdi)
c     nx*ny-(antall udefinerte verdier) ord upakket
c
c-16: 20 ord felt-identifikasjon
c-    16 (oppsett som type 12)
c-     0
c-    iundef
c-    lengde data og bitmatrise (16 bits ord)
c     (nx*ny+15)/16 ord bitmatrise (0=udefinert verdi)
c     nbits ....................................)
c     incbas ...................................)
c     ifirst ...................................) * ny ... ikke udef
c     (nx*nbits+15)/16 ord pakket data .........)
c
c-17: 20 ord felt-identifikasjon
c-    17 (oppsett som type 14)
c-    imult
c-    iundef
c-    lengde data og bitmatrise (16 bits ord)
c     (nx*ny+15)/16 ord bitmatrise (0=udefinert verdi)
c     nbits ....................................)
c     incbas ...................................)
c     ifirst ...................................) * ny ... ikke udef
c     (nx*nbits+15)/16 ord pakket data .........)
c
c     nb! gjelder for nbits=1-15.
c         nbits= 0: bare 'ifirst' etter 'nbits' (for hver j)
c         nbits=16: nx ord upakket data etter 'nbits' (for hver j)
c
c-----------------------------------------------------------------------
c
      npak=0
      lpak=ldata
      iok=0
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
      j=0
      ny=0
      lbits=0
      nbits=0
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
      if(ncall.eq.0) then
c..setup
        call sunpk1(0)
        ncall=1
      end if
c
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c     write(6,6001) identp
c6001 format(' funpk2. ',/,4x,12i6,/,4x,12i6)
c     write(6,*) '   ldata: ',ldata
c     write(6,*) '   ipack: ',(ipack(i),i=1,5)
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
c..20 ord identifikasjon upakket
      do i=1,20
        ident(i)=identp(i)
      end do
c
c..evt. ekstra geometri-identifikasjon etter felt-data
      lgeom=0
      igtype=ident(9)
      if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
c
      itype=identp(21)
      nundef=0
      imult=1
      nopack=0
      ibmap=0
      if(itype.eq.10) then
        nopack=1
      elseif(itype.eq.11) then
	continue
      elseif(itype.eq.12) then
        nundef=1
        iundef=identp(23)
      elseif(itype.eq.13) then
        imult=identp(22)
      elseif(itype.eq.14) then
        imult=identp(22)
        nundef=1
        iundef=identp(23)
      elseif(itype.eq.15) then
        iundef=identp(23)
        nopack=1
        ibmap=1
      elseif(itype.eq.16) then
        iundef=identp(23)
        ibmap=1
      elseif(itype.eq.17) then
        imult=identp(22)
        iundef=identp(23)
        ibmap=1
      else
        write(6,*) ' **** funpk2 ** ukjent pakke-type: ',itype
        return
      end if
c
      nx=ident(10)
      ny=ident(11)
      nxny=nx*ny
      if(nxny+lgeom.gt.lfmax) then
        write(6,*) ' **** funpk2 ****'
        write(6,*) ' **** felt for stort.  grid-nr: ',ident(2)
        write(6,*) ' **** lengde,max lengde("maxij"): ',
     +             nxny+lgeom,lfmax
        return
      end if
c
      npak=0
      iuzero=iundef
      if(ibmap.eq.1) then
        npak=(nxny+15)/16
        if(iundef.eq.0) iundef=-32767
      end if
c
      do i1=1,nxny,nx
c
        i2=i1+nx-1
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
        j=j+1
        lbits=nbits
c+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c
        nud=0
        if(ibmap.eq.1) then
          do i=i1,i2
            iw=(i+15)/16
            ib=i-(iw-1)*16-1
            if(btest(ipack(iw),ib)) then
              ifelt(i)=0
            else
              ifelt(i)=iundef
              nud=nud+1
            end if
          end do
        end if
c
        if(nopack.eq.1) then
c
c..upakket data
          if(npak+nx-nud.gt.ldata) goto 910
          if(ibmap.eq.0) then
            do i=i1,i2
              npak=npak+1
              ifelt(i)=ipack(npak)
            end do
          else
            do i=i1,i2
              if(ifelt(i).eq.0) then
                npak=npak+1
                ifelt(i)=ipack(npak)
              end if
            end do
          end if
c
        else
c
          npak=npak+1
          if(npak.gt.lpak) goto 910
          nbits=ipack(npak)
          if(nbits.lt.0 .or. nbits.gt.16) goto 920
c
          if(nbits.eq.0) then
c
c..alle verdier like for denne j
            if(npak+1.gt.lpak) goto 910
            npak=npak+1
            ifirst=ipack(npak)
            if(ibmap.eq.0) then
              do i=i1,i2
                ifelt(i)=ifirst
              end do
            else
              do i=i1,i2
                if(ifelt(i).eq.0) then
                  ifelt(i)=ifirst
                end if
              end do
            end if
c
          elseif(nbits.eq.16) then
c
c..upakket data for denne j
            if(npak+nx-nud.gt.lpak) goto 910
            if(ibmap.eq.0) then
              do i=i1,i2
                npak=npak+1
                ifelt(i)=ipack(npak)
              end do
            else
              do i=i1,i2
                if(ifelt(i).eq.0) then
                  npak=npak+1
                  ifelt(i)=ipack(npak)
                end if
              end do
            end if
c
          else
c
c..data pakket (nbits=1-15)
            ibgn=i1+1-nundef
            iend=i2-nud
            imap=i1
            length=((iend-ibgn+1)*nbits+15)/16
            if(npak+2+length.gt.lpak) goto 910
            incbas=ipack(npak+1)
            ifirst=ipack(npak+2)
            npak=npak+2
            if(konpk1(nbits).eq.0) call sunpk1(nbits)
            do ix=ibgn,iend,16
              i0=ix-1
              il=min(i0+16,iend)
              ni=il-i0
              do i=1,ni
                iunpk(i)=0
              end do
              npk=konpk3(ni,nbits)
              do ipk=1,npk
                i    =konpk2(1,ipk,nbits)
                n    =konpk2(2,ipk,nbits)+npak
                nshft=konpk2(3,ipk,nbits)
                nand =konpk2(4,ipk,nbits)
cpc             iunpk(i)=ior(ifelt(i),
cpc  +                       iand(ishft(ipack(n),nshft),nand))
c----------------------------------------------------------------------
                ifelt4=iunpk(i)
                ipack4=ipack(n)
                iunpk(i)=ior(ifelt4,iand(ishft(ipack4,nshft),nand))
c----------------------------------------------------------------------
              end do
              npak=npak+konpk2(2,npk,nbits)
              if(ibmap.eq.0) then
                do i=1,ni
                  ifelt(i0+i)=iunpk(i)
                end do
              else
                i=0
		if(imap.eq.i1) then
		  do while (ifelt(imap).ne.0)
		    imap=imap+1
		  end do
		end if
                do while (i.lt.ni)
                  imap=imap+1
                  if(ifelt(imap).eq.0) then
                    i=i+1
                    ifelt(imap)=iunpk(i)
                  end if
                end do
              end if
            end do
c
            if(nundef.eq.0 .and. ibmap.eq.0) then
              last=ifirst
              ifelt(i1)=ifirst
              do i=i1+1,i2
                ifelt(i)=ifelt(i)+last+incbas
                last=ifelt(i)
              end do
            elseif(ibmap.eq.0) then
              iudef=inclim(nbits)
              last=ifirst
              do i=i1,i2
                if(ifelt(i).ne.iudef) then
                  ifelt(i)=ifelt(i)+incbas+last
                  last=ifelt(i)
                else
                  ifelt(i)=iundef
                end if
              end do
            else
c..bitmap
              last=ifirst
	      nf=0
              do i=i1,i2
                if(ifelt(i).ne.iundef) then
		  if(nf.eq.1) then
                    ifelt(i)=ifelt(i)+incbas+last
                    last=ifelt(i)
		  else
                    ifelt(i)=last
                    nf=1
		  end if
                end if
              end do
            end if
c
          end if
c
        end if
c
c.....end do i1=1,nxny,nx
      end do
c
      if(imult.gt.1 .and. nundef.eq.0 .and. ibmap.eq.0) then
        do i=1,nxny
          ifelt(i)=imult*ifelt(i)
        end do
      elseif(imult.gt.1) then
        do i=1,nxny
          if(ifelt(i).ne.iundef) ifelt(i)=imult*ifelt(i)
        end do
      end if
c
      if(ibmap.eq.1 .and. iuzero.ne.iundef) then
        do i=1,nxny
          if(ifelt(i).eq.iundef) ifelt(i)=iuzero
        end do
      end if
c
      if(lgeom.gt.0) then
c..ekstra geometri-identifikasjon etter felt-data
        do i=1,lgeom
          ifelt(nxny+i)=ipack(npak+i)
        end do
        npak=npak+lgeom
      end if
c
c..zero padding of 'long' records (n*ipunit words, ipunit=1,2,3,...)
      if(npak.lt.lpak .and. npak+ipunit-1.ge.lpak) npak=lpak
c
      if(npak.ne.lpak) then
        write(6,*) ' ***********************************************'
        write(6,*) ' ** feil i funpk2 ***** felt akseptert *********'
        write(6,*) ' ** npak,lpak,ldata:  ',npak,lpak,ldata
        write(6,*) ' ***********************************************'
      end if
c
      iok=1
      return
c
  910 write(6,*) ' **************************************************'
      write(6,*) ' ** feil i funpk2 *********************************'
      write(6,*) ' ** npak,lpak,ldata:  ',npak,lpak,ldata
      write(6,*) ' ** j,ny,lbits,nbits: ',j,ny,lbits,nbits
      write(6,*) ' **************************************************'
      iok=0
      return
c
  920 write(6,*) ' **************************************************'
      write(6,*) ' ** feil i funpk2 *********************************'
      write(6,*) ' ** nbits: ',nbits
      write(6,*) ' ** j,ny,lbits,nbits: ',j,ny,lbits,nbits
      write(6,*) ' **************************************************'
      iok=0
      return
c
      end
c
c***********************************************************************
c
      subroutine sunpk1(isetup)
c
c        setup for 'funpk1/funpk2'
c
c        tabeller for utpakking av 16 16-bits tall.
c        tallene som er pakket er inkrementer (fra tallet foran).
c        16 tall pakkes alltid inn i helt antall 16-bits ord.
c
      common/cpack2/inclim(16),konpk1(15),konpk2(4,32,15),konpk3(16,15)
cpc   integer*2 inclim,konpk1,konpk2,konpk3
      integer*4 inclim,konpk1,konpk2,konpk3
c
cpc   integer*2 nb,ib,ib1,ib2,iset
      integer*4 nb,ib,ib1,ib2,iset
c
      if(isetup.lt.1 .or. isetup.gt.15) then
c
	do nbits=1,15
	  maxnum=2**nbits-1
	  inclim(nbits)=maxnum
	  konpk1(nbits)=0
	end do
	nbits=16
	inclim(nbits)=32767
c
	return
c
      end if
c
      nbits=isetup
      nb=nbits-1
c
c        konpk2(1,n,nbits): rel. ord nr. output
c              (2,n,nbits): rel. ord nr. input
c              (3,n,nbits): for 'ishft'
c              (4,n,nbits): for 'iand'
c
      npk=0
      nin=0
      ib2=0
      do nout=1,16
        if(ib2.eq.0) then
          nin=nin+1
          ib2=16
        end if
        ib1=ib2-1
        ib2=ib2-nbits
        if(ib2.ge.0) then
          iset=0
          do ib=0,nb
            iset=ibset(iset,ib)
          end do
          npk=npk+1
          konpk2(1,npk,nbits)=nout
          konpk2(2,npk,nbits)=nin
          konpk2(3,npk,nbits)=-ib2
          konpk2(4,npk,nbits)=iset
        else
          iset=0
          do ib=-ib2,nb
            iset=ibset(iset,ib)
          end do
          npk=npk+1
          konpk2(1,npk,nbits)=nout
          konpk2(2,npk,nbits)=nin
          konpk2(3,npk,nbits)=-ib2
          konpk2(4,npk,nbits)=iset
          nin=nin+1
          ib1=-ib2-1
          ib2=ib2+16
          iset=0
          do ib=0,ib1
            iset=ibset(iset,ib)
          end do
          npk=npk+1
          konpk2(1,npk,nbits)=nout
          konpk2(2,npk,nbits)=nin
          konpk2(3,npk,nbits)=-ib2
          konpk2(4,npk,nbits)=iset
        end if
        konpk3(nout,nbits)=npk
      end do
c
      konpk1(nbits)=npk
c
      return
      end
c
c**********************************************************************
c
      subroutine rdata(ifilin,ipcfile,idata,ldata,nread,ios,ierr)
c
c..lesing av file 'uten' record-struktur (access=direct)
c
      parameter (lrecf=1024)
c
      common/buff/nbuff,ibuff(lrecf)
      integer   nbuff
      integer*2 ibuff
c
      integer   ifilin,ipcfile,ldata,nread,ios,ierr
      integer*2 idfile
      integer*2 idata(ldata)
c
      integer   lfilin
      logical   swap
c
      data      lfilin/-1/
      data      swap/.false./
c
      i2=0
c
      if(nread.eq.0) then
        nread=nread+1
        read(ifilin,rec=nread,iostat=ios,err=910) ibuff
        idfile=ibuff(1)
        if(idfile.ne.101) call bswap2(1,idfile)
        if(idfile.ne.101) then
          write(6,*) ' ***rdata*** not a packed field file'
          ios=0
          ierr=1
          return
        end if
        swap= (idfile.ne.ibuff(1))
        nbuff=0
        lfilin=ifilin
      elseif(ifilin.ne.lfilin) then
        write(6,*) ' ***rdata*** continue on wrong file'
        ios=0
        ierr=1
        return
      end if
c
      nrec=(nbuff+ldata+lrecf-1)/lrecf
c
      do irec=1,nrec
        if(nbuff.eq.lrecf) then
          nread=nread+1
          read(ifilin,rec=nread,iostat=ios,err=910) ibuff
          nbuff=0
        end if
        i1=i2+1
        i2=min(i2+lrecf-nbuff,ldata)
        do i=i1,i2
          nbuff=nbuff+1
          idata(i)=ibuff(nbuff)
	end do
      end do
c
      if(swap) call bswap2(ldata,idata)
c
      ios=0
      ierr=0
      return
c
  910 ierr=1
c
      if(ipcfile.eq.1) then
c..the last record on a PC type file is usually not a complete record
c..(the last record may also be the first)
c..read as much as possible (as fast as possible)
	j1=0
	j2=lrecf
	do while (j1.lt.j2)
	  jm=(j1+j2+1)/2
          read(ifilin,rec=nread,iostat=ios) (ibuff(i),i=1,jm)
	  if(ios.eq.0) then
	    j1=jm
	  else
	    j2=jm-1
	  end if
	end do
	if(j1.ge.ldata-i2) then
          read(ifilin,rec=nread,iostat=ios) (ibuff(i),i=1,j1)
	  do i=j1+1,lrecf
	    ibuff(i)=0
	  end do
	  nbuff=ldata-i2
	  do i=1,nbuff
	    idata(i2+i)=ibuff(i)
	  end do
	  if(swap) call bswap2(ldata,idata)
	  ierr=0
	else
	  ios=-999
	end if
      end if
c
      return
      end
