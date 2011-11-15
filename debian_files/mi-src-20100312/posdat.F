      program posdat
c
c        ***** utplukk av posisjons-data (tidsserier)         ******
c        ***** meteogram (mi,ecmwf), bolgediagram, havdiagram ******
c        ***** evt. konvertering til annen maskin-type        ******
c
c        punkt-verdier fra/til "lager"-filer.
c        (en input-file, en eller flere output-filer)
c
c        NB! det nye 'binary.output....' formatet kan IKKE leses inn!
c
c
c-----------------------------------------------------------------------
c posdat.input   example:
c=======================================================================
c *** posdat.lam50s  ('posdat.input')
c ***
c *=> Meteogram data from LAM50S.
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    posdat  posdat.lam50s  pool50s.dat  standard
c *=>                           <input>      <format>
c ***
c ***------------------------------------------------------------------
c **
c ** Options:
c ** --------
c ** FILE=<input_data_file>
c ** ADD.DATA.FROM.FILE=<input_data_file>
c ** ADD.PARAMETER=<param_in,param_add,param_in,param_add,...>
c ** DATA.METEOGRAM
c ** DATA.WAVE
c ** DATA.SEA
c ** BINARY.OUTPUT.SORT.POS-PARAM-TIME .............. (default)
c ** BINARY.OUTPUT.SORT.TIME-PARAM-POS
c ** FORMAT.STANDARD ................................ (default)
c ** FORMAT.SWAP
c ** FORMAT.PC
c ** FORMAT.IBM
c ** FORMAT.PRINT_AUTOMAT_1
c ** FORMAT.PRINT_NHLVIND
c ** FORMAT.PRINT_VEI_1
c ** FORMAT.PRINT_VEST
c ** FORMAT.PRINT_ICECAST
c ** FORMAT.PRINT_VERIFI
c ** VERIFI.STATION.FILE=<file_name> ....... for 'format.print_verifi'
c ** VERIFI.TABLE.NAME=<table_name>  ....... for 'format.print_verifi'
c ** FORMAT.PRINT_NORMEM
c ** FORMAT.PRINT_MULTIUSE
c ** FORMAT.PRINT_MULTIUSE+POS
c ** FORMAT.PRINT_SDV
c ** FORMAT.PRINT_PARAM ... .. print parameters in "print_param=..."
c ** PRINT_PARAM= parnum, "name", "format", "description"
c ** NAME_SUFFIX=%   ....................... for corrected meteograms
c ** POS_PARAM.ON ................... default if meteogogram and not pc
c ** POS_PARAM.OFF .................. default otherwise
c ** PROG_LIMIT=<min_prog_hour,max_prog_hour> ... (default = no limit)
c ** PROG_OUT=<prog_hour,prog_hour,...> ......... (default: all)
c ** PROG.HOUR.ADD=<+/-hours> ................... (valid time unchanged)
c ** WHEN.DELETE.TIME.ADD.PARAM= <param,...> (default: 15,16,17,19,20)
c ** PARAM_OUT=<param,param,..............> ..... (default: all)
c ** CHANGE_PARAM_NUMBER= <param,param_new, param,param_new,...>
c ** NEW.TEXT.OUT= <one text line> .............. (repeatable)
c ** ADD.TEXT.OUT= <one text line> .............. (repeatable)
c ** LIST_INPUT.ALL
c ** LIST_INPUT.SOME
c ** LIST_USED
c ** LIST_NOT_USED
c ** END
c **
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c ** Options    Remember '......' syntax.
c ** ($... = environment var. ; #n = coomand line arg. no. n)
c *>
c 'FILE= #2'               <<< 'FILE= pool50s.dat'
c 'DATA.METEOGRAM'
c 'FORMAT.#3'              <<< 'FORMAT.STANDARD'
c 'NAME_SUFFIX=%'
c 'END'
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c **
c **
c *>
c '>>>>>>>>>>>>', 'met1.dat'  <<<--------------- output file
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
c '>>>>>>>>>>', 'met2.dat'  <<<----------------- output file
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
c        div. max. antall er satt i 'parameter':
c          maxpos - posisjoner/stasjoner (input)
c          maxtid - tidspunkt
c          maxpar - parametre
c          maxout - max antall stasjoner output (hvis ikke alle)
c
c------------------------------------------------------
c  parametre (for standard presentasjon):
c ----------------------------------------
c  meteogram (mi): 1 - trykk
c                  2 - temperatur
c                  3 - frontal nedb@r
c                  4 - konvektiv nedb@r
c                  5 - u
c                  6 - v
c                  7 - skydekke  -  t$ke (stratus/fog)
c                  8 - skydekke  -  cl (lave)
c                  9 - skydekke  -  cm (midlere)
c                 10 - skydekke  -  ch (hoye)
c
c  meteogram (ec): 1 - trykk
c                  2 - temperatur
c                  3 - total nedb@r
c                  4 - u
c                  5 - v
c                  6 - skydekke  -  totalt
c
c  wave:           1 - ff,  vind    (knop)
c                  2 - dd,  vindretning (grader relativt nord)
c                  3 - hst, signifikant b@lgeh@yde ......... totalsj@
c                  4 - tst, signifikant periode
c                  5 - hsp, signifikant b@lgeh@yde ......... vindsj@
c                  6 - hsp, periode med max. energi
c                  7 - ddpp,-retning (grader relativt nord)
c                  8 - hsd, signifikant b@lgeh@yde ......... d@nning
c                  9 - hsd, periode med max. energi
c                 10 - ddpd,-retning (grader relativt nord)
c
c  sea:            1 - vannstand (overflate-hevning) (m)
c                  2 - u, str@m-komponent x-retning (m/s)
c                  3 - v, str@m-komponent y-retning (m/s)
c------------------------------------------------------
c
c-----------------------------------------------------------------------
c
c      DNMI library subroutines:  rlunit
c                                 rcomnt
c                                 getvar
c                                 keywrd
c                                 chcase
c                                 prhelp
c                                 hrdiff
c                                 vtime
c                                 rmfile
c                                 bget*
c                                 bput*
c
c
c=======================================================================
c  FILE=<input_data_file>
c  ADD.DATA.FROM.FILE=<input_data_file>
c  ADD.PARAMETER=<param_in,param_add,param_in,param_add,...>
c  DATA.METEOGRAM
c  DATA.WAVE
c  DATA.SEA
c  BINARY.OUTPUT.SORT.POS-PARAM-TIME .............. (default)
c  BINARY.OUTPUT.SORT.TIME-PARAM-POS
c  FORMAT.STANDARD ................................ (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  FORMAT.PRINT_AUTOMAT_1
c  FORMAT.PRINT_NHLVIND
c  FORMAT.PRINT_VEI_1
c  FORMAT.PRINT_VEST
c  FORMAT.PRINT_ICECAST
c  FORMAT.PRINT_VERIFI
c  VERIFI.STATION.FILE=<file_name> .......... for 'format.print_verifi'
c  VERIFI.TABLE.NAME=<table_name>  .......... for 'format.print_verifi'
c  FORMAT.PRINT_NORMEM
c  FORMAT.PRINT_MULTIUSE
c  FORMAT.PRINT_MULTIUSE+POS
c  FORMAT.PRINT_SDV
c  FORMAT.PRINT_PARAM ... .. print parameters in "print_param=..."
c  PRINT_PARAM= parnum, "name", "format", "description"
c  NAME_SUFFIX=%   .......................... for corrected meteograms
c  POS_PARAM.ON ..................... default if meteogogram and not pc
c  POS_PARAM.OFF .................... default otherwise
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> ..... (default = no limit)
c  PROG_OUT=<prog_hour,prog_hour,...> ......... (default: all)
c  WHEN.DELETE.TIME.ADD.PARAM= <param,...> (default: 15,16,17,19,20)
c  PARAM_OUT=<param,param,..............> ....... (default: all)
c  CHANGE_PARAM_NUMBER= <param,param_new, param,param_new,...>
c  NEW.TEXT.OUT= <one text line> ................ (repeatable)
c  ADD.TEXT.OUT= <one text line> ................ (repeatable)
c  LIST_INPUT.ALL
c  LIST_INPUT.SOME
c  LIST_USED
c  LIST_NOT_USED
c  END
c=======================================================================
c
c-----------------------------------------------------------------------
c  DNMI/FoU  xx.03.1988  Anstein Foss, L-A Breivik ... IBM
c  DNMI/FoU  xx.12.1990  Anstein Foss
c  DNMI/FoU  xx.04.1991  Anstein Foss
c  DNMI/VA   xx.07.1992  Audun Christoffersen . kalman-suffix i met.navn
c  DNMI/FoU  22.12.1992  Anstein Foss ......... Unix
c  DNMI/VA   31.03.1993  Audun Christoffersen . print_nhlvind
c  DNMI/VA   10.11.1993  Audun Christoffersen . print_verifi test code
c  DNMI/FoU  15.12.1993  Anstein Foss ......... print_vei_1
c  DNMI/FoU  18.02.1994  Anstein Foss ......... print_verifi
c  DNMI/FoU  12.03.1994  Anstein Foss
c  DNMI/VA   10.11.1994  Audun Christoffersen . print_icecast
c  DNMI/FoU  02.01.1995  Anstein Foss
c  DNMI/VA   11.01.1995  Audun Christoffersen . icecast/verifi
c  DNMI/FoU  07.04.1995  Anstein Foss ......... bget* and bput* routines
c  DNMI/FoU  05.05.1995  Anstein Foss ......... add.data.from.file
c  DNMI/FoU  13.06.1995  Anstein Foss ......... Hirlam precip in pr....
c  DNMI/FoU  02.10.1995  Anstein Foss ......... print_normem
c  DNMI/VA   25.01.1996  A Christoffersen ..... print_multiuse
c  DNMI/FoU  04.03.1996  Anstein Foss ......... corrected verifi clouds
c  DNMI/FoU  04.07.1996  Anstein Foss ......... prog_out=
c  DNMI/FoU  28.01.1997  Anstein Foss ......... binary.output.sort
c  DNMI/FoU  07.01.1998  Jan Erik Haugen ...... print_vest
c  DNMI/FoU  16.03.1999  Anstein Foss ......... CHANGE_PARAM_NUMBER=...
c  DNMI/FoU  28.04.2000  A Christoffersen, L B Sveen . RH in print_multiuse
c  DNMI/FoU  28.04.2000  Lars B Sveen ......... print_sdv
c  DNMI/FoU  30.06.2001  Anstein Foss ......... print_param
c  DNMI/FoU  15.01.2002  Anstein Foss ......... more strict name matching
c  DNMI/FoU  21.01.2002  Anstein Foss ......... icecast names without suffix
c  DNMI/FoU  03.06.2003  Anstein Foss ... format.standard always bigendian
c  DNMI/FoU  04.06.2003  Anstein Foss ... file signature 211,212 (first word)
c  DNMI/FoU  12.11.2003  Anstein Foss ... print_multiuse+pos
c  DNMI/FoU  12.01.2004  Anstein Foss ... liten endring print_multiuse(+pos)
c  DNMI/FoU  19.04.2004  Anstein Foss ... print_multiuse+pos: ff in m/s
c  DNMI/FoU  10.02.2005  Anstein Foss ... prog.hour.add=HH (for Diana PROG+HH)
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c-----------------------------------------------------------------------
c
c
      include 'posdat.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for posdat.f
c
c  maxpos - max antall input        posisjoner/stasjoner
c  maxpar - max antall input/output parametre
c  maxtid - max antall input/output tidspunkt
c  maxdat - max total  input        datamengde (npos*npar*ntid)
c  mtekst - max antall input/output tekst-linjer
c  mxpar  - max antall input/output ekstra parametre (ikke tidsavhengig,
c                                   posisjon og/eller 'parameter'-felt)
c  maxout - max antall output       posisjoner/stasjoner, en file
c  maxdt1 - max total  output       datamengde (npos*npar*ntid), en file
c  maxall - max antall output       posisjoner/stasjoner, alle filer
c  maxfil - max antall output       filer
c  madfil - max antall input        filer for 'add.data.from.file'
c  madpar - max antall input        parametre for 'add.parameter'
c  mparprt- max antall "print_param=..."
c
c..input
ccc   parameter (maxpos=2000,maxpar=20,maxtid=90)
ccc   parameter (maxdat=800000)
ccc   parameter (mtekst=10,mxpar=8)
ccc   parameter (madfil=8,madpar=16)
c
c..en output-file
ccc   parameter (maxout=200)
ccc   parameter (maxdt1=100000)
c
c..alle output-filer
ccc   parameter (maxall=2000,maxfil=60)
c
ccc   parameter (mparprt=50)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c..buffer and file record length
      parameter (lbuff=512)
c
      integer*2 buff(lbuff)
c
      integer   npos,npar,ntid,ntidin,linfo,iopt
      integer   itime(5,maxtid),info(40)
      integer   ipcwave,ivtime(5,maxtid)
      integer   konpar(maxpar),iskal(maxpar)
      integer   konprx(mxpar),ixskal(mxpar)
      integer*2 dat(maxdat),xdat(maxpos*mxpar)
      real      skal(maxpar),xskal(mxpar)
c
      integer   iprog(maxtid+1),ideladd(maxpar+1),iprout(maxtid)
      integer   nused(maxall)
      integer   iphouradd
c
      integer   ltekst,ktekst(2,mtekst),ntnavn(maxpos),nsuff(maxpos)
      character*80 tekst(mtekst)
      character*30 navn(maxpos),navns(maxpos),suffix(maxpos)
c
      integer   ntnav1(maxout),ntnav3(maxall),numpos(maxout)
      integer*2 dat1(maxdt1),xdat1(maxpos*mxpar)
      character*30 navn1(maxout),navn1s(maxout)
      character*30 navn2(maxall),navn3(maxall)
c
      integer listot(2,maxfil)
c
      integer itimev(5,maxtid),ntimev(maxtid)
      real    hlpdat(maxpar*maxtid)
c
      character*1  csfix
c
      integer iparm(maxpar),jparm(maxpar),ichparm(2,maxpar)
      integer ltekstn,ktekstn(2,mtekst)
      character*80 tekstn(mtekst)
c
      character txtin1*30, txtin2*60
c
      character*256 filein,fileot(maxfil),vsfile,vtable
c
      integer   iadpar(2,madpar+1),jadpar(2,madfil)
      integer   infoad(40),itimead(5,maxtid)
      integer   konparad(maxpar),iskalad(maxpar)
      integer*2 datad(maxpos)
      character*256 addfil(madfil)
      character*30  adnavn(maxpos)
      character*1   tchar
c
      integer      iparprt(mparprt)
      character*16 tparprt(mparprt)
      character*8  fparprt(mparprt)
      character*80 dparprt(mparprt)
c
      integer      jparprt(mparprt),kparprt(mparprt),jparam(maxpar)
      integer      kfprt(6)
c
c..resorted data binary files
      parameter (lhead=16)
      integer   iheadf(8),ihead(lhead)
      integer*2 isorted(maxtid*maxpar)
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
c..get machine dependant termination character for free format read
c..(machine dependant)
      call termchar(tchar)
c
c..file unit for 'posdat.input'
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
        write(6,*) '   usage: posdat <posdat.input>'
        write(6,*) '      or: posdat <posdat.input> <arguments>'
        write(6,*) '      or: posdat <posdat.input> ?     (to get help)'
        write(6,*)
        stop 1
      end if
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error:'
        write(6,*) finput(1:lenstr(finput,1))
        stop 1
      end if
c
      if(narg.eq.2) then
        call getarg(2,cinput)
        if(cinput.eq.'?') then
          call prhelp(iuinp,'*=>')
          close(iuinp)
          stop 1
        end if
      end if
c
c
      write(6,*) 'reading input file:'
      write(6,*) finput(1:lenstr(finput,1))
c
      nlines = 0
c
      ibsort =-1
      iformt =-1
      linfo  = 0
      ipospa =-1
      lstdat = 0
      lstusd = 0
      lstnot = 0
      nparm  = 0
      nchparm= 0
      newtxt = 0
      ltekstn= 0
c
      iphouradd=0
      ipcwave=0
c
      csfix  =' '
c
      filein ='*'
c
c..for format.print_verifi ... station file
      vsfile='*'
c..for format.print_verifi ... table name
      vtable='*'
c
      iprmin = -32767
      iprmax = +32767
      nprog  = 0
      ndeladd= 0
      nadfil = 0
      nadpar = 0
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
c..check if input as environment variables or command line arguments
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
c======================================================================
c  FILE=<input_data_file>
c  ADD.DATA.FROM.FILE=<input_data_file>
c  ADD.PARAMETER=<param_in,param_add,param_in,param_add,...>
c  DATA.METEOGRAM
c  DATA.WAVE
c  DATA.SEA
c  BINARY.OUTPUT.SORT.POS-PARAM-TIME .............. (default)
c  BINARY.OUTPUT.SORT.TIME-PARAM-POS
c  FORMAT.STANDARD ................................ (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  FORMAT.PRINT_AUTOMAT_1
c  FORMAT.PRINT_NHLVIND
c  FORMAT.PRINT_VEI_1
c  FORMAT.PRINT_VEST
c  FORMAT.PRINT_ICECAST
c  FORMAT.PRINT_VERIFI
c  VERIFI.STATION.FILE=<file_name> .......... for 'format.print_verifi'
c  VERIFI.TABLE.NAME=<table_name>  .......... for 'format.print_verifi'
c  FORMAT.PRINT_NORMEM
c  FORMAT.PRINT_MULTIUSE
c  FORMAT.PRINT_MULTIUSE+POS
c  FORMAT.PRINT_SDV
c  FORMAT.PRINT_PARAM ... .. print parameters in "print_param=..."
c  PRINT_PARAM= parnum, "name", "format", "description"
c  NAME_SUFFIX=%   .......................... for corrected meteograms
c  POS_PARAM.ON ..................... default if meteogogram and not pc
c  POS_PARAM.OFF .................... default otherwise
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> ..... (default = no limit)
c  PROG_OUT=<prog_hour,prog_hour,...> ......... (default: all)
c  PROG.HOUR.ADD=<+/-hours> ................... (valid time unchanged)
c  WHEN.DELETE.TIME.ADD.PARAM= <param,...> (default: 15,16,17,19,20)
c  PARAM_OUT=<param,param,..............> ....... (default: all)
c  CHANGE_PARAM_NUMBER= <param,param_new, param,param_new,...>
c  NEW.TEXT.OUT= <one text line> ................ (repeatable)
c  ADD.TEXT.OUT= <one text line> ................ (repeatable)
c  LIST_INPUT.ALL
c  LIST_INPUT.SOME
c  LIST_USED
c  LIST_NOT_USED
c  END
c======================================================================
c
          if(cinput(k1:k2).eq.'file') then
c..file=<input_data_file>
            if(filein(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            filein=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'add.data.from.file') then
c..add.data.from.file=<input_data_file>
	    nadfil=nadfil+1
	    if(nadfil.gt.madfil) goto 231
            if(kv1.lt.1) goto 213
	    addfil(nadfil)=cinput(kv1:kv2)
	    jadpar(1,nadfil)=nadpar+1
	    jadpar(2,nadfil)=nadpar
          elseif(cinput(k1:k2).eq.'add.parameter') then
c..add.parameter=<param_in,param_add,param_in,param_add,...>
            if(kv1.lt.1) goto 213
            if(nadfil.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nadpar+1
            i2=i1-1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.madpar) goto 231
              i2=i2+1
              read(cipart,*,iostat=ios) ((iadpar(j,i),j=1,2),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nadpar=i2
	    jadpar(2,nadfil)=nadpar
          elseif(cinput(k1:k2).eq.'data.meteogram') then
c..data.meteogram
            if(linfo.ne.0) goto 214
            linfo=11
          elseif(cinput(k1:k2).eq.'data.wave') then
c..data.wave
            if(linfo.ne.0) goto 214
            linfo=8
	    ipcwave=1
          elseif(cinput(k1:k2).eq.'data.sea') then
c..data.sea
            if(linfo.ne.0) goto 214
            linfo=8
          elseif(cinput(k1:k2).eq.
     +		'binary.output.sort.pos-param-time') then
c..binary.output.sort.pos-param-time
            if(ibsort.ne.-1) goto 214
            ibsort=0
          elseif(cinput(k1:k2).eq.
     +		'binary.output.sort.time-param-pos') then
c..binary.output.sort.time-param-pos
            if(ibsort.ne.-1) goto 214
            ibsort=1
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
          elseif(cinput(k1:k2).eq.'format.print_automat_1') then
c  format.print_automat_1
            if(iformt.ne.-1) goto 214
            iformt=1001
          elseif(cinput(k1:k2).eq.'format.print_nhlvind') then
c  format.print_nhlvind
            if(iformt.ne.-1) goto 214
            iformt=1010
          elseif(cinput(k1:k2).eq.'format.print_vei_1') then
c  format.print_vei_1
            if(iformt.ne.-1) goto 214
            iformt=1021
          elseif(cinput(k1:k2).eq.'format.print_vest') then
c  format.print_vest
            if(iformt.ne.-1) goto 214
            iformt=1022
          elseif(cinput(k1:k2).eq.'format.print_icecast') then
c  format.print_icecast
            if(iformt.ne.-1) goto 214
            iformt=1025
          elseif(cinput(k1:k2).eq.'format.print_verifi') then
c  format.print_verifi
            if(iformt.ne.-1) goto 214
            iformt=1030
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
          elseif(cinput(k1:k2).eq.'format.print_normem') then
c  format.print_normem
            if(iformt.ne.-1) goto 214
            iformt=1040
          elseif(cinput(k1:k2).eq.'format.print_multiuse+pos') then
c  format.print_multiuse+pos
            if(iformt.ne.-1) goto 214
            iformt=1051
          elseif(cinput(k1:k2).eq.'format.print_multiuse') then
c  format.print_multiuse
            if(iformt.ne.-1) goto 214
            iformt=1050
          elseif(cinput(k1:k2).eq.'format.print_sdv') then
c  format.print_sdv
            if(iformt.ne.-1) goto 214
            iformt=1055
          elseif(cinput(k1:k2).eq.'format.print_param') then
c  format.print_param
            if(iformt.ne.-1) goto 214
            iformt=1060
          elseif(cinput(k1:k2).eq.'print_param') then
c  print_param= parnum, "name", "format", "description"
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
          elseif(cinput(k1:k2).eq.'name_suffix') then
c..name_suffix=
            if(kv1.lt.1) goto 213
            if(kv1.ne.kv2) goto 213
            if(csfix.ne.' ') goto 214
            csfix=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'pos_param.on') then
c..pos_param.on
            if(ipospa.ne.-1) goto 214
            ipospa=1
          elseif(cinput(k1:k2).eq.'pos_param.off') then
c..pos_param.off
            if(ipospa.ne.-1) goto 214
            ipospa=0
          elseif(cinput(k1:k2).eq.'prog_limit') then
c..prog_limit=<min_prog_hour,max_prog_hour>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) iprmin,iprmax
            if(iprmin.gt.iprmax) goto 213
          elseif(cinput(k1:k2).eq.'prog_out') then
c..prog_out=<prog_hour_1,prog_hour_2,...>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
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
          elseif(cinput(k1:k2).eq.'prog.hour.add') then
c..prog.hour.add=<+/-hours> ................... (valid time unchanged)
            if(kv1.lt.1) goto 213
            if(iphouradd.ne.0) goto 214
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) iphouradd
          elseif(cinput(k1:k2).eq.'when.delete.time.add.param') then
c  when.delete.time.add.param= <param,...> (default: 15,16,17,19,20)
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=ndeladd+1
            i2=i1-1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxpar) goto 231
              i2=i2+1
              read(cipart,*,iostat=ios) (ideladd(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            ndeladd=i2
          elseif(cinput(k1:k2).eq.'param_out') then
c..param_out=
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
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
          elseif(cinput(k1:k2).eq.'change_param_number') then
c  change_param_number= <param,param_new, param,param_new,...>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nchparm+1
            i2=i1-1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxpar) goto 231
              i2=i2+1
              read(cipart,*,iostat=ios) ((ichparm(j,i),j=1,2),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nchparm=i2
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
          elseif(cinput(k1:k2).eq.'list_input.all') then
c..list_input.all
            lstdat=2
          elseif(cinput(k1:k2).eq.'list_input.some') then
c..list_input.some
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
          end if
c
        end do
c
      end do
c
c..file=
      if(filein(1:1).eq.'*') goto 216
c..data.xxxxxx=
      if(linfo.eq.0) goto 217
c
c..default output file format
      if(iformt.lt.0) iformt=0
c
c.."valid time" on file only if DATA.WAVE and FORMAT.PC
      if(iformt.ne.2) ipcwave=0
c
      if(iformt.eq.1060 .and. nparprt.lt.1) goto 218
c
c..default pos_param.on/off
      if(ipospa.lt.0) then
        ipospa=1
        if(iformt.eq.2 .or. linfo.ne.11) ipospa=0
        if(iformt.gt.1000) ipospa=0
        if(iformt.eq.1051) ipospa=1
      end if
c
c..default binary output sort
      if(ibsort.lt.0) ibsort=0
      if(iformt.gt.4) ibsort=0
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
            end if
          end if
        end if
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
  217 write(6,*) 'no input data type specified.'
      goto 240
  218 write(6,*) 'format.print_param but no "print_param=..."'
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
        write(6,*) 'help from ''posdat.input'':'
        call prhelp(iuinp,'*=>')
      end if
      close(iuinp)
      stop 1
c
  250 close(iuinp)
c
      if(ndeladd.eq.0) then
	ideladd(1)=17
	if(maxpar.ge.2) ideladd(2)=19
	if(maxpar.ge.3) ideladd(3)=20
	if(maxpar.ge.4) ideladd(4)=15
	if(maxpar.ge.5) ideladd(5)=16
	ndeladd=min(maxpar,5)
      end if
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
      write(6,*)  filein(1:lenstr(filein,1))
c
c        max antall tegn i fast tekst
      mtt=len(tekst(1))
c        max antall tegn i hvert navn
      mtn=len(navn(1))
c
      open(iunit,file=filein,
     +           access='direct',form='unformatted',
     +           recl=(lbuff*2)/lrunit,
#ifndef G77
     +           action='read',
#endif
     +           status='old',iostat=ios,err=920)
c
      irec=0
      ibuff=lbuff
c
      call bgeti4(iunit,irec,buff,lbuff,ibuff,info,linfo,ierr)
      if(ierr.ne.0) goto 960
c
ccc   nrec=  info(1)
      idinp= info(1)
      npos=  info(2)
      npar=  info(3)
      ntid=  info(4)
      ntidin=info(5)
      ltekst=info(6)
      ntt=   info(7)
      ntn=   info(8)
ccc.....................bare hvis meteogram type data (fra metdat)
ccc   iprod= info(9)
ccc   nrgrid=info(10)
ccc   interp=info(11)
c
      iprod=0
      if(linfo.ge.9) iprod=info(9)
c
      ndat=npos*npar*ntid
c
c        sjekk input
      k=0
      if(npos.lt.1 .or. npos.gt.maxpos) then
        write(6,*) ' *** antall posisjoner:',npos
        write(6,*) ' ***    min,max lovlig:  1 ',maxpos,'  (maxpos)'
        k=1
      end if
      if(npar.lt.1 .or. npar.gt.maxpar) then
        write(6,*) ' *** antall parametre: ',npar
        write(6,*) ' ***   min,max lovlig:   1 ',maxpar,'  (maxpar)'
        k=1
      end if
      if(ntid.lt.1 .or. ntid.gt.maxtid) then
        write(6,*) ' *** antall tidspunkt: ',ntid
        write(6,*) ' ***    min,max lovlig:  1 ',maxtid,'  (maxtid)'
        k=1
      end if
      if(ntt.ne.mtt) then
        write(6,*) ' *** lengde av ''tekst'' input:       ',ntt
        write(6,*) ' *** lengde av ''tekst'' tillatt her: ',mtt
        k=1
      end if
      if(ntn.ne.mtn) then
        write(6,*) ' *** lengde av ''navn'' input:       ',ntn
        write(6,*) ' *** lengde av ''navn'' tillatt her: ',mtn
        k=1
      end if
      if(ndat.gt.maxdat) then
        write(6,*) ' *** lengde av data input:       ',ndat
        write(6,*) ' *** lengde av data tillatt her: ',maxdat
        k=1
      end if
c
      if(k.eq.1) goto 960
c
      write(6,*) '    npos,npar,ntid: ',npos,npar,ntid
c
      call bgetch(iunit,irec,buff,lbuff,ibuff,tekst,ltekst,ntt,ierr)
      if(ierr.ne.0) goto 960
      call bgeti4(iunit,irec,buff,lbuff,ibuff,ktekst,2*ltekst,ierr)
      if(ierr.ne.0) goto 960
      call bgetch(iunit,irec,buff,lbuff,ibuff,navn,npos,ntn,ierr)
      if(ierr.ne.0) goto 960
      call bgeti4(iunit,irec,buff,lbuff,ibuff,ntnavn,npos,ierr)
      if(ierr.ne.0) goto 960
      call bgeti4(iunit,irec,buff,lbuff,ibuff,itime,5*ntid,ierr)
      if(ierr.ne.0) goto 960
      call bgeti4(iunit,irec,buff,lbuff,ibuff,konpar,npar,ierr)
      if(ierr.ne.0) goto 960
      call bgeti4(iunit,irec,buff,lbuff,ibuff,iskal,npar,ierr)
      if(ierr.ne.0) goto 960
c
      call bgeti2(iunit,irec,buff,lbuff,ibuff,dat,ndat,ierr)
      if(ierr.ne.0) goto 960
c
      nxpar=0
      if(ipospa.eq.1) then
        i=0
        call bgeti4(iunit,irec,buff,lbuff,ibuff,i,1,ierr)
        if(ierr.eq.0 .and. i.eq.1001) then
          call bgeti4(iunit,irec,buff,lbuff,ibuff,nxpar,1,ierr)
          if(ierr.eq.0 .and. nxpar.gt.mxpar) then
            write(6,*) 'too many (extra) position parameters input'
            write(6,*) '    input: ',nxpar,'   max: ',mxpar
            nxpar=0
          elseif(ierr.eq.0 .and. nxpar.gt.0) then
            call bgeti4(iunit,irec,buff,lbuff,ibuff,konprx,nxpar,ierr)
            if(ierr.ne.0) goto 960
            call bgeti4(iunit,irec,buff,lbuff,ibuff,ixskal,nxpar,ierr)
            if(ierr.ne.0) goto 960
            nxdat=npos*nxpar
            call bgeti2(iunit,irec,buff,lbuff,ibuff,xdat,nxdat,ierr)
            if(ierr.ne.0) goto 960
          end if
        end if
        if(nxpar.eq.0) write(6,*) 'no (extra) position parameters input'
      end if
c
      close(iunit)
c
c..stripper suffixet fra alle navn.
      call strsfix(csfix,npos,ntnavn,navn,nsuff,navns,suffix)
c
c.......................................................................
c..add.data.from.file...................................................
c
      do 280 iadf=1,nadfil
c
      write(6,*) 'add data from file:'
      write(6,*)  addfil(iadf)(1:lenstr(addfil(iadf),1))
c
      open(iunit,file=addfil(iadf),
     *           access='direct',form='unformatted',
     *           recl=(lbuff*2)/lrunit,
     *           status='old',iostat=ios)
      if(ios.ne.0) then
	write(6,*) '     open error.  iostat= ',ios
	goto 280
      end if
c
      irec=0
      ibuff=lbuff
c
      call bgeti4(iunit,irec,buff,lbuff,ibuff,infoad,linfo,ierr)
      if(ierr.ne.0) goto 295
c
      nposad=  infoad(2)
      nparad=  infoad(3)
      ntidad=  infoad(4)
      ltekstad=infoad(6)
      nttad=   infoad(7)
      ntnad=   infoad(8)
      if(nposad.ne.npos .or. ntidad.ne.ntid .or. ntnad.ne.ntn) then
        write(6,*) '   not able to add data'
	write(6,*) '     (main. npos,ntid,ntn: ',npos  ,ntid  ,ntn  ,')'
	write(6,*) '     (add.  npos,ntid,ntn: ',nposad,ntidad,ntnad,')'
	goto 295
      end if
c
      nttad2=nttad/2
c
      call bgetjp(iunit,irec,buff,lbuff,ibuff,ltekstad*nttad2,ierr)
      if(ierr.ne.0) goto 295
      call bgetjp(iunit,irec,buff,lbuff,ibuff,2*ltekstad,ierr)
      if(ierr.ne.0) goto 295
      call bgetch(iunit,irec,buff,lbuff,ibuff,adnavn,npos,ntnad,ierr)
      if(ierr.ne.0) goto 295
      call bgetjp(iunit,irec,buff,lbuff,ibuff,nposad,ierr)
      if(ierr.ne.0) goto 295
      call bgeti4(iunit,irec,buff,lbuff,ibuff,itimead,5*ntidad,ierr)
      if(ierr.ne.0) goto 295
      call bgeti4(iunit,irec,buff,lbuff,ibuff,konparad,nparad,ierr)
      if(ierr.ne.0) goto 295
      call bgeti4(iunit,irec,buff,lbuff,ibuff,iskalad,nparad,ierr)
      if(ierr.ne.0) goto 295
c
c..check date/time
      ierr=0
      do n=1,ntid
	do i=1,5
	  if(itimead(i,n).ne.itime(i,n)) ierr=1
	end do
      end do
      if(ierr.ne.0) then
        write(6,*) '   not same date/time as the main file'
	goto 295
      end if
c
c..check stations
      if(csfix.ne.' ') then
c..remove suffix if found
	do n=1,npos
	  kf=index(adnavn(n),csfix)
	  if(kf.gt.1) then
	    do k=kf,ntnad
	      adnavn(n)(k:k)=' '
	    end do
	  end if
	end do
      end if
      ll=len(adnavn(1))
      if(ll.gt.len(navns(1))) ll=len(navns(1))
      do n=1,npos
	if(adnavn(n).ne.navns(n)) then
	  l1=1
	  l2=1
	  do k=1,ll
	    if(adnavn(n)(k:k).ne.' ') l1=k
	    if( navns(n)(k:k).ne.' ') l2=k
	  end do
	  l1=min(l1,l2)
	  if(adnavn(n)(1:l1).ne.navns(n)(1:l1)) ierr=1
	end if
      end do
      if(ierr.ne.0) then
        write(6,*) '   not the same stations as in the main file'
	goto 295
      end if
c
      irdata=irec
      iwdata=ibuff
c
      nadpp=jadpar(2,iadf)-jadpar(1,iadf)+1
      nadp=nadpp
      if(nadpp.le.0) nadp=nparad
c
      do 290 jadp=1,nadp
c
      if(nadpp.gt.0) then
	iadp=jadpar(1,iadf)-1+jadp
        np=0
        do i=1,nparad
	  if(np.eq.0 .and. konparad(i).eq.iadpar(1,iadp)) np=i
        end do
        if(np.eq.0) then
	  write(6,*) '   not able to add parameter ',iadpar(1,iadp)
	  write(6,*) '     (not found in file)'
	  goto 290
        end if
	iadp1=iadpar(1,iadp)
	iadp2=iadpar(2,iadp)
      else
	np=jadp
	iadp1=konparad(np)
	iadp2=konparad(np)
      end if
c
      if(npar+1.gt.maxpar) then
	write(6,*) '   not able to add parameter ',iadp1
	write(6,*) '     (too many parameters)'
	goto 290
      end if
      if(ndat+npos*ntid.gt.maxdat) then
	write(6,*) '   not able to add parameter ',iadp1
	write(6,*) '     (too much data)'
	goto 290
      end if
c
      if(jadp.gt.1) then
        irec=irdata-1
        ibuff=lbuff
        call bgeti4(iunit,irec,buff,lbuff,ibuff,i,1,ierr)
        if(ierr.ne.0) goto 295
        ibuff=iwdata
      end if
c
c..add space in data array for a new parameter
      call addpar(npos,npar,ntid,dat,dat)
c
      npar=npar+1
      konpar(npar)=iadp2
      iskal(npar) =iskalad(np)
c
      njump1=nposad*(np-1)
      njump2=nposad*(nparad-np)
c
      do nt=1,ntid
c
        call bgetjp(iunit,irec,buff,lbuff,ibuff,njump1,ierr)
        if(ierr.ne.0) goto 295
        call bgeti2(iunit,irec,buff,lbuff,ibuff,datad(1),npos,ierr)
        if(ierr.ne.0) goto 295
        call bgetjp(iunit,irec,buff,lbuff,ibuff,njump2,ierr)
        if(ierr.ne.0) goto 295
c
c..put one timestep of one parameter into the data array
	iadr=npos*npar*(nt-1)+npos*(npar-1)
	do n=1,npos
	  dat(iadr+n)=datad(n)
	end do
c
      end do
c
      ndat=ndat+npos*ntid
c
  290 continue
c
  295 close(iunit)
c
  280 continue
c
      if(nadfil.gt.0)
     +   write(6,*) '    npos,npar,ntid: ',npos,npar,ntid
c
c.......................................................................
c.......................................................................
c
c--------------------------------------------------------------------
c
      if(lstdat.eq.1 .or. lstdat.eq.2) then
        nxdim=nxpar
        if(nxpar.lt.1) nxdim=1
        call list1(lstdat,linfo,info,npos,npar,ntid,ltekst,tekst,
     *             ktekst,navn,ntnavn,itime,konpar,iskal,dat,
     *             nxpar,nxdim,konprx,ixskal,xdat)
      end if
c
c--------------------------------------------------------------------
c
      if(iprmin.gt.-32767 .or. iprmax.lt.+32767
     +			  .or. nprog.gt.0) then
        if(iprmin.gt.-32767 .or. iprmax.lt.+32767) then
          write(6,*) 'time limit (forecast):   ',iprmin,iprmax
	end if
	if(nprog.gt.0) then
	  write(6,*) 'time steps (forecast):   ',nprog
	  write(6,fmt='(10(1x,i6))') (iprog(i),i=1,nprog)
	end if
	if(ndeladd.gt.0) then
	  nd=0
	  do n=1,ndeladd
	    k=0
	    do np=1,npar
	      if(ideladd(n).eq.konpar(np)) k=1
	    end do
	    if(k.eq.1) then
	      nd=nd+1
	      ideladd(nd)=ideladd(n)
	    end if
	  end do
	  ndeladd=nd
	end if
	if(ndeladd.gt.0) then
	  write(6,*) 'when deleting timesteps, adding parameters:'
	  write(6,fmt='(10(1x,i6))') (ideladd(i),i=1,ndeladd)
	end if
        write(6,*) 'input  no. of timesteps: ',ntid
        call deltim(npos,npar,ntid,dat,itime,konpar,iprmin,iprmax,
     +		    nprog,iprog,ndeladd,ideladd,iprout)
        write(6,*) 'output no. of timesteps: ',ntid
        if(ntid.lt.1) goto 960
        info(4)=ntid
        info(5)=ntid
      end if
c
      if(nparm.gt.0) then
c..list of output parameters (not changing sequence of parameters)
        write(6,*) 'input  parameters:   n= ',npar
        write(6,*) (konpar(i),i=1,npar)
        call delpar(npos,npar,ntid,dat,konpar,iskal,
     *              nparm,iparm,jparm,nparout)
        npar=nparout
        if(npar.lt.1) then
          write(6,*) '** no parameters output (param_out=)'
          goto 960
        end if
        write(6,*) 'output parameters:   n= ',npar
        write(6,*) (konpar(i),i=1,npar)
        info(3)=npar
      end if
c
      if(nchparm.gt.0) then
        nchp=0
	do j=1,nchparm
	  do i=1,npar
	    if(konpar(i).eq.ichparm(1,j)) nchp=nchp+1
	  end do
	end do
	if(nchp.gt.0) then
	  write(6,*) 'changing output parameter numbers from:'
	  write(6,*) (konpar(i),i=1,npar)
	  do j=1,nchparm
	    do i=1,npar
	      if(konpar(i).eq.ichparm(1,j)) konpar(i)=ichparm(2,j)
	    end do
	  end do
	  write(6,*) 'to:'
	  write(6,*) (konpar(i),i=1,npar)
	end if
      end if
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
        info(6)=ltekst
      end if
c
      do n=1,npar
        skal(n)=10.**iskal(n)
      end do
c
      do n=1,nxpar
        xskal(n)=10.**ixskal(n)
      end do
c
      do n=1,npos
        nused(n)=0
      end do
c
      if(iphouradd.ne.0) then
        do n=1,ntid
          do i=1,4
	    itimev(i,n)=itime(i,n)
	  end do
	  itimev(5,n)=-iphouradd
	  call vtime(itimev(1,n),ierror)
          do i=1,4
	    itime(i,n)=itimev(i,n)
	  end do
	  itime(5,n)=itime(5,n)+iphouradd
	end do
      end if
c
      nposip=npos
      nparip=npar
      ntidip=ntid
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
      end if
c
      write(6,*) 'output konvertering.  konvc,konvd: ',konvc,konvd
c
c
c..utvalg for output....................................................
c
      do 300 nfil=1,nfile
c
      write(6,*) 'output file: ',fileot(nfil)(1:lenstr(fileot(nfil),1))
c
      n1=listot(1,nfil)
      n2=listot(2,nfil)
ccc   write(6,*) ' ............ n1,n2: ',n1,n2
c
      npos=nposip
      iall=1
      if(n1.gt.n2) goto 350
      iall=0
c
      mtn=len(navn(1))
      nk1=len(navn1(1))
      nk2=len(navn2(1))
      np=0
c
      do 310 m=n1,n2
        if(navn2(m)(1:8).eq.'<<rest>>') goto 320
        nf=0
	ipos=0
        do n=1,npos
          if(navns(n).eq.navn2(m)) then
            nf=nf+1
	    if(nf.eq.1) ipos=n
	  end if
        end do
	if(nf.eq.0) then
c..ikke funnet i loekka over, kan skyldes for langt navn slik
c..at kalman-suffix'et har overskrevet bakerste del av navnet
	  kk=1
	  do k=2,nk2
	    if(navn2(m)(k:k).ne.' ') kk=k
	  end do
	  do n=1,npos
	    if(ntnavn(n).eq.mtn .and. nsuff(n).gt.0) then
	      kkn=ntnavn(n)-nsuff(n)-1
	      if(navns(n)(kkn:kkn).eq.' ') kkn=kkn-1
	      if(kk.gt.kkn) then
	        if(navns(n)(1:kkn).eq.navn2(m)(1:kkn)) then
	          nf=nf+1
	          if(nf.eq.1) ipos=n
		end if
	      end if
	    end if
	  end do
	  if(nf.gt.1) nf=0
	end if
        if(nf.gt.0) then
	  n=ipos
          np=np+1
          if (nsuff(n).gt.0) then
            navn1s(np)=navn3(m)
            navn1(np)=navn3(m)
	    if(ntnav3(m)+nsuff(n).gt.nk1) ntnav3(m)=nk1-nsuff(n)
            navn1(np)(ntnav3(m)+1:ntnav3(m)+nsuff(n)) =
     *                suffix(n)(1:nsuff(n))
            ntnav1(np)=ntnav3(m) + nsuff(n)
          else
            navn1s(np)=navn3(m)
            navn1(np)=navn3(m)
            ntnav1(np)=ntnav3(m)
          end if
          numpos(np)=n
          nused(n)=nused(n)+1
        end if
        if(nf.lt.1) then
          write(6,*) ' position not found: ',navn2(m)
        elseif(nf.gt.1) then
          write(6,*) ' position: ',navn2(m),' found ',nf,' times'
        end if
  310 continue
      goto 340
c
  320 m=0
      do 330 n=1,npos
        if(nused(n).eq.0) then
          if(np.lt.maxout) then
            np=np+1
            if (nsuff(n).gt.0) then
              navn1s(np)=navns(n)
              navn1(np)=navn(n)
              navn1(np)(ntnavn(n)-nsuff(n)+1:ntnavn(n)) =
     *              suffix(n)(1:nsuff(n))
              ntnav1(np)=ntnavn(n)
            else
              navn1s(np)=navns(n)
              navn1(np)=navn(n)
              ntnav1(np)=ntnavn(n)
            end if
            numpos(np)=n
            nused(n)=nused(n)+1
            m=m+1
          else
            write(6,*) ' <<rest>>  ikke plass for: ',navn(n)
          end if
        end if
  330 continue
      write(6,*) ' <<rest>>  antall stasjoner: ',m
c
  340 npos=np
c
      if(npos.lt.1) then
        write(6,*) ' ***** ingen stasjoner funnet *****'
        goto 350
      end if
c
      if(npos*npar*ntid.gt.maxdt1) then
        npos=maxdt1/(npar*ntid)
        write(6,*) ' ...... for mye data output'
        write(6,*) ' ...... antall posisjoner reduseres fra ',np
        write(6,*) ' ......                             til ',npos
        if(npos.lt.1) goto 350
      end if
c
      call movdat(nposip,npos,npar,ntid,numpos,dat,dat1)
c
      if(nxpar.gt.0) call movdat(nposip,npos,nxpar,1,numpos,xdat,xdat1)
c
c--------------------------------------------------------------------
c
c..output og evt. konvertering
c
  350 continue
c
      write(6,*) '    npos,npar,ntid: ',npos,npar,ntid
c
c-print----------------------------------------
      if(iformt.eq.1001) then
c..format.print_automat_1 ..... print automat-stasjon
        if(iall.eq.1) then
          call praut1(iunit,fileot(nfil),npos,npar,ntid,
     *                konpar,iskal,itime,dat,navn)
        else
          call praut1(iunit,fileot(nfil),npos,npar,ntid,
     *                konpar,iskal,itime,dat1,navn1)
        end if
        goto 300
      end if
c
      if(iformt.eq.1010) then
c..format.print_nhlvind ..... print vinddata til nhl
        if(iall.eq.1) then
          call nhlvind(iunit,fileot(nfil),npos,npar,ntid,
     *                 konpar,iskal,itime,dat,navn,hlpdat)
        else
          call nhlvind(iunit,fileot(nfil),npos,npar,ntid,
     *                 konpar,iskal,itime,dat1,navn1,hlpdat)
        end if
        goto 300
      end if
c
      if(iformt.eq.1021) then
c..format.print_vei_1 ..... print vei-stasjon
        if(iall.eq.1) then
          call prvei1(iunit,fileot(nfil),npos,npar,ntid,
     *                konpar,iskal,itime,dat,navn,ltekst,tekst,ktekst)
        else
          call prvei1(iunit,fileot(nfil),npos,npar,ntid,
     *                konpar,iskal,itime,dat1,navn1,ltekst,tekst,ktekst)
        end if
        goto 300
      end if
c
      if(iformt.eq.1022) then
c..format.print_vest ..... print vei-stasjon
        if(iall.eq.1) then
          call prvest(iunit,fileot(nfil),npos,npar,ntid,
     *                konpar,iskal,itime,dat,navn,ltekst,tekst,ktekst)
        else
          call prvest(iunit,fileot(nfil),npos,npar,ntid,
     *                konpar,iskal,itime,dat1,navn1,ltekst,tekst,ktekst)
        end if
        goto 300
      end if
c
      if(iformt.eq.1025) then
c..format.print_icecast ..... print icecast list
        if(iall.eq.1) then
          call priccast(iunit,fileot(nfil),npos,npar,ntid,
     *                  konpar,iskal,itime,dat,navns,
     *			ltekst,tekst,ktekst)
        else
          call priccast(iunit,fileot(nfil),npos,npar,ntid,
     *                  konpar,iskal,itime,dat1,navn1s,
     *			ltekst,tekst,ktekst)
        end if
        goto 300
      end if
c
      if(iformt.eq.1030) then
c..format.print_verifi ..... print for verifikasjons-systemet
        if(iall.eq.1) then
          call prverifi(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat,navns,ltekst,tekst,ktekst,
     *                  vsfile,vtable,itimev,ntimev,hlpdat,ierror)
        else
          call prverifi(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat1,navn1s,ltekst,tekst,ktekst,
     *                  vsfile,vtable,itimev,ntimev,hlpdat,ierror)
        end if
        nfilot=nfil
        if(ierror.ne.0) goto 960
        goto 300
      end if
c
      if(iformt.eq.1040) then
c..format.print_normem ..... print for NORMEM (MEMbrain)
        if(iall.eq.1) then
          call prnormem(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat,navns,ltekst,tekst,ktekst,
     *                  iprod,itimev,hlpdat,ierror)
        else
          call prnormem(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat1,navn1s,ltekst,tekst,ktekst,
     *                  iprod,itimev,hlpdat,ierror)
        end if
        nfilot=nfil
        if(ierror.ne.0) goto 960
        goto 300
      end if
c
      if(iformt.eq.1050 .or. iformt.eq.1051) then
c..format.print_multiuse/multiuse+pos .. print av alle typer diagrammer
	iopt=iformt-1050
        if(iall.eq.1) then
          call prmuluse(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat,navn,ltekst,tekst,ktekst,
     *                  itimev,hlpdat,iopt,nxpar,konprx,xskal,xdat,
     *                  ierror)
        else
          call prmuluse(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat1,navn1,ltekst,tekst,ktekst,
     *                  itimev,hlpdat,iopt,nxpar,konprx,xskal,xdat1,
     *                  ierror)
        end if
        goto 300
      end if
c
      if(iformt.eq.1055) then
c..format.print_sdv ..... print SDV-type ascii output
        if(iall.eq.1) then
          call prsdv(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *               itime,dat,navn,ltekst,tekst,ktekst,
     *               itimev,hlpdat,ierror)
        else
          call prsdv(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *               itime,dat1,navn1,ltekst,tekst,ktekst,
     *               itimev,hlpdat,ierror)
        end if
        goto 300
      end if
c
      if(iformt.eq.1060) then
c..format.print_param ..... ascii output
        if(iall.eq.1) then
          call prtparam(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat,navn,ltekst,tekst,ktekst,
     *                  itimev,hlpdat,
     *		        nparprt,iparprt,tparprt,fparprt,dparprt,
     *                  jparprt,kparprt,jparam,ierror)
        else
          call prtparam(iunit,fileot(nfil),npos,npar,ntid,konpar,skal,
     *                  itime,dat1,navn1,ltekst,tekst,ktekst,
     *                  itimev,hlpdat,
     *		        nparprt,iparprt,tparprt,fparprt,dparprt,
     *                  jparprt,kparprt,jparam,ierror)
        end if
        goto 300
      end if
c-print-----------------------------------------
c
c..binary output formats
c
      call rmfile(fileot(nfil),0,ierror)
c
      open(iunit,file=fileot(nfil),
     *           access='direct',form='unformatted',
     *           recl=(lbuff*2)/lrunit,
     *           status='unknown',iostat=ios,err=930)
c
      nfilot=nfil
c
c        max antall tegn i fast tekst
      ntt=len(tekst(1))
c        max antall tegn i hvert navn
      ntn=len(navn1(1))
c
      if(ibsort.eq.0) then
c
      ndat=npos*npar*ntid
c
c--------------------------------------------------------------------
c        antall 16 bits ord ut.
c        nb| character*nn tekst(.) .... nn=2*n, n=1,2,3,...
c            character*nn navn(..) .... nn=2*n, n=1,2,3,...
c
      nword= linfo
     +      +ltekst*(ntt/2)+ltekst*2
     +      +npos*(ntn/2)+npos
     +      +5*ntid
     +      +npar+npar
     +      +ndat
     +      +1
c
      nxdat=npos*nxpar
      if(nxpar.gt.0) nword = nword + 1+nxpar+nxpar+nxdat
c
      nrec=(nword+lbuff-1)/lbuff
c
ccc   write(6,*) 'antall ord:     ',nword
ccc   write(6,*) 'antall records: ',nrec
ccc   write(6,*) 'record-lengde:  ',lbuff
c
      irec=0
      ibuff=0
c
ccc   info( 1)=nrec
      info( 1)=211
      if(linfo.eq.8) info( 1)=212
      info( 2)=npos
ccc   info( 3)=npar
ccc   info( 4)=ntid
ccc   info( 5)=ntidin
ccc   info( 6)=ltekst
ccc   info( 7)=ntt
ccc   info( 8)=ntn
ccc...................... meteogram
ccc   info( 9)=iprod
ccc   info(10)=nrgrid
ccc   info(11)=interp
c
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,info,linfo,ierr)
      if(ierr.ne.0) goto 960
c
      if(npos.lt.1) goto 380
c
      call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		  tekst,ltekst,ntt,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		  ktekst,2*ltekst,ierr)
      if(ierr.ne.0) goto 960
      if(iall.eq.1) then
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		    navn,npos,ntn,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnavn,npos,ierr)
        if(ierr.ne.0) goto 960
      else
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		    navn1,npos,ntn,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnav1,npos,ierr)
        if(ierr.ne.0) goto 960
      end if
      if(ipcwave.ne.1) then
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		    itime,5*ntid,ierr)
      else
	do n=1,ntid
	  do i=1,5
	    ivtime(i,n)=itime(i,n)
	  end do
	  call vtime(ivtime(1,n),ierr)
	end do
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		    ivtime,5*ntid,ierr)
      end if
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,konpar,npar,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iskal,npar,ierr)
      if(ierr.ne.0) goto 960
