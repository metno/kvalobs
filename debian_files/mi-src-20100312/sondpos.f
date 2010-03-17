      program sondpos
c
c        ***** prognostiske sonderinger                ******
c        *****                                         ******
c        ***** utplukk av posisjons-data               ******
c        ***** evt. konvertering til annen maskin-type ******
c
c        punkt-verdier fra/til "lager"-filer.
c        (en input-file, en eller flere output-filer)
c
c-----------------------------------------------------------------------
c sondpos.input   example:
c=======================================================================
c *** sondpos.lam50s  ('sondpos.input')
c ***
c *=> Prognostic sounding data from LAM50S.
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    sondpos  sondpos.lam50s  sond50s.dat  standard
c *=>                             <input>      <format>
c ***
c ***------------------------------------------------------------------
c **
c ** Options:
c ** --------
c ** FILE=<input_data_file>
c ** FORMAT.STANDARD .............................. (default)
c ** FORMAT.SWAP
c ** FORMAT.PC
c ** FORMAT.IBM
c ** FORMAT.PRINT
c ** FORMAT.PRINT_HEIGHT
c ** FORMAT.PRINT_SPOTWIND.1
c ** FORMAT.PRINT_SPOTWIND.2
c ** FORMAT.PRINT_SPOTWIND.3
c ** SPOTWIND.IDENTIFICATION= <FBENXX> ......... (default FBEN41,FBEN42)
c ** FORMAT.PRINT_PARAM ... .. print parameters in "print_param=..."
c ** PRINT_PARAM= parnum, "name", "format", "description"
c ** FORMAT.PRINT_VERIFI
c ** VERIFI.STATION.FILE=<file_name> ....... for 'format.print_verifi'
c ** VERIFI.TABLE.NAME=<table_name>  ....... for 'format.print_verifi'
c ** PROG_LIMIT=<min_prog_hour,max_prog_hour,...> . (default: no limit)
c ** PROG_OUT=<prog_hour,_prog_hour,......>   ..... (default: all)
c ** PARAM_OUT=<param,param,..............>   ..... (default: all)
c ** LEVEL_OUT=<level_index,level_index,..>   ..... (default: all)
c ** NEW.TEXT.OUT= <one text line> .............. (repeatable)
c ** ADD.TEXT.OUT= <one text line> .............. (repeatable)
c ** LIST_DATA
c ** LIST_USED
c ** LIST_NOT_USED
c ** END
c **
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c ** Options    Remember '......' syntax.
c ** ($... = environment var. ; #n = coomand line arg. no. n (n>1) )
c *>
c 'FILE= #2'               <<< 'FILE= sond50s.dat'
c 'FORMAT.#3'              <<< 'FORMAT.STANDARD'
c 'END'
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c **
c **
c *>
c '>>>>>>>>>>>>', 'sond1.dat'  <<<--------------- output file
c 'OSLO - BLINDERN',      'OSLO'
c 'FERDER FYR',           'FERDER'
c 'LINDESNES FYR',        'LINDESNES'
c 'JOTUNHEIMEN',          '*'
c 'SOLA', 'STAVANGER',    '*'
c 'BERGEN - FLORIDA',     'BERGEN'
c 'VIGRA - $LESUND',      '$LESUND'
c 'R@RVIK (TRONDHEIM)',   'TRONDHEIM'
c 'BOD@',                 '*'
c 'TROMS@ - LANGNES',     'TROMS@'
c 'VARD@',                '*'
c 'SVALBARD LUFTHAVN',    'LONGYEARBYEN'
c 'STATFJORD',            '*'
c 'MIKE - POLARFRONT',    'MIKE'
c '>>>>>>>>>>', 'sond2.dat'  <<<----------------- output file
c 'AMSTERDAM',  '*'
c 'ATHEN',      '*'
c 'BEOGRAD',    '*'
c 'BERLIN',     '*'
c 'BRUSSEL',    '*'
c 'GENEVE',     '*'
c 'HELSINGFORS',            '*'
c 'K@BENHAVN',  '*'
c 'LONDON',  '*'
c 'MADRID',  '*'
c 'MOSKVA',  '*'
c 'OSLO - BLINDERN',      'OSLO'
c 'PARIS',   '*'
c 'WIEN',       '*'
c '===========',          '*'         <<<------- end
c **-------------------------------------------------------------------
c *
c *-------------------------------------------------
c * All positions (only format conversion):
c * ---------------------------------------
c * '>>>>', 'posall.dat'
c * '====', '*'
c *-------------------------------------------------
c=======================================================================
c
c
c-----------------------------------------------------------------------
c standard for plot programs:
c ---------------------------
c        parameter no. 1: p        (hpa)
c                      2: t        (celsius)
c                      3: td       (celsius)
c                      4: u (e/w)  (knots)
c                      5: v (n/s)  (knots)
c                      6: omega    (hpa/s)
c-----------------------------------------------------------------------
c
c        div. max. antall er satt i 'parameter':
c          maxpos - stasjoner
c          maxniv - niv$
c          maxtid - tidspunkt (file,data-type,tids-parameter)
c          maxpar - parametre
c          maxout - max antall stasjoner output (hvis ikke alle)
c-----------------------------------------------------------------------
c
c      DNMI library subroutines:  rlunit
c                                 rcomnt
c                                 getvar
c                                 keywrd
c                                 chcase
c                                 prhelp
c                                 rmfile
c                                 daytim
c                                 bget*
c                                 bput*
c
c
c=======================================================================
c  FILE=<input_data_file>
c  FORMAT.STANDARD .............................. (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  FORMAT.PRINT
c  FORMAT.PRINT_HEIGHT
c  FORMAT.PRINT_SPOTWIND.1
c  FORMAT.PRINT_SPOTWIND.2
c  FORMAT.PRINT_SPOTWIND.3
c  SPOTWIND.IDENTIFICATION= <FBENXX> ......... (default FBEN41,FBEN42)
c  FORMAT.PRINT_PARAM ... .. print parameters in "print_param=..."
c  PRINT_PARAM= parnum, "name", "format", "description"
c  FORMAT.PRINT_VERIFI
c  VERIFI.STATION.FILE=<file_name> ........... for 'format.print_verifi'
c  VERIFI.TABLE.NAME=<table_name>  ........... for 'format.print_verifi'
c  PROG_LIMIT=<min_prog_hour,max_prog_hour,...> . (default: no limit)
c  PROG_OUT=<prog_hour,_prog_hour,......>   ..... (default: all)
c  PARAM_OUT=<param,param,..............>   ..... (default: all)
c  LEVEL_OUT=<level_index,level_index,..>   ..... (default: all)
c  NEW.TEXT.OUT= <one text line> ................ (repeatable)
c  ADD.TEXT.OUT= <one text line> ................ (repeatable)
c  LIST_DATA
c  LIST_USED
c  LIST_NOT_USED
c  END
c=======================================================================
c
c-----------------------------------------------------------------------
c  DNMI/FoU  08.01.1993  Anstein Foss
c  DNMI/VA   10.11.1993  Audun Christoffersen . print_verifi test code
c  DNMI/FoU  18.02.1994  Anstein Foss ......... print_verifi
c  DNMI/FoU  12.03.1994  Anstein Foss
c  DNMI/VA   27.03.1995  Audun Christoffersen . print_verifi update
c  DNMI/FoU  27.03.1995  Anstein Foss ......... print_spotwind.2
c  DNMI/FoU  10.04.1995  Anstein Foss ......... bget* and bput* routines
c  DNMI/FoU  02.05.1995  Anstein Foss ......... print_spotwind.2 update
c  DNMI/FoU  31.05.1995  Anstein Foss ......... print_spotwind.2 update
c  DNMI/FoU  29.12.1995  Anstein Foss
c  DNMI/FoU  30.09.1996  Anstein Foss ......... print_verifi correction
c  DNMI/FoU  21.05.1997  Anstein Foss ......... print_spotwind.2 update
c  DNMI/FoU  03.07.2001  Anstein Foss ......... print_param (NILU)
c  DNMI/FoU  09.01.2003  Anstein Foss ......... spotwind.identification=...
c  DNMI/FoU  31.10.2003  Anstein Foss ... format.standard always bigendian
c  DNMI/FoU  28.11.2005  Anstein Foss ... name bugfix, subr. prtparam
c  DNMI/IT   27.03.2008  Rebecca Rudsar ....... changed dd to 360 for spotwind format 2 ref. Dan.Ronny.Vangelsten@avinor.no
c  DNMI/IT   13.01.2009  Rebecca Rudsar ....... added Spotwind format 3 (same as format 2 but with actual time in 'VALID AT....')
c-----------------------------------------------------------------------
c
c
      include 'sondpos.inc'
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for sondpos.f
c
c  maxpos - max antall input        posisjoner/stasjoner
c  maxpar - max antall input/output parametre
c  maxniv - max antall input/output nivaa
c  maxtid - max antall input/output tidspunkt
c  maxdat - max total  input        datamengde (npos*npar*ntid)
c  mtekst - max antall input/output tekst-linjer
c  mposid - max antall input/output ekstra parametre (ikke tidsavhengig,
c                                   posisjon og/eller 'parameter'-felt)
c  mcont  - max antall input/output spesifikasjoner i header
c  midnum - max antall input/output ord i foerste header spesifikasjon
c  maxout - max antall output       posisjoner/stasjoner, en file
c  maxall - max antall output       posisjoner/stasjoner, alle filer
c  maxfil - max antall output       filer
c  mparprt- max antall "print_param=..."
c
c..input
ccc   parameter (maxpos=300,maxpar=12,maxniv=50,maxtid=24)
ccc   parameter (maxdat=300000)
ccc   parameter (mtekst=10,mposid=8)
ccc   parameter (mcont=40,midnum=20)
c
c..en output-file
ccc   parameter (maxout=300)
c
c..alle output-filer
ccc   parameter (maxall=300,maxfil=50)
c
ccc   parameter (mparprt=50)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c..buffer and file record length
      parameter (lbuff=512)
c
      integer*2 buff(lbuff)
c
      integer   ltekst,nposid
      integer   iposid(mposid,maxpos),iposit(2,mposid)
      integer   iparam(maxpar),iskalp(maxpar),itime(5,maxtid)
      integer   ktekst(2,mtekst),ntnavn(maxpos),ktidtx(maxtid)
      integer*2 dat(maxdat)
c
      character*80 tekst(mtekst)
      character*30 navn(maxpos)
      character*16 tidtxt(maxtid)
c
      integer   numpos(maxpos),iposi1(mposid,maxout)
      integer   ntnav1(maxout),ntnav3(maxall)
      character*30 navn1(maxout),navn2(maxall),navn3(maxall)
c
      integer   ihead(8),icont(mcont),idnum(midnum)
      real      skalp(maxpar),skalid(mposid)
      integer   nused(maxall),listot(2,maxfil)
      integer   iprog(maxtid),iparm(maxpar),ilevl(maxniv)
      integer   jprog(maxtid),jparm(maxpar),jlevl(maxniv)
      integer   itimev(5,maxtid)
      real      hlpdat(maxpar*maxniv*maxtid)
c
      integer   ltekstn,ktekstn(2,mtekst)
      character*80 tekstn(mtekst)
c
      integer      iparprt(mparprt)
      character*16 tparprt(mparprt)
      character*8  fparprt(mparprt)
      character*80 dparprt(mparprt)
c
      integer      jparprt(mparprt),kparprt(mparprt),jparam(maxpar)
      integer      kfprt(6)
c
      character txtin1*30, txtin2*60
c
      character*256 filein,fileot(maxfil),vsfile,vtable
      character*32 spotwindid
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*256 cinput,cipart
      character*256 finput
c
      logical bigendian
c
      nfilot=0
c
      iprhlp=0
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c..file unit for 'sondpos.input'
      iuinp=9
c
c..file unit for input data file and all output data files
      iunit=20
c
c--------------------------------------------------------------------
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: sondpos <sondpos.input>'
        write(6,*) '      or: sondpos <sondpos.input> <arguments>'
        write(6,*) '      or: sondpos <sondpos.input> ?   (to get help)'
        write(6,*)
        stop 1
      endif
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error:'
        write(6,*) finput
        stop 1
      endif
c
      if(narg.eq.2) then
        call getarg(2,cinput)
        if(cinput.eq.'?') then
          call prhelp(iuinp,'*=>')
          close(iuinp)
          stop 1
        endif
      endif
c
c
      write(6,*) 'reading input file:'
      write(6,*) finput
c
      nlines = 0
c
      iformt =-1
      nprog  = 0
      nparm  = 0
      nlevl  = 0
      lstdat = 0
      lstusd = 0
      lstnot = 0
      newtxt = 0
      ltekstn= 0
c
      filein ='*'
c
c..for format.print_verifi ... station file
      vsfile='*'
c..for format.print_verifi ... table name
      vtable='*'
c..spotwind.identification (FBENXX)
      spotwindid='*'
c
      iprmin = -32767
      iprmax = +32767
c
      nparprt= 0
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
      iend=0
c
      do while (iend.eq.0)
c
        nlines=nlines+1
        read(iuinp,*,iostat=ios,err=211,end=212) cinput
c
c..check if input as environment variables, command line arguments
c                    or possibly as 'user questions'.
c
        call getvar(1,cinput,1,1,1,ierror)
        if(ierror.ne.0) goto 215
c
        mkey=maxkey
        call keywrd(1,cinput,'=',';',mkey,kwhere,nkey,ierror)
        if(ierror.ne.0) goto 213
c
        do ikey=1,nkey
c
ccc         l=kwhere(1,ikey)
           k1=kwhere(2,ikey)
           k2=kwhere(3,ikey)
          kv1=kwhere(4,ikey)
          kv2=kwhere(5,ikey)
c
c=======================================================================
c  FILE=<input_data_file>
c  FORMAT.STANDARD .............................. (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  FORMAT.PRINT
c  FORMAT.PRINT_HEIGHT
c  FORMAT.PRINT_SPOTWIND.1
c  FORMAT.PRINT_SPOTWIND.2
c  FORMAT.PRINT_SPOTWIND.3
c  SPOTWIND.IDENTIFICATION= <FBENXX> ......... (default FBEN41,FBEN42)
c  FORMAT.PRINT_PARAM ... .. print parameters in "print_param=..."
c  PRINT_PARAM= parnum, "name", "format", "description"
c  FORMAT.PRINT_VERIFI
c  VERIFI.STATION.FILE=<file_name> ........... for 'format.print_verifi'
c  VERIFI.TABLE.NAME=<table_name>  ........... for 'format.print_verifi'
c  PROG_LIMIT=<min_prog_hour,max_prog_hour,...> . (default: no limit)
c  PROG_OUT=<prog_hour,_prog_hour,......>   ..... (default: all)
c  PARAM_OUT=<param,param,..............>   ..... (default: all)
c  LEVEL_OUT=<level_index,level_index,..>   ..... (default: all)
c  NEW.TEXT.OUT= <one text line> ................ (repeatable)
c  ADD.TEXT.OUT= <one text line> ................ (repeatable)
c  LIST_DATA
c  LIST_USED
c  LIST_NOT_USED
c  END
c=======================================================================
c
          if(cinput(k1:k2).eq.'file') then
c  file=<input_data_file>
            if(filein(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            filein=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'format.standard') then
c..format.standard
            if(iformt.ne.-1) goto 214
            iformt=0
          elseif(cinput(k1:k2).eq.'format.swap') then
c..format.swap
            if(iformt.ne.-1) goto 214
            iformt=1
          elseif(cinput(k1:k2).eq.'format.pc') then
c..format.pc
            if(iformt.ne.-1) goto 214
            iformt=2
          elseif(cinput(k1:k2).eq.'format.ibm') then
c..format.ibm
            if(iformt.ne.-1) goto 214
            iformt=3
          elseif(cinput(k1:k2).eq.'format.print') then
c..format.print
            if(iformt.ne.-1) goto 214
            iformt=1001
          elseif(cinput(k1:k2).eq.'format.print_height') then
c..format.print_height
            if(iformt.ne.-1) goto 214
            iformt=1002
          elseif(cinput(k1:k2).eq.'format.print_param') then
c..format.print_param
            if(iformt.ne.-1) goto 214
            iformt=1003
          elseif(cinput(k1:k2).eq.'print_param') then
c..print_param= parnum, "name", "format", "description"
	    nparprt=nparprt+1
            if(nparprt.gt.mparprt) goto 237
            cipart=cinput(kv1:kv2)
c..strings inside a string... however this should work...
	    do i=1,6
	      k=index(cipart,'"')
	      if(k.lt.1) goto 213
	      cipart(k:k)=' '
	      kfprt(i)=k
	    end do
	    if(kfprt(1)+1.gt.kfprt(2)-1) goto 213
	    if(kfprt(3)+1.gt.kfprt(4)-1) goto 213
	    if(kfprt(4)+1.gt.kfprt(6)-1) goto 213
            read(cipart,*,err=213) iparprt(nparprt)
            tparprt(nparprt)= cipart(kfprt(1)+1:kfprt(2)-1)
            fparprt(nparprt)= cipart(kfprt(3)+1:kfprt(4)-1)
            dparprt(nparprt)= cipart(kfprt(5)+1:kfprt(6)-1)
c..require a real format
	    call chcase(1,1,fparprt(nparprt))
            if(fparprt(nparprt)(1:1).ne.'f' .and.
     +	       fparprt(nparprt)(1:1).ne.'e') goto 213
          elseif(cinput(k1:k2).eq.'format.print_spotwind.1') then
c..format.print_spotwind.1
            if(iformt.ne.-1) goto 214
            iformt=2001
          elseif(cinput(k1:k2).eq.'format.print_spotwind.2') then
c..format.print_spotwind.2
            if(iformt.ne.-1) goto 214
            iformt=2002
          elseif(cinput(k1:k2).eq.'format.print_spotwind.3') then
c..format.print_spotwind.3
            if(iformt.ne.-1) goto 214
            iformt=2003
          elseif(cinput(k1:k2).eq.'spotwind.identification') then
c..spotwind.identification= <FBENXX>
            if(spotwindid(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            spotwindid=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'format.print_verifi') then
c  format.print_verifi
            if(iformt.ne.-1) goto 214
            iformt=3000
          elseif(cinput(k1:k2).eq.'verifi.station.file') then
c  verifi.station.file=
            if(vsfile(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            vsfile=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'verifi.table.name') then
c  verifi.table.name=
            if(vtable(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            vtable=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'prog_limit') then
c..prog_limit=<min_prog_hour,max_prog_hour>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) iprmin,iprmax
            if(iprmin.gt.iprmax) goto 213
          elseif(cinput(k1:k2).eq.'prog_out') then
c..prog_out=<prog_hour,_prog_hour,...>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//char(0)
            i1=nprog+1
            i2=i1-1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxtid) goto 231
              i2=i2+1
              read(cipart,*,iostat=ios) (iprog(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nprog=i2
          elseif(cinput(k1:k2).eq.'param_out') then
c..param_out=<param,param,...........>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//char(0)
            i1=nparm+1
            i2=i1-1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxpar) goto 231
              i2=i2+1
              read(cipart,*,iostat=ios) (iparm(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nparm=i2
          elseif(cinput(k1:k2).eq.'level_out') then
c..level_out=<level_index,level_index,...>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//char(0)
            i1=nlevl+1
            i2=i1-1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxniv) goto 231
              i2=i2+1
              read(cipart,*,iostat=ios) (ilevl(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlevl=i2
          elseif(cinput(k1:k2).eq.'new.text.out') then
c..new.text.out=
            if(kv1.lt.1) goto 213
            if(newtxt.ne.0 .and. newtxt.ne.1) goto 213
            newtxt=1
            ltekstn=ltekstn+1
            if(ltekstn.gt.mtekst) goto 231
            tekstn(ltekstn)=cinput(kv1:kv2)
            ktekstn(1,ltekstn)=kv2-kv1+1
          elseif(cinput(k1:k2).eq.'add.text.out') then
c..add.text.out=
            if(kv1.lt.1) goto 213
            if(newtxt.ne.0 .and. newtxt.ne.2) goto 213
            newtxt=2
            ltekstn=ltekstn+1
            if(ltekstn.gt.mtekst) goto 231
            tekstn(ltekstn)=cinput(kv1:kv2)
            ktekstn(1,ltekstn)=kv2-kv1+1
          elseif(cinput(k1:k2).eq.'list_data') then
c..list_data
            lstdat=1
          elseif(cinput(k1:k2).eq.'list_used') then
c..list_used
            lstusd=1
          elseif(cinput(k1:k2).eq.'list_not_used') then
c..list_not_used
            lstnot=1
          elseif(cinput(k1:k2).eq.'end') then
c..end
            iend=+1
          else
            goto 213
          endif
c
        end do
c
      end do
c
c..file=
      if(filein(1:1).eq.'*') goto 216
c
c..default output file format
      if(iformt.lt.0) iformt=0
c
      if(iformt.eq.1003 .and. nparprt.lt.1) goto 217
c
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
c
c--------------------------------------------------------------------
c
c..posisjoner (navn) ... '>>>>',       'output_file' . <-- start file
c..                      'input_navn', '*' ........... uforandret navn
c..                      'input_navn', 'output_navn' . nytt navn
c..                      '<<rest>>',   '*' alle posisjoner som tidligere
c..                                        ikke er benyttet
c..                                        (m} v{re siste 'navn')
c..                      '====',       '*' ........... slutt
c..nb| hvis ingen navn f@r slutt/file => alle posisjoner til output-file
c
      nfile=0
      nall =0
      irest=0
c
      iend=0
c
      do while (iend.eq.0)
c
        nlines=nlines+1
        read(iuinp,*,iostat=ios,err=211,end=212) txtin1,txtin2
c..slutt
        if(txtin1(1:2).eq.'==') then
          iend=+1
        elseif(irest.eq.1) then
          goto 236
c..ny output-file
        elseif(txtin1(1:2).eq.'>>') then
          if(nfile.eq.maxfil) goto 232
          nfile=nfile+1
          fileot(nfile)=txtin2
c..check if input as environment variables, command line arguments
c                    or possibly as 'user questions'.
          call getvar(1,fileot(nfile),1,1,1,ierror)
          if(ierror.ne.0) goto 215
          listot(1,nfile)=nall+1
          listot(2,nfile)=nall
        elseif(nfile.eq.0) then
          goto 233
        elseif(nall.eq.maxall) then
          goto 234
        else
          nall=nall+1
          listot(2,nfile)=nall
          if(listot(2,nfile)-listot(1,nfile)+1.gt.maxout) goto 235
          navn2(nall)=txtin1
          navn3(nall)=txtin2
          if(txtin1(1:2).eq.'<<') then
            call chcase(1,1,txtin1)
            if(txtin1(1:8).eq.'<<rest>>') then
              navn2(nall)=txtin1
              navn3(nall)=txtin2
              irest=1
            endif
          endif
        endif
c
      end do
c
      goto 250
c
  210 write(6,*) 'error reading comment lines.'
      goto 240
  211 write(6,*) 'error reading input.'
      goto 240
  212 write(6,*) 'end of file not o.k.'
      goto 240
  213 write(6,*) 'error in input.  input text:'
      write(6,*) cinput
      goto 240
  214 write(6,*) 'option already set.  input text:'
      write(6,*) cinput
      goto 240
  215 iprhlp=1
      goto 240
  216 write(6,*) 'no output file specified.'
      goto 240
  217 write(6,*) 'format.print_param but no "print_param=..."'
      goto 240
c
  231 write(6,*) 'too many specifications of the type below:'
      write(6,*) cinput
      goto 240
  232 write(6,*) 'too many output files.  max: ',maxfil
      goto 240
  233 write(6,*) 'no file specified before first position name.'
      goto 240
  234 write(6,*) 'too many output positions.  max: ',maxall
      goto 240
  235 write(6,*) 'too many positions on one output file. max: ',maxout
      goto 240
  236 write(6,*) '<<rest>> must be the last ''name''.'
      goto 240
  237 write(6,*) 'too many "print_param=...",  max: ',mparprt
      goto 240
c
  240 write(6,*) 'error at line no. ',nlines,'   (or below)'
      if(iprhlp.eq.1) then
        write(6,*) 'help from ''sondpos.input'':'
        call prhelp(iuinp,'*=>')
      endif
      close(iuinp)
      stop 1
c
  250 close(iuinp)
c
      write(6,*) 'input o.k.'
c
c--------------------------------------------------------------------
c
      mtn=len(navn3(1))
      do n=1,nall
        if(navn3(n)(1:1).eq.'*') navn3(n)=navn2(n)
        kt=1
        do k=1,mtn
          if(navn3(n)(k:k).ne.' ') kt=k
        end do
        ntnav3(n)=kt
      end do
c
c--------------------------------------------------------------------
c
c..innlesning
c
      write(6,*) 'read data from file:'
      write(6,*)  filein
c
c...................................................................
c..tester av antall tegn er ikke lagt inn i denne versjon (ikke bra)
c        max antall tegn i fast tekst
ccc   mct=len(tekst(1))
c        max antall tegn i hvert navn
ccc   mcn=len(navn(1))
c        max antall tegn i tekst for tid (prognose-tid)
ccc   mcx=len(tidtxt(1))
c        forsiktig, leser to og to tegn input
ccc   mct=(mct/2)*2
ccc   mcn=(mcn/2)*2
ccc   mcx=(mcx/2)*2
c...................................................................
c
      open(iunit,file=filein,
     *           access='direct',form='unformatted',
     *           recl=(lbuff*2)/lrunit,status='old',iostat=ios,err=920)
c
      irec=0
      ibuff=lbuff
c
c---------------------------------------------------------------------
c        ihead    - file header
c        ihead(1) - file identification, 201 = prognostic soundings
c             (2) - word length in bytes
c             (3) - record length (max) in words
c             (4) - character type, 0 = standard ascii
c             (5) - not used
c             (6) - not used
c             (7) - not used
c             (8) - length (in words) of next array (icont)
c
c        icont      - file contents
c        icont(n)   - type of information
c                      101 - no. of positions,parameters etc.
c                      201 - time
c                      301 - text
c                      401 - position names
c                      501 - position coordinates etc.
c                      601 - parameters
c        icont(n+1) - length (in words) of information
c
c---------------------------------------------------------------------
c
      irec=0
      ibuff=lbuff
c
      in101=0
      in201=0
      in301=0
      in401=0
      in501=0
      in601=0
c
c..header
      call bgeti4(iunit,irec,buff,lbuff,ibuff,ihead,8,ierr)
      if(ierr.ne.0) goto 960
c
c..check file identification, file format and if it contains anything
      if(ihead(1).ne.201 .or. ihead(2).ne.2
     *                   .or. ihead(3).ne.lbuff) then
        write(6,*) ' *** data-filen kan ikke leses'
        write(6,*) ' *** (feil type eller format)'
        goto 930
      endif
      ncont=ihead(8)
      if(ncont.lt.2) then
        write(6,*) ' *** filen inneholder ikke data'
        write(6,*) ' *** (men er av riktig type)'
        goto 930
      endif
c
c..contents
      if(ncont.gt.mcont) then
        write(6,*) ' *** filen inneholder for mange spesifikasjoner'
        write(6,*) ' ***       antall, max tillatt: ',ncont,mcont
        goto 930
      endif
      call bgeti4(iunit,irec,buff,lbuff,ibuff,icont,ncont,ierr)
      if(ierr.ne.0) goto 960
c
      npos=-1
      ntid=-1
      npar=-1
      nniv=-1
      ltekst=0
      nposid=0
      do 291 n=1,maxpar
        iparam(n)=0
        iskalp(n)=0
  291 continue
      do 292 n=1,mposid
        iposit(1,n)=0
        iposit(2,n)=0
  292 continue
c
c
      do 300 ic=1,ncont-1,2
c
      ityp=icont(ic)
      ilen=icont(ic+1)
c
      lenx=0
c
      if(ityp.eq.101 .and. ilen.ge.4) then
c..'101'
        nidnum=ilen
        if(nidnum.gt.midnum) then
          lenx=nidnum-midnum
          nidnum=midnum
        endif
        call bgeti4(iunit,irec,buff,lbuff,ibuff,idnum,nidnum,ierr)
        if(ierr.ne.0) goto 960
        npos=idnum(1)
        ntid=idnum(2)
        npar=idnum(3)
        nniv=idnum(4)
        if(npos.lt.1 .or. ntid.lt.1 .or. npar.lt.1 .or. nniv.lt.1) then
          write(6,*) ' ** filen inneholder ikke data'
          write(6,*) ' ** (npos,ntid,npar,nniv:',npos,ntid,npar,nniv,')'
          goto 930
        endif
        if(npos.gt.maxpos .or. ntid.gt.maxtid .or.
     *     npar.gt.maxpar .or. nniv.gt.maxniv) then
          write(6,*) ' ** filen inneholder for mye data'
          write(6,*) ' ** antall, max tillatt her:'
          write(6,*) ' ** npos,maxpos: ',npos,maxpos
          write(6,*) ' ** ntid,maxtid: ',ntid,maxtid
          write(6,*) ' ** npar,maxpar: ',npar,maxpar
          write(6,*) ' ** nniv,maxniv: ',nniv,maxniv
          goto 930
        endif
cc      if(nidnum.ge.5) iprod= idnum(5)
cc      if(nidnum.ge.6) igrid= idnum(6)
cc      if(nidnum.ge.7) ivertk=idnum(7)
cc      if(nidnum.ge.8) interp=idnum(8)
        in101=1
c
      elseif(ityp.eq.201 .and. ntid.gt.0) then
c..'201'
        call bgeti4(iunit,irec,buff,lbuff,ibuff,itime,5*ntid,ierr)
        if(ierr.ne.0) goto 960
        call bgeti4(iunit,irec,buff,lbuff,ibuff,ktidtx,ntid,ierr)
        if(ierr.ne.0) goto 960
        do 320 n=1,ntid
          tidtxt(n)=' '
          l=((ktidtx(n)+1)/2)*2
          call bgetch(iunit,irec,buff,lbuff,ibuff,tidtxt(n),1,l,ierr)
          if(ierr.ne.0) goto 960
  320   continue
        in201=1
c
      elseif(ityp.eq.301) then
c..'301'
        call bgeti4(iunit,irec,buff,lbuff,ibuff,ltekst,1,ierr)
        if(ierr.ne.0) goto 960
        if(ltekst.gt.mtekst) then
          write(6,*) ' ** filen inneholder for mange tekst-linjer'
          write(6,*) ' ** (ltekst,mtekst: ',ltekst,mtekst,')'
          goto 930
        endif
        call bgeti4(iunit,irec,buff,lbuff,ibuff,ktekst,ltekst*2,ierr)
        if(ierr.ne.0) goto 960
        do 330 n=1,ltekst
          tekst(n)=' '
          l=((ktekst(1,n)+1)/2)*2
          call bgetch(iunit,irec,buff,lbuff,ibuff,tekst(n),1,l,ierr)
          if(ierr.ne.0) goto 960
  330   continue
        in301=1
c
      elseif(ityp.eq.401 .and. npos.gt.0) then
c..'401'
        call bgeti4(iunit,irec,buff,lbuff,ibuff,ntnavn,npos,ierr)
        if(ierr.ne.0) goto 960
        do 340 n=1,npos
          navn(n)=' '
          l=((ntnavn(n)+1)/2)*2
          call bgetch(iunit,irec,buff,lbuff,ibuff,navn(n),1,l,ierr)
          if(ierr.ne.0) goto 960
  340   continue
        in401=1
c
      elseif(ityp.eq.501 .and. npos.gt.0) then
c..'501'
        call bgeti4(iunit,irec,buff,lbuff,ibuff,nposid,1,ierr)
        if(ierr.ne.0) goto 960
        if(nposid.gt.mposid) then
          write(6,*) ' ** filen inneholder for mange pos.-info.'
          write(6,*) ' ** (nposid,mposid: ',nposid,mposid,')'
          goto 930
        endif
        call bgeti4(iunit,irec,buff,lbuff,ibuff,iposit,nposid*2,ierr)
        if(ierr.ne.0) goto 960
        do 350 n=1,npos
          call bgeti4(iunit,irec,buff,lbuff,ibuff,iposid(1,n),nposid,
     *                                                         ierr)
          if(ierr.ne.0) goto 960
  350   continue
        in501=1
c
      elseif(ityp.eq.601 .and. npar.gt.0) then
c..'601'
        call bgeti4(iunit,irec,buff,lbuff,ibuff,iparam,npar,ierr)
        if(ierr.ne.0) goto 960
        call bgeti4(iunit,irec,buff,lbuff,ibuff,iskalp,npar,ierr)
        if(ierr.ne.0) goto 960
        in601=1
c
      else
c..'???'
ccc     write(6,*) ' ** filen har spesifikasjoner som ikke benyttes'
ccc     write(6,*) ' ** (ityp,ilen: ',ityp,ilen,')'
        lenx=ilen
      endif
c
      if(lenx.gt.0) then
        call bgetjp(iunit,irec,buff,lbuff,ibuff,lenx,ierr)
        if(ierr.ne.0) goto 960
      endif
c
  300 continue
c
      do n=1,npar
        skalp(n)=10.**iskalp(n)
      end do
      do n=1,nposid
        skalid(n)=10.**iposit(2,n)
      end do
c
c..data ......................dat(nniv,npar,ntid,npos)
c..data skaleres ikke her (lagres som integer*2)
c
      ndat=nniv*npar*ntid*npos
      if(ndat.gt.maxdat) then
        write(6,*) ' *************************************************'
        write(6,*) ' *************************************************'
        write(6,*) ' ** filen inneholder for mye data (totalt)'
        write(6,*) ' ** antall, max tillatt her:'
        write(6,*) ' ** ndat,maxdat: ',ndat,maxdat
        write(6,*) ' ** reduserer antall posisjoner input.'
        write(6,*) ' ** antall posisjoner i filen:   ',npos
        npos=maxdat/(nniv*npar*ntid)
        write(6,*) ' ** antall posisjoner som leses: ',npos
        write(6,*) ' *************************************************'
        write(6,*) ' *************************************************'
        if(npos.lt.1) goto 930
        ndat=nniv*npar*ntid*npos
ccc     goto 960
      endif
c
      call bgeti2(iunit,irec,buff,lbuff,ibuff,dat,ndat,ierr)
      if(ierr.ne.0) goto 960
c
      close(iunit)
c
c
c-----------------------------------------------------------------------
      if(lstdat.eq.1)
     *          call dliste(nniv,npar,ntid,npos,dat,skalp,itime,navn)
c-----------------------------------------------------------------------
c
      nnivip=nniv
      nparip=npar
      ntidip=ntid
      nposip=npos
c
c..list of output timesteps
      if(iprmin.gt.-32767 .or. iprmax.lt.+32767) then
        ntid=0
        do n=1,ntidip
          if(itime(5,n).ge.iprmin .and. itime(5,n).le.iprmax) then
            ntid=ntid+1
            jprog(ntid)=n
          end if
        end do
      else
        do n=1,ntidip
          jprog(n)=n
        end do
        ntid=ntidip
      endif
      if(nprog.gt.0) then
        nt=ntid
        ntid=0
        do n=1,nt
          k=0
          do i=1,nprog
            if(itime(5,jprog(n)).eq.iprog(i)) k=1
          end do
          if(k.eq.1) then
            ntid=ntid+1
            jprog(ntid)=jprog(n)
          end if
        end do
      end if
      if(ntid.lt.1) then
        write(6,*) '** no timesteps output (prog_limit and/or prog_out)'
        goto 960
      endif
c
c..list of output parameters (not changing sequence of parameters)
      if(nparm.gt.0) then
        npar=0
        do n=1,nparip
          k=0
          do i=1,nparm
            if(iparm(i).eq.iparam(n)) k=1
          end do
          if(k.gt.0) then
            npar=npar+1
            jparm(npar)=n
          end if
        end do
      else
        do n=1,nparip
          jparm(n)=n
        end do
        npar=nparip
      end if
      if(npar.lt.1) then
        write(6,*) '** no parameters output (param_out=)'
        goto 960
      endif
c
c..list of output levels (level indecies, not changing the sequence)
      if(nlevl.gt.0) then
        nniv=0
        do n=1,nnivip
          k=0
          do i=1,nlevl
            if(ilevl(i).eq.n) k=1
          end do
          if(k.gt.0) then
            nniv=nniv+1
            jlevl(nniv)=n
          end if
        end do
      else
        do n=1,nnivip
          jlevl(n)=n
        end do
        nniv=nnivip
      end if
      if(nniv.lt.1) then
        write(6,*) '** no levels output (level_out=)'
        goto 960
      endif
c
      if(nniv.lt.nnivip .or.
     *   npar.lt.nparip .or.
     *   ntid.lt.ntidip) then
        call deldat(nnivip,nparip,ntidip,nposip,
     *              dat,iparam,iskalp,itime,ktidtx,tidtxt,
     *              nniv,npar,ntid,jlevl,jparm,jprog,dat)
      endif
c
      if(newtxt.eq.1 .or. newtxt.eq.2) then
        if(newtxt.eq.1) then
          write(6,*) 'new text replaces the old text'
          nn=0
          k=ktekst(2,1)
        else
          write(6,*) 'new text added to the old text'
          nn=ltekst
          k=ktekst(2,ltekst)
          if(ltekst+ltekstn.gt.10) ltekstn=10-ltekst
        end if
        do l=1,ltekstn
          tekst(nn+l)=tekstn(l)
          ktekst(1,nn+l)=ktekstn(1,l)
          ktekst(2,nn+l)=k
        end do
        ltekst=nn+ltekstn
      end if
c
c..tekst: total lengde ved lagring
      lttext=0
      do l=1,ltekst
        lttext=lttext+(ktekst(1,l)+1)/2
      end do
c
c..tids-tekst: total lengde ved lagring
      lttitx=0
      do n=1,ntid
        lttitx=lttitx+(ktidtx(n)+1)/2
      end do
c
c
      do n=1,npos
        nused(n)=0
      end do
c
c
c        konvc - character:   0 = ingen
c                             1 = ascii (standard) -> ascii (pc)
c                             2 = ascii (standard) -> ebcdic (ibm)
c                            -1 = ascii (standard) -> ascii (pc),
c				  og forhindrer byteswap p.g.a. konvd=1,
c				  (for aa simulere gamle pc-rutiner)
c        konvd - data:        0 = ingen
c                             1 = snu high- og low-byte (pc/vax)
c
      konvc=0
      konvd=0
      if(iformt.eq.0 .and. .not.bigendian()) then
        konvd=1
      elseif(iformt.eq.1 .and. bigendian()) then
c..format.swap
        konvd=1
      elseif(iformt.eq.2) then
c..format.pc .... konvc=-1 to keep things as they used to be
        konvc=-1
	if(bigendian()) konvd=1
      elseif(iformt.eq.3) then
c..format.ibm
        konvc=2
	if(.not.bigendian()) konvd=1
      endif
c
      write(6,*) 'output konvertering.  konvc,konvd: ',konvc,konvd
c
c
c..utvalg for output....................................................
c
      do 400 nfil=1,nfile
c
      write(6,*) 'output file: ',fileot(nfil)
c
      nfilot=nfil
c
      n1=listot(1,nfil)
      n2=listot(2,nfil)
ccc   write(6,*) ' ............ n1,n2: ',n1,n2
c
      npos=nposip
      iall=1
      if(n1.gt.n2) goto 450
      iall=0
c
      np=0
c
      do 410 m=n1,n2
        if(navn2(m)(1:8).eq.'<<rest>>') goto 420
        nf=0
        do 415 n=1,npos
          if(navn(n).eq.navn2(m)) then
            nf=nf+1
            if(nf.eq.1) then
              np=np+1
               navn1(np)=navn3(m)
              ntnav1(np)=ntnav3(m)
              numpos(np)=n
              nused(n)=nused(n)+1
            endif
          endif
  415   continue
        if(nf.lt.1) then
          write(6,*) ' position not found: ',navn2(m)
        elseif(nf.gt.1) then
          write(6,*) ' position: ',navn2(m),' found ',nf,' times'
        endif
  410 continue
      goto 440
c
  420 m=0
      do 430 n=1,npos
        if(nused(n).eq.0) then
          if(np.lt.maxout) then
            np=np+1
             navn1(np)=navn(n)
            ntnav1(np)=ntnavn(n)
            numpos(np)=n
            nused(n)=nused(n)+1
            m=m+1
          else
            write(6,*) ' <<rest>>  no space for: ',navn(n)
          endif
        endif
  430 continue
      write(6,*) ' <<rest>>  no. of stations: ',m
c
  440 npos=np
c
      if(npos.lt.1) then
        write(6,*) ' ***** no stations found *****'
        goto 400
      endif
c
c--------------------------------------------------------------------
c
c..output og evt. konvertering
c
  450 continue
c
      ltnavn=0
      write(6,*) 'debug  iall:', iall
      if(iall.eq.0) then
        do np=1,npos
          n=numpos(np)
          ltnavn=ltnavn+(ntnav1(n)+1)/2
          do i=1,nposid
            iposi1(i,np)=iposid(i,n)
          end do
        end do
      else
        do n=1,npos
          numpos(n)=n
          ltnavn=ltnavn+(ntnavn(n)+1)/2
        end do
      endif
c
      write(6,*) '    npos,ntid,npar,nniv: ',npos,ntid,npar,nniv
c
c-print----------------------------------------
      if(iformt.eq.1001 .or. iformt.eq.1002) then
c..1001 - format.print
c..1002 - format.print_height (needs height as input parameter, par. 1)
        if(iall.eq.0) then
          call print1(iunit,fileot(nfil),iformt,
     *                nniv,npar,ntid,nposip,npos,numpos,dat,
     *                iparam,skalp,itime,ltekst,tekst,navn1)
        else
          call print1(iunit,fileot(nfil),iformt,
     *                nniv,npar,ntid,nposip,npos,numpos,dat,
     *                iparam,skalp,itime,ltekst,tekst,navn)
        endif
        goto 400
      endif
c
      if(iformt.eq.1003) then
c..1003 - format.print_param
        if(iall.eq.0) then
          call prtparam(iunit,fileot(nfil),iformt,
     *                  nniv,npar,ntid,nposip,npos,numpos,dat,
     *                  iparam,skalp,itime,ltekst,tekst,navn1,
     *		        nparprt,iparprt,tparprt,fparprt,dparprt,
     *                  jparprt,kparprt,jparam,itimev,hlpdat)
        else
          call prtparam(iunit,fileot(nfil),iformt,
     *                  nniv,npar,ntid,nposip,npos,numpos,dat,
     *                  iparam,skalp,itime,ltekst,tekst,navn,
     *		        nparprt,iparprt,tparprt,fparprt,dparprt,
     *                  jparprt,kparprt,jparam,itimev,hlpdat)
        endif
        goto 400
      endif
c
      if(iformt.eq.2001 .or. iformt.eq.2002.or. iformt.eq.2003) then
c..2001 - format.print_spotwind.1 (export print format, Bodo)
c..2002 - format.print_spotwind.2 (export print format, Royken,...)
c..2003 - format.print_spotwind.3 (new export print format for AMAN)
        if(iall.eq.0) then
          call spotw1(iunit,fileot(nfil),iformt,spotwindid,
     *                nniv,npar,ntid,nposip,npos,numpos,dat,
     *                iparam,skalp,itime,ltekst,tekst,navn1)
        else
          call spotw1(iunit,fileot(nfil),iformt,spotwindid,
     *                nniv,npar,ntid,nposip,npos,numpos,dat,
     *                iparam,skalp,itime,ltekst,tekst,navn)
        endif
        goto 400
      endif
c
      if(iformt.eq.3000) then
c..3000 - format.print_verifi (verification input)
        if(iall.eq.0) then
          call prverifi(iunit,fileot(nfil),iformt,
     *                  nniv,npar,ntid,nposip,npos,numpos,dat,
     *                  iparam,skalp,itime,ltekst,tekst,ktekst,navn1,
     *                  vsfile,vtable,itimev,hlpdat,ierror)
        else
          call prverifi(iunit,fileot(nfil),iformt,
     *                  nniv,npar,ntid,nposip,npos,numpos,dat,
     *                  iparam,skalp,itime,ltekst,tekst,ktekst,navn,
     *                  vsfile,vtable,itimev,hlpdat,ierror)
        endif
        if(ierror.ne.0) goto 960
        goto 400
      endif
c-print----------------------------------------
c
c
c-----------------------------------------------------------------------
c..file header
c        ihead    - file header
c        ihead(1) - file identification, 201 = prognostic soundings
c             (2) - word length in bytes
c             (3) - record length (max) in words
c             (4) - character type, 0 = standard ascii
c             (5) - 0, not used
c             (6) - 0, not used
c             (7) - 0, not used
c             (8) - length (in words) of next array (icont)
c
c..geografisk posisjon og idnetifikasjon for evt. 'temp'
c        iposit(1,n) - parameter-nr. eller identifikasjon
c                         -1 = geografisk bredde
c                         -2 = geografisk lengde
c                         -3 = wmo sone-nr. (99=bevegelig)
c                         -4 = stasjons-nr.
c                         -5 = max. avst. i g.bredde (bevegelig st.)
c                         -6 = max. avst. i g.lengde (bevegelig st.)
c        iposit(2,n) - skalering
c-----------------------------------------------------------------------
c
cc..................... as input
cc    ihead(1)=201
cc    ihead(2)=2
cc    ihead(3)=lbuff
cc    ihead(4)=0
cc    ihead(5)=0
cc    ihead(6)=0
cc    ihead(7)=0
cc    ihead(8)=0
c
c        icont      - file contents
c        icont(n)   - type of information
c                      101 - no. of positions,parameters etc.
c                      201 - time
c                      301 - text
c                      401 - position names
c                      501 - position coordinates etc.
c                      601 - parameters
c        icont(n+1) - length (in words) of information
c
      ncont=0
c
c..'101' - no.
      idnum(1)=npos
      idnum(2)=ntid
      idnum(3)=npar
      idnum(4)=nniv
cc........................ as input
cc    idnum(5)=igrd(1,1)
cc    idnum(6)=igrd(2,1)
cc    if(ngrd.gt.1) idnum(6)=0
cc    idnum(7)=ivcoor
cc    idnum(8)=interp
cc    nidnum=8
      icont(ncont+1)=101
      icont(ncont+2)=nidnum
      ncont=ncont+2
c
c..'201' - time     (year,month,day,hour,h.prog. and time text for menu)
      icont(ncont+1)=201
      icont(ncont+2)=ntid*5+ntid+lttitx
      ncont=ncont+2
c
c..'301' - text
      icont(ncont+1)=301
      icont(ncont+2)=1+ltekst*2+lttext
      ncont=ncont+2
c
c..'401' - positions (length of names, names)
      icont(ncont+1)=401
      icont(ncont+2)=npos+ltnavn
      ncont=ncont+2
c
c..'501' - position coordinates etc. (lat,long coordinates, obs. ident.)
cc................... as input
cc    nposid=2+2+2
      icont(ncont+1)=501
      icont(ncont+2)=1+nposid*2+nposid*npos
      ncont=ncont+2
c
c..'601' - parameters   (parameter no., scaling)
      icont(ncont+1)=601
      icont(ncont+2)=npar*2
      ncont=ncont+2
c
      ihead(8)=ncont
c
      call rmfile(fileot(nfil),0,ierror)
c
      open(iunit,file=fileot(nfil),
     *           access='direct',form='unformatted',
     *           recl=(lbuff*2)/lrunit,
     *           status='unknown',iostat=ios,err=930)
c
      irec=0
      ibuff=0
c
c..header
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ihead,8,ierr)
      if(ierr.ne.0) goto 960
c..contents
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,icont,ncont,ierr)
      if(ierr.ne.0) goto 960
c..'101'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,idnum,nidnum,ierr)
      if(ierr.ne.0) goto 960
c..'201'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,itime,5*ntid,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ktidtx,ntid,ierr)
      if(ierr.ne.0) goto 960
      do n=1,ntid
        l=((ktidtx(n)+1)/2)*2
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		    tidtxt(n),1,l,ierr)
        if(ierr.ne.0) goto 960
      end do
c..'301'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ltekst,1,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		  ktekst,ltekst*2,ierr)
      if(ierr.ne.0) goto 960
      do n=1,ltekst
        l=((ktekst(1,n)+1)/2)*2
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		    tekst(n),1,l,ierr)
        if(ierr.ne.0) goto 960
      end do
c..'401'
      if(iall.eq.0) then
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnav1,npos,ierr)
        if(ierr.ne.0) goto 960
        do n=1,npos
          l=((ntnav1(n)+1)/2)*2
          call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		      navn1(n),1,l,ierr)
          if(ierr.ne.0) goto 960
        end do
      else
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnavn,npos,ierr)
        if(ierr.ne.0) goto 960
        do n=1,npos
          l=((ntnavn(n)+1)/2)*2
          call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		      navn(n),1,l,ierr)
          if(ierr.ne.0) goto 960
        end do
      endif
