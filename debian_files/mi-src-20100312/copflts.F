c***********************************************************************
c
c copflts.f - subroutines for copflt.f
c
c***********************************************************************
c
      subroutine fcopy(iunit,iunitc,nloops,newinh,iformt,iswapb,
     *                 nogap,irescale,ipkversion,mpksize,iupgap)
c
c  create and copy fields
c
c----------------------------------------------------------------
c
      include 'copflt.inc'
c
c..note: array(maxdim+1) for nice overflow testing
c
      common/a/ntim(2,maxset),nlev(2,maxset),npar(2,maxset),
     *         itimef(5),igridi(3,mloops),
     *         ictime(mloops),info(mloops),numset(2,mloops),
     *         igridc(mloops),igrido(7,mloops),gridot(6,mloops),
     *         iaddpa(5,maddpa),icontrol(3),
     *         nchang,nchnga,interp,nintpm,nxyvec,ncompu,naddpa,naddpt,
     *         nacspa,
     *         idrec1(mwrec),idrec2(2,mwrech),idata(limit+mwrec,4),
     *         istore(2,mstore),ichang(3,mchang),ichnga(4,2,mchnga),
     *         iintpm(2,mintpm+1),ixvec(mxyvec+1),iyvec(mxyvec+1),
     *         icompu(2,mcompu),isigma(4),iaddpt(maddpt),
     *         iacspa(3,macspa),
     *         id19ctrl(2,mloops),
     *         comput(mcompu),
     *         fileot,filein(mloops),filnam
      integer*2 idrec1,idrec2,idata
      integer*2 istore,ichang,iintpm,ixvec,iyvec
      integer*2 icompu,isigma,iaddpt,iacspa,id19ctrl
      character*40 comput
      character*256 fileot,filein,filnam
c
      common/b/iarkiv,nodatind,nnocppar,nnocplev,nnocpvco,
     *         ktimea(3),itimea(5,3),itimefa(4,3,mloops),
     *         nocppar(maxncp+1),nocplev(maxncp+1),nocpvco(maxncp+1),
     *         ismooth,nsmooth,npsmooth,npasmpsc,
     *         ipsmooth(mpsmooth+1,3),ipasmpsc(2,mpasmpsc+1)
      integer iarkiv,nodatind,nnocppar,nnocplev,nnocpvco
      integer ktimea,itimea,itimefa
      integer nocppar,nocplev,nocpvco
      integer ismooth,nsmooth,npsmooth,npasmpsc,ipsmooth,ipasmpsc
c
      common/p/npkscl,npkdir,npknot,
     *         ipkscl(6,mpkscl),ipkdir(mpkdir+1),ipknot(mpknot+1),
     *         ihead(16),idpack(24),ipack(limit),ipackw(limit)
      integer*2 ipkscl,ipkdir,ipknot
      integer*2 ihead,idpack,ipack,ipackw
c
      real      field(maxij,3)
c
      integer   itimeo(5),ihelpr(6),ierr(3)
      integer*2 iterm(3),idfile(32),ireadf(6,mreadf),iparam
      integer*2 innh(16,minrec),in(16)
      integer*2 idprv(20),idprvi(20),idprvo(20)
      integer*2 ibuffw(mwrec)
c..rfturbo.........................................................
      integer*2 idrec1r(mwrec),idrec2r(mwrec),innhrf(16,minrec)
c..rfturbo.........................................................
      real      undef
c
      logical   swapfile
c
      data undef/+1.e+35/
      data ihelpr/6*0/
c
c----------------------------------------------------------------------
c  packed file:
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
c             (13)= max. field length (unpacked, 16 bit words)
c             (14)= ipunit, length unit for idpack(24), in words
c             (15)= iupgap, 0=unpack with data gaps  1=without data gaps
c             (16)= 0  (not used)
c----------------------------------------------------------------------
c
      call rlunit(lrunit)
c
      nwrec =mwrec
      nwrech=mwrech
      ninrec=minrec
c-nord--------------------------------------------------------
      if(iformt.eq.1) then
        nwrec =mwrec/4
        nwrech=mwrech/4
        ninrec=minrec/4
      end if
c-nord--------------------------------------------------------
c
      iwhead=0
c
      if(iarkiv.eq.0) then
        do i=1,5
          itimeo(i)=itimef(i)
        end do
        ntstep=1
      else
c..find all date/time's in input file(s)
        filnam=filein(1)
        do iloop=2,nloops
          if(filnam.ne.filein(iloop)) then
            filnam=filein(iloop)
            write(6,*) 'check (archive) time range on file:'
            write(6,*) filnam(1:lenstr(filnam,1))
            open(iunitc,file=filnam,
     *                  form='unformatted',access='direct',
     *                  recl=2048/lrunit,
     *                  status='old',iostat=ios)
            if(ios.eq.0) read(iunitc,rec=1,iostat=ios) idfile
            if(ios.eq.0 .and. swapfile(-iunitc)) call bswap2(32,idfile)
            close(iunitc)
            if(ios.ne.0) then
              write(6,*) 'open/read error. iostat= ',ios
              filnam='*'
              itimefa(1,1,iloop)=-99999
              itimefa(1,2,iloop)=-99999
              itimefa(1,3,iloop)=-99999
            elseif(idfile(1).eq.997 .or. idfile(2).eq.998) then
c..archive or cyclic_archive input file
c..get first and last date/time
              itimefa(1,1,iloop)=idfile(20)
              itimefa(2,1,iloop)=idfile(21)/100
              itimefa(3,1,iloop)=idfile(21)-(idfile(21)/100)*100
              itimefa(4,1,iloop)=idfile(22)/100
              itimefa(1,2,iloop)=idfile(23)
              itimefa(2,2,iloop)=idfile(24)/100
              itimefa(3,2,iloop)=idfile(24)-(idfile(24)/100)*100
              itimefa(4,2,iloop)=idfile(25)/100
              do i=1,4
                itimefa(i,3,iloop)=0
                if(idfile(29).eq.i) itimefa(i,3,iloop)=idfile(30)
              end do
            else
c..standard felt file, one date/time
              itimefa(1,1,iloop)=idfile(5)
              itimefa(2,1,iloop)=idfile(6)/100
              itimefa(3,1,iloop)=idfile(6)-(idfile(6)/100)*100
              itimefa(4,1,iloop)=idfile(7)/100
              do i=1,4
                itimefa(i,2,iloop)=itimefa(i,1,iloop)
                itimefa(i,3,iloop)=0
              end do
              itimefa(4,3,iloop)=1
            end if
          else
            itimefa(1,1,iloop)=-99999
            itimefa(1,2,iloop)=-99999
            itimefa(1,3,iloop)=-99999
          end if
        end do
        filnam='*'
        do n=1,2
c..find first and last date/time on the file(s)
          if(ktimea(n).eq.0) then
            iltgt=-1
            if(n.eq.2) iltgt=+1
            do i=1,4
              itimea(i,n)=itimefa(i,n,1)
            end do
            do iloop=2,nloops
              if(itimefa(1,n,iloop).ne.-99999) then
                ltgt=0
                do i=1,4
                  if(ltgt.eq.0 .and.
     *               itimefa(i,n,iloop).lt.itimea(i,n)) ltgt=-1
                  if(ltgt.eq.0 .and.
     *               itimefa(i,n,iloop).gt.itimea(i,n)) ltgt=+1
                end do
                if(ltgt.eq.iltgt) then
                  do i=1,4
                    itimea(i,n)=itimefa(i,n,iloop)
                  end do
                end if
              end if
            end do
          end if
        end do
        if(ktimea(3).eq.0) then
c..find (minimum) time reolution
          ktresol=1
          itresol=32767
          do iloop=1,nloops
            if(itimefa(1,3,iloop).ne.-99999) then
              k=0
              do i=1,4
                if(itimefa(i,3,iloop).gt.0) k=i
              end do
              if(k.gt.ktresol) then
                ktresol=k
                itresol=itimefa(k,3,iloop)
              elseif(k.eq.ktresol .and.
     *               itimefa(k,3,iloop).lt.itresol) then
                itresol=itimefa(k,3,iloop)
              end if
            end if
          end do
          do i=1,4
            itimea(i,3)=0
          end do
          itimea(ktresol,3)=itresol
        else
          do i=1,4
            if(itimea(i,3).gt.0) then
              ktresol=i
              itresol=itimea(i,3)
            end if
          end do
        end if
c..find no. of (archive) timesteps
        itimea(5,1)=0
        itimea(5,2)=0
        itimea(5,3)=0
        if(ktresol.le.2) then
          iy=itimea(1,1)
          im=itimea(2,1)
          ntstep=0
          k=0
          do while (k.eq.0)
            ntstep=ntstep+1
            iy=iy+itimea(1,3)
            im=im+itimea(2,3)
            do while (im.gt.12)
              im=im-12
              iy=iy+1
            end do
            if(iy.gt.itimea(1,2)) then
              k=1
            elseif(iy.eq.itimea(1,2) .and. im.gt.itimea(2,2)) then
              k=1
            end if
          end do
        else
          ihstep=itimea(3,3)*24+itimea(4,3)
          call hrdiff(0,0,itimea(1,1),itimea(1,2),ihours,ierr1,ierr2)
          ntstep=ihours/ihstep +1
        end if
        if(ntstep.lt.1) ntstep=1
        if(ntstep*newinh.gt.32767) goto 950
        do i=1,5
          itimeo(i)=itimea(i,1)
        end do
      end if
c
      filnam='*'
      nchfld=0
c
      do i=1,nwrec
        idrec1(i)=0
      end do
      do i=1,nwrech
        idrec2(1,i)=0
        idrec2(2,i)=0
      end do
c
      if(iformt.eq.0 .or. iformt.eq.1) then
c
c..vanlig felt-file (1 dato/termin)
        idrec1(1)=999
c
c..normal arkiv felt-file (flere datoer/terminer)
        if(iarkiv.eq.1) idrec1(1)=998
c
c..sirkulaer arkiv felt-file (flere datoer/terminer)
        if(iarkiv.eq.2) idrec1(1)=997
c
        if(iswapb.eq.1) call bswap2(1,idrec1)
c
c..skriver record 1 og 2   ("blank")
        irec=1
        write(iunit,rec=irec,err=910,iostat=ios) (idrec1(n),n=1,nwrec)
        irec=2
        write(iunit,rec=irec,err=910,iostat=ios)
     *                                 ((idrec2(i,n),i=1,2),n=1,nwrech)
c
        if(iswapb.eq.1) call bswap2(1,idrec1)
c
        nrec=2+(newinh*ntstep+ninrec-1)/ninrec
        idrec1( 8)=nrec
        idrec1( 9)=nrec
        idrec1(10)=0
        idrec1(11)=newinh*ntstep
        idrec1(12)=0
        idrec1(13)=nwrec
        idrec1(14)=nogap
c..set update flag
        idrec1(15)=1
c
      elseif(iformt.eq.2 .or. iformt.eq.3) then
c..find no. of producers involved
        nprod=0
        lprod=0
        do iloop=1,nloops
          if(igridi(1,iloop).ne.lprod) then
            lprod=igridi(1,iloop)
            nprod=nprod+1
          end if
        end do
c..get system date/time
        call daytim(iyear,month,iday,ihour,minut,isecn)
c..make packed file header
        ihead( 1)=101
        ihead( 2)=iyear
        ihead( 3)=month*100+iday
        ihead( 4)=ihour*100+minut
        ihead( 5)=itimef(1)
        ihead( 6)=itimef(2)*100+itimef(3)
        ihead( 7)=itimef(4)*100
        ihead( 8)=newinh*ntstep
        ihead( 9)=nprod
        ihead(10)=igridi(1,1)
        ihead(11)=1
        if(iformt.eq.2) ihead(11)=2
        ihead(12)=24
c.old   ihead(13)=maxij
        ihead(13)=32767-20
	if(ipkversion.eq.1) then