c
      if(iall.eq.1) then
        call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,dat,ndat,ierr)
      else
        call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,dat1,ndat,ierr)
      end if
      if(ierr.ne.0) goto 960
c
c..ekstra parametre (ikke tidsserier)
c.........f.eks: bredde(-1),lengde(-2),topografi(101)
      if(nxpar.le.0) then
        i=0
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,i,1,ierr)
        if(ierr.ne.0) goto 960
      else
        i=1001
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,i,1,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,nxpar,1,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,konprx,nxpar,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ixskal,nxpar,ierr)
        if(ierr.ne.0) goto 960
        if(iall.eq.1) then
          call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,
     +		      xdat,nxdat,ierr)
        else
          call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,
     +		      xdat1,nxdat,ierr)
        end if
        if(ierr.ne.0) goto 960
      end if
c
  380 call bputnd(iunit,irec,buff,lbuff,ibuff,konvd,ierr)
      if(ierr.ne.0) goto 960
c--------------------------------------------------------------------
c
      elseif(ibsort.eq.1) then
c
c--------------------------------------------------------------------
c
      iprod=0
      igrid=0
      if(linfo.ge. 9) iprod=info( 9)
      if(linfo.ge.10) igrid=info(10)
c
c..start of station data (after nrecstart records + nwrdstart words)
      nwords= 8+lhead+
     +       +ltekst*((ntt/2)+2)
     +       +ntid*5
     +       +npar*2
     +       +nxpar*(2+npos)
      nrecstart=nwords/lbuff
      nwrdstart=nwords-nrecstart*lbuff