c..'501'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,nposid,1,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		  iposit,nposid*2,ierr)
      if(ierr.ne.0) goto 960
      if(iall.eq.0) then
        do n=1,npos
          call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iposi1(1,n),
     *                                                 nposid,ierr)
          if(ierr.ne.0) goto 960
        end do
      else
        do n=1,npos
          call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iposid(1,n),
     *                                                 nposid,ierr)
          if(ierr.ne.0) goto 960
        end do
      endif
c..'601'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iparam,npar,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iskalp,npar,ierr)
      if(ierr.ne.0) goto 960
c
c..data .................. dat(nniv,npar,ntid,npos)
      ndat=nniv*npar*ntid
      do nn=1,npos
        n=numpos(nn)
        i=nniv*npar*ntid*(n-1)+1
        call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,dat(i),ndat,ierr)
        if(ierr.ne.0) goto 960
      end do
c
      call bputnd(iunit,irec,buff,lbuff,ibuff,konvd,ierr)
      if(ierr.ne.0) goto 960
c
      close(iunit)
c
      nfilot=0
c
  400 continue
c
c--------------------------------------------------------------------
c
      if(lstusd.eq.1) then
        write(6,*) '-------------------------------------------'
        write(6,*) 'antall ganger hver posisjon er benyttet'
        do n=1,nposip
          write(6,*) nused(n),'   ',navn(n)
        end do
        write(6,*) '-------------------------------------------'
      endif
      if(lstnot.eq.1) then
        write(6,*) '-------------------------------------------'
        write(6,*) 'posisjoner som ikke er benyttet:'
        m=0
        do n=1,nposip
          if(nused(n).eq.0) then
            write(6,*) navn(n)
            m=m+1
          endif
        end do
        write(6,*) 'antall posisjoner ikke benyttet: ',m
        write(6,*) '-------------------------------------------'
      endif