c..default (old)
          ihead(14)=0
	  ipunit=1
	else
c..new ....... packed field length in unit ipunit words
	  ipunit=(mpksize+32767-1)/32767
	  ihead(14)=ipunit
	end if
	mpksize=min(32767*ipunit,limit)
        ihead(15)=iupgap
        ihead(16)=0
	if(ipkversion.eq.1) then
c..write packed file header (only for packing version 1, for PC/DOS)
	  if(iswapb.eq.1) call bswap2(16,ihead)
	  call wpkdata(1,iunit,16,ihead,ierror)
	  if(ierror.ne.0) goto 925
	  iwhead=1
	end if
c..for print of file and field data size reduction
        nwrdpk=16
        nwrdup=(2+((newinh*ntstep+ninrec-1)/ninrec))*nwrec
        iwrdpk=0
        iwrdup=0
        jwrdup=0
        nfelt=0
      elseif(iformt.eq.4) then
        nfelt=0
      end if
c
      lprod=0
      ireci=2
      ni=0
c
      nwih=0
c
c..for print of some grid identifications
      do i=1,20
        idprv(i)=0
        idprvi(i)=-32767
        idprvo(i)=-32767
      end do
      idprv( 1)=1
      idprv( 2)=1
      idprv(10)=1
      idprv(11)=1
      idprv(15)=1
      idprv(16)=1
      idprv(17)=1
      idprv(18)=1
c
      notcp=nnocppar+nnocplev+nnocpvco
      ksmooth=0
      if(ismooth.ne.0 .or. npsmooth.gt.0) ksmooth=1
c
c
      do 170 itstep=1,ntstep
c
      if(itstep.gt.1) then
        if(ktresol.le.2) then
          itimeo(1)=itimeo(1)+itimea(1,3)
          itimeo(2)=itimeo(2)+itimea(2,3)
          do while (itimeo(2).gt.12)
            itimeo(2)=itimeo(2)-12
            itimeo(1)=itimeo(1)+1
          end do
        else
          itimeo(5)=itimea(3,3)*24+itimea(4,3)
          call vtime(itimeo,ierror)
        end if
      end if
c
      iterm(1)=itimeo(1)
      iterm(2)=itimeo(2)*100+itimeo(3)
      iterm(3)=itimeo(4)*100
c
      if(iarkiv.ne.0) then
        write(6,
     *   fmt='('' Archive: '',2(i2.2,'':''),i4.4,1x,i2.2,'' utc'')')
     *            itimeo(3),itimeo(2),itimeo(1),itimeo(4)
      end if
c
      do 180 iloop=1,nloops
c
      iut=info(iloop)
      icopy=1
      ingaps=1
c
      if(filnam.ne.filein(iloop)) then
        if(filnam(1:1).ne.'*') close(iunitc)
        ihelpr(1)=0
        filnam=filein(iloop)
        write(6,*) 'read fields from file: ',filnam(1:lenstr(filnam,1))
        open(iunitc,file=filnam,
     *              form='unformatted',access='direct',
     *              recl=2048/lrunit,
     *              status='old',iostat=ios)
        if(ios.eq.0) read(iunitc,rec=1,iostat=ios) idfile
        if(ios.eq.0 .and. swapfile(-iunitc)) call bswap2(32,idfile)
        if(ios.ne.0) then
          write(6,*) 'open/read error. iostat= ',ios,'    no copy.'
          icopy=0
          filnam='*'
        else
          idater=999
          if(ictime(iloop).eq.0) idater=0
          if(idfile(1).eq.998 .or. idfile(1).eq.997) idater=999
          if(iarkiv.ne.0) idater=999
	  if(idfile(14).eq.1) ingaps=0
        end if
      end if
c
      if(igridi(1,iloop).ne.lprod) then
        lprod=igridi(1,iloop)
        idrec2(1,lprod)=ireci+1
        idrec2(2,lprod)=16*ni+1
      end if
c
c..vector rotation (we possibly need both components to make one)
      ivrot=0
      if(igridc(iloop).eq.1 .and. igrido(7,iloop).ne.0) ivrot=1
      if(igridc(iloop).eq.2)                            ivrot=1
      if(igridc(iloop).eq.3 .and. igrido(7,iloop).ne.0) ivrot=1
c
c        "sett"-lokke
      do 190 ks=numset(1,iloop),numset(2,iloop)
c
      if(ntim(1,ks).lt.1 .or. nlev(1,ks).lt.1
     *                   .or. npar(1,ks).lt.1) goto 190
c
      nt1=ntim(2,ks)
      nt2=ntim(2,ks)+ntim(1,ks)-1
      nl1=nlev(2,ks)
      nl2=nlev(2,ks)+nlev(1,ks)-1
      np1=npar(2,ks)
      np2=npar(2,ks)+npar(1,ks)-1
c
c        tids-lokke
      do 200 nt=nt1,nt2
c        nivaa-lokke
      do 210 nl=nl1,nl2
c
      ivectr=0
      iwrite=0
      icompx=0
c
c        parameter-lokke
      do 220 np=np1,np2
c
c..count the no. of requested fields
      icontrol(1)=icontrol(1)+1
      id19err=0
c
      idrec1(100+lprod)=idrec1(100+lprod)+1
      ni=ni+1
c
      innh( 1,ni)=igridi(1,iloop)
      innh( 2,ni)=igridi(2,iloop)
      innh( 3,ni)=iterm(1)
      innh( 4,ni)=iterm(2)
      innh( 5,ni)=iterm(3)
      innh( 6,ni)=-32767
      innh( 7,ni)=-32767
      innh( 8,ni)=-32767
      innh( 9,ni)=istore(1,nt)
      innh(10,ni)=istore(2,nt)
      innh(11,ni)=istore(1,np)
      innh(12,ni)=istore(2,np)
      innh(13,ni)=istore(1,nl)
      innh(14,ni)=istore(2,nl)
      innh(15,ni)=igridi(3,iloop)
      innh(16,ni)=-32767
c
      jcopy=icopy
      if(notcp.gt.0 .and. jcopy.eq.1) then
        do n=1,nnocpvco
          if(innh(11,ni).eq.nocpvco(n)) jcopy=0
        end do
        do n=1,nnocppar
          if(innh(12,ni).eq.nocppar(n)) jcopy=0
        end do
        do n=1,nnocplev
          if(innh(13,ni).eq.nocplev(n)) jcopy=0
        end do
      end if
c
      if(jcopy.eq.0) then
	iwrite=0
	goto 250
      end if
c
      if(ivectr.ne.0) then
        ivectr=0
        iwd=2
        goto 250
      end if
c
      do i=1,16
        in(i)=innh(i,ni)
      end do
c
      nread =1
      ivectr=0
      iwrite=1
      icompx=0
      iaddf =0
      iacsf =0
c
      do i=1,6
        ireadf(i,1)=in(8+i)
      end do
      iparam=in(12)
c
      if(ivrot.ne.0 .and. np.lt.np2) then
c..vector rotation, tested in calling program:
c..always the 'x' parameter before the 'y' parameter
        n=0
        do i=1,nxyvec
          if(ixvec(i).eq.iparam .and. n.eq.0) n=i
        end do
        if(n.gt.0) then
          nread=2
          ivectr=1
          do i=1,6
            ireadf(i,2)=ireadf(i,1)
          end do
          ireadf(4,2)=iyvec(n)
        end if
      end if
c
      if(ncompu.gt.0 .and. nread.eq.1) then
c..check if 'compute' option on this parameter,
c..possibly more fields needed
        do i=1,ncompu
          if(icompu(1,i).eq.iparam .and. icompx.eq.0) icompx=i
        end do
        if(icompx.gt.0 .and. comput(icompx)(1:6).eq.'sigma.') then
          nread=2
          ireadf(1,2)=ireadf(1,1)
          ireadf(2,2)=ireadf(2,1)
          ireadf(3,2)=isigma(1)
          ireadf(4,2)=isigma(2)
          ireadf(5,2)=isigma(3)
          ireadf(6,2)=isigma(4)
        end if
      end if
c
      if(naddpa.gt.0 .and. nread.eq.1) then
c..check if 'add_param/rel.hour' option on this parameter (add fields)
        iaddf=0
        do i=1,naddpa
          if(iaddpa(1,i).eq.iparam .and. iaddf.eq.0) iaddf=i
        end do
        if(iaddf.gt.0) then
          nread=0
c..relative time (forecast length) loop
          do n=iaddpa(4,iaddf),iaddpa(5,iaddf)
c..parameter loop
            do k=iaddpa(2,iaddf),iaddpa(3,iaddf)
              nread=nread+1
              do i=1,6
                ireadf(i,nread)=in(8+i)
              end do
              ireadf(2,nread)=ireadf(2,nread)+iaddpt(n)
              ireadf(4,nread)=iaddpt(k)
            end do
          end do
          do i=1,maxij
            field(i,1)=0.
          end do
        end if
      end if
c
      if(nacspa.gt.0 .and. nread.eq.1) then
c..check if 'tot.accum.to.step' option on this parameter (subtract fields)
c..(at forecast +0 the field is assumed to be 0 and not read)
        iacsf=0
        do i=1,nacspa
          if(iacspa(1,i).eq.iparam .and. iacsf.eq.0) iacsf=i
        end do
        if(iacsf.gt.0) then
	  ireadf(4,1)=iacspa(2,iacsf)
	  if(ireadf(2,1)-iacspa(3,iacsf).gt.0) then
	    nread=2
            do i=1,6
              ireadf(i,2)=ireadf(i,1)
            end do
	    ireadf(2,1)=ireadf(2,1)-iacspa(3,iacsf)
	  end if
	end if
      end if
c
      ird=0
c
      do iread=1,nread
c
        do i=1,6
          in(8+i)=ireadf(i,iread)
        end do
        ird=ird+1
        ierr(1)=0
        ierr(2)=idater
        ierr(3)=0
ccc     call rfelt(iunitc,ip,in,idata(1,ird),limit,ierr,ihelpr)
        call rfturbo(iunitc,ip,in,idata(1,ird),limit,ierr,ihelpr,
     *               idrec1r,idrec2r,innhrf)
        if(ip.ne.1 .and. iut.ne.0) then
          if(idater.eq.0) then
            write(6,1020) ip,ierr,(in(i),i=1,2),(in(i),i=9,14)
 1020       format(' field not found.  ip=',i4,'  ierr=',3i9,
     *             /,6x,'in(1-2,9-14):',8i7)
          else
            write(6,1021) ip,ierr,(in(i),i=1,5),(in(i),i=9,14)
 1021       format(' field not found.  ip=',i4,'  ierr=',3i9,
     *             /,6x,'in(1-5): ',5i7,/,6x,'in(9-14):',6i7)
          end if
        end if
        if(ip.ne.1) then
          iwrite=0
          if(nodatind.eq.1) goto 250
          ni=ni-1
          goto 220
        end if
c
        if(id19ctrl(1,iloop).eq.1 .and.
     *     id19ctrl(2,iloop).ne.idata(19,ird)) id19err=id19err+1
c
        k=0
        do i=1,20
          if(idprv(i).eq.1 .and. idprvi(i).ne.idata(i,ird)) k=1
          idprvi(i)=idata(i,ird)
        end do
        if(k.ne.0) write(6,fmt='(''  input  grid: '',8i7)')
     *                          (idprvi(i),i=1,2),(idprvi(i),i=10,11),
     *                                            (idprvi(i),i=15,18)
c
        if(iaddf.gt.0) then
c..add fields (allow undefined values)
c..(but skipping check of field size and grid parameters)
          nx=idata(10,1)
          ny=idata(11,1)
          iscale=idata(20,1)
          scale=10.**iscale
          ld=nx*ny
          ud=undef*0.9
          do i=1,ld
            if(field(i,1).lt.ud .and. idata(20+i,1).ne.-32767) then
              field(i,1)=field(i,1)+scale*real(idata(20+i,1))
            else
              field(i,1)=undef
            end if
          end do
          ird=0
        end if