c
c..file header
      iheadf(1)=221
      iheadf(2)=0
      iheadf(3)=2
      iheadf(4)=lbuff
      iheadf(5)=lhead
      iheadf(6)=0
      iheadf(7)=0
      iheadf(8)=0
c
c..data header
      do i=1,lhead
	ihead(i)=0
      end do
      ihead( 1)=nrecstart
      ihead( 2)=nwrdstart
      ihead( 3)=npos
      ihead( 4)=npar
      ihead( 5)=ntid
      ihead( 6)=ntidin
      ihead( 7)=nxpar
      ihead( 8)=ltekst
      ihead( 9)=ntt
      ihead(10)=ntn
      ihead(11)=iprod
      ihead(12)=igrid
c
      irec=0
      ibuff=0
c
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iheadf,8,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ihead,lhead,ierr)
      if(ierr.ne.0) goto 960
c
      if(npos.lt.1) goto 480
c
      call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		  tekst,ltekst,ntt,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		  ktekst,2*ltekst,ierr)
      if(ierr.ne.0) goto 960
      if(iall.eq.1) then
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		    navn,npos,ntn,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnavn,npos,ierr)
        if(ierr.ne.0) goto 960
      else
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +		    navn1,npos,ntn,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnav1,npos,ierr)
        if(ierr.ne.0) goto 960
      end if
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,itime,5*ntid,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,konpar,npar,ierr)
      if(ierr.ne.0) goto 960
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iskal,npar,ierr)
      if(ierr.ne.0) goto 960