c
c--------------------------------------------------------------------
c
      goto 990
c
c
  920 write(6,*) 'open error.  input file.   iostat: ',ios
      write(6,*) filein
      goto 960
c
  930 write(6,*) 'open error.  output file.   iostat: ',ios
      write(6,*) fileot(nfil)
      goto 960
c
  960 write(6,*) ' ======== error exit ========='
      if(nfilot.gt.0) then
        close(iunit)
        nfil=nfilot
        call rmfile(fileot(nfil),0,ierror)
      endif
      stop 3
c
  990 continue
c
      end
c
c**********************************************************************
c
      subroutine deldat(nnivip,nparip,ntidip,nposip,
     *                  dat,iparam,iskalp,itime,ktidtx,tidtxt,
     *                  nniv,npar,ntid,jlevl,jparm,jprog,datout)
c
c       remove levels, parameters and timesteps from input arrrays
c
      integer   nnivip,nparip,ntidip,nposip,nniv,npar,ntid
      integer   iparam(nparip),iskalp(nparip)
      integer   itime(5,ntidip),ktidtx(ntidip)
      integer   jlevl(nniv),jparm(npar),jprog(ntid)
      integer*2 dat(nnivip,nparip,ntidip,nposip)
      integer*2 datout(nniv,npar,ntid,nposip)
      character*(*) tidtxt(ntidip)