c
      end do
c
      if(iaddf.gt.0) then
c..fields are added ... reset time (forecast length) and parameter
        idata(4,1)=innh(10,ni)
        idata(6,1)=innh(12,ni)
        nx=idata(10,1)
        ny=idata(11,1)
        iscale=idata(20,1)
        scale=10.**(-iscale)
        ld=nx*ny
        ud=undef*0.9
c..lower scaling if necessary
        fmin=+undef
        fmax=-undef
        do i=1,ld
          if(field(i,1).lt.ud) then
            fmin=min(fmin,field(i,1))
            fmax=max(fmax,field(i,1))
          end if
        end do
        if(fmin.le.fmax) then
          fmax=max(abs(fmin),abs(fmax))
          imax=nint(scale*fmax)
          do while (imax.ge.32767)
            iscale=iscale+1
            scale=10.**(-iscale)
            imax=nint(scale*fmax)
          end do
          idata(20,1)=iscale
        end if
        do i=1,ld
          if(field(i,1).lt.ud) then
            idata(20+i,1)=nint(scale*field(i,1))
          else
            idata(20+i,1)=-32767
          end if
        end do
      end if
c
      if(iacsf.gt.0) then
c..fields are subtracted ... f(t) = f(t) - f(t-dt) ... f(t) leses sist.
c..(at forecast +0 the field is assumed to be 0. and not read)
c..reset time (forecast length) and parameter (of the first read field)
        idata(4,1)=innh(10,ni)
        idata(6,1)=innh(12,ni)
	if(nread.eq.2) then
          nx=idata(10,1)
          ny=idata(11,1)
          scale1=10.**(idata(20,1))
          scale2=10.**(idata(20,2))
          scale =10.**(-idata(20,2))
          ld=nx*ny
          do i=1,ld
            if(idata(20+i,1).ne.-32767 .and.
     +         idata(20+i,2).ne.-32767) then
	      f1=scale1*real(idata(20+i,1))
	      f2=scale2*real(idata(20+i,2))
              idata(20+i,1)=nint(scale*(f2-f1))
            else
              idata(20+i,1)=-32767
            end if
          end do
	end if
      end if
c
      if(icompx.gt.0) then
c..coputations always done in input grid.
        call extra1(comput(icompx),limit,idata,nread,iwrite)
c..change parameter no.
        idata(6,1) =icompu(2,icompx)
        innh(12,ni)=icompu(2,icompx)
        if(iwrite.eq.0) goto 250
      end if
c
      if(igridc(iloop).ne.0) then
        if(igridc(iloop).eq.2) then
          intp=interp
          do i=1,nintpm
            if(iintpm(1,i).eq.iparam) intp=iintpm(2,i)
          end do
        end if
        call newgrd(ivectr,igridc(iloop),igrido(1,iloop),
     *              gridot(1,iloop),limit,idata,
     *              maxij,field,intp,iwrite)
      end if
      iwd=1
c
  250 continue
c
      if(iwrite.eq.1 .and. ksmooth.eq.1) then
	jsmooth =ismooth
	iterlowb=nsmooth
	do i=1,npsmooth
	  if(ipsmooth(i,1).eq.innh(12,ni)) then
	    jsmooth =ipsmooth(i,2)
	    iterlowb=ipsmooth(i,3)
	  end if
	end do
	if(jsmooth.ne.0) then
	  newscale=idata(20,iwd)
	  do i=1,npasmpsc
	    if(ipasmpsc(1,i).eq.innh(12,ni)) newscale=ipasmpsc(2,i)
	  end do
	  call filters(jsmooth,iterlowb,
     +		       idata(1,iwd),idata(21,iwd),newscale,undef)
	end if
      end if
c
      if(igridc(iloop).ne.0) innh(2,ni)=igrido(1,iloop)
c
      if(iwrite.eq.0 .and. (nchang.gt.0 .or. nchnga.gt.0)) then
c..make dummy identification for change of ident and 'index'
        iwd=1
        do i=1,20
          idata(i,iwd)=-32767
        end do
        idata( 1,iwd)=innh(1,ni)
        idata( 2,iwd)=innh(2,ni)
        do i=3,9
          idata(i,iwd)=innh(i+6,ni)
        end do
        idata(12,iwd)=innh(3,ni)
        idata(13,iwd)=innh(4,ni)
        idata(14,iwd)=innh(5,ni)
      end if
c
      nch=0
c
      if(nchang.gt.0) then
c..change identification and 'index' ('index' also if no field data)
c..checked in calling program: not allowed to change producer or size.
        do n=1,nchang
          i=ichang(1,n)
          if(idata(i,iwd).eq.ichang(2,n)) then
            idata(i,iwd)=ichang(3,n)
            if(i.le.2) then
              innh(i,ni)=ichang(3,n)
            elseif(i.le.9) then
              innh(i+6,ni)=ichang(3,n)
            elseif(i.ge.12 .and. i.le.14) then
              innh(i-9,ni)=ichang(3,n)
            end if
            nch=1
          end if
        end do
      end if
c
      if(nchnga.gt.0) then
c..change identification and 'index' ('index' also if no field data)
c..change_a: vertical coord., parameter, level_1, level_2
c..          (-32767 => unchanged value)
        do n=1,nchnga
          ich=1
          do i=1,4
            if(ichnga(i,1,n).ne.-32767 .and.
     *         ichnga(i,1,n).ne.idata(4+i,iwd)) ich=0
          end do
          if(ich.eq.1) then
            do i=1,4
              if(ichnga(i,1,n).ne.-32767) then
                idata(4+i,iwd)=ichnga(i,2,n)
                innh(10+i,ni) =ichnga(i,2,n)
              end if
            end do
            nch=1
          end if
        end do
      end if
c
      nchfld=nchfld+nch
c
      if(iwrite.eq.1) then
c
        k=0
        do i=1,20
          if(idprv(i).eq.1 .and. idprvo(i).ne.idata(i,iwd)) k=1
          idprvo(i)=idata(i,iwd)
        end do
        if(k.ne.0) write(6,fmt='(''  output grid: '',8i7)')
     *                          (idprvo(i),i=1,2),(idprvo(i),i=10,11),
     *                                            (idprvo(i),i=15,18)
c
c..count no. of output fields and fields with bad ident(19)
        icontrol(2)=icontrol(2)+1
        icontrol(3)=icontrol(3)+id19err
c
        nx=idata(10,iwd)
        ny=idata(11,iwd)
        ldata=nx*ny
c
c-rescale----------------------------------------------------------
        jpkscl=0
        jpkdir=0
	if(irescale.eq.1) then
          if(npkscl.gt.0) then
c..rescale.  check vertical coordinate, parameter and level_1 range
            do n=1,npkscl
              if(ipkscl(1,n).eq.idata(5,iwd) .and.
     *           ipkscl(2,n).eq.idata(6,iwd) .and.
     *           ipkscl(3,n).le.idata(7,iwd) .and.
     *           ipkscl(4,n).ge.idata(7,iwd)) jpkscl=n
            end do
          end if
          if(npkdir.gt.0 .and. jpkscl.gt.0) then
c..check if a 'direction' parameter (dd) .... change 0 to 360 deg. 
            do n=1,npkdir
              if(ipkdir(n).eq.idata(6,iwd)) jpkdir=n
            end do
          end if
c..do the scaling now if not a packed format
          if(jpkscl.gt.0 .and.
     +	     (iformt.eq.0 .or. iformt.eq.1 .or. iformt.eq.4)) then
            iscalo=ipkscl(5,jpkscl)
            iscali=idata(20,iwd)
	    if(iscalo.gt.iscali) then
              idiv=10**(iscalo-iscali)
              iadd=(idiv+1)/2
	      if(jpkdir.eq.0) then
		do i=21,20+ldata
                  if(idata(i,iwd).gt.0) then
                    idata(i,iwd)=(idata(i,iwd)+iadd)/idiv
                  elseif(idata(i,iwd).ne.-32767) then
                    idata(i,iwd)=(idata(i,iwd)-iadd)/idiv
                  end if
		end do
	      else
                ip360=nint(360.*10.**(-iscalo))
		do i=21,20+ldata
		  idatax=idata(i,iwd)
                  if(idata(i,iwd).gt.0) then
                    idata(i,iwd)=(idata(i,iwd)+iadd)/idiv
                  elseif(idata(i,iwd).ne.-32767) then
                    idata(i,iwd)=(idata(i,iwd)-iadd)/idiv
                  end if
                  if(idata(i,iwd).eq.0 .and. idatax.ne.0)
     +					     idata(i,iwd)=ip360
		end do
	      end if
	      idata(20,iwd)=iscalo
	    end if
          end if
	end if
c-rescale----------------------------------------------------------
c
        if(iformt.eq.0 .or. iformt.eq.1) then
	  lgeom=0
          igtype=idata(9,iwd)
          if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
          nw=20+ldata+lgeom
          if(iswapb.eq.1) call bswap2(nw,idata(1,iwd))
          irecda=idrec1( 8)
          irecdb=idrec1(12)
          irecd0=irecda+irecdb*32767
          if(nogap.ne.1) then
            nrec=(nw+nwrec-1)/nwrec
            irecd1=irecd0+1
            irecd2=irecd0+nrec
            iword=1
            lword=nwrec
	    do i=nw+1,nwrec*nrec
	      idata(i,iwd)=0
	    end do
            i2=0
            do irec=irecd1,irecd2
              i1=i2+1
              i2=i2+nwrec
ccc           if(i2.gt.nw) i2=nw
              write(iunit,rec=irec,iostat=ios,err=910)
     *                                   (idata(i,iwd),i=i1,i2)
            end do
          else
            lword=idrec1(13)
            irecd1=irecd0
            if(lword.ge.nwrec) then
              lword=0
              irecd1=irecd1+1
            end if
            iword=lword+1
            nrec=(lword+nw+nwrec-1)/nwrec
            irecd2=irecd1+nrec-1
	    do i=lword+nw+1,nwrec*nrec
	      idata(i,iwd)=0
	    end do
            i2=0
            do irec=irecd1,irecd2
              if(irec.eq.irecd1 .and. lword.gt.0) then
                i1=i2+1
                i2=i2+nwrec-lword
ccc             if(i2.gt.nw) i2=nw
                write(iunit,rec=irec,iostat=ios,err=910)
     *                   (ibuffw(i),i=1,lword),(idata(i,iwd),i=i1,i2)
              else
                i1=i2+1
                i2=i2+nwrec
ccc             if(i2.gt.nw) i2=nw
                write(iunit,rec=irec,iostat=ios,err=910)
     *                                     (idata(i,iwd),i=i1,i2)
              end if
            end do
            if(irecd2.gt.irecd1) lword=0
            i0=i1-1
            do i=i1,i2
              ibuffw(lword+i-i0)=idata(i,iwd)
            end do
            lword=lword+i2-i0
          end if
          irecdb=(irecd1-1)/32767
          irecda=irecd1-irecdb*32767
          nwb=(nw-1)/32767
          nwa=nw-nwb*32767
          innh( 6,ni)=irecda
          innh( 7,ni)=iword
          innh( 8,ni)=nwa
          innh(15,ni)=idata(9,iwd)
          innh(16,ni)=nwb*100+irecdb
          irecdb=(irecd2-1)/32767
          irecda=irecd2-irecdb*32767
          idrec1( 8)=irecda
          idrec1(12)=irecdb
          idrec1(13)=lword
          idrec1(10)=idrec1(10)+1
        elseif(iformt.eq.2 .or. iformt.eq.3) then
          jpknot=0
          if(npknot.gt.0) then