c
c..ekstra parametre (ikke tidsserier)
c.........f.eks: bredde(-1),lengde(-2),topografi(101)
      if(nxpar.gt.0) then
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,konprx,nxpar,ierr)
        if(ierr.ne.0) goto 960
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ixskal,nxpar,ierr)
        if(ierr.ne.0) goto 960
        if(iall.eq.1) then
          call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,
     +		      xdat,npos*nxpar,ierr)
        else
          call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,
     +		      xdat1,npos*nxpar,ierr)
        end if
        if(ierr.ne.0) goto 960
      end if
c
      ndat=ntid*npar
c
      if(iall.eq.1) then
	do n=1,npos
	  do j=1,npar
            no=(j-1)*ntid
	    do i=1,ntid
	      ni=npos*npar*(i-1)+npos*(j-1)+n
              isorted(no+i)=dat(ni)
	    end do
	  end do
	  call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,
     +		      isorted,ndat,ierr)
	  if(ierr.ne.0) goto 960
	end do
      else
	do n=1,npos
	  do j=1,npar
            no=(j-1)*ntid
	    do i=1,ntid
	      ni=npos*npar*(i-1)+npos*(j-1)+n
              isorted(no+i)=dat1(ni)
	    end do
	  end do
	  call bputi2(iunit,irec,buff,lbuff,ibuff,konvd,
     +		      isorted,ndat,ierr)
	  if(ierr.ne.0) goto 960
	end do
      end if
c
  480 call bputnd(iunit,irec,buff,lbuff,ibuff,konvd,ierr)
      if(ierr.ne.0) goto 960
c--------------------------------------------------------------------
c
      end if
c
      close(iunit)
c
      nfilot=0
c
  300 continue
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
      end if
      if(lstnot.eq.1) then
        write(6,*) '-------------------------------------------'
        write(6,*) 'posisjoner som ikke er benyttet:'
        m=0
        do n=1,nposip
          if(nused(n).eq.0) then
            write(6,*) navn(n)
            m=m+1
          end if
        end do
        write(6,*) 'antall posisjoner ikke benyttet: ',m
        write(6,*) '-------------------------------------------'
      end if
c
c--------------------------------------------------------------------
c
      goto 990
c
c
  920 write(6,*) 'open error.  input file.   iostat: ',ios
      write(6,*) filein(1:lenstr(filein,1))
      goto 960
c
  930 write(6,*) 'open error.  output file.   iostat: ',ios
      write(6,*) fileot(nfil)(1:lenstr(fileot(nfil),1))
      goto 960
c
  960 write(6,*) ' ======== ERROR EXIT ========='
      if(nfilot.gt.0) then
        close(iunit)
        nfil=nfilot
        call rmfile(fileot(nfil),0,ierror)
      end if
      stop 3
c
  990 continue
c
      end
c
c**********************************************************************
c
      subroutine addpar(npos,npar,ntid,datin,datot)
c
c	add space for a new parameter in the data array.
c	note that 'datin' and 'datot' really are the same array.
c
      integer   npos,npar,ntid
      integer*2 datin(npos,npar,ntid)
      integer*2 datot(npos,npar+1,ntid)
c
      do nt=ntid,1,-1
	do np=npar,1,-1
	  do nn=npos,1,-1
	    datot(nn,np,nt)=datin(nn,np,nt)
	  end do
	end do
      end do
c
      do nt=1,ntid
	do nn=1,npos
	  datot(nn,npar+1,nt)=0
	end do
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine deltim(npos,npar,ntid,dat,itime,konpar,iprmin,iprmax,
     +		        nprog,iprog,ndeladd,ideladd,iprout)
c
c        remove timsteps if possible,
c        forecasts or analysis followed by forecats.
c
      integer   npos,npar,ntid,nprog,ndeladd
      integer   itime(5,ntid),konpar(npar)
      integer   iprog(nprog),ideladd(ndeladd),iprout(ntid)
      integer*2 dat(npos,npar,ntid)
c
      integer   itimer(5)
c
      do nt=1,ntid
	iprout(nt)=1
      end do
c
      if(iprmin.le.iprmax .and.
     +   (iprmin.gt.-32767 .or. iprmax.lt.32767)) then
c
c..find a reference time
        nt0=0
        do nt=2,ntid
          if(nt0.eq.0 .and.
     +       itime(5,nt-1).le.0 .and. itime(5,nt).gt.0) nt0=nt-1
        end do
	if(nt0.eq.0) nt0=1
        itimer(1)=itime(1,nt0)
        itimer(2)=itime(2,nt0)
        itimer(3)=itime(3,nt0)
        itimer(4)=itime(4,nt0)
        itimer(5)=0
        do nt=1,ntid
          call hrdiff(0,0,itimer,itime(1,nt),ihours,ierr1,ierr2)
          if(ierr1.ne.0 .or. ierr2.ne.0) return
          if(ihours.lt.iprmin .or. ihours.gt.iprmax) iprout(nt)=0
        end do
c
      end if
c
      if(nprog.gt.0) then
c
	do nt=1,ntid
	  k=0
	  do n=1,nprog
	    if(itime(5,nt).eq.iprog(n)) k=1
	  end do
	  iprout(nt)=k
	end do
c
      end if
c
c..possibly add (percipitation,...) parameters before deleting timesteps
c
      if(ndeladd.gt.0) then
c
        do np=1,npar
	  k=0
	  do n=1,ndeladd
	    if(konpar(np).eq.ideladd(n)) k=1
	  end do
	  if(k.eq.1) then
	    do nt=1,ntid-1
	      if(iprout(nt).ne.1) then
	        do n=1,npos
		  dat(n,np,nt+1)=dat(n,np,nt)+dat(n,np,nt+1)
	        end do
	      end if
	    end do
	  end if
	end do
c
      end if
c
c..delete time steps
c
      nto=0
      do nt=1,ntid
	if(iprout(nt).eq.1) then
          nto=nto+1
	  if(nto.ne.nt) then
            do np=1,npar
              do n=1,npos
                dat(n,np,nto)=dat(n,np,nt)
              end do
            end do
            do i=1,5
              itime(i,nto)=itime(i,nt)
            end do
	  end if
	end if
      end do
c
      ntid=nto
c
      return
      end
c
c**********************************************************************
c
      subroutine delpar(npos,npar,ntid,dat,konpar,iskal,
     *                  nparm,iparm,jparm,nparout)
c
c        remove parameters.
c        not changing parameter sequence !
c
      integer   npos,npar,ntid,nparm
      integer   konpar(npar),iskal(npar),iparm(nparm),jparm(nparm)
      integer*2 dat(npos,npar*ntid)
c
      nparout=0
      do n=1,npar
        k=0
        do i=1,nparm
          if(iparm(i).eq.konpar(n)) k=1
        end do
        if(k.gt.0) then
          nparout=nparout+1
          jparm(nparout)=n
        end if
      end do
      if(nparout.lt.1 .or. nparout.eq.npar) return
c
      do n=1,nparout
        np=jparm(n)
        konpar(n)=konpar(np)
         iskal(n)= iskal(np)
      end do
c
      do nt=1,ntid
        do np=1,nparout
          npto=(nt-1)*nparout+np
          npti=(nt-1)*npar+jparm(np)
          do n=1,npos
            dat(n,npto)=dat(n,npti)
          end do
        end do
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine movdat(nposip,npos,npar,ntid,numpos,dat,dat1)
c
      integer   nposip,npos,npar,ntid
      integer   numpos(npos)
      integer*2 dat(nposip,npar,ntid),dat1(npos,npar,ntid)
c
      do n=1,npos
        np=numpos(n)
        do l=1,ntid
          do k=1,npar
            dat1(n,k,l)=dat(np,k,l)
          end do
        end do
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine list1(list,linfo,info,npos,npar,ntid,
     *                            ltekst,tekst,ktekst,
     *                 navn,ntnavn,itime,konpar,iskal,dat,
     *                 nxpar,nxdim,konprx,ixskal,xdat)
c
      integer   linfo,npos,npar,ntid,ltekst,nxpar,nxdim
      integer   info(linfo),ktekst(2,ltekst)
      integer   ntnavn(npos),itime(5,ntid),konpar(npar),iskal(npar)
      integer   konprx(nxdim),ixskal(nxdim)
      integer*2 dat(npos,npar,ntid)
      integer*2 xdat(npos,nxdim)
      character*(*) tekst(ltekst),navn(npos)
c
      write(6,*)
      write(6,*) '---------------------------------------------------'
      write(6,*) '---------- input ----------------------------------'
      write(6,*) 'npos,npar,ntid,nxpar,ltekst:'
      write(6,fmt='(1x,20i7)') npos,npar,ntid,nxpar,ltekst
      write(6,*) 'info:'
      write(6,fmt='(1x,20i7)') info
      write(6,*) 'tekst:'
      do l=1,ltekst
        write(6,*) tekst(l)
      end do
      write(6,*) 'ktekst:'
      do l=1,ltekst
        write(6,fmt='(1x,20i7)') ktekst(1,l),ktekst(2,l)
      end do
      write(6,*) 'navn,ntnavn:'
      do n=1,npos
        write(6,*) navn(n),'    ',ntnavn(n)
      end do
      write(6,*) 'tider:'
      do n=1,ntid
        write(6,fmt='(1x,20i7)') (itime(i,n),i=1,5)
      end do
      write(6,*) 'konpar:'
      write(6,fmt='(1x,20i7)') konpar
      write(6,*) 'iskal:'
      write(6,fmt='(1x,20i7)') iskal
c
      if(nxpar.gt.0) then
        write(6,*) 'posisjons-data, konprx:'
        write(6,fmt='(1x,20i7)') konprx
        write(6,*) 'posisjons-data, ixskal:'
        write(6,fmt='(1x,20i7)') ixskal
        write(6,*) 'posisjons-data for hver stasjon:'
        do n=1,npos
          write(6,*) navn(n)
          write(6,fmt='(1x,20i7)') (xdat(n,k),k=1,nxpar)
        end do
      end if
c
      if(list.lt.2) goto 900
c
      write(6,*) 'data for hver stasjon:'
      do n=1,npos
        write(6,*) navn(n)
        do nt=1,ntid
          write(6,fmt='(1x,20i7)') (dat(n,k,nt),k=1,npar)
        end do
      end do
c
  900 write(6,*) '---------------------------------------------------'
      write(6,*)
c
      return
      end
c
c***********************************************************************
c
      subroutine strsfix(csfix,npos,ntnavn,navn,nsuff,navns,suffix)
c
c- soker etter suffikset %xxx i meteogram-navnet.
c- suffikset blir lagt i suffix(n), og resten av navnet i navns(n)
c
      character*1   csfix
      integer       npos
      integer       ntnavn(npos),nsuff(npos)
      character*(*) navn(npos),navns(npos),suffix(npos)
c
      integer   i,j,k
c
      if(csfix.eq.' ') then
c
        do i = 1,npos
           nsuff(i)  = 0
           navns(i)  = navn(i)
           suffix(i) = ' '
        enddo
c
      else
c
        do i = 1,npos
          k = 0
          do j = 2,ntnavn(i)-1
            if (navn(i)(j:j).eq.csfix) k = j
          enddo
          if (k.gt.0) then
            nsuff(i)  = ntnavn(i) - k
            navns(i)  = navn(i)(1:k-1)
            suffix(i) = navn(i)(k+1:k+nsuff(i))
          else
            nsuff(i)  = 0
            navns(i)  = navn(i)
            suffix(i) = ' '
          end if
        enddo
c
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine praut1(iunit,filnam,npos,npar,ntid,
     *                  konpar,iskal,itime,dat,navn)
c
c       formatert output.  automatstasjon.
c
c       interpolasjon til hver hele time.
c
c       parametre  input: t2m,u,v,rh2m,nedb@r
c       parametre output: t2m,dd,ff,rh2m,nedb@r
c                   file:  tt,dd,ff,  uu,    rr
c
c
      integer   iunit,npos,npar,ntid
      integer   konpar(npar),iskal(npar),itime(5,ntid)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
c
      integer      itime1(5),itime2(5),itimeo(5)
c
      it=0
      iu=0
      iv=0
      irh=0
      irr1=0
      irr2=0
      irrt=0
      do 100 n=1,npar
c..t2m (celsius)
        if(konpar(n).eq.31) it=n
c..u10m (e/w component, knots)
        if(konpar(n).eq.33) iu=n
c..v10m (n/s component, knots)
        if(konpar(n).eq.34) iv=n
c..rh2m (percent)
        if(konpar(n).eq.32) irh=n
c..nedbor, frontal
        if(konpar(n).eq.15) irr1=n
c..nedbor, konvektiv
        if(konpar(n).eq.16) irr2=n
c..nedbor, total
c..(ikke akkum fra +0, metdat har laget n timers nedboer)
        if(konpar(n).eq.17) irrt=n
  100 continue
c
      if(irrt.gt.0) then
	irr1=irrt
	irr2=irrt
      end if
c
      if( it.eq.0 .or.   iu.eq.0 .or.   iv.eq.0 .or.
     *   irh.eq.0 .or. irr1.eq.0 .or. irr2.eq.0) then
        write(6,*) '***praut1*** not correct input data.'
        return
      end if
c
      sct =10.**iskal(it)
      scu =10.**iskal(iu)
      scv =10.**iskal(iv)
      scrh=10.**iskal(irh)
      scr1=10.**iskal(irr1)
      scr2=10.**iskal(irr2)
c
      if(irr1.eq.irr2) scr2=0.
c
c
c..only print data for one station on each file
      ipos=1
c
      write(6,*) 'praut1'
      if(npos.gt.1) write(6,*) 'warning: only one station on each file.'
      write(6,*) 'station: ',navn(ipos)
      write(6,*) 'file:    ',filnam(1:lenstr(filnam,1))
c
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      grad=180./3.1415927
      toms=1852./3600.
c
      do 120 nt1=1,ntid-1