c
      if(npar.lt.nparip) then
        do n=1,npar
          iparam(n)=iparam(jparm(n))
          iskalp(n)=iskalp(jparm(n))
        end do
      end if
c
      if(ntid.lt.ntidip) then
        do n=1,ntid
          do i=1,5
            itime(i,n)=itime(i,jprog(n))
          end do
          ktidtx(n)=ktidtx(jprog(n))
          tidtxt(n)=tidtxt(jprog(n))
        end do
      end if
c
      do n=1,nposip
        do ko=1,ntid
          k=jprog(ko)
          do jo=1,npar
            j=jparm(jo)
            do io=1,nniv
              i=jlevl(io)
              datout(io,jo,ko,n)=dat(i,j,k,n)
            end do
          end do
        end do
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine dliste(nniv,npar,ntid,npos,dat,skalp,itime,navn)
c
c        skriver ut data.
c
      integer   nniv,npar,ntid,npos
      integer*2 dat(nniv,npar,ntid,npos)
      integer   itime(5,ntid)
      real      skalp(npar)
      character*(*) navn(npos)
c
      real      datpri(10)
c
      ifil=6
      write(ifil,*)
c
      npstep=7
c
      do n=1,npos
        do nt=1,ntid
          do np1=1,npar,npstep
            np2=np1+npstep-1
            if(np2.gt.npar) np2=npar
            nnp=np2-np1+1
            np0=np1-1
            write(ifil,*) ' '
            write(ifil,*) navn(n)
            write(ifil,*) (itime(i,nt),i=1,5)
            do nl=1,nniv
              do i=1,nnp
                datpri(i)=dat(nl,np0+i,nt,n)*skalp(np0+i)
	      end do