c..check if parameter should not be packed (i.e. keep 16 bit words)
            do n=1,npknot
              if(ipknot(n).eq.idata(6,iwd)) jpknot=n
            end do
          end if
          ipcode=0
          iscalo=idata(20,iwd)
          idivot=1
          if(jpkdir.gt.0) ipcode=1
          if(jpknot.gt.0) ipcode=-1
          if(jpkscl.gt.0) then
            iscalo=ipkscl(5,jpkscl)
            idivot=ipkscl(6,jpkscl)
          end if
          call fpack2(ldata,idata(1,iwd),idata(21,iwd),
     *                ipcode,iscalo,idivot,ipkversion,
     *		      ipunit,idpack,ipack,ipackw,lpack)
c.old     lpk=24+lpack
	  if(iwhead.eq.0 .and. ipkversion.eq.2) then
	    nxout=idpack(10)
	    nyout=idpack(11)
	    lgeom=0
            igtype=idpack(9)
            if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
	    if(nxout*nyout+lgeom.gt.mpksize) then
c..correct header, possible only before output of first field
	      ipunit=(nxout*nyout+lgeom+32767-1)/32767
	      mpksize=min(32767*ipunit,limit)
	      ihead(14)=ipunit
	      idpack(24)=(lpack+ipunit-1)/ipunit
	    end if
	  end if
	  if(lpack.gt.mpksize) goto 930
	  if(iwhead.eq.0) then
c..write header before first field
	    if(iswapb.eq.1) call bswap2(16,ihead)
	    call wpkdata(1,iunit,16,ihead,ierror)
	    if(ierror.ne.0) goto 925
	    iwhead=1
	  end if
	  lpk1=24 + lpack + 1
	  lpk =24 + idpack(24)*ipunit
	  do i=lpk1,lpk
	    ipack(i)=0
	  end do
          if(iswapb.eq.1) call bswap2(lpk,idpack)
	  call wpkdata(2,iunit,lpk,idpack,ierror)
	  if(ierror.ne.0) goto 925
c..for print of file and field data size reduction
          iwrdpk=iwrdpk+lpk
	  lup=20+ldata
	  if(ipkversion.eq.2 .and. idpack(9).ge.1000) then
	    igtype=idpack(9)
	    lgeom=igtype-(igtype/1000)*1000
	    lup=lup+lgeom
	  end if
	  if(ingaps.eq.1) then
            iwrdup=iwrdup+((lup+nwrec-1)/nwrec)*nwrec
	  else
	    iwrdup=iwrdup+lup
	  end if
          jwrdup=jwrdup+lup
          nfelt=nfelt+1
        elseif(iformt.eq.4) then
	  lgeom=0
          igtype=idata(9,iwd)
          if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
          nw=20+ldata+lgeom
          write(iunit,iostat=ios,err=920) (idata(i,iwd),i= 1,20)
          write(iunit,iostat=ios,err=920) (idata(i,iwd),i=21,nw)
          nfelt=nfelt+1
        end if
c
      end if
c
      if(ni.eq.ninrec) then
        if(iformt.eq.0 .or. iformt.eq.1) then
c..skriver en record med innh.fort.
c-nord---------------------------------------------
          if(iformt.eq.1) then
            do n=1,ni
              if(innh(6,n).gt.0) then
                innh(6,n)=innh(6,n)-1
                if(innh(6,n).eq.0) then
                  innh( 6,n)=innh( 6,n)+32767
                  innh(16,n)=innh(16,n)-1
                end if
              end if
            end do
          end if
c-nord---------------------------------------------
          if(iswapb.eq.1) call bswap2(nwrec,innh)
          ireci=ireci+1
          irec=ireci
          write(iunit,rec=irec,iostat=ios,err=910)
     *                                  ((innh(i,n),i=1,16),n=1,ninrec)
          ni=0
        else
          ni=0
        end if
        nwih=nwih+ninrec
      end if
c
  220 continue
  210 continue
  200 continue
c
  190 continue
c
  180 continue
c
  170 continue
c
      if(filnam(1:1).ne.'*') close(iunitc)
      filnam='*'
c
      if((iformt.eq.2 .or. iformt.eq.3) .and. iwhead.eq.0) then
c..no fields found, write header (as old version always did)
	if(iswapb.eq.1) call bswap2(16,ihead)
	call wpkdata(1,iunit,16,ihead,ierror)
	if(ierror.ne.0) goto 925
	iwhead=1
      end if
c
      if(ni.gt.0) then
        if(iformt.eq.0 .or. iformt.eq.1) then
c-nord---------------------------------------------
          if(iformt.eq.1) then
            do n=1,ni
              if(innh(6,n).gt.0) then
                innh(6,n)=innh(6,n)-1
                if(innh(6,n).eq.0) then
                  innh( 6,n)=innh( 6,n)+32767
                  innh(16,n)=innh(16,n)-1
                end if
              end if
            end do
          end if
c-nord---------------------------------------------
          do n=ni+1,ninrec
            do i=1,16
              innh(i,n)=-1
            end do
          end do
          if(iswapb.eq.1) call bswap2(nwrec,innh)
          ireci=ireci+1
          irec=ireci
          write(iunit,rec=irec,iostat=ios,err=910)
     *                                  ((innh(i,n),i=1,16),n=1,ninrec)
          if(nodatind.eq.0 .and. ireci.lt.idrec1(9)) then
            do n=1,ninrec
              do i=1,16
                innh(i,n)=-1
              end do
            end do
            if(iswapb.eq.1) call bswap2(nwrec,innh)
            n=ireci+1
            ireci=idrec1(9)
            do irec=n,ireci
              write(iunit,rec=irec,iostat=ios,err=910)
     *                                  ((innh(i,n),i=1,16),n=1,ninrec)
            end do
          end if
        end if
        nwih=nwih+ni
      end if
c
      if(iformt.eq.0 .or. iformt.eq.1) then
        if(ireci.ne.idrec1(9) .or. nwih.ne.idrec1(11)) goto 960
      end if
c
      if(nchang.gt.0) write(6,*) 'no. of field ident. changes: ',nchfld
c
      if(iformt.eq.0 .or. iformt.eq.1) then
c
c..skriver record 1 og 2
        if(idrec1(10).gt.0) then
          call daytim(iyear,month,iday,ihour,minut,isecn)
          idrec1(2)=iyear
          idrec1(3)=month*100+iday
          idrec1(4)=ihour*100+minut
        end if
        idrec1(5)=itimeo(1)
        idrec1(6)=itimeo(2)*100+itimeo(3)
        idrec1(7)=itimeo(4)*100
        if(iarkiv.gt.0) then
          idrec1(20)=itimea(1,1)
          idrec1(21)=itimea(2,1)*100+itimea(3,1)
          idrec1(22)=itimea(4,1)*100
          idrec1(23)=itimeo(1)
          idrec1(24)=itimeo(2)*100+itimeo(3)
          idrec1(25)=itimeo(4)*100
          idrec1(26)=ntstep
          idrec1(27)=newinh
          idrec1(28)=lprod
          idrec1(29)=ktresol
          idrec1(30)=itresol
        end if
c
c-nord---------------------------------------------
        if(iformt.eq.1) then
          idrec1(8)=idrec1(8)-1
          if(idrec1(8).eq.0) then
            idrec1( 8)=idrec1( 8)+32767
            idrec1(12)=idrec1(12)-1
          end if
          idrec1(9)=idrec1(9)-1
          do n=1,99
            if(idrec2(1,n).gt.0) idrec2(1,n)=idrec2(1,n)-1
          end do
        end if
c-nord---------------------------------------------
c
c..reset update flag
        idrec1(15)=0
c
        if(iswapb.eq.1) then
          call bswap2(nwrec,idrec1)
          call bswap2(nwrec,idrec2)
        end if
c
        irec=2
        write(iunit,rec=irec,err=910,iostat=ios)
     *                                 ((idrec2(i,n),i=1,2),n=1,nwrech)
        irec=1
        write(iunit,rec=irec,err=910,iostat=ios) (idrec1(n),n=1,nwrec)
c
        if(iswapb.eq.1) then
c..swap back (for print)
          call bswap2(nwrec,idrec1)
          call bswap2(nwrec,idrec2)
        end if
c
      elseif(iformt.eq.2) then
c..
c..utvider filen til 'n*1024' 16 bits ord .... minst 24 ord |
        nword=nwrdpk+iwrdpk
        nxrec=(nword+24+1024-1)/1024
        lword=nxrec*1024-nword
        if(lword.gt.0) then
	  call wpkdata(3,iunit,1,ihead,ierror)
	  if(ierror.ne.0) goto 925
          nwrdpk=nwrdpk+lword
        end if
c
      elseif(iformt.eq.3) then
c
        call wpkdata(4,iunit,1,ihead,ierror)
	if(ierror.ne.0) goto 925
c
      end if
c
      if(iformt.eq.2 .or. iformt.eq.3) then
        nwrdpk=nwrdpk+iwrdpk
        nwrdup=nwrdup+iwrdup
        fsave=100.*real(nwrdup-nwrdpk)/real(nwrdup)
        if(jwrdup.gt.0) then
          dsave=100.*real(jwrdup-iwrdpk)/real(jwrdup)
        else
          dsave=0.
        end if
        write(6,fmt='('' no. of fields:          '',i6)') nfelt
        write(6,fmt='('' file. packing reduction:'',f6.1,'' %'')') fsave
        write(6,fmt='('' data. packing reduction:'',f6.1,'' %'')') dsave
      elseif(iformt.eq.4) then
        write(6,fmt='('' no. of fields: '',i6)') nfelt
      end if
c
      goto 990
c
  910 write(6,*) 'write error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '             record,iostat: ',irec,ios
      goto 970
  920 write(6,*) 'write error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '             iostat: ',ios
      goto 970
  925 write(6,*) 'write error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      goto 970
  930 write(6,*) 'too big output packed field.'
      write(6,*) '        size,max: ',lpack,mpksize
      goto 970
c
  950 write(6,*) 'too many fields specified: ',ntstep*newinh
      write(6,*) '             max is 32767'
      write(6,*) '   no. of timesteps:           ',ntstep
      write(6,*) '   no. of fields per timestep: ',newinh
      goto 970
c
  960 write(6,*) '***************************************************'
      write(6,*) 'PROGRAM ERROR. ireci,idrec1( 9): ',ireci,idrec1( 9)
      write(6,*) '               nwih, idrec1(11): ',nwih, idrec1(11)
      write(6,*) '***************************************************'
      goto 970
c
  970 close(iunit)
      if(filnam(1:1).ne.'*') close(iunitc)
ccc   stop 2
      write(6,*) 'copflt ***** stop 2 *****'
      call exit(2)
c
  990 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine newgrd(ivectr,igridc,igrido,gridot,limit,idata,
     +                  maxij,field,interp,iwrite)
c
c  subgrid, grid interpolation and vector rotation.
c  grid interpolation only for polarstereographic grids
c
      integer   ivectr,igridc,igrido(7),limit,maxij,interp,iwrite
      integer*2 idata(limit,4)
      real      gridot(6),field(maxij,3)
c
      parameter (mindexg=750)
c
      integer   ixp,iyp,idg,ifi
      integer   indexg(mindexg)
      real      gridin(6),gridix(6)
c
      call gridpar(+1,limit,idata(1,1),igtype,nxi,nyi,gridin,ierror)
      if(ierror.ne.0) write(6,*) 'GRIDPAR ERROR'
      if(ierror.ne.0) goto 980
c
      lgeom=0
      igt=idata(9,1)
      if(idata(9,1).ge.1000) lgeom=igt-(igt/1000)*1000
      ldi=20+nxi*nyi+lgeom
c
      nd=1
      if(ivectr.ne.0) nd=2
c
      if(igridc.eq.1 .or. igridc.eq.3) then
c
c..sub-grid og/eller midling av flere gridpunkt (3*3,5*5,...)
c
        ix1=igrido(2)
        ix2=igrido(3)
        iy1=igrido(4)
        iy2=igrido(5)
        ijstep=igrido(6)
        irotat=igrido(7)
        do i=1,6
          gridot(i)=gridin(i)
        end do