c
        nt2=nt1+1
        do 130 i=1,5
          itime1(i)=itime(i,nt1)
          itime2(i)=itime(i,nt2)
  130   continue
        call hrdiff(1,1,itime1,itime2,nhours,ierr1,ierr2)
        nh1=1
        if(nt1.eq.1) nh1=0
        rhours=nhours
c
c..t2m
        t1=real(dat(ipos,it,nt1))*sct
        t2=real(dat(ipos,it,nt2))*sct
c..u10m,v10m
        u1=real(dat(ipos,iu,nt1))*scu
        u2=real(dat(ipos,iu,nt2))*scu
        v1=real(dat(ipos,iv,nt1))*scv
        v2=real(dat(ipos,iv,nt2))*scv
c..nedb:r
        rr1=( real(dat(ipos,irr1,nt1))*scr1
     *       +real(dat(ipos,irr2,nt1))*scr2)/rhours
        rr2=( real(dat(ipos,irr1,nt2))*scr1
     *       +real(dat(ipos,irr2,nt2))*scr2)/rhours
c..rh2m
        rh1=real(dat(ipos,irh,nt1))*scrh
        rh2=real(dat(ipos,irh,nt2))*scrh
c
        do 140 nh=nh1,nhours
          r=nh/rhours
c..tid
          itimeo(1)=itime1(1)
          itimeo(2)=itime1(2)
          itimeo(3)=itime1(3)
          itimeo(4)=itime1(4)
          itimeo(5)=itime1(5)+nh
          call vtime(itimeo,ierror)
c..t2m
          tt=t1+(t2-t1)*r
          itt=nint(tt*10.)
c..u10m,v10m ... (input i knop, output dd i grader og ff i hele m/s)
          uew=u1+(u2-u1)*r
          vns=v1+(v2-v1)*r
          ff=sqrt(uew*uew+vns*vns)*toms
          if(ff.gt.0.5) then
            iff=nint(ff)
            dd=270.-grad*atan2(vns,uew)
            if(dd.gt.360.) dd=dd-360.
            if(dd.lt.  0.) dd=dd+360.
            idd=nint(dd)
            if(idd.eq.0) idd=360
          else
            idd=0
            iff=0
          end if
c..nedb:r ... ikke interpolasjon ... sum frontal+konvektiv
          rr=rr2
          if(nt1.eq.1 .and. nh.eq.0) rr=rr1
          irr=nint(rr*10.)
c..rh2m
          rh=rh1+(rh2-rh1)*r
          iuu=nint(rh)
c
          write(iunit,fmt='(1x,i4,3(1x,i2.2),5(1x,i6))')
     *                     (itimeo(i),i=1,4),itt,idd,iff,irr,iuu
c
  140   continue
c
  120 continue
c
      close(iunit)
c
      return
c
  900 write(6,*) '***praut1*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
      return
      end
c
c**********************************************************************
c
      subroutine nhlvind(iunit,filnam,npos,npar,ntid,
     *                  konpar,iskal,itime,dat,navn,vdat)
c
c       Formatert output.  Vind data til NHL.
c
c       interpolasjon til hver hele time.
c
c       parametre  input: u,v
c       parametre output: dd,ff
c                   file:
c
c
      integer   iunit,npos,npar,ntid
      integer   konpar(npar),iskal(npar),itime(5,ntid)
      integer*2 dat(npos,npar,ntid)
      real      vdat(2,ntid)
      character*(*) filnam
      character*(*) navn(npos)
c
      iu=0
      iv=0
c
      do 100 n=1,npar
c..u10m (e/w component, knots)
        if(konpar(n).eq.33) iu=n
c..v10m (n/s component, knots)
        if(konpar(n).eq.34) iv=n
  100 continue
c
      if( iu.eq.0 .or. iv.eq.0 ) then
        write(6,*) '***nhlvind*** not correct input data.'
        return
      end if
c
      write(6,*) 'nhlvind'
      write(6,*) 'file:    ',filnam(1:lenstr(filnam,1))
c
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)

c
      write(iunit,*) 'EC PROGNOSER VIND-STYRKE OG -RETNING  * DNMI'
      write(iunit,*) 'DATO: ',(itime(i,1),i=1,3)
      write(iunit,*) 'LINJE 4: NPOS, NTID   LINJE 5: PROG.TIDER'
      write(iunit,*) npos,ntid
      write(iunit,'(1x,25i4)') (itime(5,i),i=1,ntid)
      write(iunit,*) ' '
c
      do 130 n=1,npos
      do 120 nt=1,ntid
       vdat(1,nt) = vindsty(dat(n,iu,nt),dat(n,iv,nt),
     *                      iskal(iu),iskal(iv))
       if (vdat(1,nt).eq.0.0) then
         vdat(2,nt) = 0.0
       else
         vdat(2,nt) = vindret(dat(n,iu,nt),dat(n,iv,nt),
     *                        iskal(iu),iskal(iv))
       end if
 120  continue
      write(iunit,1007) navn(n), (vdat(1,nt),vdat(2,nt),nt=1,ntid)
      write(iunit,*) ' '
 130  continue
c
 1007 format(1x,a12,1x,6f7.1,/,(10f7.1))
c
      close(iunit)
c
      return
c
  900 write(6,*) '***nhlvind*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
      return
      end
c
c**********************************************************************
c
      real function vindsty(u,v,iskalu,iskalv)
c
      integer*2 u,v
      integer   iskalu,iskalv
      real      uu,vv,test,skalu,skalv,toms
c
      toms=1852./3600.
c
      skalu = 10.0**iskalu
      uu = u*skalu
      skalv = 10.0**iskalv
      vv = v*skalv
c
      test = (uu*uu + vv*vv)
      if (test.lt.0.00001) then
        vindsty = 0.0
      else
        vindsty = sqrt(test)*toms
      end if
c
      return
      end
c
c**********************************************************************
c
      real function vindret(u,v,iskalu,iskalv)
c
      integer*2 u,v
      integer   iskalu,iskalv
      real      uu,vv,test,skalu,skalv,grad,toms
c
      grad=180./3.1415927
      toms=1852./3600.
c
      skalu = 10.0**iskalu
      uu = u*skalu * toms
      skalv = 10.0**iskalv
      vv = v*skalv * toms
c
      test = grad*atan2(-uu,-vv)
      if (test.lt.0.0) then
         test = test + 360.0
      end if
      vindret = test
c
      return
      end
c
c*********************************************************************
c
      subroutine prvei1(iunit,filnam,npos,npar,ntid,
     *                  konpar,iskal,itime,dat,navn,
     *                  ltekst,tekst,ktekst)
c
c       formatert output.  vei-stasjon.
c
c       parametre  input:
c          mslp,t2m,u,v,nedboer,taake,cl,cm,ch,rh2m
c       parametre output:
c          mslp,t2m,td2m,rh2m,dd,ff,nedboer,taake,cl,cm,ch
c
c       note: input  u  is  east/west  wind component in unit knots
c             input  v  is north/south wind component in unit knots
c             output ff in unit m/s
c
c
      integer   iunit,npos,npar,ntid,ltekst
      integer   konpar(npar),iskal(npar),itime(5,ntid)
      integer   ktekst(2,ltekst)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
c
      integer      itimev(5)
c
c
      real ewt(41)
c
c        vanndampens metningstrykk, ewt(41).
c        t i grader celsius: -100,-95,-90,...,90,95,100.
c            tc=...    x=(tc+105)*0.2    i=x
c            et=ewt(i)+(ewt(i+1)-ewt(i))*(x-i)
c
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
c
      imslp=0
      it   =0
      iu   =0
      iv   =0
      irr1 =0
      irr2 =0
      irrt =0
      ifog =0
      icl  =0
      icm  =0
      ich  =0
      irh  =0
      do 100 n=1,npar
c..mslp (hpa)
        if(konpar(n).eq.58) imslp=n
c..t2m (celsius)
        if(konpar(n).eq.31) it=n
c..u10m (e/w component, knots)
        if(konpar(n).eq.33) iu=n
c..v10m (n/s component, knots)
        if(konpar(n).eq.34) iv=n
c..nedboer, frontal
        if(konpar(n).eq.15) irr1=n
c..nedboer, konvektiv
        if(konpar(n).eq.16) irr2=n
c..nedbor, total
c..(ikke akkum fra +0, metdat har laget n timers nedboer)
        if(konpar(n).eq.17) irrt=n
c..skydekke (taake,lave,middelshoeye,hoeye)
        if(konpar(n).eq.39) then
          if(ifog.eq.0) then
            ifog=n
          elseif(icl.eq.0) then
            icl=n
          elseif(icm.eq.0) then
            icm=n
          elseif(ich.eq.0) then
            ich=n
          end if
        end if
c..td2m (celsius)
        if(konpar(n).eq.32) irh=n
  100 continue
c
      if(irrt.gt.0) then
	irr1=irrt
	irr2=irrt
      end if
c
      if(imslp.eq.0 .or.   it.eq.0 .or.   iu.eq.0 .or.  iv.eq.0 .or.
     *    irr1.eq.0 .or. irr2.eq.0 .or. ifog.eq.0 .or. icl.eq.0 .or.
     *     icm.eq.0 .or.  ich.eq.0 .or.  irh.eq.0) then
        write(6,*) '***prvei1*** not correct input data.'
        return
      end if
c
      scmslp =10.**iskal(imslp)
      sct    =10.**iskal(it)
      scu    =10.**iskal(iu)
      scv    =10.**iskal(iv)
      scrr1  =10.**iskal(irr1)
      scrr2  =10.**iskal(irr2)
      scfog  =10.**iskal(ifog)
      sccl   =10.**iskal(icl)
      sccm   =10.**iskal(icm)
      scch   =10.**iskal(ich)
      scrh   =10.**iskal(irh)
c
      if(irr1.eq.irr2) scrr2=0.
c
      write(6,*) 'prvei1'
      write(6,*) 'file:    ',filnam(1:lenstr(filnam,1))
c
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      do l=1,ltekst
        write(iunit,*) tekst(l)(1:ktekst(1,l))
      end do
c
      grad=180./3.1415927
      toms=1852./3600.
c
c..grenser for akseptabel rh
      rhmin=  1.
      rhmax=100.
c
      do 110 ipos=1,npos
c
      write(iunit,*) ' '
      write(iunit,*) navn(ipos)
c
      write(iunit,fmt='(1x,''YYYY MM DD HH  MSLP     T    TD   '',
     *                     ''RH DD FF.M/S N.MM T$KE CL  CM  CH'')')
c
      do 120 nt=1,ntid
c
        do 130 i=1,5
          itimev(i)=itime(i,nt)
  130   continue
        call vtime(itimev,ierror)
c
c..mslp
        p=real(dat(ipos,imslp,nt))*scmslp
c..t2m
        t=real(dat(ipos,it,nt))*sct
c..u10m,v10m
        u=real(dat(ipos,iu,nt))*scu
        v=real(dat(ipos,iv,nt))*scv
c..nedboer (sum)
        rr= real(dat(ipos,irr1,nt))*scrr1
     *     +real(dat(ipos,irr2,nt))*scrr2
c..skyer
        fog=real(dat(ipos,ifog,nt))*scfog
        cl =real(dat(ipos,icl, nt))*sccl
        cm =real(dat(ipos,icm, nt))*sccm
        ch =real(dat(ipos,ich, nt))*scch
c..rh2m
        rh=real(dat(ipos,irh,nt))*scrh
c
c..u10m,v10m ... (input i knop, output dd i grader og ff i hele m/s)
        uew=u
        vns=v
        ff=sqrt(uew*uew+vns*vns)*toms
        if(ff.gt.0.5) then
          iff=nint(ff)
          dd=270.-grad*atan2(vns,uew)
          if(dd.gt.360.) dd=dd-360.
          if(dd.lt.  0.) dd=dd+360.
          idd=nint(dd)
          if(idd.eq.0) idd=360
        else
          idd=0
          iff=0
        end if
c
c..rh+t(c)->td(c) ... from relative humidity
c..                   to dew point temp. (celsius)
        x=(t+105.)*0.2
        i=int(x)
        et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
        if(rh.lt.rhmin) rh=rhmin
        if(rh.gt.rhmax) rh=rhmax
        etd=rh*0.01*et
        do while (ewt(i).gt.etd .and. i.gt.1)
          i=i-1
        end do
        x=(etd-ewt(i))/(ewt(i+1)-ewt(i))
        td=-105.+(real(i)+x)*5.
c
        jrh =nint(rh)
        jfog=nint(fog)
        jcl =nint(cl)
        jcm =nint(cm)
        jch =nint(ch)
c
        write(iunit,fmt='(1x,i4,3(1x,i2.2),1x,f6.1,2f6.1,3i4,f7.1,4i4)')
     *                    (itimev(i),i=1,4),
     *                    p,t,td,jrh,idd,iff,rr,jfog,jcl,jcm,jch
c
  120 continue
c
  110 continue
c
      close(iunit)
c
      return
c
  900 write(6,*) '***prvei1*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
      return
      end
c
c*********************************************************************
c
      subroutine prvest(iunit,filnam,npos,npar,ntid,
     *                  konpar,iskal,itime,dat,navn,
     *                  ltekst,tekst,ktekst)
c
c       formatert output.  vei-stasjon.
c
c       parametre  input:
c          mslp,t2m,u,v,nedboer,taake,cl,cm,ch,rh2m
c       parametre output:
c          mslp,t2m,td2m,rh2m,dd,ff,nedboer,taake,cl,cm,ch
c
c       note: input  u  is  east/west  wind component in unit knots
c             input  v  is north/south wind component in unit knots
c             output ff in unit m/s
c
c=====================================================================
c     modified version of subr. prvei1 with output of cloudiness and
c     cloudtype for Vestfold area (northern, southern and inner part).
c     the subroutine is generating the same as prvei1, and in addition
c     a table with hourly values from 3-hourly input data.
c     j.e.haugen, 10/12 -97.
c=====================================================================
c
      integer   iunit,npos,npar,ntid,ltekst
      integer   konpar(npar),iskal(npar),itime(5,ntid)
      integer   ktekst(2,ltekst)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
c
      integer      itimev(5)
c
c     table length in terms of hours
c
      parameter(nprog=7)
c
      integer ih_arr(0:nprog),ncloud(0:nprog)
      character*1 ccloud(0:nprog)
      character*5 area(3)
c
c
      real ewt(41)
c
c        vanndampens metningstrykk, ewt(41).
c        t i grader celsius: -100,-95,-90,...,90,95,100.
c            tc=...    x=(tc+105)*0.2    i=x
c            et=ewt(i)+(ewt(i+1)-ewt(i))*(x-i)
c
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
c
      imslp=0
      it   =0
      iu   =0
      iv   =0
      irr1 =0
      irr2 =0
      irrt =0
      ifog =0
      icl  =0
      icm  =0
      ich  =0
      irh  =0
      do 100 n=1,npar
c..mslp (hpa)
        if(konpar(n).eq.58) imslp=n
c..t2m (celsius)
        if(konpar(n).eq.31) it=n
c..u10m (e/w component, knots)
        if(konpar(n).eq.33) iu=n
c..v10m (n/s component, knots)
        if(konpar(n).eq.34) iv=n
c..nedboer, frontal
        if(konpar(n).eq.15) irr1=n
c..nedboer, konvektiv
        if(konpar(n).eq.16) irr2=n
c..nedbor, total
c..(ikke akkum fra +0, metdat har laget n timers nedboer)
        if(konpar(n).eq.17) irrt=n
c..skydekke (taake,lave,middelshoeye,hoeye)
        if(konpar(n).eq.39) then
          if(ifog.eq.0) then
            ifog=n
          elseif(icl.eq.0) then
            icl=n
          elseif(icm.eq.0) then
            icm=n
          elseif(ich.eq.0) then
            ich=n
          end if
        end if
c..td2m (celsius)
        if(konpar(n).eq.32) irh=n
  100 continue
c
      if(irrt.gt.0) then
	irr1=irrt
	irr2=irrt
      end if
c
      if(imslp.eq.0 .or.   it.eq.0 .or.   iu.eq.0 .or.  iv.eq.0 .or.
     *    irr1.eq.0 .or. irr2.eq.0 .or. ifog.eq.0 .or. icl.eq.0 .or.
     *     icm.eq.0 .or.  ich.eq.0 .or.  irh.eq.0) then
        write(6,*) '***prvest*** not correct input data.'
        return
      end if
c
      scmslp =10.**iskal(imslp)
      sct    =10.**iskal(it)
      scu    =10.**iskal(iu)
      scv    =10.**iskal(iv)
      scrr1  =10.**iskal(irr1)
      scrr2  =10.**iskal(irr2)
      scfog  =10.**iskal(ifog)
      sccl   =10.**iskal(icl)
      sccm   =10.**iskal(icm)
      scch   =10.**iskal(ich)
      scrh   =10.**iskal(irh)
c
      if(irr1.eq.irr2) scrr2=0.
c
      write(6,*) 'prvest'
      write(6,*) 'file:    ',filnam(1:lenstr(filnam,1))
c
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      do l=1,ltekst
        write(iunit,*) tekst(l)(1:ktekst(1,l))
      end do
c
      grad=180./3.1415927
      toms=1852./3600.
c
c..grenser for akseptabel rh
      rhmin=  1.
      rhmax=100.
c
      do 110 ipos=1,npos
c
      write(iunit,*) ' '
      write(iunit,*) navn(ipos)
c
      write(iunit,fmt='(1x,''YYYY MM DD HH  MSLP     T    TD   '',
     *                     ''RH DD FF.M/S N.MM T$KE CL  CM  CH'')')
c
      do 120 nt=1,ntid