ccc           write(ifil,fmt='(7(1x,f9.4))') (datpri(i),i=1,nnp)
              write(ifil,fmt='(7(1x,f10.4))') (datpri(i),i=1,nnp)
	    end do
	  end do
	end do
      end do
      write(ifil,*)
c
      return
      end
c
c**********************************************************************
c
      subroutine print1(iunit,filnam,iformt,
     *                  nniv,npar,ntid,nposip,npos,numpos,dat,
     *                  iparam,skalp,itime,ltekst,tekst,navn)
c
c       formatert output.
c
c           iformt=1001: p,t,td,dd,ff,omega
c           iformt=1002: p,t,td,dd,ff,omega,height
c
      integer   iunit,iformt,nniv,npar,ntid,nposip,npos,ltekst
      integer   numpos(npos),iparam(npar),itime(5,ntid)
      real      skalp(npar)
      integer*2 dat(nniv,npar,ntid,nposip)
      character*(*) filnam
      character*(*) tekst(ltekst)
      character*(*) navn(npos)
c
      iprt=iformt-1000
      if(iprt.lt.1) iprt=1
      if(iprt.gt.3) iprt=3
      write(6,*) '<<<print1>>>  format no. ',iprt
c
      ip =0
      it =0
      itd=0
      iu =0
      iv =0
      iom=0
      ih =0
      do n=1,npar
c..p (hpa)
        if(iparam(n).eq. 8) ip=n
c..t (celsius)
        if(iparam(n).eq. 4) it=n
c..td (celsius)
        if(iparam(n).eq. 5) itd=n
c..u (e/w component, knots)
        if(iparam(n).eq. 2) iu=n