c
        if(ijstep.lt.1) ijstep=1
        if(irotat.ne.0 .and.
     +     (igtype.ne.0 .and. igtype.ne.1 .and. igtype.ne.4)) then
          write(6,*) 'Grid rotation not possible for grid type:',igtype
          irotat=0
        end if
c..safety first, checked and message printed in calling program
        if(irotat.ne.-180 .and. irotat.ne.-90 .and.
     +     irotat.ne.+180 .and. irotat.ne.+90) irotat=0
        if(ix1.eq.0 .and. ix2.eq.0 .and. iy1.eq.0 .and. iy2.eq.0) then
          ix1=  1
          ix2=nxi
          iy1=  1
          iy2=nyi
        elseif(ix1.lt.1 .or. ix1.gt.ix2 .or. ix2.gt.nxi .or.
     +         iy1.lt.1 .or. iy1.gt.iy2 .or. iy2.gt.nyi) then
          ig=idata(2,1)
          write(6,*) '*** sub-grid error.  grid: ',igrido(1)
          write(6,*) '*** ix1,ix2,iy1,iy2:   ',ix1,ix2,iy1,iy2
          write(6,*) '*** input: grid,nx,ny: ',ig,nxi,nyi
          goto 980
        end if
c
        if(igridc.eq.3) then
          do id=1,nd
            call grmean(nxi,nyi,ix1,ix2,iy1,iy2,ijstep,idata(21,id))
          end do
        end if
c
        nxs=(ix2-ix1)/ijstep+1
        nys=(iy2-iy1)/ijstep+1
        nxo=nxs
        nyo=nys
        if(irotat.eq.-90 .or. irotat.eq.+90) then
          nxo=nys
          nyo=nxs
        end if
        igsub=1
        if(ix1.eq.1 .and. ix2.eq.nxi .and.
     +     iy1.eq.1 .and. iy2.eq.nyi .and. ijstep.eq.1) igsub=0
        ldo=20+nxo*nyo
        iw=nd+1
        if(igsub.ne.0 .or. irotat.ne.0) then
        do id=1,nd
          if(igsub.eq.0) then
            do i=1,ldo
              idata(i,iw)=idata(i,id)
            end do
          else
              do i=1,6
                gridin(i)=gridot(i)
              end do
            call grdsub(nxi,nyi,ix1,iy1,ijstep,nxs,nys,
     +                    idata(1,id),idata(21,id),gridin,igtype,
     +                    idata(1,iw),idata(21,iw),gridot,ierror)
              if(ierror.ne.0) goto 980
          end if
          if(irotat.eq.0) then
            do i=1,ldo
              idata(i,id)=idata(i,iw)
            end do
          else
c..only if polarstereographic grid (checked above)
              do i=1,6
                gridin(i)=gridot(i)
              end do
            call grdrot(irotat,nxs,nys,nxo,nyo,
     +                    idata(1,iw),idata(21,iw),gridin,igtype,
     +                    idata(1,id),idata(21,id),gridot,ierror)
              if(ierror.ne.0) goto 980
          end if
        end do
        end if
        if(ivectr.ne.0 .and. irotat.ne.0) then
          do i=1,ldo
            idata(i,3)=idata(i,1)
            idata(i,4)=idata(i,2)
          end do
          ndx=3
          ndy=4
          isignx=+1
          isigny=+1
          if(irotat.eq.-90) then
            ndx=4
            ndy=3
            isignx=-1
          elseif(irotat.eq.+90) then
            ndx=4
            ndy=3
            isigny=-1
          elseif(irotat.eq.-180 .or. irotat.eq.+180) then
            isignx=-1
            isigny=-1
          end if
c..identification, careful with scaling and undefined values
          idata(20,1)=idata(20,ndx)
          idata(20,2)=idata(20,ndy)
          if(isignx.eq.+1) then
            do i=21,ldo
              idata(i,1)=idata(i,ndx)
            end do
          else
            do i=21,ldo
              if(idata(i,ndx).ne.-32767) then
                idata(i,1)=-idata(i,ndx)
              else
                idata(i,1)=-32767
              end if
            end do
          end if
          if(isigny.eq.+1) then
            do i=21,ldo
              idata(i,2)=idata(i,ndy)
            end do
          else
            do i=21,ldo
              if(idata(i,ndy).ne.-32767) then
                idata(i,2)=-idata(i,ndy)
              else
                idata(i,2)=-32767
              end if
            end do
          end if
        end if
c
      elseif(igridc.eq.2) then
c
c..grid interpolation, only for polarstereographic grids
c
        if((igtype.ne.1 .and. igtype.ne.4) .or.
     +     gridin(3).eq.0.0 .or. gridot(3).eq.0.0 .or.
     +     abs(gridin(5)-gridot(5)).gt.0.1) then
          write(6,*) 'interpolation only for polarstereographic'
          write(6,*) 'grids true at same latitude'
          write(6,*) 'ident(9,15:18): ',idata(9,1),(idata(i,1),i=15,18)
          goto 980
        end if
        nxo=igrido(2)
        nyo=igrido(3)
        ldo=20+nxo*nyo
        iscalo=-32767
        undef=+1.e+35
        iundef=0
        nundef=0
        ldf=nxi*nyi
        iw=nd+1
        do id=1,nd
          if(iscalo.lt.idata(20,id)) iscalo=idata(20,id)
          scalei=10.**idata(20,id)
          do i=1,ldf
            if(idata(20+i,id).ne.-32767) then
              field(i,iw)=scalei*real(idata(20+i,id))
            else
              field(i,iw)=undef
              iundef=1
            end if
          end do
          call ginter(interp,iundef,undef,
     +                nxi,nyi,field(1,iw),gridin,
     +                nxo,nyo,field(1,id),gridot,nud)
          nundef=nundef+nud
c..check after interpolation
          fmin=+1.e+35
          fmax=-1.e+35
          if(nud.eq.0) then
            do i=1,nxo*nyo
              fmin=min(fmin,field(i,id))
              fmax=max(fmax,field(i,id))
            end do
          else
            do i=1,nxo*nyo
              if(field(i,id).lt.undef) then
                fmin=min(fmin,field(i,id))
                fmax=max(fmax,field(i,id))
              end if
        end do
          end if
          if(fmin.le.fmax) then
            fmax=max(abs(fmax),abs(fmin))
            i=nint(fmax*10.**(-iscalo))
            if(i.lt.-32766 .or. i.gt.+32767) iscaleo=iscaleo+1
          end if
        end do
        if(ivectr.ne.0) then
          call gintuv(nundef,undef,nxo,nyo,
     +                field(1,1),field(1,2),gridin,gridot)
        end if
        ldf=nxo*nyo
        scaleo=10.**(-iscalo)
        do id=1,nd
          if(nundef.eq.0) then
            do i=1,ldf
                n=nint(scaleo*field(i,id))
                idata(20+i,id)=n
            end do
          else
            do i=1,ldf
              if(field(i,id).lt.undef) then
                n=nint(scaleo*field(i,id))
                idata(20+i,id)=n
              else
                idata(20+i,id)=-32767
              end if
            end do
          end if
        end do
c
      elseif(igridc.eq.4) then
c
c..geographic grid input and output
c..(not necessary to handle wind components, u(e/w),v(n/s) )
c
        if(igtype.ne.2 .or. gridin(5).ne.0. .or. gridin(6).ne.0.) then
          write(6,*) 'geographic output grid, but not geographic',
     +               ' input grid.'
          write(6,*) 'ident(9,15:18): ',idata(9,1),(idata(i,1),i=15,18)
          goto 980
        end if
c
        nloninp =nxi
        nlatinp =nyi
        igloninp=nint(gridin(1)*10000.)
        iglatinp=nint(gridin(2)*10000.)
        idloninp=nint(gridin(3)*10000.)
        idlatinp=nint(gridin(4)*10000.)
        nlonout =igrido(2)
        nlatout =igrido(3)
        iglonout=nint(gridot(1)*10000.)
        iglatout=nint(gridot(2)*10000.)
        idlonout=nint(gridot(3)*10000.)
        idlatout=nint(gridot(4)*10000.)
        if(idlonout.eq.0) idlonout=idloninp
        if(idlatout.eq.0) idlatout=idlatinp
        lonstep=idlonout/idloninp
        latstep=idlatout/idlatinp
        i=(iglonout-igloninp)/idloninp
        j=(iglatout-iglatinp)/idlatinp
        if(lonstep*idloninp.ne.idlonout .or.
     +     latstep*idlatinp.ne.idlatout .or.
     +     i*idloninp.ne.iglonout-igloninp .or.
     +     j*idlatinp.ne.iglatout-iglatinp) then
          ig=idata(2,1)
          write(6,*) 'Geographic input and output grid.'
          write(6,*) 'Interpolation not possible.'
          write(6,*) 'The gridpoints does not match in the two grids.'
          write(6,*) '  output grid: ',igrido(1)
          write(6,*) '    pos. (1,1) long,lat: ',gridot(1),gridot(2)
          write(6,*) '    resolution long,lat: ',gridot(3),gridot(4)
          write(6,*) '  input  grid: ',ig
          write(6,*) '    pos. (1,1) long,lat: ',gridin(1),gridin(2)
          write(6,*) '    resolution long,lat: ',gridin(3),gridin(4)
          goto 980
        end if
	lonbase=1+i
	latbase=1+j
        if(mod(3600000,idloninp).eq.0) then
          lonwrap=3600000/idloninp
          if(lonwrap.lt.0) lonwrap=-lonwrap
        else
          lonwrap=0
        end if
        id=1
        iw=nd+1
        ldo=20+nlonout*nlatout
        do i=1,ldi
          idata(i,iw)=idata(i,id)
        end do
        do i=21,ldo
          idata(i,id)=-32767
        end do
	ilat1=1
	l=latbase
	do while ((l.lt.1 .or. l.gt.nlatinp) .and. ilat1.le.nlatout)
	  ilat1=ilat1+1
	  l=l+latstep
	end do
	ilat2=nlatout
	l=latbase+(ilat2-1)*latstep
	do while ((l.lt.1 .or. l.gt.nlatinp) .and. ilat2.ge.1)
	  ilat2=ilat2-1
	  l=l-latstep
	end do
        do ilon1=1,nlonout,mindexg
	  ilon2=min(ilon1+mindexg-1,nlonout)
	  nlon=ilon2-ilon1+1
	  missin=0
	  l=lonbase+(ilon1-2)*lonstep
          do n=1,nlon
	    l=l+lonstep
	    if(l.lt.1) then
	      l=l+lonwrap
	    elseif(l.gt.lonwrap) then
	      l=l-lonwrap
	    end if
	    if(l.ge.1 .and. l.le.nloninp) then
	      indexg(n)=l
	    else
	      indexg(n)=0
	      missin=1
	    end if
          end do
	  do ilat=ilat1,ilat2
	    iadro=20+(ilat-1)*nlonout+ilon1-1
	    ilati=latbase+(ilat-1)*latstep
	    iadri=20+(ilati-1)*nloninp
	    if(missin.eq.0) then
	      do n=1,nlon
	        idata(iadro+n,id)=idata(iadri+indexg(n),iw)
	      end do
	    else
	      do n=1,nlon
		if(indexg(n).gt.0)
     +	           idata(iadro+n,id)=idata(iadri+indexg(n),iw)
	      end do
	    end if
	  end do
        end do
        nxo=nlonout
        nyo=nlatout
c
      else
c
        write(6,*) '***************************************************'
        write(6,*) '**newgrd** program error.  igridc= ',igridc
        write(6,*) '***************************************************'
        goto 980
c
      end if
c
      do id=1,nd
        idata(2,id)=igrido(1)
        call gridpar(-1,limit,idata(1,id),igtype,nxo,nyo,gridot,ierror)
        if(ierror.ne.0) write(6,*) 'GRIDPAR ERROR'
        if(ierror.ne.0) goto 980
      end do