c
        do 130 i=1,5
          itimev(i)=itime(i,nt)
  130   continue
        call vtime(itimev,ierror)
c
c..mslp
        p=real(dat(ipos,imslp,nt))*scmslp
c..t2m
        t=real(dat(ipos,it,nt))*sct
c..u10m,v10m
        u=real(dat(ipos,iu,nt))*scu
        v=real(dat(ipos,iv,nt))*scv
c..nedboer (sum)
        rr= real(dat(ipos,irr1,nt))*scrr1
     *     +real(dat(ipos,irr2,nt))*scrr2
c..skyer
        fog=real(dat(ipos,ifog,nt))*scfog
        cl =real(dat(ipos,icl, nt))*sccl
        cm =real(dat(ipos,icm, nt))*sccm
        ch =real(dat(ipos,ich, nt))*scch
c..rh2m
        rh=real(dat(ipos,irh,nt))*scrh
c
c..u10m,v10m ... (input i knop, output dd i grader og ff i hele m/s)
        uew=u
        vns=v
        ff=sqrt(uew*uew+vns*vns)*toms
        if(ff.gt.0.5) then
          iff=nint(ff)
          dd=270.-grad*atan2(vns,uew)
          if(dd.gt.360.) dd=dd-360.
          if(dd.lt.  0.) dd=dd+360.
          idd=nint(dd)
          if(idd.eq.0) idd=360
        else
          idd=0
          iff=0
        end if
c
c..rh+t(c)->td(c) ... from relative humidity
c..                   to dew point temp. (celsius)
        x=(t+105.)*0.2
        i=int(x)
        et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
        if(rh.lt.rhmin) rh=rhmin
        if(rh.gt.rhmax) rh=rhmax
        etd=rh*0.01*et
        do while (ewt(i).gt.etd .and. i.gt.1)
          i=i-1
        end do
        x=(etd-ewt(i))/(ewt(i+1)-ewt(i))
        td=-105.+(real(i)+x)*5.
c
        jrh =nint(rh)
        jfog=nint(fog)
        jcl =nint(cl)
        jcm =nint(cm)
        jch =nint(ch)
c
        write(iunit,fmt='(1x,i4,3(1x,i2.2),1x,f6.1,2f6.1,3i4,f7.1,4i4)')
     *                    (itimev(i),i=1,4),
     *                    p,t,td,jrh,idd,iff,rr,jfog,jcl,jcm,jch
 
  120 continue
c
  110 continue
c
c=====================================================================
c
c     additional output:
c     generate a table with hourly values of cloudiness and cloudtype
c     from 3-hourly input values. Convert from cloudiness in percent to
c     octets and compute one output cloudiness and cloudtype from
c     input fog,cl,cm,ch.
c
c     the subroutine is coded for 3 areas/stations in Vestfold only.
c
      if (npos.eq.3) then
         write(iunit,*) ' '
         write(iunit,'(1x,74(''=''))')
         write(iunit,*) ' '
         write(iunit,*) 'VESTFOLD NORD = ',navn(1)
         write(iunit,*) '         SOER = ',navn(2)
         write(iunit,*) '         INDRE= ',navn(3)
         area(1)='Nord '
         area(2)='Soer '
         area(3)='Indre'
      else
         write(6,*) '***prvest*** modify output for npos.ne.3'
         return
      endif
c
c     loop over start time UTC+3, UTC+6, UTC+9, UTC+12
c
      do istart=3,12,3
c
c     write heading for this table
c
      do i=1,4
        itimev(i)=itime(i,1)
      end do
C##############################################
      NHLOCAL=+1
C##############################################
      itimev(5)=itime(5,1)+istart+NHLOCAL
c     write(6,*) (itimev(i),i=1,5)
      call vtime(itimev,ierror)
c     write(6,*) (itimev(i),i=1,5)
c
      write(iunit,*) ' '
      write(iunit,'(1x,10(''*''),'' Prognose fra '',
     +   i2,''/'',i2,''-'',i4,'' kl '',i2,'':00 norsk normaltid '',
     +   10(''*''))')
     +   itimev(3),itimev(2),itimev(1),itimev(4)
      write(iunit,*) ' '
      write(iunit,'(1x,74(''-''))')
c
c     print hours of the day for the table elements
c
      do iprog=0,nprog
c
      do i=1,4
        itimev(i)=itime(i,1)
      end do
      itimev(5)=itime(5,1)+istart+iprog+NHLOCAL
      call vtime(itimev,ierror)
c
      ih_arr(iprog)=itimev(4)
c
      end do
c
      write(iunit,'(1x,''Norsk normaltid'',3x,10(2x,i2,'':00''))')
     +   (ih_arr(i),i=0,nprog)
      write(iunit,'(1x,74(''-''))')
c
c     loop over positions
c
      do ipos=1,npos
c
c     loop over forecast lengths starting at forecast hour istart
c
      do iprog=0,nprog
c
c       forecast length from UTC
c
        ihour=istart+iprog
c
c       find elements to interpolate between
c
        it=0
        do nt=1,ntid-1
           if (ihour.ge.itime(5,nt) .and. ihour.lt.itime(5,nt+1)) then
              it=nt
              goto 200
           endif
        enddo
 200    continue
        if (it.eq.0) then
           write(6,*) '***prvest*** failed to find valid time'
           return
        endif
c
c..skyer
        fog=real(dat(ipos,ifog,it))*scfog
        cl =real(dat(ipos,icl ,it))*sccl
        cm =real(dat(ipos,icm ,it))*sccm
        ch =real(dat(ipos,ich ,it))*scch
        fogp1=real(dat(ipos,ifog,it+1))*scfog
        clp1 =real(dat(ipos,icl ,it+1))*sccl
        cmp1 =real(dat(ipos,icm ,it+1))*sccm
        chp1 =real(dat(ipos,ich ,it+1))*scch
c
        c=real(ihour-itime(5,it))/(itime(5,it+1)-itime(5,it))
c
        fogx=fog+c*(fogp1-fog)
        clx =cl +c*(clp1 -cl )
        cmx =cm +c*(cmp1 -cm )
        chx =ch +c*(chp1 -ch )
c
c       fog is treated as low clouds
c
        clx=max(clx,fogx)
c
c       cloudiness in octets
c
        if (clx.lt.1.) then
           nclx=0
        else
           nclx=int((clx+12.4)/100.*8.)
        endif
        if (cmx.lt.1.) then
           ncmx=0
        else
           ncmx=int((cmx+12.4)/100.*8.)
        endif
        if (chx.lt.1.) then
           nchx=0
        else
           nchx=int((chx+12.4)/100.*8.)
        endif
c
c       compute one cloudiness and one cloud type
c
        ncmax=max(nclx,ncmx,nchx)
c
        if (ncmax.eq.0) then
           ncloud(iprog)=0
           ccloud(iprog)=' '
        elseif (nclx.eq.ncmax .or. nclx.ge.5) then
           ncloud(iprog)=nclx
           ccloud(iprog)='L'
        elseif (ncmx.eq.ncmax .or. ncmx.ge.5) then
           ncloud(iprog)=ncmx
           ccloud(iprog)='M'
        else
           ncloud(iprog)=nchx
           if (nchx.eq.3) then
              ncloud(iprog)=2
           elseif(nchx.gt.3) then
              ncloud(iprog)=nchx-2
           endif
           ccloud(iprog)='H'
        endif
c
c       write(6,'(1x,''istart,ipos,iprog,ihour,it,c='',5i4,f7.2)')
c    +                 istart,ipos,iprog,ihour,it,c
c       write(6,'(1x,''cl,clp1,clx='',3f7.2)') cl,clp1,clx
c       write(6,'(1x,''cm,cmp1,cmx='',3f7.2)') cm,cmp1,cmx
c       write(6,'(1x,''ch,chp1,chx='',3f7.2)') ch,chp1,chx
c       write(6,'(1x,''nclx,ncmx,nchx,ncmax,ncloud(iprog)'',
c    +                 ''ccloud(iprog)='',
c    +                 5i2,a2)')
c    +                 nclx,ncmx,nchx,ncmax,ncloud(iprog),ccloud(iprog)
c
      end do
c
      write(iunit,'(1x,a5,''  dekke(0-8) '',10(4x,i1,2x))')
     +   area(ipos),(ncloud(i),i=0,nprog)
      write(iunit,'(1x,74(''-''))')
c     write(iunit,'(1x,5x,''  type(L,M,H)'',10(4x,a1,2x))')
c    +   (ccloud(i),i=0,nprog)
c     write(iunit,'(1x,74(''-''))')
c
      end do
c
      end do
c
c=====================================================================
c
      close(iunit)
c
      return
c
  900 write(6,*) '***prvest*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
      return
      end
c
c*********************************************************************
c
      subroutine priccast(iunit,filnam,npos,npar,ntid,
     *                    konpar,iskal,itime,dat,navn,
     *                    ltekst,tekst,ktekst)
c
c       formatert output.  icecast.
c
c       parametre  input:
c          mslp,t2m,u,v,nedboer,taake,cl,cm,ch,ctot,rh2m
c       parametre output:
c          t2m,td2m,ff,enumerated precip (binary),cl,ctot,
c          enumerated main cloud type (1-4)
c
c       note: input  u  is  east/west  wind component in unit knots
c             input  v  is north/south wind component in unit knots
c             output ff  in unit m/s*10
c             output t2m in unit degrees*10
c             output td2m in unit degrees*10
c             output cl in unit octa  
c             output ctot in unit octa  
c
c
      integer   iunit,npos,npar,ntid,ltekst
      integer   konpar(npar),iskal(npar),itime(5,ntid)
      integer   ktekst(2,ltekst)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
c
      integer      itimev(5)
c
c
      real ewt(41)
c
c        vanndampens metningstrykk, ewt(41).
c        t i grader celsius: -100,-95,-90,...,90,95,100.
c            tc=...    x=(tc+105)*0.2    i=x
c            et=ewt(i)+(ewt(i+1)-ewt(i))*(x-i)
c
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
c
      imslp=0
      it   =0
      iu   =0
      iv   =0
      irr1 =0
      irr2 =0
      irrt =0
      ifog =0
      icl  =0
      icm  =0
      ich  =0
      ictot=0
      irh  =0      
      jrr  =999
      iff  =999
      do 100 n=1,npar
c..mslp (hpa)
        if(konpar(n).eq.58) imslp=n
c..t2m (celsius)
        if(konpar(n).eq.31) it=n
c..u10m (e/w component, knots)
        if(konpar(n).eq.33) iu=n
c..v10m (n/s component, knots)
        if(konpar(n).eq.34) iv=n
c..nedboer, frontal
        if(konpar(n).eq.15) irr1=n
c..nedboer, konvektiv
        if(konpar(n).eq.16) irr2=n
c..nedbor, total
c..(ikke akkum fra +0, metdat har laget n timers nedboer)
        if(konpar(n).eq.17) irrt=n
c..skydekke (taake,lave,middelshoeye,hoeye)
        if(konpar(n).eq.39) then
          if(ifog.eq.0) then
            ifog=n
          elseif(icl.eq.0) then
            icl=n
          elseif(icm.eq.0) then
            icm=n
          elseif(ich.eq.0) then
            ich=n
          end if
        end if
c..totalt skydekke
        if(konpar(n).eq.25) ictot=n
c..td2m (celsius)
        if(konpar(n).eq.32) irh=n
  100 continue
c
      if(irrt.gt.0) then
	irr1=irrt
	irr2=irrt
      end if
c
      if(imslp.eq.0 .or.   it.eq.0 .or.   iu.eq.0 .or.  iv.eq.0 .or.
     *    irr1.eq.0 .or. irr2.eq.0 .or. ifog.eq.0 .or. icl.eq.0 .or.
     *     icm.eq.0 .or.  ich.eq.0 .or.  irh.eq.0 .or. ictot.eq.0) then
        write(6,*) '***pricecast*** not correct input data.'
        return
      end if
c
      cmslp =10.**iskal(imslp)
      sct   =10.**iskal(it)
      scu   =10.**iskal(iu)
      scv   =10.**iskal(iv)
      scrr1 =10.**iskal(irr1)
      scrr2 =10.**iskal(irr2)
      scfog =10.**iskal(ifog)
      sccl  =10.**iskal(icl)
      sccm  =10.**iskal(icm)
      scch  =10.**iskal(ich)
      sctot =10.**iskal(ictot)
      scrh  =10.**iskal(irh)
c
      if(irr1.eq.irr2) scrr2=0.
c
      write(6,*) 'pricecast'
      write(6,*) 'file:    ',filnam(1:lenstr(filnam,1))
c
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
cc      do l=1,ltekst
cc        write(iunit,*) tekst(l)(1:ktekst(1,l))
cc      end do
c
      grad=180./3.1415927
      toms=1852./3600.
c
c..grenser for akseptabel rh
      rhmin=  1.
      rhmax=100.
c
      do 110 ipos=1,npos
c
      write(iunit,*) ' '
      write(iunit,*) '@ ',navn(ipos)
c
      do 120 nt=1,ntid-1
c
        do 130 i=1,5
          itimev(i)=itime(i,nt)
  130   continue
        call vtime(itimev,ierror)
c
c..mslp
        p=real(dat(ipos,imslp,nt))*scmslp
c..t2m
        t=real(dat(ipos,it,nt))*sct
c..u10m,v10m
        u1=real(dat(ipos,iu,nt))*scu
        v1=real(dat(ipos,iv,nt))*scv
        u2=real(dat(ipos,iu,nt+1))*scu
        v2=real(dat(ipos,iv,nt+1))*scv
c..nedboer (sum)
        rr=real(dat(ipos,irr1,nt+1))*scrr1
     *    +real(dat(ipos,irr2,nt+1))*scrr2
        if (t.lt.0. .and. rr.gt.2.) then
           jrr = 5
        else if (t.lt.0. .and. rr.gt.0.1) then
           jrr = 4
        else if (rr.ge.8.) then
           jrr = 3
        else if (rr.ge.2.) then
           jrr = 2
        else if (rr.ge.0.1) then
           jrr = 1
        else
           jrr = 0
        end if
c..skyer
        fog1=real(dat(ipos,ifog,nt))*scfog
        cl1 =real(dat(ipos,icl, nt))*sccl
        cm1 =real(dat(ipos,icm, nt))*sccm
        ch1 =real(dat(ipos,ich, nt))*scch
        fog2=real(dat(ipos,ifog,nt+1))*scfog
        cl2 =real(dat(ipos,icl, nt+1))*sccl
        cm2 =real(dat(ipos,icm, nt+1))*sccm
        ch2 =real(dat(ipos,ich, nt+1))*scch
        fog =(fog1+fog2)/2.0
        cl  =(cl1+cl2)/2.0
        cm  =(cm1+cm2)/2.0
        ch  =(ch1+ch2)/2.0
        cmax =max(fog,cl,cm,ch)
        if (fog.ge.99.9) then
          jcmax =1
        elseif (cl.eq.cmax.or.cl.ge.62.5.or.fog.ge.62.5) then
          jcmax =1
        elseif (cm.eq.cmax.or.cm.ge.62.5) then
          jcmax =2
        else
          jcmax =3
        end if
        ctot1=real(dat(ipos,ictot,nt  ))*sctot
        ctot2=real(dat(ipos,ictot,nt+1))*sctot
        ctot =(ctot1+ctot2)/2.0
c..rh2m
        rh=real(dat(ipos,irh,nt))*scrh
c
c..u10m,v10m ... (input i knop, output ff i hele m/s)
        ff1=sqrt(u1*u1+v1*v1)*toms
        ff2=sqrt(u2*u2+v2*v2)*toms
        ff = (ff1+ff2)/2.0
        if(ff.gt.0.5) then
          iff=nint(ff)*10
c          dd=270.-grad*atan2(vns,uew)
c          if(dd.gt.360.) dd=dd-360.
c          if(dd.lt.  0.) dd=dd+360.
c          idd=nint(dd)
c          if(idd.eq.0) idd=360
        else
c          idd=0
          iff=0
        end if
c
c..rh+t(c)->td(c) ... from relative humidity
c..                   to dew point temp. (celsius)
        x=(t+105.)*0.2
        i=int(x)
        et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
        if(rh.lt.rhmin) rh=rhmin
        if(rh.gt.rhmax) rh=rhmax
        etd=rh*0.01*et
        do while (ewt(i).gt.etd .and. i.gt.1)
          i=i-1
        end do
        x=(etd-ewt(i))/(ewt(i+1)-ewt(i))
        td=-105.+(real(i)+x)*5. 
c
        jt   = nint(t*10)
        jtd  = nint(td*10)        
        jcl  = nint(cl*8./100.)
        jctot= nint(ctot*8./100.)
c       Cloudtype undefined for Total Cloud Amount=0
        if (jctot.eq.0) jcmax=0
c
        write(iunit,fmt='(1x,i2,a3,1x,i2,a1,i2,a1,i4,1x,
     *  6(A7,i4))')
     *  itimev(4),':00',itimev(3),'/',itimev(2),'/',itimev(1),
     *  ' FAT= ',jt,' FDT= ',jtd,' FWS= ',iff,
     *  ' FTC= ',jctot,' FCT= ',jcmax,' FPS= ',jrr
c
  120 continue
c
  110 continue
c
      write(iunit,*) '#'
      close(iunit)
c
      return
c
  900 write(6,*) '***pricecast*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
      return
      end
c
c*********************************************************************
c
      subroutine prverifi(iunit,filnam,npos,npar,ntid,iparam,skalp,
     *                    itime,dat,navn,ltekst,tekst,ktekst,
     *                    vsfile,vtable,itimev,ntimev,hlpdat,ierror)