c..v (n/s component, knots)
        if(iparam(n).eq. 3) iv=n
c..omega (hpa/s)
        if(iparam(n).eq.13) iom=n
c..height (m)
        if(iparam(n).eq. 1) ih=n
      end do
c
      if(iprt.eq.1) then
        if(ip.eq.0 .or. it.eq.0 .or. itd.eq.0 .or.
     *     iu.eq.0 .or. iv.eq.0 .or. iom.eq.0) then
          write(6,*) '***print1*** not correct input data.'
          return
        endif
      elseif(iprt.eq.2) then
        if(ip.eq.0 .or. it.eq.0 .or. itd.eq.0 .or.
     *     iu.eq.0 .or. iv.eq.0 .or. iom.eq.0 .or. ih.eq.0) then
          write(6,*) '***print1*** not correct input data.'
          return
        endif
      else
        write(6,*) '***print1*** unknown format: ',iformt
        return
      endif
c
      write(6,*) 'file:    ',filnam
c
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      write(iunit,*) '--------------------------------'
      write(iunit,*) 'DNMI    PROGNOSTISKE SONDERINGER'
      write(iunit,*) '--------------------------------'
      write(iunit,*)
c
      do l=1,ltekst
        write(iunit,*) tekst(l)
      end do
c
      grad=180./3.1415927
c
      do 100 n=1,npos
      ipos=numpos(n)
      write(iunit,*)
c
      do 110 l=1,ntid
      write(iunit,*)
      write(iunit,1001) itime(3,l),itime(2,l),itime(1,l),
     *                             itime(4,l),itime(5,l),navn(n)
 1001 format(1x,2(i2.2,':'),i4.4,1x,i2.2,' UTC ',sp,i4,ss,4x,a30)
c
      if(iprt.eq.1) then
        write(iunit,1011)
 1011   format(1x,'----P------T------Td----DD---FF(knop)---Omega-')
      elseif(iprt.eq.2) then
        write(iunit,1012)
 1012   format(1x,'----P------T------Td----DD---FF(knop)---Omega-',
     *            '---H(m)-')
      endif
c
      do k=1,nniv
        uew=dat(k,iu,l,ipos)*skalp(iu)
        vns=dat(k,iv,l,ipos)*skalp(iv)
        ff=sqrt(uew*uew+vns*vns)
        if(ff.gt.0.01) then
          dd=270.-grad*atan2(vns,uew)
          if(dd.gt.360.) dd=dd-360.
          if(dd.le.  0.) dd=dd+360.
        else
          dd=0.
          ff=0.
        endif
        p =dat(k,ip, l,ipos)*skalp(ip)
        t =dat(k,it, l,ipos)*skalp(it)
        td=dat(k,itd,l,ipos)*skalp(itd)
        om=dat(k,iom,l,ipos)*skalp(iom)
        if(iprt.eq.1) then
          write(iunit,1021) p,t,td,dd,ff,om
 1021     format(1x,f7.1,2(1x,f6.1),2(1x,f6.1),3x,f8.5)
        elseif(iprt.eq.2) then
          h =dat(k,ih, l,ipos)*skalp(ih)
	  if(p.lt.50. .and. h.lt.0.) h=h*(-10.)
          write(iunit,1022) p,t,td,dd,ff,om,h
 1022     format(1x,f7.1,2(1x,f6.1),2(1x,f6.1),3x,f8.5,1x,f7.0)
        endif
      end do
c
  110 continue
c
  100 continue
c
      close(iunit)
c
      return
c
  900 write(6,*) '***print1*** open error.   iostat= ',ios
      write(6,*)  filnam
      return
      end
c
c**********************************************************************
c
      subroutine prtparam(iunit,filnam,iformt,
     *                    nniv,npar,ntid,nposip,npos,numpos,dat,
     *                    iparam,skalp,itime,ltekst,tekst,navn,
     *		          nparprt,iparprt,tparprt,fparprt,dparprt,
     *                    jparprt,kparprt,jparam,itimev,hlpdat)
c
c       formatert output ... "format.print_param"
c	parameters specified with "print_param=....."
c
c           iformt=1003: the only yet...
c
c	WARNING: Units m/s and Celsius dependant on proper handling
c		 in sondat, NOT identified by parameter no.
c
      integer   iunit,iformt,nniv,npar,ntid,nposip,npos,ltekst
      integer   numpos(npos),iparam(npar),itime(5,ntid),itimev(5,ntid)
      real      skalp(npar)
      integer*2 dat(nniv,npar,ntid,nposip)
      real	hlpdat(npar,nniv,ntid)
      character*(*) filnam
      character*(*) tekst(ltekst)
      character*(*) navn(npos)
c
c..iparprt: param no.
c..tparprt: name/text (short)
c..fparprt: fortran format
c..dparprt: parameter description
c
      integer       nparprt
      integer       iparprt(nparprt)
      character*(*) tparprt(nparprt)
      character*(*) fparprt(nparprt)
      character*(*) dparprt(nparprt)
      integer       jparprt(nparprt),kparprt(nparprt),jparam(npar)
c
      character*256 cfmt,head
      character*32  testfmt,teststr
      character*2   tx(9)
c
      data tx/'1x','2x','3x','4x','5x','6x','7x','8x','9x'/
c
c..check that all parameters exist, and handle multiple parameters
c..with same parameter no. (should probably not happen here...)
c
      do n=1,npar
	nn=0
	do j=1,n
	  if(iparam(j).eq.iparam(n)) nn=nn+1
	end do
	jparam(n)=nn
      end do
c
      do n=1,nparprt
	nn=0
	do j=1,n
	  if(iparprt(j).eq.iparprt(n)) nn=nn+1
	end do
	jparprt(n)=nn
      end do
c
      nprt=0
c
      do n=1,nparprt
	nn=0
	j=0
	do while (nn.eq.0 .and. j.lt.npar)
	  j=j+1
	  if(iparam(j).eq.iparprt(n) .and.
     +	     jparam(j).eq.jparprt(n)) nn=j
	end do
	if(nn.gt.0) then
	  nprt=nprt+1
	  jparprt(nprt)=n
	  kparprt(nprt)=nn
	end if
      end do
c
      if(nprt.lt.1) then
        write(6,*) '***prtparam*** no suitable data for printing.'
        return
      end if
c
      ltxt=0
      cfmt='('
      lfmt=1
      head=''
      lhead=0
c
      do nn=1,nprt
	n=jparprt(nn)
	lp=lenstr(tparprt(n),1)
	if(ltxt.lt.lp) ltxt=lp
	l=lenstr(fparprt(n),1)
	testfmt='('//fparprt(n)(1:l)//')'
	teststr=' '
	write(teststr,fmt=testfmt) 0.0
	ls=lenstr(teststr,1)
	lx=0
        if(ls.lt.lp) then
	  lx=min(lp-ls,8)
	  ls=ls+lx
	end if
        lx=lx+1
	if(nn.eq.1) then
	  cfmt(lfmt+1:lfmt+3+l)= tx(lx)//','//fparprt(n)(1:l)
	  lfmt=lfmt+3+l
	else
	  cfmt(lfmt+1:lfmt+4+l)= ','//tx(lx)//','//fparprt(n)(1:l)
	  lfmt=lfmt+4+l
	end if
	if(ls.ge.lp) then
	  head(lhead+2+ls-lp:lhead+1+ls)=tparprt(n)(1:lp)
	else
	  head(lhead+2:lhead+1+ls)=tparprt(n)(1:ls)
	end if
        lhead=lhead+1+ls
      end do
c
      lfmt=lfmt+1
      cfmt(lfmt:lfmt)=')'
c
c..valid time
      nerr=0
      do n=1,ntid
        do i=1,5
          itimev(i,n)=itime(i,n)
        end do
        call vtime(itimev(1,n),ierr)
        if(ierr.ne.0) then
          write(6,*) '***prtparam*** : illegal date,time,prog:'
          write(6,*) (itime(i,n),i=1,5)
          nerr=nerr+1
        end if
        itimev(5,n)=itime(5,n)
      end do
      if(nerr.gt.0) return
c
      write(6,*) 'prtparam.  file: ',filnam(1:lenstr(filnam,1))
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      write(iunit,fmt='(a)') '--------------------------------'
      write(iunit,fmt='(a)') 'DNMI : PROGNOSTISKE SONDERINGER'
      write(iunit,fmt='(a)') '--------------------------------'
c
      do n=1,ltekst
        write(iunit,fmt='(a)') tekst(n)(1:lenstr(tekst(n),1))
      end do
      write(iunit,fmt='(a)') '--------------------------------'
c
      do n=1,nprt
	j=jparprt(n)
	l=lenstr(dparprt(j),1)
	write(iunit,fmt='(a,'' : '',a)') tparprt(j)(1:ltxt),
     +					 dparprt(j)(1:l)
      end do
      write(iunit,fmt='(a)') '--------------------------------'
c
      do 100 n=1,npos
c
        ipos=numpos(n)
	lnavn=lenstr(navn(n),1)
c
        do l=1,ntid
          do k=1,nniv
	    do i=1,nprt
	      j=kparprt(i)
	      hlpdat(i,k,l)=dat(k,j,l,ipos)*skalp(j)
	    end do
	  end do
	end do
c
        do l=1,ntid
c
          write(iunit,fmt='(a)')
          write(iunit,fmt='(a)') navn(n)(1:lnavn)
          write(iunit,1001) (itimev(i,l),i=1,5)
 1001     format(i4.4,3i4.2,' UTC    PROG',sp,i5,ss)
          write(iunit,fmt='(a)') '--------------------------------'
          write(iunit,fmt='(a)') head(1:lhead)
c
          do k=1,nniv
            write(iunit,fmt=cfmt) (hlpdat(i,k,l),i=1,nprt)
          end do
          write(iunit,fmt='(a)') '--------------------------------'
c
	end do
c
  100 continue
c
      close(iunit)
c
      return
c
  900 write(6,*) '***prtparam*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
      return
      end
c
c**********************************************************************
c
      subroutine spotw1(iunit,filnam,iformt,spotwindid,
     *                  nniv,npar,ntid,nposip,npos,numpos,dat,
     *                  iparam,skalp,itime,ltekst,tekst,navn)
c
c       Formatted output, SPOTWIND export format(s).
c
c       iformt=2001, 'format.print_spotwind.1' (FBEN41 for Bodo):
c	   input:  u(e/w),v(n/s),t
c          output: dd,ff,t
c	      pressure levels: 400,300,250,200,150
c	      flight   levels: 240,300,340,390,450
c	         timesteps: +6 and +12 on two files (*1 and *2)
c       Print formats by J.O. Flaeten, 20/3-92 (program: pan3 & wpan3)
c
c       iformt=2002, 'format.print_spotwind.2' (FBEN42 for Royken):
c	   input:  u(e/w),v(n/s),t
c          output: dd,ff,t
c	      pressure levels: 925,850,700,500,400,300,250,200,150,100
c	      flight   levels:  30, 50,100,180,240,300,340,390,450,530
c	         timesteps: +6,+12,+18 on one file
c       Print formats by Luftfartsverket,
c	IRS (Interface Requirement Specification) Rev.B (27.04.1995).
c       And changes in fax 31.05.95 .
c
      integer   iunit,iformt,nniv,npar,ntid,nposip,npos,ltekst
      integer   numpos(npos),iparam(npar),itime(5,ntid)
      real      skalp(npar)
      integer*2 dat(nniv,npar,ntid,nposip)
      character*(*) filnam
      character*(*) spotwindid
      character*(*) tekst(ltekst)
      character*(*) navn(npos)