c
      iwrite=1
      return
c
  980 iwrite=0
c
      return
      end
c
c***********************************************************************
c
      subroutine grmean(nx,ny,ix1,ix2,iy1,iy2,ijstep,idata)
c
c..compute mean values for the output grid (for ijstep=3,5,7,.....)
c
      integer   nx,ny,ix1,ix2,iy1,iy2,ijstep
      integer*2 idata(nx,ny)
c
      integer   isum
c
      ijs=ijstep
      ijm=ijs/2
c
c..check if undefined values
      i1=max(ix1-ijm, 1)
      i2=min(ix2+ijm,nx)
      j1=max(iy1-ijm, 1)
      j2=min(iy2+ijm,ny)
      nudef=0
      do j=j1,j2
        do i=i1,i2
          if(idata(i,j).eq.-32767) nudef=1
        end do
      end do
c
      do iy=iy1,iy2,ijs
        j1=max(iy-ijm, 1)
        j2=min(iy+ijm,ny)
        do ix=ix1,ix2,ijs
          i1=max(ix-ijm, 1)
          i2=min(ix+ijm,nx)
          isum=0
c
          if(nudef.eq.0) then
            do j=j1,j2
              do i=i1,i2
                isum=isum+idata(i,j)
              end do
            end do
            np=(j2-j1+1)*(i2-i1+1)
            idata(ix,iy)=nint(real(isum)/real(np))
          else
            np=0
            do j=j1,j2
              do i=i1,i2
                if(idata(i,j).ne.-32767) then
                  isum=isum+idata(i,j)
                  np=np+1
                end if
              end do
            end do
            if(np.gt.0) then
              idata(ix,iy)=nint(real(isum)/real(np))
            else
              idata(ix,iy)=-32767
            end if
          end if
c
        end do
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine grdsub(nxi,nyi,ix1,iy1,ijstep,nxo,nyo,
     +                  identi,idatai,gridin,igtype,
     +                  idento,idatao,gridot,ierror)
c
c..make subgrid and adjust field identification
c
      integer   nxi,nyi,ix1,iy1,ijstep,nxo,nyo,igtype,ierror
      integer*2 identi(20),idatai(nxi,nyi)
      integer*2 idento(20),idatao(nxo,nyo)
      real      gridin(6),gridot(6)
c
      ijs=ijstep
      iy=iy1-ijs
      do j=1,nyo
        iy=iy+ijs
        ix=ix1-ijs
        do i=1,nxo
          ix=ix+ijs
          idatao(i,j)=idatai(ix,iy)
        end do
      end do
c
c..update grid parameters
c..note: idento(15:18) corrected by subr. gridpar later
      do i=1,20
        idento(i)=identi(i)
      end do
      idento(10)=nxo
      idento(11)=nyo
      dx=ix1-1
      dy=iy1-1
      call movegrid(igtype,gridin,dx,dy,gridot,ierror)
      if(ierror.ne.0 .and. igtype.eq.0) ierror=0
      if(ierror.ne.0) write(6,*) 'MOVEGRID ERROR.  grid type:',igtype
      if(ierror.eq.0 .and. ijs.ne.1) then
c..resoloution
        if(igtype.eq.1 .or. igtype.eq.4) then
c..polarstereographic ... correct "an"
	  gridot(1)=(gridot(1)-1.)/real(ijs) + 1.
	  gridot(2)=(gridot(2)-1.)/real(ijs) + 1.
          gridot(3)=gridot(3)/real(ijs)
        elseif(igtype.eq.2 .or. igtype.eq.3 .or. igtype.eq.5) then
c..geographic(2)   rotated sphaeric(3)  mercator (unrotated) (5)
          gridot(3)=gridot(3)*real(ijs)
          gridot(4)=gridot(4)*real(ijs)
        end if
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine grdrot(irotat,nxi,nyi,nxo,nyo,
     +                  identi,idatai,gridin,igtype,
     +                  idento,idatao,gridot,ierror)
c
c..rotate grid -90, +90 or 180 degrees and adjust field identification
c..(not taking care of vector components)
c..only for polarstereographic grids,
c..but also accepting an undefined gridtype (0).
c
      integer   irotat,nxi,nyi,nxo,nyo,igtype,ierror
      integer*2 identi(20),idatai(nxi,nyi)
      integer*2 idento(20),idatao(nxo,nyo)
      real      gridin(6),gridot(6)
c
      ierror=0
c
      if(igtype.ne.0 .and. igtype.ne.1 .and. igtype.ne.4) then
        write(6,*) 'GRDROT.  BAD grid type: ',igtype
        ierror=1
        return
      end if
c
      do i=1,20
        idento(i)=identi(i)
      end do
      idento(10)=nxo
      idento(11)=nyo
c..note: idento(15:18) updated by subr. gridpar later
      do i=1,6
        gridot(i)=gridin(i)
      end do
c
      nxip=nxi+1
      nyip=nyi+1
c
      if(irotat.eq.-90) then
c..-90 degrees rotation
        do j=1,nyo
          do i=1,nxo
            idatao(i,j)=idatai(j,nyip-i)
          end do
        end do
        if(igtype.eq.1 .or. igtype.eq.4) then
          gridot(1)=nyip-gridin(2)
          gridot(2)=gridin(1)
          gridot(4)=gridin(4)-90.
          if(gridot(4).le.-180.) gridot(4)=gridot(4)+360.
        end if
c
      elseif(irotat.eq.+90) then
c..+90 degrees rotation
        do j=1,nyo
          do i=1,nxo
            idatao(i,j)=idatai(nxip-j,i)
          end do
        end do
        if(igtype.eq.1 .or. igtype.eq.4) then
          gridot(1)=gridin(2)
          gridot(2)=nxip-gridin(1)
          gridot(4)=gridin(4)+90.
          if(gridot(4).gt.+180.) gridot(4)=gridot(4)-360.
        end if
c
      elseif(irotat.eq.-180 .or. irotat.eq.+180) then
c..+/-180 degrees rotation
        do j=1,nyo
          do i=1,nxo
            idatao(i,j)=idatai(nxip-i,nyip-j)
          end do
        end do
        if(igtype.eq.1 .or. igtype.eq.4) then
          gridot(1)=nxip-gridin(1)
          gridot(2)=nyip-gridin(2)
          gridot(4)=gridin(4)+180.
          if(gridot(4).gt.+180.) gridot(4)=gridot(4)-360.
        end if
c
      else
        write(6,*) '***************************************************'
        write(6,*) '**grdrot** program error.  irotat= ',irotat
        write(6,*) '***************************************************'
        ierror=1
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine extra1(comput,limit,idata,nread,iwrite)
c
c  misc. computations in pressure and sigma surfaces
c
c  undefined values allowed.
c
c  comput: t(k)->t(c)        - temp.kelvin  -> temp.celsius
c          t(c)->t(k)        - temp.celsius -> temp.kelvin
c          pressure.th->t(k) - pressure level: pot.temp. -> temp.kelvin
c          pressure.th->t(c) - pressure level: pot.temp. -> temp.celsius
c          sigma.th->t(k)    - sigma    level: pot.temp. -> temp.kelvin
c          sigma.th->t(c)    - sigma    level: pot.temp. -> temp.celsius
c
c  sigma levels: second input field is ps (surface pressure)
c
c  for all temperature computations: not changing field scaling
c
c
      character*(*) comput
      integer       limit,nread,iwrite
      integer*2     idata(limit,nread)
c
      real          r,cp,rcp,t0
      integer*2     it0
c
c
      nx=idata(10,1)
      ny=idata(11,1)
      i1=20+1
      i2=20+nx*ny
c
c..constants:
      r = 287.
      cp=1004.
      rcp=r/cp
c
      if(comput.eq.'t(k)->t(c)' .or.
     *   comput.eq.'t(c)->t(k)') then
c
        iscale=idata(20,1)
        it0=nint(273.15*10.**(-iscale))
        if(comput.eq.'t(k)->t(c)') it0=-it0
c
        do i=i1,i2
          if(idata(i,1).ne.-32767) idata(i,1)=idata(i,1)+it0
        end do
c
      elseif(comput.eq.'pressure.th->t(k)' .or.
     *       comput.eq.'pressure.th->t(c)') then
c
        p     =idata( 7,1)
        iscale=idata(20,1)
        si=10.**iscale
        so=10.**(-iscale)
        c=si*(p/1000.)**rcp
        t0=0.
        if(comput.eq.'pressure.th->t(c)') t0=-273.15
c
        do i=i1,i2
          if(idata(i,1).ne.-32767) then
            t=c*real(idata(i,1))+t0
            idata(i,1)=nint(t*so)
          end if
        end do
c
      elseif(comput.eq.'sigma.th->t(k)' .or.
     *       comput.eq.'sigma.th->t(c)') then
        if(nread.lt.2) goto 920
        if(idata(10,1).ne.idata(10,2) .or.
     *     idata(11,1).ne.idata(11,2)) goto 930
c
        sigma =idata(19,1)*0.0001
        iscale=idata(20,1)
        pt    =idata(19,2)
        iscps =idata(20,2)
        si=10.**iscale
        so=10.**(-iscale)
        sps=10.**iscps
        t0=0.
        if(comput.eq.'sigma.th->t(c)') t0=-273.15
c
        do i=i1,i2
          if(idata(i,1).ne.-32767 .and. idata(i,2).ne.-32767) then
            p=pt+sigma*(sps*real(idata(i,2))-pt)
            c=si*(p*0.001)**rcp
            t=c*real(idata(i,1))+t0
            idata(i,1)=nint(t*so)
          else
            idata(i,1)=-32767
          end if
        end do
c
      else
        goto 910
      end if
c
      iwrite=1
      return
c
  910 write(6,*) '**extra1** error. unknown compute type:'
      write(6,*) '**  compute_',comput
      goto 990
c
  920 write(6,*) '**extra1** program error. ps field not read.'
      write(6,*) '**  compute_',comput
      goto 990
c
  930 write(6,*) '**extra1** error. different temp. and ps fields.'
      write(6,*) '**  compute_',comput
      write(6,*) '**  temp. grid,nx,ny: ',idata(1,1),idata(10,1),
     *                                               idata(11,1)
      write(6,*) '**  ps    grid,nx,ny: ',idata(1,2),idata(10,2),
     *                                               idata(11,2)
      goto 990
c
  990 iwrite=0
c
      return
      end
c
c***********************************************************************
c
      subroutine filters(ismooth,iterlowb,ident,ifield,newscale,undef)
c
c	filters (smoothing)
c       all routines handle 'undefined' values
c
c	ismooth = 1 : shap2
c	ismooth = 2 : shap4
c	ismooth = 3 : low-bandpass filter with 'iterlowb' iterations
c
c       all routines handles 'undefined' values
c
      include 'copflt.inc'
c
      integer   ismooth,iterlowb,newscale
      integer*2 ident(20),ifield(maxij)
      real      undef
c
      real      f(maxij*4)
      complex   c(maxij*2)
c
      if(ismooth.lt.1 .or.  ismooth.gt.3)  return
      if(ismooth.eq.3 .and. iterlowb.lt.1) return
c
      nx=ident(10)
      ny=ident(11)
      iscale=ident(20)
      scale=10.**iscale
      ld=nx*ny
      ud=undef*0.9
      nundef=0
c
      ia1=1*ld+1
      ia2=2*ld+1
      ia3=3*ld+1
c
      do i=1,ld
	if(ifield(i).ne.-32767) then
	  f(i)=scale*real(ifield(i))
	else
	  f(i)=undef
	  nundef=1
	end if
      end do