c
c       formatert output.  verifikasjon (input til verifi-systemet).
c
c       parametre  input: u,v,t2m,nedb@r,ch,cm,cl,fg,totsky,mslp
c       parametre output: dd,ff,t2m,rr3,ch,cm,cl,fg,totsky,mslp
c
c   note: input u10m(33) is  east/west  wind component in unit knots
c         input v10m(34) is north/south wind component in unit knots
c         input t2m(31)  in unit degrees celsius
c        output ff       in unit m/s
c
c
      integer   iunit,npos,npar,ntid,ltekst,ierror
      integer   iparam(npar),itime(5,ntid)
      integer   ktekst(2,ltekst),itimev(5,ntid),ntimev(ntid)
      real      skalp(npar),hlpdat(npar,ntid)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
      character*(*) vsfile
      character*(*) vtable
c
      parameter (maxsyn=1000)
      character*30 snavn(maxsyn),stemp
      integer      isnum(maxsyn),lsnavn(maxsyn),iantsyn
      integer      init
c
      parameter  (mverpar=12)
      integer     iverpar(mverpar),idatnum(mverpar)
      character*4  verpar(mverpar),verparout(mverpar)
c
      data  verpar/'  DD', '  FF', '  TT', '  TD',
     *             '  RR', '   N', '   P',
     *             '  FG', '  CL', '  CM', '  CH',
     *		   'TT_K'/
      data iverpar/   33,     34,     31,      5,
     *                17,     25,     58,
     *                39,     39,     39,     39,
     *		   10031/
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
            write(6,*) vsfile(1:lenstr(vsfile,1))
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
              write(6,*) vsfile(1:lenstr(vsfile,1))
              write(6,*) '    iostat= ',ios,'  at line no. ',iasyn+1
              isyno=0
            end if
          end do
          close(iunit)
          if(iasyn.gt.maxsyn) then
            write(6,*) '***prverifi*** : too many stations on file:'
            write(6,*) vsfile(1:lenstr(vsfile,1))
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
      nverpar=0
      nuu=0
      nvv=0
      nrr=0
      nrrxx=0
      n39=0
      do n=1,mverpar
        k=0
        if(iverpar(n).eq.39) then
c..clouds in several heights (all with parameter no. 39)
          n39=n39+1
          i39=0
          do i=1,npar
            if(iparam(i).eq.39) i39=i39+1
            if(iparam(i).eq.39 .and. i39.eq.n39) k=i
          end do
        else
          do i=1,npar
            if(iparam(i).eq.iverpar(n)) k=i
          end do
	  if(k.eq.0 .and. iverpar(n).eq.17) then
c..if total precipitation (17) missing, try 19+20 and 15+16
            do i=1,npar-1
              if(iparam(i).eq.19 .and. iparam(i+1).eq.20) k=i
            end do
	    if(k.eq.0) then
              do i=1,npar-1
                if(iparam(i).eq.15 .and. iparam(i+1).eq.16) k=i
              end do
	    end if
	  end if
        end if
c..don't even try to overload the hlpdat array (nverpar<=npar)
        if(k.gt.0 .and. nverpar.lt.npar) then
          nverpar=nverpar+1
          idatnum(nverpar)=k
          verparout(nverpar)=verpar(n)
          if(iverpar(n).eq.33) nuu=nverpar
          if(iverpar(n).eq.34) nvv=nverpar
          if(iverpar(n).eq.17) nrr=nverpar
          if(iverpar(n).eq.17 .and. iparam(k).eq.19) nrrxx=nverpar
          if(iverpar(n).eq.17 .and. iparam(k).eq.15) nrrxx=nverpar
        end if
      end do
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
c..check if more than one forecast handled (for accum.precipitation)
      ntimev(1)=1
      do n=2,ntid
	ntimev(n)=0
	if(itime(1,n).ne.itime(1,n-1) .or.
     +     itime(2,n).ne.itime(2,n-1) .or.
     +     itime(3,n).ne.itime(3,n-1) .or.
     +     itime(4,n).ne.itime(4,n-1) .or.
     +     itime(5,n).le.itime(5,n-1)) ntimev(n)=1
      end do
c
      write(6,*) 'prverifi'
      write(6,*) 'file: ',filnam(1:lenstr(filnam,1))
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
c..constants used below
      grad=180./3.1415927
      toms=1852./3600.
c
      write(iunit,*) 'VERIFI 200'
c
      if(vtable(1:1).eq.'*') then
        write(iunit,*) tekst(1)(1:ktekst(1,1))
      else
        write(iunit,*) vtable(1:lenstr(vtable,1))
      end if
c
      do 100 ipos=1,npos
c
        isyno=0
        i=0
        do while (isyno.eq.0 .and. i.lt.iantsyn)
          i=i+1
          if(navn(ipos).eq.snavn(i)) isyno=i
        end do
        if (isyno.eq.0) then
          if(iantsyn.ne.-1)
     *       write(6,*) '***prverifi*** no number for ',navn(ipos)
          stemp = navn(ipos)
          isyno = 9999
        else
          stemp = snavn(isyno)
          isyno = isnum(isyno)
        end if
c
        write(iunit,'(1x,i6,2x,a30)') isyno,stemp
c
        write(iunit,'(1x,6a5,20(4x,a4,:))')
     *              'AAR','MND','DAG','TIM','PROG','NIVA',
     *              (verparout(n),n=1,nverpar)
c
        do n=1,nverpar
          i=idatnum(n)
          do l=1,ntid
            hlpdat(n,l)=dat(ipos,i,l)*skalp(i)
          end do
        end do
c
        if(nuu.gt.0 .and. nvv.gt.0) then
c..from u(e/w),v(n/s) in knots to dd,ff(m/s)
          grad=180./3.1415927
          toms=1852./3600.
          do l=1,ntid
            uew=hlpdat(nuu,l)
            vns=hlpdat(nvv,l)
            ff=sqrt(uew*uew+vns*vns)*toms
            if(ff.gt.0.01) then
              dd=270.-grad*atan2(vns,uew)
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
            else
              dd=0.
              ff=0.
            end if
            hlpdat(nuu,l)=dd
            hlpdat(nvv,l)=ff
          end do
        end if
c
c..add the two types of precipitation
        if(nrrxx.gt.0) then
	  irrxx=idatnum(nrrxx)+1
          do l=1,ntid
            hlpdat(nrrxx,l)= hlpdat(nrrxx,l)
     *                      +dat(ipos,irrxx,l)*skalp(irrxx)
          end do
        end if
c
c..precipitation accumulated from start of forecast
c..(first timestep not necessarily 0 hours forecast)
c..no accumualtion if different forecasts
c..(assuming that program metdat always has made 'step precipitation')
        if(nrr.gt.0) then
          do l=2,ntid
	    if(ntimev(l).eq.0)
     +         hlpdat(nrr,l)=hlpdat(nrr,l-1)+hlpdat(nrr,l)
          end do
        end if
c
        nivp=0
c
        do l=1,ntid
c
          write(iunit,fmt='(1x,6(i5),20(1x,f7.2,:))')
     *                (itimev(i,l),i=1,5),nivp,
     *                (hlpdat(n,l),n=1,nverpar)
c
        end do
c
c..end of station
        write(iunit,fmt='(1x,6(i5),20(1x,f7.2,:))')
     *              9999,99,99,99,99,99,
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
      write(6,*)  filnam(1:lenstr(filnam,1))
c
      return
      end
c
c*********************************************************************
c
      subroutine prnormem(iunit,filnam,npos,npar,ntid,iparam,skalp,
     *                    itime,dat,navn,ltekst,tekst,ktekst,
     *                    iprod,itimev,hlpdat,ierror)
c
c       formatert output.  verifikasjon (input til verifi-systemet).
c
c       parametre  input: u,v,t2m,nedb@r,ch,cm,cl,fg,totsky,mslp
c       parametre output: dd,ff,t2m,rr3,ch,cm,cl,fg,totsky,mslp
c
c   note: input u10m(33) is  east/west  wind component in unit knots
c         input v10m(34) is north/south wind component in unit knots
c         input t2m(31)  in unit degrees celsius
c        output ff       in unit m/s
c
c
      integer   iunit,npos,npar,ntid,ltekst,iprod,ierror
      integer   iparam(npar),itime(5,ntid)
      integer   ktekst(2,ltekst),itimev(5,ntid)
      real      skalp(npar),hlpdat(npar,ntid)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
c
      parameter  (mparam=10)
      integer     ipar(mparam),ifmt(2,mparam),kpar(mparam),jpar(mparam)
      character*20 par(mparam)
c
      character*256 ptext
      character*128 cfmt
      character*40  tmptxt
c
      data par/'TEMP[degC]',
     +	       'SPEED[m/s]',
     +	       'DIR[deg]',
     +	       'PRESSURE[hPa]',
     +	       'PRECIP[mm]',
     +	       'FOG[pct]',
     +	       'LOW_CLOUDS[pct]',
     +	       'MED_CLOUDS[pct]',
     +	       'HI_CLOUDS[pct]',
     +	       'TOT_CLOUDS[pct]'/
c
      data ipar/31,  34,  33,  58,  17,  39,  39,  39,  39,  25/
c
      data ifmt/6,1, 5,1, 4,0, 6,1, 5,1, 4,0, 4,0, 4,0, 4,0, 4,0/
c
      ierror=1
c
      nparam=0
      nuu=0
      nvv=0
      n39=0
      n25=0
      do n=1,mparam
        k=0
        do i=1,npar
          if(iparam(i).eq.ipar(n)) k=i
        end do
	if(ipar(n).eq.25 .and. n39.gt.0) k=0
        if(ipar(n).eq.39 .and. k.gt.0) then
c..clouds in several heights (all with parameter no. 39)
          n39=n39+1
          i39=0
	  k=0
          do i=1,npar
            if(iparam(i).eq.39) i39=i39+1
            if(iparam(i).eq.39 .and. i39.eq.n39) k=i
          end do
	end if
        if(k.gt.0) then
          nparam=nparam+1
          kpar(nparam)=k
          if(ipar(n).eq.33) nuu=nparam
          if(ipar(n).eq.34) nvv=nparam
          jpar(nparam)=n
        end if
      end do
c..don't even try to overload the hlpdat array
      if(nparam.gt.npar) nparam=npar
c
      if(nparam.lt.1) then
        write(6,*) '***prnormem*** no suitable data for NORMEM output.'
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
          write(6,*) '***prnormem*** : illegal date,time,prog:'
          write(6,*) (itime(i,n),i=1,5)
          nerr=nerr+1
        end if
        itimev(5,n)=itime(5,n)
      end do
      if(nerr.gt.0) return
c
      call hrdiff(0,0,itime(1,1),itime(1,ntid),nhours,ierr1,ierr2)
c
c..constants used below
      grad=180./3.1415927
      toms=1852./3600.
c
      write(6,*) 'prnormem'
      write(6,*) 'file: ',filnam(1:lenstr(filnam,1))
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
      call daytim(iyear,month,iday,ihour,minute,isecond)
c
      write(iunit,fmt='(''[GENERAL]'')')
      write(iunit,fmt='(''Organisation = "DNMI"'')')
      write(iunit,fmt='(''Responsible = "SALTBONES,JØRGEN"'')')
      write(iunit,fmt=
     + '(''Production date = '',i4.4,2(''.'',i2.2),1x,i2.2,'':'',i2.2)')
     +			iyear,month,iday,ihour,minute
      write(iunit,fmt='(''File format = SIMULATION'')')
      write(iunit,*)
      write(iunit,fmt='(''[SIMULATION]'')')
      write(iunit,fmt='(''Simulation type = METEOGRAM'')')
c
      ptext='TIME[time]'
      ktext=16
      kx2=0
      cfmt='(i4.4,2(''.'',i2.2),1x,i2.2,'':'',i2.2'
      kfmt=lenstr(cfmt,0)
      do n=1,nparam
	if(kx2.lt.0) ktext=ktext-kx2
	ktext=ktext+1
	kx0=1
	if(kx2.gt.0) kx0=1+kx2
	j=jpar(n)
	kk=lenstr(par(j),1)
	kd=index(par(j),'[')
	kkfmt=ifmt(1,j)
	kdfmt=ifmt(2,j)
	kx1=(kd-1)-(kkfmt-1-kdfmt)
	kx2=(kk-kd)-kdfmt
	if(kx1.lt.0) ktext=ktext-kx1
	ptext(ktext+1:ktext+kk)=par(j)(1:kk)
	ktext=ktext+kk
	if(kx0+kx1.gt.0) then
	  write(tmptxt,fmt='('','',i2.2,''x,f'',i1,''.'',i1)')
     +				kx0+kx1,ifmt(1,j),ifmt(2,j)
	  cfmt(kfmt+1:kfmt+9)=tmptxt(1:9)
	  kfmt=kfmt+9
	else
	  write(tmptxt,fmt='('',f'',i1,''.'',i1)') ifmt(1,j),ifmt(2,j)
	  cfmt(kfmt+1:kfmt+5)=tmptxt(1:5)
	  kfmt=kfmt+5
	end if 
      end do
      cfmt(kfmt+1:kfmt+1)=')'
      kfmt=kfmt+1
c
      do 100 ipos=1,npos
c
        write(iunit,*)
        write(iunit,fmt='(''[METEOGRAM]'')')
	if(iprod.eq.88) then
          write(iunit,fmt='(''Meteogram name = METEOSHORT'')')
	else
          write(iunit,fmt='(''Meteogram name = METEOLONG'')')
	end if
	k=lenstr(navn(ipos),1)
	write(iunit,fmt='(''Location = "'',a,''"'')') navn(ipos)(1:k)
        write(iunit,fmt='(''Table type = TIMESERIES'')')
        write(iunit,*)
c
        write(iunit,fmt='(a)') ptext(1:ktext)
c
        do n=1,nparam
          i=kpar(n)
          do l=1,ntid
            hlpdat(n,l)=dat(ipos,i,l)*skalp(i)
          end do
        end do
c
        if(nuu.gt.0 .and. nvv.gt.0) then
c..from u(e/w),v(n/s) in knots to dd,ff(m/s)
          do l=1,ntid
            uew=hlpdat(nuu,l)
            vns=hlpdat(nvv,l)
            ff=sqrt(uew*uew+vns*vns)*toms
            if(ff.gt.0.01) then
              dd=270.-grad*atan2(vns,uew)
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
            else
              dd=0.
              ff=0.
            end if
            hlpdat(nuu,l)=dd
            hlpdat(nvv,l)=ff
          end do
        end if
c
	minute=00
c
        do l=1,ntid
c
          write(iunit,fmt=cfmt) (itimev(i,l),i=1,4),minute,
     +				(hlpdat(n,l),n=1,nparam)
c
        end do
c
  100 continue
c
      close(iunit)
      ierror=0
c
      return
c
  900 write(6,*) '***prnormem*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
c
      return
      end
c
c*********************************************************************
c
      subroutine prmuluse(iunit,filnam,npos,npar,ntid,iparam,skalp,
     *                    itime,dat,navn,ltekst,tekst,ktekst,
     *                    itimev,hlpdat,iopt,nxpar,ixparam,xskal,xdat,
     *			  ierror)
c
c       formatert output..meteogrammer,hav- og bølge-diagrammer
c
c	iopt=0: Stasjonsnavn
c		ff in unit knots
c	iopt=1: Stasjonsnavn + "  lat=xx.xx lon=xx.xx"
c		ff in unit m/s
c
c       parametre  input: u,v,t2m,nedb@r,ch,cm,cl,fg,totsky,mslp,td,rr,
c                         hs,stu,stv,hst,tst,hsp,tsp,ddpp,hsd,tsd,ddpd
c       parametre output: dd,ff,t2m,rr3,ch,cm,cl,fg,totsky,mslp,td,rr
c                         hs,stdd,stff,hst,tst,hsp,tsp,ddpp,hsd,tsd,ddpd
c
c   note: input u10m(33) is  east/west  wind component in unit knots
c         input v10m(34) is north/south wind component in unit knots
c         input t2m(31)  in unit degrees celsius
c        output ff       in unit knots (iopt=0) or m/s (iopt=1)
c        output stff     in unit m/s
c
c
      integer   iunit,npos,npar,ntid,ltekst,iopt,ierror
      integer   iparam(npar),itime(5,ntid)
      integer   ktekst(2,ltekst),itimev(5,ntid)
      real      skalp(npar),hlpdat(npar,ntid)
      integer*2 dat(npos,npar,ntid)
      integer   nxpar
      integer   ixparam(nxpar)
      real      xskal(nxpar)
      integer*2 xdat(npos,nxpar)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
c
      parameter  (mverpar=25)
      integer     iverpar(mverpar),idatnum(mverpar)
      character*4  verpar(mverpar),verparout(mverpar)
      character*8  clat,clon
c
      data  verpar/'  DD', '  FF', '  TT', '  TD',
     *             '  RH', '  RR', '   N', '   P',
     *             '  FG', '  CL', '  CM', '  CH',
     *		   '  HS', 'STDD', 'STFF', 
     *             '  FF', '  DD', ' HST', ' TST',
     *             ' HSP', ' TSP', 'DDPP', ' HSD',
     *             ' TSD', 'DDPD'/
      data iverpar/   33,     34,     31,      5,
     *                32,     17,     25,     58,
     *                39,     39,     39,     39,
     *		     301,    302,    303, 
     *               298,    299,    200,    203,
     *               210,    211,    212,    220,
     *               221,    222/
c
c
      ierror=1
c
      idiatyp = 1
      nverpar=0
      nuu=0
      nvv=0
      nstuu=0
      nstvv=0
      nrr15=0
      nrr17=0
      n39=0
      do n=1,mverpar
        k=0
        if(iverpar(n).eq.39) then