c
      parameter (mprt=3,mlvl=10,mtim=3)
c
      integer nlvl(mprt),iplvl(mlvl,mprt),iflvl(mlvl,mprt),klvl(mlvl)
      integer nfctim(mprt),ifctim(mtim,mprt),itim(mtim)
      integer itimev(5,ntid)
      integer iout(3,mlvl)
c
      character*1  cr,lf
      character*12 textid
      character*40 cfmt1a,cfmt1b,cfmt2a,cfmt2b
c
c..iplvl: pressure levels (hpa)
c..iflvl: flight levels   (100 feet)
c
      data iplvl/400,300,250,200,150, -1, -1, -1, -1, -1,
     +		 925,850,700,500,400,300,250,200,150,100,
     +		 925,850,700,500,400,300,250,200,150,100/
      data iflvl/240,300,340,390,450, -1, -1, -1, -1, -1,
     +		  30, 50,100,180,240,300,340,390,450,530,
     +		  30, 50,100,180,240,300,340,390,450,530/
      data nlvl/5,10,10/
c
      data ifctim/ +6,+12, -1,
     +		   +6,+12,+18,
     +		   +3,+6,+9/
      data nfctim/2,3,3/
c
c..carriage return
      cr=char(13)
c..line feed
      lf=char(10)
c
      iprt=iformt-2000
      if(iprt.lt.   1) iprt=1
      if(iprt.gt.mprt) iprt=mprt
      write(6,*) '<<<spotw1>>>  format no. ',iprt
c
      if (len(spotwindid).lt.6) spotwindid='*'
c
      ip =0
      it =0
      iu =0
      iv =0
      do n=1,npar
c..p (hpa)
        if(iparam(n).eq. 8) ip=n
c..u (e/w component, knots)
        if(iparam(n).eq. 2) iu=n
c..v (n/s component, knots)
        if(iparam(n).eq. 3) iv=n
c..t (celsius)
        if(iparam(n).eq. 4) it=n
      end do
c
      if(ip.eq.0 .or. iu.eq.0 .or. iv.eq.0 .or. it.eq.0) then
        write(6,*) '***spotw1*** not correct input data, parameters.'
        return
      end if
c
      ierror=0
c
      do n=1,nfctim(iprt)
	itim(n)=0
	do i=1,ntid
	  if(itime(5,i).eq.ifctim(n,iprt)) itim(n)=i
	end do
	if(itim(n).eq.0) ierror=1
      end do
c
      if(ierror.ne.0) then
        write(6,*) '***spotw1*** not correct input data, time.'
        return
      end if
c
c..find the levels
      ipos1=numpos(1)
      itim1=itim(1)
      do l=1,nlvl(iprt)
        klvl(l)=0
      end do
      do k=1,nniv
        p=dat(k,ip,itim1,ipos1)*skalp(ip)
        ipp=nint(p)
	do l=1,nlvl(iprt)
	  if(ipp.eq.iplvl(l,iprt)) klvl(l)=k
	end do
      end do
      do l=1,nlvl(iprt)
        if(klvl(l).eq.0) ierror=1
      end do
c
c..needs pressure levels input (no vetical interpolation here)
c..check that all positions have the same levels at all times
      if(ierror.eq.0) then
        do ll=1,nfctim(iprt)
	  l=itim(ll)
          do kk=1,nlvl(iprt)
	    k=klvl(kk)
            do n=1,npos
              ipos=numpos(n)
              if(dat(k,ip,l,ipos).ne.dat(k,ip,itim1,ipos1)) ierror=1
            end do
          end do
        end do
      end if
c
      if(ierror.ne.0) then
        write(6,*) '***spotw1*** not correct input data, levels.'
        return
      end if
c
c..get machine time
      call daytim(jyear,jmonth,jday,jhour,jminut,jsecnd)
c
      lfname=index(filnam,' ')
      if(lfname.lt.1) then
	lfname=len(filnam)
      elseif(iprt.eq.2.or.iprt.eq.3) then
	lfname=lfname-1
      end if
c
      nlvl1=nlvl(iprt)
      nlvl2=0
      if(nlvl1.gt.5) then
	nlvl2=nlvl1
	nlvl1=5
      end if
c
      if(iprt.eq.1) then
	cfmt1a='(3x,5i10)'
	cfmt2a='(1x,a4,5(2x,i3.3,i3.3,i2.2))'
	if(nlvl2.gt.0) then
	  cfmt1b='(3x,5i10)'
	  cfmt2b='(1x,4x,5(2x,i3.3,i3.3,i2.2))'
	end if
      elseif(iprt.eq.2.or.iprt.eq.3) then
ccc	cfmt1a='(1x,5i11)'
ccc	cfmt2a='(a4,5(3x,i3.3,i3.3,i2.2))'
	write(cfmt1a,
     +	      fmt='(''(1x,'',i2,''i11,a1)'')')
     +	      nlvl1
	write(cfmt2a,
     +	      fmt='(''(a4,'',i2,''(3x,i3.3,i3.3,i2.2),a1)'')')
     +	      nlvl1
	if(nlvl2.gt.0) then
ccc	  cfmt1b='(1x,5i11)'
ccc	  cfmt2b='(4x,5(3x,i3.3,i3.3,i2.2))'
	  write(cfmt1b,
     +	        fmt='(''(1x,'',i2,''i11,a1)'')')
     +	        nlvl2-nlvl1
	  write(cfmt2b,
     +	        fmt='(''(4x,'',i2,''(3x,i3.3,i3.3,i2.2),a1)'')')
     +	        nlvl2-nlvl1
	end if
      end if
c
      do jtim=1,nfctim(iprt)
c
      nt=itim(jtim)
      newfil=0
c
      if(iprt.eq.1) then
c
	write(textid,fmt='(i1)') jtim
	filnam(lfname:lfname)=textid(1:1)
	textid='FBEN41 ENMI '
	newfil=1
c
      elseif(iprt.eq.2 .and. jtim.eq.1) then
c
	textid='FBEN42 ENMI '
	newfil=1
c
      elseif(iprt.eq.3 .and. jtim.eq.1) then
c
	textid='FBEN46 ENMI '
	newfil=1
c
      end if
c
      if(spotwindid(1:1).ne.'*') textid(1:6)=spotwindid(1:6)
c
      if(newfil.eq.1) then
        write(6,*) 'file:    ',filnam(1:lfname)
c
        open(iunit,file=filnam,
     *             access='sequential',form='formatted',
     *             status='unknown',iostat=ios,err=900)
c
        write(iunit,fmt='(''ZCZC'',2a1)') cr,cr
        write(iunit,fmt='(a12,3i2.2,2a1)') textid,
     *                                     jday,jhour,jminut,cr,cr
        write(iunit,fmt='(a1)') cr
        write(iunit,fmt='(''SPOTWIND FORECAST'',a1)') cr
c
        iyear =itime(1,nt)
        imonth=itime(2,nt)
        iday  =itime(3,nt)
        ihour =itime(4,nt)*100
	if(iprt.eq.1) then
          write(iunit,fmt='(''OBS. TIME: '',i2.2,''/'',i2.2,
     *                      1x,i4.4,'' Z'',a1)') iday,imonth,ihour,cr
	elseif(iprt.eq.2.or.iprt.eq.3) then
          write(iunit,fmt='(''TIME OF OBS: '',i2,''/'',i2.2,
     *                      1x,i4.4,'' Z'',a1)') iday,imonth,ihour,cr
	end if
      end if
c
      iprog =itime(5,nt)
      if(iprt.eq.1.or.iprt.eq.2) then
         write(iunit,fmt='(a1)') cr
         write(iunit,fmt='(''VALID AT +'',i2,
     *                  '' HRS AFTER OBS.:'',a1)') iprog,cr
      elseif(iprt.eq.3) then
c..valid time
      nerr=0
      do i=1,5
        itimev(i,nt)=itime(i,nt)
      end do
      call vtime(itimev(1,nt),ierr)
      if(ierr.ne.0) then
        write(6,*) '***spotw1*** : illegal date,time,prog:'
        write(6,*) (itime(i,nt),i=1,5)
        nerr=nerr+1
      end if
      itimev(5,nt)=itime(5,nt)
      if(nerr.gt.0) return
      iyearv =itimev(1,nt)
      imonthv=itimev(2,nt)
      idayv  =itimev(3,nt)
      ihourv =itimev(4,nt)*100
      write(iunit,fmt='(a1)') cr
      write(iunit,fmt='(''VALID AT  '',i2.2,''/'',i2.2,
     *                      1x,i4.4,'' Z'',a1)') idayv,imonthv,ihourv,cr
      end if
c
      if(iprt.eq.1) then
        write(iunit,fmt='(''       FLIGHT LEVEL'',a1)') cr
        write(iunit,fmt=cfmt1a) (iflvl(i,iprt),i=1,nlvl1)
        write(iunit,fmt='(a1)') cr
        if(nlvl2.gt.0) then
          write(iunit,fmt=cfmt1b) (iflvl(i,iprt),i=nlvl1+1,nlvl2)
          write(iunit,fmt='(a1)') cr
        end if
        write(iunit,fmt='('' SITE'',a1)') cr
      elseif(iprt.eq.2.or.iprt.eq.3) then
        write(iunit,fmt='(''    FLIGHT LEVEL'',a1)') cr
        write(iunit,fmt=cfmt1a) (iflvl(i,iprt),i=1,nlvl1),cr
ccc     write(iunit,fmt='(a1)') cr
        if(nlvl2.gt.0) then
          write(iunit,fmt=cfmt1b) (iflvl(i,iprt),i=nlvl1+1,nlvl2),cr
ccc       write(iunit,fmt='(a1)') cr
        end if
        write(iunit,fmt='(''SITE'',a1)') cr
      end if
c
      grad=180./3.1415927
c
      do n=1,npos
c
        ipos=numpos(n)
c
        do lvl=1,nlvl(iprt)
c
          k=klvl(lvl)
          uew=dat(k,iu,nt,ipos)*skalp(iu)
          vns=dat(k,iv,nt,ipos)*skalp(iv)
          t  =dat(k,it,nt,ipos)*skalp(it)
          ff=sqrt(uew*uew+vns*vns)
          if(ff.gt.0.01) then
            dd=270.-grad*atan2(vns,uew)
            if(dd.gt.360.) dd=dd-360.
            if(dd.le.  0.) dd=dd+360.
          else
            dd=0.
            ff=0.
          endif
          iout(1,lvl)=nint(dd)
c.......  iout(2,lvl)=nint(ff)
c..according to the old code:
          iout(2,lvl)=nint(ff/5.)*5
          if(iout(1,lvl).eq.0 .and. iout(2,lvl).gt.0) iout(1,lvl)=360