c
      if(ismooth.eq.1) then
	call fshap2(nx,ny,f(1),f(ia1),nundef,undef,f(ia2),f(ia3))
      elseif(ismooth.eq.2) then
	call fshap4(nx,ny,f(1),f(ia1),c(ia1),c(ia2),
     +		    nundef,undef,f(ia2),f(ia3))
      elseif(ismooth.eq.3 .and. iterlowb.gt.0) then
	call fsmooth(iterlowb,nx,ny,f(1),f(ia1),
     +		     nundef,undef,f(ia2),f(ia3))
      end if
c
      ident(20)=newscale
      scale=10.**(-newscale)
c
      if(nundef.eq.0) then
	do i=1,ld
	  ifield(i)=nint(scale*f(i))
	end do
      else
	do i=1,ld
	  if(f(i).lt.ud) then
	    ifield(i)=nint(scale*f(i))
	  else
	    ifield(i)=-32767
	  end if
	end do
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine fshap2(nx,ny,z,a,nundef,undef,w1,w2)
c
c        2. ordens shapiro-filter (glatting av felt)
c        udefinerte punkt gitt med verdi=undef (og nundef>0)
c
c        input:   z(nx,ny)
c                 a(nx,ny) ... arbeids-matrise
c                w1(nx,ny) ... arbeids-matrise (nundef>0)
c                w2(nx,ny) ... arbeids-matrise (nundef>0)
c        output:  z(nx,ny)
c
      integer nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
c
      nxm1=nx-1
      nym1=ny-1
c
      if(nundef.le.0) then
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
       	    end do
	  end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
	  end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
	    end do
	  end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
	  end do
          s=-0.5
        end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
	  do i=2,nxm1
	    w1(i,j)=1.
	    if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
	  end do
        end do
        do j=2,nym1
	  do i=1,nx
	    w2(i,j)=1.
	    if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
	  end do
        end do
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
       	    end do
	  end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
	  end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
	    end do
	  end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
	  end do
          s=-0.5
        end do
c
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine fshap4(nx,ny,z,a,zc,ac,nundef,undef,w1,w2)
c
c        4. ordens shapiro-filter (glatting av felt)
c        udefinerte punkt gitt med verdi=undef (og nundef>0)
c
c        input:   z(nx,ny)
c                 a(nx,ny) ... arbeids-matrise (real)
c                zc(nx,ny) ... arbeids-matrise (complex)
c                ac(nx,ny) ... arbeids-matrise (complex)
c                w1(nx,ny) ... arbeids-matrise (nundef>0)  (real)
c                w2(nx,ny) ... arbeids-matrise (nundef>0)  (real)
c        output:  z(nx,ny)
c
      integer nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
      complex zc(nx,ny),ac(nx,ny)
c
      real    s
      complex sc
c
      nxm1=nx-1
      nym1=ny-1
c
      if(nundef.le.0) then
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
       	    end do
	  end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
	  end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
	    end do
	  end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
	  end do
          s=-0.5
        end do
c
        do j=1,ny
          do i=1,nx
            zc(i,j)=cmplx(z(i,j))
	  end do
	end do
c
        sc=cmplx(0.,0.5)
        do k=1,2
          sc=sc*0.5
c
          do j=1,ny
            do i=2,nxm1
              ac(i,j)=zc(i,j)+sc*(zc(i-1,j)+zc(i+1,j)-2.*zc(i,j))
	    end do
	  end do
          do j=1,ny
            ac( 1,j)=zc( 1,j)
            ac(nx,j)=zc(nx,j)
	  end do
          do j=2,nym1
            do i=1,nx
              zc(i,j)=ac(i,j)+sc*(ac(i,j-1)+ac(i,j+1)-2.*ac(i,j))
	    end do
	  end do
          do i=1,nx
            zc(i, 1)=ac(i, 1)
            zc(i,ny)=ac(i,ny)
	  end do
          sc=cmplx(0.,-0.5)
        end do
c
        do j=1,ny
          do i=1,nx
            z(i,j)=real(zc(i,j))
	  end do
	end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
	  do i=2,nxm1
	    w1(i,j)=1.
	    if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
	  end do
        end do
        do j=2,nym1
	  do i=1,nx
	    w2(i,j)=1.
	    if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
	  end do
        end do
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
       	    end do
	  end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
	  end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
	    end do
	  end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
	  end do
          s=-0.5
        end do
c
        do j=1,ny
          do i=1,nx
            zc(i,j)=cmplx(z(i,j))
	  end do
	end do
c
        sc=cmplx(0.,0.5)
        do k=1,2
          sc=sc*0.5
c
          do j=1,ny
            do i=2,nxm1
	      if(w1(i,j).ne.0.)
     *          ac(i,j)=zc(i,j)+sc*(zc(i-1,j)+zc(i+1,j)-2.*zc(i,j))
	    end do
	  end do
          do j=1,ny
            ac( 1,j)=zc( 1,j)
            ac(nx,j)=zc(nx,j)
	  end do
          do j=2,nym1
            do i=1,nx
	      if(w2(i,j).ne.0.)
     *          zc(i,j)=ac(i,j)+sc*(ac(i,j-1)+ac(i,j+1)-2.*ac(i,j))
	    end do
	  end do
          do i=1,nx
            zc(i, 1)=ac(i, 1)
            zc(i,ny)=ac(i,ny)
	  end do
          sc=cmplx(0.,-0.5)
        end do
c
        do j=1,ny
          do i=1,nx
            z(i,j)=real(zc(i,j))
	  end do
	end do
c
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine fsmooth(nsmooth,nx,ny,z,a,nundef,undef,w1,w2)
c
c        low-bandpass filter, removing short wavelengths
c        (not a 2. or 4. order shapiro filter)
c
c	 G.J.Haltiner, Numerical Weather Prediction,
c		          Objective Analysis,
c			     Smoothing and filtering
c
c        input:   nshap    - no. of iterations
c                 z(nx,ny) - the field
c                 a(nx,ny) - a work matrix
c                w1(nx,ny) - a work matrix (nundef>0)
c                w2(nx,ny) - a work matrix (nundef>0)
c        output:  z(nx,ny) - the field
c
      integer nsmooth,nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
c
      nxm1=nx-1
      nym1=ny-1
c
      s=0.25
c
      if(nundef.le.0) then
c
        do n=1,nsmooth
c
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
c
        end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
	  do i=2,nxm1
	    w1(i,j)=1.
	    if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
	  end do
        end do
        do j=2,nym1
	  do i=1,nx
	    w2(i,j)=1.
	    if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
	  end do
        end do
c
        do n=1,nsmooth
c
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
c
        end do
c
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine fpack2(ldata,ident,idata,
     *                  ipcode,iscalo,idivot,ipkversion,
     *                  ipunit,idpack,ipack,ipackw,lpack)
c
c  utpakking av felt-data (type 10,11,12,13,14,15,16,17)
c
c  ipcode:     0 = ikke spesiell behandling
c              1 = pakket boelge-retning (dd): 0 erstattes med 360.
c  ipunit:     length unit (in words)
c  ipkversion: 1 = bitmap ikke tillatt (ikke oppdatert PC-utpakking)
c              2 = bitmap tillatt (for definerte/udefinerte verdier)
c  lpack:      antall ord benyttet for feltet (output)
c
c-----------------------------------------------------------------------
c  DNMI/FoU   1989-1996  Anstein Foss
c  DNMI/FoU  15.09.1997  Anstein Foss ... type 15,16,17 (and updates)
c-----------------------------------------------------------------------
c
      integer   ldata,ipcode,iscalo,idivot,ipkversion,ipunit,lpack
      integer*2 ident(20),idata(ldata)
      integer*2 idpack(24),ipack(ldata),ipackw(ldata)
c
      common/cpack1/inclim(16),konpk1(15),konpk2(4,32,15),konpk3(16,15)
c
      integer*2 iundef,ip360
c
      integer*4 ip4(16),i4
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
      if(ncall.eq.0) then
c..'setup'
        call spack1(0)
        ncall=1
      end if
c
c..evt. ekstra geometri-identifikasjon etter felt-data
      lgeom=0
      igtype=ident(9)
      if(ipkversion.eq.2) then
        if(igtype.ge.1000) lgeom=igtype-(igtype/1000)*1000
      elseif(igtype.ge.1000) then
c..reset for version 1 packing (PC)
	igtype=igtype/1000
      end if
c
      nx=ident(10)
      ny=ident(11)
      nxny=nx*ny
c
c..sjekk om udefinerte verdier finnes (angitt med 'iundef')
c
      iundef=-32767
c
      nundef=0
      do i=1,nxny
        if(idata(i).eq.iundef) nundef=nundef+1
      end do
c
c..ikke pakk data
      if(ipcode.eq.-1) goto 810
c
c..nx<5 => pakker ikke (kan ikke spare noe plass)
      if(nx.lt.5) goto 810
c
      iscali=ident(20)
      iscale=ident(20)
      if(iscale.gt.iscalo-5 .and. iscale.lt.iscalo) iscale=iscalo
      idiv=1
      if(idivot.gt.1 .and. idivot.lt.10000) idiv=idivot
c
      npack=0
      ibmap=0
c
  100 continue
c
      if(iscale.gt.iscali .or. idiv.gt.1) then
        iscal=iscale-iscali
        idivi=idiv*10**iscal
        iadd=(idivi+1)/2
        if(nundef.eq.0) then
          do i=1,nxny
            if(idata(i).gt.0) then
              ipackw(i)=(idata(i)+iadd)/idivi
            else
              ipackw(i)=(idata(i)-iadd)/idivi
            end if
          end do
        else
          do i=1,nxny
            if(idata(i).eq.iundef) then
              ipackw(i)=iundef
            elseif(idata(i).gt.0) then
              ipackw(i)=(idata(i)+iadd)/idivi
            else
              ipackw(i)=(idata(i)-iadd)/idivi
            end if
          end do
        end if
        if(ipcode.eq.1) then
          ip360=360*10**(-iscali)
          ip360=(ip360+iadd)/idivi
          do i=1,nxny
            if(ipackw(i).eq.0 .and. idata(i).ne.0) ipackw(i)=ip360
          end do
        end if
      else
        do i=1,nxny
          ipackw(i)=idata(i)
        end do
      end if
c
      nwsave=0
c
      do i1=1,nxny,nx
c
        i2=i1+nx-1
        incmin=+32768*2
        incmax=-32768*2
        nudef=0
c
	if(ibmap.eq.1) then
	  n=i1-1
	  do i=i1,i2
	    if(ipackw(i).ne.iundef) then
	      n=n+1
	      ipackw(n)=ipackw(i)
	    end if
	  end do
	  i2=n
	end if
c
        if(nundef.eq.0 .or. ibmap.eq.1) then
c
          last=ipackw(i1)
          do i=i1+1,i2
            inc=ipackw(i)-last
            if(incmin.gt.inc) incmin=inc
            if(incmax.lt.inc) incmax=inc
            last=ipackw(i)
          end do
          ifirst=ipackw(i1)
          incbas=incmin
          inc=incmax-incmin
          ix1=i1+1
          ncl=2
c
        else
c
          i=i1
          do while (ipackw(i).eq.iundef .and. i.lt.i2)
            i=i+1
          end do
          if(ipackw(i).eq.iundef) then
c..alle punkt har udefinert verdi
            ifirst=iundef
            incmin=0
            incmax=0
            nudef=0
          else
            ix1=i
            nudef=nudef+(ix1-i1)
            last=ipackw(ix1)
            do i=ix1+1,i2
              if(ipackw(i).ne.iundef) then
                inc=ipackw(i)-last
                if(incmin.gt.inc) incmin=inc
                if(incmax.lt.inc) incmax=inc
                last=ipackw(i)
              else
                nudef=nudef+1
              end if
            end do
            ifirst=ipackw(ix1)
c..nb! inkrement for foerste verdi er 0 (test i=ix1+1,... over)
c..    naar det kan vaere udefinerte, forsoek aa spare plass:
            if(incmin.gt.0) then
              if(ifirst-incmin.ge.-32767) then
                ifirst=ifirst-incmin
              else
                incmin=0
              end if
            elseif(incmax.lt.0) then
              if(ifirst-incmax.le.+32767) then
                ifirst=ifirst-incmax
              else
                incmax=0
              end if
            end if
            incbas=incmin
            inc=incmax-incmin+1
            ix1=i1
            ncl=3
          end if