c..clouds in several heights (all with parameter no. 39)
          n39=n39+1
          i39=0
          do i=1,npar
            if(iparam(i).eq.39) i39=i39+1
            if(iparam(i).eq.39 .and. i39.eq.n39) k=i
          end do
        else
          do i=1,npar
            if(iparam(i).eq.iverpar(n)) k=i
          end do
        end if
        if(k.gt.0) then
          nverpar=nverpar+1
          idatnum(nverpar)=k
          verparout(nverpar)=verpar(n)
          if(iverpar(n).eq.33)  nuu=nverpar
          if(iverpar(n).eq.34)  nvv=nverpar
          if(iverpar(n).eq.15)  nrr15=nverpar
          if(iverpar(n).eq.17)  nrr17=nverpar
          if(iverpar(n).eq.302) nstuu=nverpar
          if(iverpar(n).eq.303) nstvv=nverpar
          if(iverpar(n).eq.200) idiatyp=3
        end if
      end do
c..don't even try to overload the hlpdat array
      if(nverpar.gt.npar) nverpar=npar
c
      if(nverpar.lt.1) then
        write(6,*) '***prmuluse*** no suitable data for printing.'
        return
      end if
c
      irr16=0
      if(nrr15.gt.0) then
c..check if two types of precipitation (param. 15 and 16)
        do i=2,npar
          if(iparam(i-1).eq.15 .and. iparam(i).eq.16) irr16=i
        end do
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
          write(6,*) '***prmuluse*** : illegal date,time,prog:'
          write(6,*) (itime(i,n),i=1,5)
          nerr=nerr+1
        end if
        itimev(5,n)=itime(5,n)
      end do
c
      nxlat=0
      nxlon=0
      if(iopt.gt.0) then
	do n=1,nxpar
	  if(ixparam(n).eq.-1) nxlat=n
	  if(ixparam(n).eq.-2) nxlon=n
	end do
	if(nxlat.lt.1 .or. nxlon.lt.1) then
	  write(6,*) '***prmuluse*** : Latitude,longitude not found'
          nerr=nerr+1
	end if
      end if
c
      if(nerr.gt.0) return
c
      write(6,*) 'prmuluse'
      write(6,*) 'file: ',filnam(1:lenstr(filnam,1))
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
c..constants used below
      grad=180./3.1415927
      toms=1852./3600.
c
      ffscale=1.0
      if (iopt.eq.1) ffscale=toms
c
      write(iunit,*) 'DNMI - PROGNOSER'
      write(iunit,*) ' '
      if (ltekst.gt.0) then
        do itx=1,ltekst
          write(iunit,*) tekst(itx)(1:ktekst(1,itx))
        enddo
        write(iunit,*) ' '
      end if
c
      do 100 ipos=1,npos
c
c..lines and posname
        maxcr=21+7*nverpar
	if(maxcr.gt.200) maxcr=200
        write(iunit,'(1x,200(a1))') ('=',ij=1,maxcr)
c
	if(iopt.eq.0) then
          write(iunit,'(1x,a30)') navn(ipos)
	else
	  write(clat,fmt='(f8.2)') real(xdat(ipos,nxlat))*xskal(nxlat)
	  write(clon,fmt='(f8.2)') real(xdat(ipos,nxlon))*xskal(nxlon)
	  kk=len(clat)
	  klat=0
	  klon=0
	  do k=1,kk
	    if(klat.eq.0 .and. clat(k:k).ne.' ') klat=k
	    if(klon.eq.0 .and. clon(k:k).ne.' ') klon=k
	  end do
	  n=lenstr(navn(ipos),0)
          write(iunit,1010) navn(ipos)(1:n),clat(klat:kk),clon(klon:kk)
 1010	  format(1x,a,'  lat=',a,' lon=',a)
	end if
c
        write(iunit,*) ('-',ij=1,30)
c
        write(iunit,'(1x,4a4,a5,25(3x,a4,:))')
     *              'AAR','MND','DAG','TIM','PROG',
     *              (verparout(n),n=1,nverpar)
c
        do n=1,nverpar
          i=idatnum(n)
          do l=1,ntid
            hlpdat(n,l)=dat(ipos,i,l)*skalp(i)
          end do
        end do
c
        if(nuu.gt.0 .and. nvv.gt.0) then
c..from u(e/w),v(n/s) in knots to dd,ff(knots) or ff(m/s)
          do l=1,ntid
            uew=hlpdat(nuu,l)
            vns=hlpdat(nvv,l)
            ff=sqrt(uew*uew+vns*vns)*ffscale
            if(ff.gt.0.01) then
              dd=270.-grad*atan2(vns,uew)
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
            else
              dd=0.
              ff=0.
            end if
            hlpdat(nuu,l)=dd
            hlpdat(nvv,l)=ff
          end do
        end if
c
c current u,v from sea-diagrams
        if(nstuu.gt.0 .and. nstvv.gt.0) then
c..from u(e/w),v(n/s) in m/s to dd,ff(m/s)
          idiatyp=2
          do l=1,ntid
            uew=hlpdat(nstuu,l)
            vns=hlpdat(nstvv,l)
            ff=sqrt(uew*uew+vns*vns)
            if(ff.gt.0.01) then
              dd=270.-grad*atan2(vns,uew)
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
            else
              dd=0.
              ff=0.
            end if
            hlpdat(nstuu,l)=dd
            hlpdat(nstvv,l)=ff
          end do
        end if
c
c
c..add the two types of precipitation
        if(nrr15.gt.0 .and. irr16.gt.0) then
          do l=1,ntid
            hlpdat(nrr15,l)= hlpdat(nrr15,l)
     *                      +dat(ipos,irr16,l)*skalp(irr16)
          end do
        end if
c
        iprogl= 0
c..print time and parameters
        do l=1,ntid
c
         if (idiatyp.eq.1) then
          write(iunit,fmt='(1x,4(i4),i5,25(1x,f6.1,:))')
     *                (itimev(i,l),i=1,5),
     *                (hlpdat(n,l),n=1,nverpar)
         else if (idiatyp.eq.2) then
          write(iunit,fmt='(1x,4(i4),i5,25(1x,f6.2,:))')
     *                (itimev(i,l),i=1,5),
     *                (hlpdat(n,l),n=1,nverpar)
         else
          itmp=0
          if (l.gt.1) itmp=itimev(4,l)-itimev(4,l-1)
          if (itmp.lt.0) itmp= 24+itmp
          iprogl= iprogl+itmp
          write(iunit,fmt='(1x,4(i4),i5,25(1x,f6.1,:))')
     *                (itimev(i,l),i=1,4),iprogl,
     *                (hlpdat(n,l),n=1,nverpar)
         end if
c
        end do
c
c..end of station
        write(iunit,*) ' '
c
  100 continue
c
c..end of file
c
      close(iunit)
      ierror=0
c
      return
c
  900 write(6,*) '***prmuluse*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
c
      return
      end
c
c*********************************************************************
c
      subroutine prsdv(iunit,filnam,npos,npar,ntid,iparam,skalp,
     *                 itime,dat,navn,ltekst,tekst,ktekst,
     *                 itimev,hlpdat,ierror)
c
c     Text output of diagram data. The SDV format is based on
c     print_multiuse, but here the columns of the ascii output are
c     separated by semicolons rather than blanks. The columns are
c     not justified. The output is designed to be fed easily into 
c     external formatting programs.
c
c     input parameters:  u,v,t2m,nedb@r,ch,cm,cl,fg,totsky,mslp,td,rr,
c                        hs,stu,stv,hst,tst,hsp,tsp,ddpp,hsd,tsd,ddpd
c     output parameters: dd,ff,t2m,rr3,ch,cm,cl,fg,totsky,mslp,td,rr
c                        hs,stdd,stff,hst,tst,hsp,tsp,ddpp,hsd,tsd,ddpd
c
c     note: input u10m(33) is  east/west  wind component in unit knots
c           input v10m(34) is north/south wind component in unit knots
c           input t2m(31)  in unit degrees celsius
c           output ff      in unit knots
c           output stff    in unit m/s
c
c
      integer   iunit,npos,npar,ntid,ltekst,ierror
      integer   iparam(npar),itime(5,ntid)
      integer   ktekst(2,ltekst),itimev(5,ntid)
      real      skalp(npar),hlpdat(npar,ntid)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
      character*256 tmpline,outline
      integer ic1,ic2
c
      parameter   (mverpar=25)
      integer     iverpar(mverpar),idatnum(mverpar)
      character*6 verpar(mverpar),verparout(mverpar)
c
      data  verpar/'    DD', '    FF', '    TT', '    TD',
     *             '    RH', '    RR', '     N', '  MSLP',
     *             '    FG', '    CL', '    CM', '    CH',
     *		   '    HS', '  STDD', '  STFF',
     *             '    FF', '    DD', '   HST', '   TST',
     *             '   HSP', '   TSP', '  DDPP', '   HSD',
     *             '   TSD', '  DDPD'/
      data iverpar/   33,     34,     31,      5,
     *                32,     17,     25,     58,
     *                39,     39,     39,     39,
     *		     301,    302,    303, 
     *               298,    299,    200,    203,
     *               210,    211,    212,    220,
     *               221,    222/
c
c
      ierror=1
c
      nverpar=0
      nuu=0
      nvv=0
      nstuu=0
      nstvv=0
      nrr15=0
      nrr17=0
      n39=0
c
      tmpline=' '
      outline=' '
c
      do n=1,mverpar
        k=0
        if(iverpar(n).eq.39) then
c..clouds in several heights (all with parameter no. 39)
          n39=n39+1
          i39=0
          do i=1,npar
            if(iparam(i).eq.39) i39=i39+1
            if(iparam(i).eq.39 .and. i39.eq.n39) k=i
          end do
        else
          do i=1,npar
            if(iparam(i).eq.iverpar(n)) k=i
          end do
        end if
        if(k.gt.0) then
          nverpar=nverpar+1
          idatnum(nverpar)=k
          verparout(nverpar)=verpar(n)
          if(iverpar(n).eq.33)  nuu=nverpar
          if(iverpar(n).eq.34)  nvv=nverpar
          if(iverpar(n).eq.15)  nrr15=nverpar
          if(iverpar(n).eq.17)  nrr17=nverpar
          if(iverpar(n).eq.302) nstuu=nverpar
          if(iverpar(n).eq.303) nstvv=nverpar
        end if
      end do
c..don't even try to overload the hlpdat array
      if(nverpar.gt.npar) nverpar=npar
c
      if(nverpar.lt.1) then
        write(6,*) '***prsdv*** no suitable data for printing.'
        return
      end if
c
      irr16=0
      if(nrr15.gt.0) then
c..check if two types of precipitation (param. 15 and 16)
        do i=2,npar
          if(iparam(i-1).eq.15 .and. iparam(i).eq.16) irr16=i
        end do
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
          write(6,*) '***prsdv*** : illegal date,time,prog:'
          write(6,*) (itime(i,n),i=1,5)
          nerr=nerr+1
        end if
        itimev(5,n)=itime(5,n)
      end do
      if(nerr.gt.0) return
c
      write(6,*) 'prsdv'
      write(6,*) 'file: ',filnam(1:lenstr(filnam,1))
c
      open(iunit,file=filnam,
     *           access='sequential',form='formatted',
     *           status='unknown',iostat=ios,err=900)
c
c..constants used below
      grad=180./3.1415927
      toms=1852./3600.
c
      if (ltekst.gt.0) then
        do itx=1,ltekst
          write(iunit,*) tekst(itx)(1:ktekst(1,itx))
        enddo
        write(iunit,*) ' '
      end if
c
      do 100 ipos=1,npos
c
c..lines and posname
        write(iunit,'(a30)') navn(ipos)
        write(tmpline,501) 'DATO/TID', (verparout(n),n=1,nverpar)
        ic2=1
        do ic1=1,100
           if(tmpline(ic1:ic1).ne.' ') then
              outline(ic2:ic2)=tmpline(ic1:ic1)
              ic2=ic2+1
           endif
        enddo
        write(iunit,fmt='(a100)') outline
        outline=' '
        tmpline=' '
c
        do n=1,nverpar
          i=idatnum(n)
          do l=1,ntid
            hlpdat(n,l)=dat(ipos,i,l)*skalp(i)
          end do
        end do
c
        if(nuu.gt.0 .and. nvv.gt.0) then
c..from u(e/w),v(n/s) in knots to dd,ff(knots)
          do l=1,ntid
            uew=hlpdat(nuu,l)
            vns=hlpdat(nvv,l)
            ff=sqrt(uew*uew+vns*vns)
C            ff=sqrt(uew*uew+vns*vns)*toms
            if(ff.gt.0.01) then
              dd=270.-grad*atan2(vns,uew)
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
            else
              dd=0.
              ff=0.
            end if
            hlpdat(nuu,l)=dd
            hlpdat(nvv,l)=ff
          end do
        end if
c
c current u,v from sea-diagrams
        if(nstuu.gt.0 .and. nstvv.gt.0) then
c..from u(e/w),v(n/s) in m/s to dd,ff(m/s)
          do l=1,ntid
            uew=hlpdat(nstuu,l)
            vns=hlpdat(nstvv,l)
            ff=sqrt(uew*uew+vns*vns)
            if(ff.gt.0.01) then
              dd=270.-grad*atan2(vns,uew)
              if(dd.gt.360.) dd=dd-360.
              if(dd.le.  0.) dd=dd+360.
            else
              dd=0.
              ff=0.
            end if
            hlpdat(nstuu,l)=dd
            hlpdat(nstvv,l)=ff
          end do
        end if
c
c
c..add the two types of precipitation
        if(nrr15.gt.0 .and. irr16.gt.0) then
          do l=1,ntid
            hlpdat(nrr15,l)=hlpdat(nrr15,l)
     *                      +dat(ipos,irr16,l)*skalp(irr16)
          end do
        end if
c
        iprogl= 0
c..print time and parameters
        do l=1,ntid
c
           write(outline,502) (itimev(i,l),i=1,4), 0, 0
           write(tmpline,503) (hlpdat(n,l),n=1,nverpar)
           ic2=20
           do ic1=1,100
              if(tmpline(ic1:ic1).ne.' ') then
                 outline(ic2:ic2)=tmpline(ic1:ic1)
                 ic2=ic2+1
              endif
           enddo
           write(iunit,fmt='(a100)') outline
           outline=' '
           tmpline=' '
c
        end do
c
c..end of station
        write(iunit,*) ' '
c
  100 continue
c
c..end of file
c
      close(iunit)
      ierror=0
c
 501  format(a8,25(';',a6,:))
 502  format(i4.4,2('-',i2.2),1x,i2.2,2(':',i2.2))
 503  format(25(';',f6.1,:))
c
      return
c
  900 write(6,*) '***prsdv*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
c
      return
      end
c
c*********************************************************************
c
      subroutine prtparam(iunit,filnam,npos,npar,ntid,iparam,skalp,
     *                    itime,dat,navn,ltekst,tekst,ktekst,
     *                    itimev,hlpdat,
     *		          nparprt,iparprt,tparprt,fparprt,dparprt,
     *                    jparprt,kparprt,jparam,ierror)
c
c       formatert output ... "format.print_param"
c	parameters specified with "print_param=....."
c
c	WARNING: Units m/s and Celsius dependant on proper handling
c		 in metdat, NOT identified by parameter no.
c		 Same for level of model level wind (special param.
c		 no. set in metdat.input to identify level here,
c		 this is NOT a general feature in metdat)
c
c
      integer   iunit,npos,npar,ntid,ltekst,ierror
      integer   iparam(npar),itime(5,ntid)
      integer   ktekst(2,ltekst),itimev(5,ntid)
      real      skalp(npar),hlpdat(npar,ntid)
      integer*2 dat(npos,npar,ntid)
      character*(*) filnam
      character*(*) navn(npos)
      character*(*) tekst(ltekst)
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
      ierror=1
c
c..check that all parameters exist, and handle multiple parameters
c..with same parameter no. (e.g. param. 39, clouds in several levels)
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
      cfmt='(i4.4,3i4.2,sp,i5,ss'
      lfmt=lenstr(cfmt,0)
      head=' AAR MND DAG UTC PROG'
      lhead=lenstr(head,0)
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
	cfmt(lfmt+1:lfmt+4+l)= ','//tx(lx)//','//fparprt(n)(1:l)
	lfmt=lfmt+4+l
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
      write(iunit,fmt='(a)') 'DNMI : PROGNOSER'
      write(iunit,fmt='(a)') '--------------------------------'
c
      do n=1,ltekst
        write(iunit,fmt='(a)') tekst(n)(1:ktekst(1,n))
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
      do 100 ipos=1,npos
c
c..lines and posname
        write(iunit,fmt='(a)')
        write(iunit,fmt='(a)') navn(ipos)(1:lenstr(navn(ipos),1))
        write(iunit,fmt='(a)') '--------------------------------'
        write(iunit,fmt='(a)') head(1:lhead)
c
        do n=1,nprt
          i=kparprt(n)
          do l=1,ntid
            hlpdat(n,l)=dat(ipos,i,l)*skalp(i)
          end do
        end do
c
c..print time and parameters
c
        do l=1,ntid
          write(iunit,fmt=cfmt) (itimev(i,l),i=1,5),
     +                          (hlpdat(n,l),n=1,nprt)
        end do
        write(iunit,fmt='(a)') '--------------------------------'
c
  100 continue
c
c..end of file
c
      close(iunit)
      ierror=0
c
      return
c
  900 write(6,*) '***prtparam*** open error.   iostat= ',ios
      write(6,*)  filnam(1:lenstr(filnam,1))
c
      return
      end