cRR 27.03.2008          if(iout(1,lvl).gt.0 .and. iout(2,lvl).eq.0) iout(1,lvl)=  0
          if(iout(1,lvl).gt.0 .and. iout(2,lvl).eq.0) iout(1,lvl)=  360
          iout(3,lvl)=nint(t)
c..according to the old code:
          if(iout(3,lvl).lt.0) then
            iout(3,lvl)=-iout(3,lvl)
            if(iout(3,lvl).gt.79) iout(3,lvl)=79
          else
            iout(3,lvl)=iout(3,lvl)+80
            if(iout(3,lvl).gt.99) iout(3,lvl)=99
          end if
c
        end do
c
	if(iprt.eq.1) then
          write(iunit,fmt=cfmt2a)
     *                     navn(n),((iout(i,k),i=1,3),k=1,nlvl1)
          write(iunit,fmt='(a1)') cr
	  if(nlvl2.gt.0) then
            write(iunit,fmt=cfmt2b)
     *                       ((iout(i,k),i=1,3),k=nlvl1+1,nlvl2)
            write(iunit,fmt='(a1)') cr
	  end if
	elseif(iprt.eq.2.or.iprt.eq.3) then
          write(iunit,fmt=cfmt2a)
     *                     navn(n),((iout(i,k),i=1,3),k=1,nlvl1),cr
	  if(nlvl2.gt.0) then
            write(iunit,fmt=cfmt2b)
     *                       ((iout(i,k),i=1,3),k=nlvl1+1,nlvl2),cr
	  end if
          if(n.lt.npos) write(iunit,fmt='(a1)') cr
	end if
c
      end do
c
      iclose=0
      if(iprt.eq.1) then
	iclose=1
      elseif(iprt.eq.2 .and. jtim.eq.nfctim(iprt)) then
	iclose=1
      elseif(iprt.eq.3 .and. jtim.eq.nfctim(iprt)) then
	iclose=1
      end if
c
      if(iclose.eq.1 .and. iprt.eq.1) then
        write(iunit,
     *   fmt='(''  FROM THE LEFT 3 DIGITS WINDDIR DEGR TRUE'',a1)') cr
        write(iunit,
     *   fmt='(''           NEXT 3 DIGITS WIND VELOCITY KTS'',a1)') cr
        write(iunit,
     *   fmt='(''           NEXT 2 DIGITS MINUS TEMP CENTIGR.'',a1)') cr
        write(iunit,
     *   fmt='('' ( IF PLUS, 80 MUST BE SUBTRACTED)'',a1)') cr
      end if
c
      if(iclose.eq.1) then
        write(iunit,fmt='(6a1)') lf,lf,lf,lf,lf,lf
        write(iunit,fmt='(''NNNN'')')
        close(iunit)
      end if
c
c.....end do jtim=1,nfctim(iprt)
      end do
c
      if(iprt.eq.1) filnam(lfname:lfname)=' '
c
      return
c
  900 write(6,*) '***spotw1*** open error.   iostat= ',ios
      write(6,*)  filnam
      if(iprt.eq.1) filnam(lfname:lfname)=' '
c
      return
      end
c
c**********************************************************************
c
      subroutine prverifi(iunit,filnam,iformt,
     *                    nniv,npar,ntid,nposip,npos,numpos,dat,
     *                    iparam,skalp,itime,ltekst,tekst,ktekst,navn,
     *                    vsfile,vtable,itimev,hlpdat,ierror)
c
c       Formatted output.  VERIFIcation input.
c
c   note: input  u(2) is  east/west  wind component in unit knots
c         input  v(3) is north/south wind component in unit knots
c         input  t(4) in unit degrees celsius
c         input td(5) in unit degrees celsius (dewpoint temp.)
c        output ff    in unit m/s
c
      integer   iunit,iformt,nniv,npar,ntid,nposip,npos,ltekst,ierror
      integer   numpos(npos),iparam(npar),itime(5,ntid)
      integer   ktekst(2,ltekst),itimev(5,ntid)
      real      skalp(npar),hlpdat(npar,nniv,ntid)
      integer*2 dat(nniv,npar,ntid,nposip)
      character*(*) filnam
      character*(*) tekst(ltekst)
      character*(*) navn(npos)
      character*(*) vsfile
      character*(*) vtable
c
      parameter (maxsyn=1000)
      character*30 snavn(maxsyn),stemp
      integer      isnum(maxsyn),lsnavn(maxsyn),iantsyn
      integer      init
c
      parameter  (mverpar=6)
      integer     iverpar(mverpar),idatnum(mverpar)
      character*4  verpar(mverpar),verparout(mverpar)
c
      data  verpar/'   Z', '  DD', '  FF', '  TT',
     *             '  TD', '  UU'/
      data iverpar/    1,      2,      3,      4,
     *                 5,     10/
c
      data init/0/
c
      save snavn, isnum, lsnavn, iantsyn
c
c
      ierror=1
c
      if(init.eq.0) then
c..leser fil med synop-nr.
        if(vsfile(1:1).eq.'*') then
          write(6,*)'***prverifi*** no station file specified'
          ios=-1
        else
          open(iunit,file=vsfile,
     *               access='sequential',form='formatted',
     *               status='old',iostat=ios)
          if(ios.ne.0) then
            write(6,*) '***prverifi*** : open error.  station file:'
            write(6,*) vsfile
          end if
        end if
        if(ios.eq.0) then
          mlnavn=len(snavn(1))
          iasyn=0
          isyno=1
          do while (isyno.gt.0)
            isyno=0
            read(iunit,*,iostat=ios) isyno,stemp
            if(ios.eq.0 .and. isyno.gt.0) then
              iasyn=iasyn+1
              if(iasyn.le.maxsyn) then
                snavn(iasyn)=stemp
                isnum(iasyn)=isyno
                lsnavn(iasyn)=1
                do k=1,mlnavn
                  if(snavn(iasyn)(k:k).ne.' ') lsnavn(iasyn)=k
                end do
              end if
            elseif(ios.ne.0) then
              write(6,*) '***prverifi*** : read error.  station file:'
              write(6,*) vsfile
              write(6,*) '    iostat= ',ios,'  at line no. ',iasyn+1
              isyno=0
            end if
          end do
          close(iunit)
          if(iasyn.gt.maxsyn) then
            write(6,*) '***prverifi*** : too many stations on file:'
            write(6,*) vsfile
            write(6,*) '    no. found on file:  ',iasyn
            write(6,*) '    max (the no. used): ',maxsyn
            iasyn=maxsyn
          end if
          iantsyn=iasyn
          init=1
        else
          iantsyn=-1
          init=0
        end if
      end if
c
      rmiss = -999.0
c
      ipniv=0
      do i=1,npar
        if(iparam(i).eq.8) ipniv=i
      end do
c
      if(ipniv.eq.0) then
        write(6,*) '***prverifi*** pressure (parameter 8) not found.'
        return
      end if
c
      nverpar=0
      nuu=0
      nvv=0
      do n=1,mverpar
        k=0
        do i=1,npar
          if(iparam(i).eq.iverpar(n)) k=i
        end do
        if(k.gt.0) then
          nverpar=nverpar+1
          idatnum(nverpar)=k
          verparout(nverpar)=verpar(n)
          if(iverpar(n).eq.2) nuu=nverpar
          if(iverpar(n).eq.3) nvv=nverpar
        end if
      end do
c..don't even try to overload the hlpdat array
      if(nverpar.gt.npar) nverpar=npar
c
      if(nverpar.lt.1) then
        write(6,*) '***prverifi*** no suitable data for verification.'
        return
      end if
c
c..valid time
      nerr=0
      do n=1,ntid
        do i=1,5
          itimev(i,n)=itime(i,n)
        end do
        call vtime(itimev(1,n),ierr)
        if(ierr.ne.0) then
          write(6,*) '***prverifi*** : illegal date,time,prog:'
          write(6,*) (itime(i,n),i=1,5)
          nerr=nerr+1
        end if
        itimev(5,n)=itime(5,n)
      end do
      if(nerr.gt.0) return
c
      write(6,*) 'prverifi'
      write(6,*) 'file: ',filnam
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      write(iunit,*) 'VERIFI 200'
c
      if(vtable(1:1).eq.'*') then
        write(iunit,*) tekst(1)(1:ktekst(1,1))
      else
        write(iunit,*) vtable(1:lenstr(vtable,1))
      end if
c
      do 100 np=1,npos
        ipos=numpos(np)
c
        isyno=0
        i=0
        do while (isyno.eq.0 .and. i.lt.iantsyn)
          i=i+1
          if(navn(np).eq.snavn(i)) isyno=i
        end do
        if (isyno.eq.0) then
          if(iantsyn.ne.-1)
     *       write(6,*) '***prverifi*** no number for ',navn(np)
          stemp = navn(np)
          isyno = 9999
        else
          stemp = snavn(isyno)
          isyno = isnum(isyno)
        end if
c
        write(iunit,'(1x,i6,2x,a30)') isyno,stemp
c
c..hvis tidspunktet skal vaere prognose-start
ccc     write(iunit,'(1x,i4,3i3)') (itime(i,1),i=1,4)
c..hvis tidspunktet skal vaere foerste tid med data
ccc        write(iunit,'(1x,i4,3i3)') (itimev(i,1),i=1,4)
c
        write(iunit,'(1x,6a5,12(6x,a4,:))')
     *              'AAR','MND','DAG','TIM','PROG','NIVA',
     *              (verparout(n),n=1,nverpar)
c
        do n=1,nverpar
          i=idatnum(n)
          do l=1,ntid
            do k=1,nniv
              hlpdat(n,k,l)=dat(k,i,l,ipos)*skalp(i)
            end do
          end do
        end do
c
        if(nuu.gt.0 .and. nvv.gt.0) then
c..from u(e/w),v(n/s) in knots to dd,ff(m/s)
          grad=180./3.1415927
          toms=1852./3600.
          do l=1,ntid
            do k=1,nniv
              uew=hlpdat(nuu,k,l)
              vns=hlpdat(nvv,k,l)
              ff=sqrt(uew*uew+vns*vns)*toms
              if(ff.gt.0.01) then
                dd=270.-grad*atan2(vns,uew)
                if(dd.gt.360.) dd=dd-360.
                if(dd.le.  0.) dd=dd+360.
              else
                dd=0.
                ff=0.
              endif
              hlpdat(nuu,k,l)=dd
              hlpdat(nvv,k,l)=ff
            end do
          end do
        end if
c
        do l=1,ntid
          do k=1,nniv
c
            p=dat(k,ipniv,l,ipos)*skalp(ipniv)
            nivp=nint(p)
c
            write(iunit,fmt='(1x,6(i5),12(1x,f9.2,:))')
     *                  (itimev(i,l),i=1,5),nivp,
     *                  (hlpdat(n,k,l),n=1,nverpar)
c
          end do
        end do
c
c..end of station
        write(iunit,fmt='(1x,6(i5),12(1x,f9.2,:))')
     *              9999,99,99,99,99,9999,
     *              (rmiss,i=1,nverpar)
c
  100 continue
c
c..end of file
      stemp='***********'
      write(iunit,'(1x,i6,2x,a30)') 99999,stemp
c
      close(iunit)
      ierror=0
c
      return
c
  900 write(6,*) '***prverifi*** open error.   iostat= ',ios
      write(6,*)  filnam
c
      return
      end