c
        end if
c
        if(incmin.eq.0 .and. incmax.eq.0 .and. nudef.eq.0) then
c
c..konstant verdi
          nbits=0
          if(npack+2.gt.nxny) goto 810
          ipack(npack+1)=nbits
          ipack(npack+2)=ifirst
          npack=npack+2
c
        else
c
          i=1
          do while (inc.gt.inclim(i) .and. i.lt.16)
            i=i+1
          end do
          nbits=i
          if(nbits.lt.16) then
            last=ifirst
            if(nudef.eq.0) then
              do i=ix1,i2
                idatax=ipackw(i)
                ipackw(i)=ipackw(i)-last-incbas
                last=idatax
              end do
            else
              iudef=inclim(nbits)
              do i=ix1,i2
                if(ipackw(i).ne.iundef) then
                  idatax=ipackw(i)
                  ipackw(i)=ipackw(i)-last-incbas
                  last=idatax
                else
                  ipackw(i)=iudef
                end if
              end do
            end if
            if(konpk1(nbits).eq.0) call spack1(nbits)
            if(npack+3.gt.nxny) goto 810
            ipack(npack+1)=nbits
            ipack(npack+2)=incbas
            ipack(npack+3)=ifirst
            npack=npack+3
            do ix=ix1,i2,16
              i0=ix-1
	      il=min(i0+16,i2)
	      ni=il-i0
	      npk=konpk3(ni,nbits)
              nw =konpk2(2,npk,nbits)
              do n=1,nw
                ip4(n)=0
              end do
              do ipk=1,npk
                i    =konpk2(1,ipk,nbits)+i0
                n    =konpk2(2,ipk,nbits)
                nshft=konpk2(3,ipk,nbits)
                nand =konpk2(4,ipk,nbits)
                i4=ipackw(i)
                ip4(n)=ior(ip4(n),iand(ishft(i4,nshft),nand))
              end do
              if(npack+nw.gt.nxny) goto 810
              do n=1,nw
                ipack(npack+n)=ip4(n)
              end do
              npack=npack+nw
            end do
          else
            if(npack+1+(i2-i1+1).gt.nxny) goto 810
            npack=npack+1
            ipack(npack)=nbits
            n=npack+1-i1
            do i=i1,i2
              ipack(n+i)=ipackw(i)
            end do
            npack=n+i2
          end if
c
        end if
c
	if(nudef.gt.0 .and. nbits.gt.0 .and. ibmap.eq.0) then
	  nw1=(nx*nbits+15)/16
	  nw2=((nx-1-nudef)*nbits+15)/16
	  nwsave=nwsave+(nw1-nw2)
	end if
c
      end do
c
      nwb=(nxny+15)/16
      if(ipkversion.eq.2 .and. ibmap.eq.0 .and. nwb.lt.nwsave) then
        do i=1,nwb
          ipack(i)=0
        end do
        do i=1,nxny
          if(idata(i).ne.iundef) then
            iw=(i+15)/16
            ib=i-(iw-1)*16-1
            ipack(iw)=ibset(ipack(iw),ib)
          end if
        end do
	npack=nwb
	ibmap=1
	goto 100
      end if
c
      if(npack.gt.nxny) goto 810
c
c..data er pakket
c
c..evt. geometri-identifikasjon etter felt-data (upakket)
      if(lgeom.gt.0) then
        do i=1,lgeom
          ipack(npack+i)=idata(nxny+i)
        end do
        npack=npack+lgeom
      end if
c
c..20 ord identifikasjon upakket
      do i=1,20
        idpack(i)=ident(i)
      end do
      idpack( 9)=igtype
      idpack(20)=iscale
c
      if(idiv.le.1 .and. nundef.eq.0) then
        idpack(21)=11
        idpack(22)=0
        idpack(23)=0
      elseif(idiv.le.1) then
        idpack(21)=12
        idpack(22)=0
        idpack(23)=iundef
	if(ibmap.eq.1) idpack(21)=16
      elseif(nundef.eq.0) then
        idpack(21)=13
        idpack(22)=idiv
        idpack(23)=0
      else
        idpack(21)=14
        idpack(22)=idiv
        idpack(23)=iundef
	if(ibmap.eq.1) idpack(21)=17
      end if

      if(ipunit.lt.2) then
        idpack(24)=npack
      else
        idpack(24)=(npack+ipunit-1)/ipunit
      end if
c
      lpack=npack
c
      return
c
c..skal ikke pakke data
  810 do i=1,20
        idpack(i)=ident(i)
      end do
      idpack(9)=igtype
      ib=0
      if(nundef.gt.0 .and. ipkversion.eq.2) then
        nwb=(nxny+15)/16
        l1=nxny
        l2=nwb + nxny - nundef
        if(l2.lt.l1) ib=1
      end if
      if(ib.eq.0) then
        idpack(21)=10
        idpack(22)=0
        idpack(23)=0
        lpack=nxny+lgeom
        do i=1,nxny+lgeom
          ipack(i)=idata(i)
        end do
      else
        idpack(21)=15
        idpack(22)=0
        idpack(23)=iundef
        lpack=nwb+nxny-nundef+lgeom
        do i=1,nwb
          ipack(i)=0
        end do
        n=nwb
        do i=1,nxny
          if(idata(i).ne.iundef) then
            n=n+1
            ipack(n)=idata(i)
            iw=(i+15)/16
            ib=i-(iw-1)*16-1
            ipack(iw)=ibset(ipack(iw),ib)
          end if
        end do
        do i=1,lgeom
          ipack(n+i)=idata(nxny+i)
        end do
      end if
      if(ipunit.lt.2) then
        idpack(24)=lpack
      else
        idpack(24)=(lpack+ipunit-1)/ipunit
      end if
      do i=1,nxny+lgeom
        ipack(i)=idata(i)
      end do
c
      lpack=nxny+lgeom
c
      return
      end
c
c***********************************************************************
c
      subroutine spack1(isetup)
c
c        setup for 'fpack1/fpack2'
c
c        tabeller for pakking av 16 16-bits tall.
c        tallene som pakkes er inkrementer (fra tallet foran).
c        16 tall pakkes alltid inn i helt antall 16-bits ord.
c
      common/cpack1/inclim(16),konpk1(15),konpk2(4,32,15),konpk3(16,15)
c
      if(isetup.lt.1 .or. isetup.gt.15) then
c
	do nbits=1,15
	  inclim(nbits)=2**nbits-1
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
c
c        konpk2(1,n,nbits): rel. ord nr. input
c              (2,n,nbits): rel. ord nr. output
c              (3,n,nbits): for 'ishft'
c              (4,n,nbits): for 'iand'
c
      npk=0
      nout=0
      ib2=0
c
      do nin=1,16
c
        if(ib2.eq.0) then
          nout=nout+1
          ib2=16
        end if
        ib1=ib2-1
        ib2=ib2-nbits
        if(ib2.ge.0) then
	  iset=0
          do ib=ib2,ib1
            iset=ibset(iset,ib)
          end do
          npk=npk+1
          konpk2(1,npk,nbits)=nin
          konpk2(2,npk,nbits)=nout
          konpk2(3,npk,nbits)=ib2
          konpk2(4,npk,nbits)=iset
        else
	  iset=0
          do ib=0,ib1
            iset=ibset(iset,ib)
          end do
          npk=npk+1
          konpk2(1,npk,nbits)=nin
          konpk2(2,npk,nbits)=nout
          konpk2(3,npk,nbits)=ib2
          konpk2(4,npk,nbits)=iset
          nout=nout+1
          ib2=ib2+16
	  iset=0
          do ib=ib2,15
            iset=ibset(iset,ib)
          end do
          npk=npk+1
          konpk2(1,npk,nbits)=nin
          konpk2(2,npk,nbits)=nout
          konpk2(3,npk,nbits)=ib2
          konpk2(4,npk,nbits)=iset
        end if
        konpk3(nin,nbits)=npk
c
      end do
c
      konpk1(nbits)=npk
c
      return
      end
c
c***********************************************************************
c
      subroutine wpkdata(icall,iunit,ldata,idata,ierror)
c
c..Write file with packed fields
c
c..icall=1: initial write
c..icall=2: write
c..icall=3: close file after padding
c..icall=4: close file without padding, old PC/DOS format...
c
      implicit none
c
c..input/output
      integer   icall,iunit,ldata,ierror
      integer*2 idata(ldata)
c
c..local
      integer lbuff
      parameter (lbuff=1024)
      integer*2 buff(lbuff)
c
      integer nd,id,nr,ir,i,ios
      integer lunit,irec,ibuff
c
      data lunit,irec,ibuff/0,0,0/
c
      save buff
c
      if(icall.eq.1) then
        lunit=iunit
        irec=0
        ibuff=0
      elseif(iunit.ne.lunit) then
        write(6,*) 'BAD CALL SEQUENCE IN WPKDATA !!!!!!!!!!!!!'
        ierror=1
        return
      end if
c
      if(icall.eq.1 .or. icall.eq.2) then
c
        nd=0
c
#ifdef linux
        if(ibuff+ldata.ge.lbuff) then
          nr=(ibuff+ldata)/lbuff
          if(ibuff.gt.0) then
            irec=irec+1
            nd=lbuff-ibuff
            write(iunit,rec=irec,iostat=ios,err=900)
     +			  (buff(i),i=1,ibuff),(idata(i),i=1,nd)
     	    ibuff=0
     	    nr=nr-1
     	  end if
     	  do ir=1,nr
     	    irec=irec+1
     	    id=nd+1
     	    nd=nd+lbuff
            write(iunit,rec=irec,iostat=ios,err=900)
     +					(idata(i),i=id,nd)
          end do
        end if
c
	if(nd.lt.ldata) then
	  do i=nd+1,ldata
	    ibuff=ibuff+1
	    buff(ibuff)=idata(i)
	  end do
	end if
#else
	write(iunit,iostat=ios,err=900) (idata(i),i=1,ldata)
	ibuff= ibuff+ldata
#endif
c
      elseif(icall.eq.3) then
c
c..utvider filen til 'n*1024' 16 bits ord .... minst 24 ord
#ifdef linux
        do i=ibuff+1,lbuff
          buff(i)=0
        end do
        irec=irec+1
        write(iunit,rec=irec,iostat=ios,err=900)
     +			  		(buff(i),i=1,lbuff)
c
	if(ibuff+24.gt.lbuff) then
          do i=1,ibuff
            buff(i)=0
          end do
          irec=irec+1
          write(iunit,rec=irec,iostat=ios,err=900)
     +			  		(buff(i),i=1,lbuff)
        end if
#else
        ibuff=lbuff-mod(ibuff,lbuff)
        do i=1,ibuff
          buff(i)=0
        end do
	write(iunit,iostat=ios,err=900) (buff(i),i=1,ibuff)
	if(ibuff.lt.24) then
          do i=1,lbuff
            buff(i)=0
          end do
	  write(iunit,iostat=ios,err=900) (buff(i),i=1,lbuff)
	end if
#endif
c
      elseif(icall.eq.4) then
c
#ifdef linux
	if(ibuff.gt.0) then
          irec=irec+1
          write(iunit,rec=irec,iostat=ios,err=900)
     +			  		(buff(i),i=1,ibuff)
     	  ibuff=0
	end if
#endif
c     	
      else
c
        write(6,*) 'BAD CALL TYPE IN WPKDATA !!!!!!!!!!!!!'
        ierror=1
        return
c
      end if     	
c
      ierror=0
      return
c
  900 continue
      write(6,*) 'WRITE ERROR in wpkdata.  iostat= ',ios
      ierror=1
c
      return
      end
