      program sondat
c
c        ***** data for prognostiske sonderinger *****
c
c        *****  input: felt fra felt-file(r) ****
c        *****         polarstereografisk/geografiske grid *******
c        ***** output: punkt-data for sonderinger,         *******
c        *****         evt. konvertering til pc-/nd-format *******
c
c        ***** nb| kan benyttes til aa ta ut 'ikke standard' data *
c
c
c        punkt-verdier tas fra felt i polarstereografisk eller
c        geografisk(e) grid.       vind plottes relativt nord.
c
c        NB! Denne versjonen av sondat kan p.g.a. den nye flt2pos
c        ikke haandtere flere input-grid samtidig.
c
c
c sondat.input:
c-----------------------------------------------------------------------
c example.
c=======================================================================
c *** sondat.input  ('sondat.input')
c ***
c *=> Data for prognostic soundings from LAM50S.
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    sondat  sondat.input  fltsXX.dat  +0,+48  sond.dat
c *=>                          <input>     <prog>  <output>
c ***
c ***------------------------------------------------------------------
c **
c ** Options:
c ** --------
c ** FILE=<output_file>
c ** FORMAT.STANDARD ................................ (default)
c ** FORMAT.SWAP
c ** FORMAT.PC
c ** FORMAT.IBM
c ** POSITIONS.HERE ................................. (default)
c ** POSITIONS.FILE=<file>
c ** GRID=<producer,grid>
c ** PROG_LIMIT=<min_prog_hour,max_prog_hour> .... (default = no limit)
c ** DATA.PRESSURE_LEVELS=<pressure_levels(mb)_from_top_to_bottom>
c ** DATA.SIGMA_LEVELS=<no_of_sigma_levels>
c ** DATA.SIGMA_LEVEL=<level_no,level_no,....>
c ** SIGMA.PS=<vert.coord.,parameter,level_1,level_2>
c ** DATA.ETA_LEVELS=<no_of_sigma_levels>
c ** DATA.ETA_LEVEL=<level_no,level_no,....>
c ** ETA.PS=<vert.coord.,parameter,level_1,level_2>
c ** DATA.SIGMA.MM5_LEVELS=<no_of_sigma_levels>
c ** DATA.SIGMA.MM5_LEVEL=<level_no,level_no,....>
c ** DATA.UM_LEVELS=<no_of_levels>
c ** DATA.UM_LEVEL=<level_no,level_no,....>
c ** TOPOGRAPHY=<datatype,hour,vert.coord.,parameter,level_1,level_2>
c ** SURFACE.OFF .................................... (default)
c ** SURFACE.ON
c ** SURFACE.TEMP=   <vert.coord.,parameter,level_1,level_2>
c ** SURFACE.REL.HUM=<vert.coord.,parameter,level_1,level_2>
c ** SURFACE.WIND.U= <vert.coord.,parameter,level_1,level_2>
c ** SURFACE.WIND.V= <vert.coord.,parameter,level_1,level_2>
c ** INTERP.BESSEL
c ** INTERP.BILINEAR ................................ (default)
c ** INTERP.NEAREST
c ** CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
c ** CHECK.UNDEF.IN.EACH.FIELD
c ** PRINT_POS
c ** PRINT_DATA_IN
c ** PRINT_DATA_OUT
c ** INFO.OFF
c ** INFO.ON ........................................ (default)
c ** INFO.MAX
c ** END
c **
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c ** Options    Remember '......' syntax.
c ** ($... = environment var. ; #n = coomand line arg. no. n, n>1)
c *>
c 'FILE= #4'                    <<< 'FILE= sond50s.dat'
c 'FORMAT.STANDARD'
c 'POSITIONS.HERE'
c 'GRID= 88,1814'
c 'PROG_LIMIT= #3'              <<< 'PROG_LIMIT= +0,+48'
c 'DATA.ETA_LEVELS= 31'
c 'SIGMA.PS=2,8,1000,0'
c 'INTERP.BILINEAR'
c 'PRINT_POS'
c 'PRINT_DATA_IN'
c 'PRINT_DATA_OUT'
c 'INFO.ON'
c 'END'
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c **
c ** 'file': 'file.dat' -  file name
c **         '$.....'   -  environment var.
c **         '#n'       -  command line argument no. n
c **         '='        -  same file as previous timestep
c **
c ** no. of timesteps
c *> each timestep: 'file',data_type,forecast_length_hour,'menu_text'
c 9
c '#2',3,0,'+0', '=',2, 6, '+6', '=',2,12,'+12', '=',2,18,'+18',
c                '=',2,24,'+24', '=',2,30,'+30', '=',2,36,'+36',
c                '=',2,42,'+42', '=',2,48,'+48'
c **
c ** 'code': '='            : no special treatment
c **         'P'            : store pressure (fixed input levels or
c **                          computed from PS and sigma values)
c **         'TH->T(C)'     : potential temp to temp(Celsius)
c **         'T(K)->T(C)'   : abs. temp (Kelvin) to temp (Celsius)
c **         'Q+T->TD(C)'   : spec.hum and temp to dewpoint_temp(Celsius)
c **                          in sigma/eta levels
c **         'Q+T->RH'      : spec.hum and temp to rel.humidity
c **                          in sigma/eta levels
c **         'RH+T->TD(C)'  : rel.hum and temp to dewpoint_temp(Celsius)
c **         'U->U.EW(K)'   : U(x) in m/s to U(e/w) in knots
c **         'V->V.NS(K)'   : V(y) in m/s to V(n/s) in knots
c **         'U->U.EW'      : U(x) to U(e/w) (no scaling, i.e. m/s)
c **         'V->V.NS'      : V(y) to V(n/s) (no scaling, i.e. m/s)
c **         'U->DD'        : U(x) to DD (direction)
c **         'V->FF'        : V(y) to FF (speed), no scaling, i.e. m/s
c **         'V->FF(K)'     : V(y) to FF (speed), m/s to knots
c **         'HEIGHT'       : store height above mean sea level,
c **                          computed from TOPOGRAPHY and temperature
c **         'H.ABOVE.SURF' : store height above surface,
c **                          computed from temperature
c **         'MIN=<value>'  : check minimum value after interpolation
c **         'MAX=<value>'  : check maximum value after interpolation
c **         'SCALE=<value>': extra scaling
c **         'DIRECTION'    : true north direction (dd)
c **         'DIRECTION.180': true north direction (dd), turned 180 deg.
c **         'PARAM=<n>'    : set a new output parameter no.
c **
c ** No. of parameters
c *> each parameter: pamater_no,'code', scale
c 6
c 00, 'P',          -1  <-- avsetter plass for trykk (leser PS)
c 18, 'TH->T(C)',   -1  <-- potensiell temp ; T(C) output
c  9, 'Q+T->TD(C)', -1  <-- spes.fukt.(sigma) ; Td(C) output
c  2, 'U->U.EW(K)', -2  <-- u(x) m/s input ; u(e/w) knop output
c  3, 'V->V.NS(K)', -2  <-- v(y) m/s input ; v(n/s) knop output
c 13, '=',          -5  <-- omega (ingen beregning)
c **
c ** No. of text lines     .... max 10 lines
c *> 'text_line(s)'        .... may include '$...' or '#n' variables
c 1
c '--- DNMI LAM50S ---'
c **
c ** When 'POSITIONS.HERE':
c **
c ** pos_type:  1 : pos_1,pos_2  - grid  i,j  (integer)
c **            2 : pos_1,pos_2  - grid  x,y  (real)
c **            3 : pos_1,pos_2  - geographic latitude,longitude (real)
c **            4 : pos_1,pos_2  - geographic latitude,longitude
c **                                          as degrees*100+minutes
c **            0 : 00000,00000  - end_of_list
c **
c ** station_number: nnnnn - zone*1000+national_number ; TEMP station
c **                 99000 - TEMP ship, search for (first) TEMP in
c **                                    geographic area:
c **                                    pos_latitude  +/- delta_lat
c **                                    pos_longitude +/- delta_long
c **                 00000 - no TEMP station
c ** (station_number is not used here, only in plotting program)
c **
c ** pos_type, pos_1,pos_2, 'name', station_number,delta_lat,delta_long
c *>
c 4, 7056, -0840, 'JAN MAYEN',       01001, 0.,0.
c 4, 7431,  1901, 'BJORNOYA',        01028, 0.,0.
c 4, 6715,  1424, 'BODO',            01152, 0.,0.
c 4, 6342,  0936, 'ORLAND',          01241, 0.,0.
c 4, 6012,  1106, 'GARDERMOEN',      01384, 0.,0.
c 4, 5825,  0540, 'SOLA',            01415, 0.,0.
c 4, 6600,  0200, 'MIKE',            99000, 2.,2.
c 4, 6106,  1029, 'LILLEHAMMER',     00000, 0.,0.
c 0, 0000,  0000, '*',               00000, 0.,0.   <<< end_of_list
c **-------------------------------------------------------------------
c======================================================================
c
c POSITIONS.FILE=....
c File format:
c -------------------------
c======================================================================
C 4, 7056, -0840, 'JAN MAYEN',       01001, 0.,0.
C 4, 7431,  1901, 'BJORNOYA',        01028, 0.,0.
C 4, 6715,  1424, 'BODO',            01152, 0.,0.
C 4, 6342,  0936, 'ORLAND',          01241, 0.,0.
C 4, 6012,  1106, 'GARDERMOEN',      01384, 0.,0.
C 4, 5825,  0540, 'SOLA',            01415, 0.,0.
C 4, 6600,  0200, 'MIKE',            99000, 2.,2.
C 4, 6106,  1029, 'LILLEHAMMER',     00000, 0.,0.
C 0, 0000,  0000, '*',               00000, 0.,0.   <<< end_of_list
c=======================================================================
c
c
c------------------------------------------------------------
c output parametre for prognostiske sonderinger (max):
c ----------------------------------------------------
c        1: trykk (hpa = mb)
c        2: temperatur (grader celsius)
c        3: duggpunkts-temperatur (grader celsius)
c        4: vind-komponent @st/vest (knop)
c        5: vind-komponent nord/s@r (knop)
c        6: omega (hpa/s)
c------------------------------------------------------------
c input parametre, sigma-flater:
c -----------------------------
c      ( 0: ps (hpa = mb) .... parameter=8, niv}=1000 (fast |) )
c        1: potensiell temperatur (grader kelvin)
c        2: spesifikk  fuktighet  (kg/kg)
c        3: vind-komponent x-retning (m/s)
c        4: vind-komponent y-retning (m/s)
c        5: omega (hpa/s)
c------------------------------------------------------------
c input parametre, trykk-flater:
c ------------------------------
c      ( 0: trykk-niv$ (hpa = mb) gitt som input til programmet )
c        1: potensiell temperatur (grader kelvin)
c        2: relativ    fuktighet  (%)
c        3: vind-komponent x-retning (m/s)
c        4: vind-komponent y-retning (m/s)
c     (( 5: omega (hpa/s) ))
c------------------------------------------------------------
c
c
c-----------------------------------------------------------------------
c
c      dnmi library subroutines:  rlunit
c                                 rcomnt
c                                 getvar
c                                 keywrd
c                                 chcase
c                                 prhelp
c                                 rmfile
c                                 bswap2
c                                 flt2pos
c                                 bput*
c
c=======================================================================
c  FILE=<output_file>
c  FORMAT.STANDARD ................................ (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  POSITIONS.HERE ................................. (default)
c  POSITIONS.FILE=<file>
c  GRID=<producer,grid>
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> .... (default = no limit)
c  DATA.PRESSURE_LEVELS=<pressure_levels(mb)_from_top_to_bottom>
c  DATA.SIGMA_LEVELS=<no_of_sigma_levels>
c  DATA.SIGMA_LEVEL=<level_no,level_no,....>
c  SIGMA.PS=<vert.coord.,parameter,level_1,level_2>
c  DATA.ETA_LEVELS=<no_of_sigma_levels>
c  DATA.ETA_LEVEL=<level_no,level_no,....>
c  ETA.PS=<vert.coord.,parameter,level_1,level_2>
c  DATA.SIGMA.MM5_LEVELS=<no_of_sigma_levels>
c  DATA.SIGMA.MM5_LEVEL=<level_no,level_no,....>
c  DATA.UM_LEVELS=<no_of_levels>
c  DATA.UM_LEVEL=<level_no,level_no,....>
c  TOPOGRAPHY=<datatype,hour,vert.coord.,parameter,level_1,level_2>
c  SURFACE.OFF .................................... (default)
c  SURFACE.ON
c  SURFACE.TEMP=   <vert.coord.,parameter,level_1,level_2>
c  SURFACE.REL.HUM=<vert.coord.,parameter,level_1,level_2>
c  SURFACE.WIND.U= <vert.coord.,parameter,level_1,level_2>
c  SURFACE.WIND.V= <vert.coord.,parameter,level_1,level_2>
c  INTERP.BESSEL
c  INTERP.BILINEAR ................................ (default)
c  INTERP.NEAREST
C  CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
C  CHECK.UNDEF.IN.EACH.FIELD
c  PRINT_POS
c  PRINT_DATA_IN
c  PRINT_DATA_OUT
c  INFO.OFF
c  INFO.ON ........................................ (default)
c  INFO.MAX
c  END
c=======================================================================
c
c-----------------------------------------------------------------------
c  DNMI/FoU  xx.xx.198x  Anstein Foss ... IBM
c  DNMI/FoU  26.09.1991  Anstein Foss
c  DNMI/FoU  18.12.1992  Anstein Foss ... Unix
c  DNMI/FoU  07.10.1993  Anstein Foss
c  DNMI/FoU  15.03.1994  Anstein Foss
c  DNMI/FoU  29.03.1995  Anstein Foss ... call flt2pos (also for Hirlam)
c  DNMI/FoU  30.03.1995  Anstein Foss ... surface data (sigma/eta)
c  DNMI/FoU  07.04.1995  Anstein Foss ... bput* routines
c  DNMI/FoU  18.08.1995  Anstein Foss ... delpos handling temp.number
c  DNMI/FoU  29.12.1995  Anstein Foss
c  DNMI/FoU  22.03.1996  Anstein Foss ... not read surf. when not used
c  DNMI/FoU  14.06.1996  Anstein Foss ... minor flt2pos update
c  DNMI/FoU  02.09.1996  Anstein Foss ... better height in eta levels
c  DNMI/FoU  13.12.1999  Anstein Foss ... sigma.MM5
c  DNMI/FoU  17.03.2001  Anstein Foss ... automatic byte swap (input)
c  DNMI/FoU  28.06.2001  Anstein Foss ... 'q+t->rh', 'h.above.surf' (NILU)
c  DNMI/FoU  31.10.2003  Anstein Foss ... format.standard always bigendian
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c-----------------------------------------------------------------------
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for sondat.f
c
c        maxpos - posisjoner/stasjoner
c        maxpar - parametre
c        maxniv - niv$
c        maxtid - tidspunkt
c        maxdat - total datamengde (npos*npar*nniv*ntid)
c        maxflt - st@rrelse av felt fra felt-file (nx*ny)
c        maxgrd - antall grid/sektorer input
c
ccc   parameter (maxpos=300,maxpar=12,maxniv=50,maxtid=24)
c
ccc   parameter (maxdat=300000)
c
ccc   parameter (maxflt=64000,maxgrd=1)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'sondat.inc'
c
c..buffer and file record length
      parameter (lbuff=512)
c
      integer*2 itid(3,maxtid),iniv(2,maxniv),ipar(8,maxpar)
      integer*2 igrd(2,maxgrd),idsave(4,maxpar*maxniv*maxtid,maxgrd)
c
      integer   iskalp(maxpar),iparam(maxpar),jparam(maxpar)
      integer   itime(5,maxtid,maxgrd),itimok(maxtid)
c
      real      pos(3,maxpos),parlim(2,maxpar)
      real      geopos(2,maxpos,maxgrd),gxypos(2,maxpos,maxgrd)
      real      dat(maxdat),dwork(maxpos),datout(maxniv*maxpar)
      real      grid(6,maxgrd),skalp(maxpar)
c
      integer   idobs(maxpos),iposid(6,maxpos),iposit(2,6)
      real      ridobs(2,maxpos)
c
      integer   jinter(maxpos*3,maxgrd)
      real      rinter(16*maxpos),vturn(4,maxpos)
c
      integer      ltekst,ktekst(2,10),ntnavn(maxpos),ktidtx(maxtid)
      character*80 tekst(10)
      character*30 navn(maxpos)
      character*16 tidtxt(maxtid)
c
      integer*2 idat(20+maxflt),buff(lbuff)
      real      felt(maxflt,2)
c
      integer   idato(5)
      integer   kpos(maxpos),ktid(maxtid)
      integer   idupl(maxpos)
      integer   inpniv(maxniv+1),inppar(maxpar),inps(4),itopog(6)
      integer   insurf(5,5)
      integer   ihead(8),icont(20),idnum(16)
      integer   iufelt(maxtid)
      real      aaa(maxniv*maxtid),bbb(maxniv*maxtid)
      real      aa2(maxniv+1),bb2(maxniv+1)
      real      pptop(maxtid)
      integer*2 idfile(32)
c
      character*64 pcode(maxpar)
c
      character*256 filepo,fltfil(maxtid),filein,fileot
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*80 cinput,cipart
      character*256 finput
c
      logical   swapfile,swap,bigendian
c
      iprhlp=0
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c..file unit for 'sondat.input'
      iuinp=9
c
c..file unit for station list file
      iunitp=10
c
c..file unit for output data file
      iunito=11
c
c..file unit for first input felt file
      iunitf=12
c
c--------------------------------------------------------------------
c
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: sondat <sondat.input>'
        write(6,*) '      or: sondat <sondat.input> <arguments>'
        write(6,*) '      or: sondat <sondat.input> ?     (to get help)'
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
        write(6,*) finput
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
      write(6,*) finput
c
      nlines = 0
c
      iformt =-1
      inppos =-1
      ngrd   = 0
      ivcoor = 0
      nniv   = 0
      interp = 0
      iprpos = 0
      iprdin = 0
      iprdot = 0
      info   = 1
      iundef =-1
c
      fileot ='*'
      filepo ='*'
c
c..default for ps (p_surface): vert.coord.,parameter,level_1,level_2
      inps(1)=2
      inps(2)=8
      inps(3)=1000
      inps(4)=0
c
c..default for topography: dtype,hour,vert.coord.,parameter,lvl_1,lvl_2
c..(used for 'height' parameter)
      itopog(1)=4
      itopog(2)=0
      itopog(3)=2
      itopog(4)=101
      itopog(5)=1000
      itopog(6)=0
c
c..default for surface parameters: temp,rel.hum,u,v
c..(pressure,height,omega handled too)
      do n=1,4
        insurf(1,n)=2
        insurf(3,n)=1000
        insurf(4,n)=0
        insurf(5,n)=0
      end do
      insurf(2,1)=31
      insurf(2,2)=32
      insurf(2,3)=33
      insurf(2,4)=34
      isurf=-1
c
      iprmin = -32767
      iprmax = +32767
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
        if(ierror.ne.0) goto 232
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
c  FILE=<output_file>
c  FORMAT.STANDARD ................................ (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  POSITIONS.HERE ................................. (default)
c  POSITIONS.FILE=<file>
c  GRID=<producer,grid>
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> .... (default = no limit)
c  DATA.PRESSURE_LEVELS=<pressure_levels(mb)_from_top_to_bottom>
c  DATA.SIGMA_LEVELS=<no_of_sigma_levels>
c  DATA.SIGMA_LEVEL=<level_no,level_no,....>
c  SIGMA.PS=<vert.coord.,parameter,level_1,level_2>
c  DATA.ETA_LEVELS=<no_of_sigma_levels>
c  DATA.ETA_LEVEL=<level_no,level_no,....>
c  ETA.PS=<vert.coord.,parameter,level_1,level_2>
c  DATA.SIGMA.MM5_LEVELS=<no_of_sigma_levels>
c  DATA.SIGMA.MM5_LEVEL=<level_no,level_no,....>
c  DATA.UM_LEVELS=<no_of_levels>
c  DATA.UM_LEVEL=<level_no,level_no,....>
c  TOPOGRAPHY=<datatype,hour,vert.coord.,parameter,level_1,level_2>
c  SURFACE.OFF .................................... (default)
c  SURFACE.ON
c  SURFACE.TEMP=   <vert.coord.,parameter,level_1,level_2>
c  SURFACE.REL.HUM=<vert.coord.,parameter,level_1,level_2>
c  SURFACE.WIND.U= <vert.coord.,parameter,level_1,level_2>
c  SURFACE.WIND.V= <vert.coord.,parameter,level_1,level_2>
c  INTERP.BESSEL
c  INTERP.BILINEAR ................................ (default)
c  INTERP.NEAREST
C  CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
C  CHECK.UNDEF.IN.EACH.FIELD
c  PRINT_POS
c  PRINT_DATA_IN
c  PRINT_DATA_OUT
c  INFO.OFF
c  INFO.ON ........................................ (default)
c  INFO.MAX
c  END
c======================================================================
c
          if(cinput(k1:k2).eq.'file') then
c..file=<output_felt_file>
            if(fileot(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            fileot=cinput(kv1:kv2)
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
          elseif(cinput(k1:k2).eq.'positions.here') then
c..positions.here
            if(inppos.ne.-1) goto 214
            inppos=0
          elseif(cinput(k1:k2).eq.'positions.file') then
c..positions.file=<file>
            if(inppos.ne.-1) goto 214
            if(kv1.lt.1) goto 213
            filepo=cinput(kv1:kv2)
            inppos=1
          elseif(cinput(k1:k2).eq.'grid') then
c..grid=<producer,grid>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            ngrd=ngrd+1
            if(ngrd.gt.maxgrd) goto 233
            read(cipart,*,err=213) (igrd(i,ngrd),i=1,2)
          elseif(cinput(k1:k2).eq.'prog_limit') then
c..prog_limit=<min_prog_hour,max_prog_hour>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) iprmin,iprmax
          elseif(cinput(k1:k2).eq.'data.pressure_levels') then
c..data.pressure_levels=<pressure_levels(mb)_from_top_to_bottom>
            if(ivcoor.ne.0 .and. ivcoor.ne.1) goto 214
            ivcoor=1
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//char(0)
            i1=nniv+1
            i2=nniv
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxniv) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (inpniv(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nniv=i2
            do i=2,nniv
              if(inpniv(i).le.inpniv(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.sigma_levels' .or.
     +           cinput(k1:k2).eq.'data.eta_levels'   .or.
     +           cinput(k1:k2).eq.'data.sigma.mm5_levels' .or.
     +           cinput(k1:k2).eq.'data.um_levels') then
c..data.sigma_levels=<no_of_sigma_levels>
c..data.eta_levels=<no_of_sigma_levels>
c..data.sigma.mm5_levels=<no_of_sigma_levels>
c..data.um_levels=<no_of_sigma_levels>
            jvcoor=2
            if(cinput(k1:k2).eq.'data.eta_levels') jvcoor=10
            if(cinput(k1:k2).eq.'data.sigma.mm5_levels') jvcoor=12
            if(cinput(k1:k2).eq.'data.um_levels') jvcoor=12
            if(ivcoor.ne.0) goto 214
            ivcoor=jvcoor
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nniv
            do i=1,nniv
              inpniv(i)=i
            end do
          elseif(cinput(k1:k2).eq.'data.sigma_level' .or.
     +           cinput(k1:k2).eq.'data.eta_level'   .or.
     +           cinput(k1:k2).eq.'data.sigma.mm5_level' .or.
     +           cinput(k1:k2).eq.'data.um_level') then
c..data.sigma_level=<level_no,level_no,....>
c..data.eta_level=<level_no,level_no,....>
c..data.sigma.mm5_level=<level_no,level_no,....>
c..data.um_level=<level_no,level_no,....>
            jvcoor=-2
            if(cinput(k1:k2).eq.'data.eta_level') jvcoor=-10
            if(cinput(k1:k2).eq.'data.sigma.mm5_level') jvcoor=-12
            if(cinput(k1:k2).eq.'data.um_level') jvcoor=-12
            if(ivcoor.ne.0 .and. ivcoor.ne.jvcoor) goto 214
            ivcoor=jvcoor
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//char(0)
            i1=nniv+1
            i2=nniv
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxniv) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (inpniv(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nniv=i2
            do i=2,nniv
              if(inpniv(i).le.inpniv(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'sigma.ps' .or.
     +           cinput(k1:k2).eq.'eta.ps') then
c..sigma.ps=<vert.coord.,parameter,level_1,level_2>
c..eta.ps=<vert.coord.,parameter,level_1,level_2>
c..(for computing pressure in sigma and eta levels
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) (inps(i),i=1,4)
          elseif(cinput(k1:k2).eq.'topography') then
c..topography=<datatype,hour,vert.coord.,parameter,level_1,level_2>
c..(for computation of height above mean sea level)
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) (itopog(i),i=1,6)
          elseif(cinput(k1:k2).eq.'surface.off') then
c  surface.off
            if(isurf.ne.-1 .and. isurf.ne.0) goto 213
            isurf=0
          elseif(cinput(k1:k2).eq.'surface.on') then
c  surface.on
            if(isurf.ne.-1 .and. isurf.ne.1) goto 213
            isurf=1
          elseif(cinput(k1:k2).eq.'surface.temp') then
c  surface.temp=   <vert.coord.,parameter,level_1,level_2>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) (insurf(i,1),i=1,4)
          elseif(cinput(k1:k2).eq.'surface.rel.hum') then
c  surface.rel.hum=<vert.coord.,parameter,level_1,level_2>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) (insurf(i,2),i=1,4)
          elseif(cinput(k1:k2).eq.'surface.wind.u') then
c  surface.wind.u= <vert.coord.,parameter,level_1,level_2>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) (insurf(i,3),i=1,4)
          elseif(cinput(k1:k2).eq.'surface.wind.v') then
c  surface.wind.v= <vert.coord.,parameter,level_1,level_2>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) (insurf(i,4),i=1,4)
          elseif(cinput(k1:k2).eq.'interp.bilinear') then
c..interp.bilinear
            if(interp.ne.0) goto 214
            interp=1
          elseif(cinput(k1:k2).eq.'interp.bessel') then
c..interp.bessel
            if(interp.ne.0) goto 214
            interp=3
          elseif(cinput(k1:k2).eq.'interp.nearest') then
c..interp.nearest
            if(interp.ne.0) goto 214
            interp=4
          elseif(cinput(k1:k2).eq.'check.undef.in.first.field') then
c  check.undef.in.first.field
            if(iundef.ne.-1) goto 214
            iundef=1
          elseif(cinput(k1:k2).eq.'check.undef.in.each.field') then
c  check.undef.in.each.field
            if(iundef.ne.-1) goto 214
            iundef=2
          elseif(cinput(k1:k2).eq.'print_pos') then
c..print_pos
            iprpos=1
          elseif(cinput(k1:k2).eq.'print_data_in') then
c..print_data_in
            iprdin=1
          elseif(cinput(k1:k2).eq.'print_data_out') then
c..print_data_out
            iprdot=1
          elseif(cinput(k1:k2).eq.'info.off') then
c..info.off
            info=0
          elseif(cinput(k1:k2).eq.'info.on') then
c..info.on
            info=1
          elseif(cinput(k1:k2).eq.'info.max') then
c..info.max
            info=2
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
c..grid=
      if(ngrd.eq.0) goto 219
c..data.......=
      if(ivcoor.eq.0) goto 220
      if(ivcoor.lt.0) ivcoor=-ivcoor
c..file=
      if(fileot(1:1).eq.'*') goto 227
c
c..default reading positions from the 'sondat.input' file
      if(inppos.lt.0) inppos=0
c
c..default output file format
      if(iformt.lt.0) iformt=0
c
c..default interpolation type is bilinear
      if(interp.eq.0) interp=1
c
c..default checking undefined field values only in first field
      if(iundef.eq.-1) iundef=1
c
c..default surface.off (and not surface if pressure levels)
      if(ivcoor.eq.1) isurf=0
      if(isurf.eq.-1) isurf=0
      if(isurf.gt. 1) isurf=1
      if(nniv+isurf.gt.maxniv) then
        write(6,*) 'too many levels when adding surface.  max: ',maxniv
        goto 240
      end if
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
c..no. of timesteps
      read(iuinp,*,iostat=ios,err=211,end=212) ntid
c
      if(ntid.lt.1 .or. ntid.gt.maxtid) then
        write(6,*) ' no. of timesteps: ',ntid
        write(6,*) '  min,max allowed:   1 ',maxtid
        goto 240
      end if
c
c        hvert tidspunkt: 'file',data-type,prognosetid og 'tekst'
c                         'file' = '=' => som forrige
      read(iuinp,*,iostat=ios,err=211,end=212)
     *             (fltfil(n),(itid(i,n),i=2,3),tidtxt(n),n=1,ntid)
c
      do n=1,ntid
        if(fltfil(n)(1:1).eq.'=') then
          if(n.eq.1) then
            write(6,*) ' no felt file name for first timestep.'
            goto 240
          end if
          fltfil(n)=fltfil(n-1)
        else
          call getvar(1,fltfil(n),1,1,1,ierror)
          if(ierror.ne.0) goto 232
        end if
      end do
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
c..no. of parameters
      read(iuinp,*,iostat=ios,err=211,end=212) npar
c
      if(npar.lt.1 .or. npar.gt.maxpar) then
        write(6,*) ' no. of parameters: ',npar
        write(6,*) '   min,max allowed:   1 ',maxpar
        goto 240
      end if
c
c-----------------------------------------------------------------------
c        inppar(n) - parameter nr.
c         pcode(n) - kode for evt. beregning m.m.
c        iskalp(n) - output skalering  (ivalue=value*10.**(-iskalp))
c
c     pcode: '='            : no special treatment
c            'p'            : store pressure (fixed input levels or
c                             computed from ps and sigma/eta values)
c            'th->t(c)'     : potential temp to temp(celsius)
c            't(k)->t(c)'   : abs. temp (kelvin) to temp (celsius)
c            'q+t->td(c)'   : spec.hum and temp to dewpoint_temp(celsius)
c                             in sigma/eta levels
c            'q+t->rh'      : spec.hum and temp to rel.humidity
c                             in sigma/eta levels
c            'rh+t->td(c)'  : rel.hum and temp to dewpoint_temp(celsius)
c            'u->u.ew(k)'   : u(x) in m/s to u(e/w) in knots
c            'v->v.ns(k)'   : v(y) in m/s to v(n/s) in knots
c            'u->u.ew'      : u(x) to u(e/w) (no scaling, i.e. m/s)
c            'v->v.ns'      : v(y) to v(n/s) (no scaling, i.e. m/s)
c            'u->dd'        : u(x) to dd (direction)
c            'v->ff'        : v(y) to ff (speed), no scaling, i.e. m/s
c            'v->ff(k)'     : v(y) to ff (speed), m/s to knots
c            'height'       : store height above mean sea level,
c                             computed from topography and temperature
c            'h.above.surf' : store height above surface,
c                             computed from temperature
c            'min=<value>'  : check minimum value after interpolation
c            'max=<value>'  : check maximum value after interpolation
c            'scale=<value>': extra scaling
c            'direction'    : true north direction (dd)
c            'direction.180': true north direction (dd), turned 180 deg.
c            'param=<n>'    : set a new output parameter no.
c-----------------------------------------------------------------------
c
      do n=1,npar
        read(iuinp,*,iostat=ios,err=211,end=212)
     *               inppar(n),pcode(n),iskalp(n)
      end do
c
      call chcase(1,npar,pcode)
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
c..text for plot
c                 nb| teksten kan plasseres ved en "kommando"
c                     bakerst p$ linjen(e):
c                     :v: => til venstre (vanlig)
c                     :s: => "sentreres"
c                     :h: => til h@yre
c                     (:v:  :s:  :h:  fjernes f@r plotting)
c
      read(iuinp,*,iostat=ios,err=211,end=212) ltekst
c
      if(ltekst.gt.10) then
        write(6,*) ' **** too many text lines (plot text): ',ltekst
        write(6,*) ' ****       max allowed:  10'
        goto 240
      end if
c
      do l=1,ltekst
        read(iuinp,*,iostat=ios,err=211,end=212) tekst(l)
        call getvar(1,tekst(l),1,1,1,ierror)
        if(ierror.ne.0) goto 232
      end do
c
c..positions .... in 'sondat.input' or separate file
c
      if(inppos.eq.0) then
c..positions in 'sondat.input'
        call rcomnt(iuinp,'*>','*',nlines,ierror)
        if(ierror.ne.0) goto 210
        iu=iuinp
      else
c..positions in separate file
        write(6,*) 'reading positions from file:'
        write(6,*)  filepo
        open(iunitp,file=filepo,
     *              access='sequential',form='formatted',
     *              status='old',iostat=ios)
        if(ios.ne.0) then
          write(6,*) 'open error.  file:'
          write(6,*)  filepo
          goto 249
        end if
        iu=iunitp
        nlines=0
      end if
c
c        "posisjons-type"   1=i,j   2=x,y   3=b,l
c                           4=b,l gitt som grader*100+minutter
c                           0=slutt
c
c        posisjons-type,pos1,pos2,'navn',istanum,dlat,dlong
c
      itype=999
      n=0
      do while (itype.gt.0 .and. n.lt.maxpos)
        n=n+1
        read(iu,*,iostat=ios,err=211,end=212)
     *          itype,(pos(i,n),i=2,3),navn(n),
     *          idobs(n),ridobs(1,n),ridobs(2,n)
        if(itype.eq.1 .or. itype.eq.2) then
          pos(1,n)=1.
        elseif(itype.eq.3) then
          pos(1,n)=2.
        elseif(itype.eq.4) then
          pos(1,n)=2.
          ilat=nint(pos(2,n))
          ilon=nint(pos(3,n))
          ilatd=ilat/100
          ilond=ilon/100
          ilatm=ilat-ilatd*100
          ilonm=ilon-ilond*100
          if(ilatd.ge. -90 .and. ilatd.le. +90 .and.
     +       ilond.ge.-360 .and. ilond.le.+360 .and.
     +       ilatm.ge. -59 .and. ilatm.le. +59 .and.
     +       ilonm.ge. -59 .and. ilonm.le. +59) then
            pos(2,n)=real(ilatd)+real(ilatm)/60.
            pos(3,n)=real(ilond)+real(ilonm)/60.
          else
            write(6,*) 'BAD LAT/LONG POSITION:'
            write(6,*) '    ',ilat,ilon,' ',navn(n)
            n=n-1
          end if
        else
          pos(1,n)=1.
        end if
      end do
      if(itype.gt.0) then
        n=maxpos+1
        read(iu,*,iostat=ios,err=211,end=212) itype
        if(itype.gt.0) then
          write(6,*) ' *** too many positions.'
          write(6,*) ' ***   max no. ("maxpos") = ',maxpos
          write(6,*) ' *** continue without the last position(s)'
        end if
      end if
c
      npos=n-1
c
      if(npos.lt.1) then
        write(6,*) ' ***  no positions.'
        goto 249
      end if
c
      if(iu.ne.iuinp) close(iu)
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
  219 write(6,*) 'no input grid specified.'
      goto 240
  220 write(6,*) 'no vertical coordinate specified.'
      goto 240
  227 write(6,*) 'no output file specified.'
      goto 240
  232 iprhlp=1
      goto 240
  233 write(6,*) 'too many specifications of the type below:'
      write(6,*) cinput
      goto 240
c
  240 write(6,*) 'error at line no. ',nlines,'   (or below)'
      if(iprhlp.eq.1) then
        write(6,*) 'help from ''sondat.input'':'
        call prhelp(iuinp,'*=>')
      end if
  249 close(iuinp)
      stop 1
c
  250 close(iuinp)
c
      write(6,*) 'input o.k.'
c
c..for interpolation (in flt2pos)
      ndimri=1
      if(interp.eq.1) ndimri= 4
      if(interp.eq.2) ndimri= 9
      if(interp.eq.3) ndimri=16
      if(interp.eq.4) ndimri= 1
c
      nniv=nniv+isurf
c
c-----------------------------------------------------------------------
c        nb| gi gridene i @kende prioritet (f.eks. grovt 'globalt' grid
c            foran finere 'begrensede' sektorer)
c
c        ivcoor  -  vertikal-koordinat (1=p 2=sigma)
c        nniv    -  antall niv}
c
c        interp: 1 => biline#r interpolasjon  ) interpolasjons-metode
c                3 => bessel-interpolasjon    )
c                4 => n#rmeste (i,j)-punkt    )
c
c        "posisjons-type"   1=i,j   2=x,y   3=b,l
c                           4=b,l gitt som grader*100+minutter
c                           0=slutt
c-----------------------------------------------------------------------
c
c        ktekst(1,1-ltekst): antall tegn
c               2          : plassering, 0=venstre 1="sentrert" 2=h@yre
c
      mt=len(tekst(1))
      lttext=0
c
      do l=1,ltekst
c..antall tegn og sentrering
        ktekst(1,l)=0
        ktekst(2,l)=0
        kt=0
        do k=1,mt
          if(tekst(l)(k:k).ne.' ') kt=k
        end do
        if(kt.lt.3) then
          continue
        elseif(tekst(l)(kt-2:kt).eq.':v:' .or.
     *         tekst(l)(kt-2:kt).eq.':v:') then
          tekst(l)(kt-2:kt)='   '
          ktekst(2,l)=0
          kt=-1
        elseif(tekst(l)(kt-2:kt).eq.':s:' .or.
     *         tekst(l)(kt-2:kt).eq.':s:') then
          tekst(l)(kt-2:kt)='   '
          ktekst(2,l)=1
          kt=-1
        elseif(tekst(l)(kt-2:kt).eq.':h:' .or.
     *         tekst(l)(kt-2:kt).eq.':h:') then
          tekst(l)(kt-2:kt)='   '
          ktekst(2,l)=2
          kt=-1
        end if
        if(kt.eq.-1) then
          kt=0
          do k=1,mt
            if(tekst(l)(k:k).ne.' ') kt=k
          end do
        end if
        ktekst(1,l)=kt
        lttext=lttext+(kt+1)/2
      end do
c
      do n=1,npos
        idupl(n)=0
      end do
      do n=1,npos
        if(idupl(n).eq.0) then
          k=1
          do i=n+1,npos
            if(navn(i).eq.navn(n)) then
              idupl(i)=1
              k=k+1
            end if
          end do
          if(k.gt.1) write(6,*) ' position name: ',navn(n),
     *                          ' found ',k,' times'
        end if
      end do
c
c--------------------------------------------------------------------
c
      nfltfil=1
      do n=2,ntid
        if(fltfil(n).ne.fltfil(n-1)) nfltfil=nfltfil+1
      end do
c
c..begerenser evt. tidsserien (bare hvis prognoser fra en file)
c
      limpro=1
      if(iprmin.le.-32767 .and. iprmax.ge.+32767) then
        limpro=0
      elseif(nfltfil.gt.1) then
        limpro=0
      end if
      if(limpro.eq.1) then
        nt=0
        do n=1,ntid
          if(itid(3,n).ge.iprmin .and. itid(3,n).le.iprmax) then
            nt=nt+1
            if(nt.ne.n) then
              fltfil(nt)=fltfil(n)
              itid(1,nt)=itid(1,n)
              itid(2,nt)=itid(2,n)
              itid(3,nt)=itid(3,n)
              tidtxt(nt)=tidtxt(n)
            end if
          end if
        end do
        if(nt.lt.1) then
          write(6,*) ' ingen tidspunkt etter prognose-begrensning:'
          write(6,*) ' prog_limit = ',iprmin,iprmax
          goto 980
        end if
        ntid=nt
      end if
c
      ndat=npos*npar*nniv*ntid
c
      if(ndat.gt.maxdat) then
        write(6,*) ' for mye data, npos*npar*nniv*ntid = ',ndat
        write(6,*) '            max tillatt ("maxdat") = ',maxdat
        write(6,*) '                              npos = ',npos
        write(6,*) '                              npar = ',npar
        write(6,*) '                              nniv = ',nniv
        write(6,*) '                              ntid = ',ntid
        ntid=maxdat/(npos*npar*nniv)
        write(6,*) ' minker antall tidspunkt til: ntid = ',ntid
        if(ntid.lt.1) goto 980
        ndat=npos*npar*nniv*ntid
      end if
c......................................................
c
c        $pner felt-file(r) og skriver ut dato,termin
c
      ifile=iunitf
      iufelt(1)=ifile
      itid(1,1)=ifile
      do n=2,ntid
        nf=0
        do i=1,n-1
          if(fltfil(n).eq.fltfil(i) .and. iufelt(i).gt.0) nf=i
        end do
        if(nf.gt.0) then
          iufelt(n)=0
          itid(1,n)=iufelt(nf)
        else
          ifile=ifile+1
          iufelt(n)=ifile
          itid(1,n)=ifile
        end if
      end do
c
      do n=1,ntid
        if(iufelt(n).gt.0) then
          filein=fltfil(n)
          ifile=iufelt(n)
          open(ifile,file=filein,
     *               access='direct',form='unformatted',
     *               recl=2048/lrunit,
     *               status='old',iostat=ios,err=900)
          swap= swapfile(-ifile)
          irec=1
          read(ifile,rec=irec,iostat=ios,err=905) idfile
          if (swap) call bswap2(32,idfile)
          idato(1)=idfile(5)
          idato(2)=idfile(6)/100
          idato(3)=idfile(6)-(idfile(6)/100)*100
          idato(4)=idfile(7)
          write(6,*) ' felt file: ',filein
          write(6,1001) idato(3),idato(2),idato(1),idato(4)
 1001     format('    time:  ',i2.2,':',i2.2,':',i4.4,1x,i4.4,' utc')
        end if
      end do
c
c---------------------------------------------------------------------
c
      nposin=npos
      ntidin=ntid
c
      do nt=1,ntid
        itimok(nt)=1
      end do
c
c
c..data-struktur:  dat(npos,npar,nniv,ntid)
c
      messag=info
      istop1=0
      istop2=2
c        hvis 'mi': stopp innlesning hvis et felt mangler
c.. removed this! iprod is not yet defined!
c      if(iprod.eq.88 .and. nfltfil.eq.1) istop2=1
c
      newpos=1
c
c
c..trykk ...... og evt. parameter hvor sigma/eta-verdi hentes fra
      npress=0
      npsigm=0
      nphght=0
      nphsrf=0
      nptt=0
      nptd=0
      npuu=0
      npvv=0
      lpc=len(pcode(1))
      do np=1,npar
        if(npress.eq.0) then
          if(pcode(np)(1:2)      .eq.'p ') npress=np
          if(pcode(np)(lpc-1:lpc).eq.' p') npress=np
          if(index(pcode(np),' p ').gt.0)  npress=np
        end if
        if(index(pcode(np),'height').gt.0 .and. nphght.eq.0) nphght=np
        if(index(pcode(np),'h.above.surf').gt.0 .and. nphsrf.eq.0)
     +     nphsrf=np
        if(np.ne.npress .and. np.ne.nphght .and.
     *     np.ne.nphsrf .and. npsigm.eq.0) npsigm=np
        if(nptt.eq.0 .and. index(pcode(np),'->t(c)') .gt.0) nptt=np
        if(nptd.eq.0 .and. index(pcode(np),'->td(c)').gt.0) nptd=np
        if(npuu.eq.0 .and. index(pcode(np),'u->')    .gt.0) npuu=np
        if(npvv.eq.0 .and. index(pcode(np),'v->')    .gt.0) npvv=np
      end do
      if(isurf.eq.1) then
        insurf(5,1)=nptt
        insurf(5,2)=nptd
        insurf(5,3)=npuu
        insurf(5,4)=npvv
      end if
      insurf(5,5)=npress
      do i=1,4
        insurf(i,5)=inps(i)
      end do
c
c
      if(ivcoor.ne.1 .and. nphght.gt.0) then
c..height, sigma/eta levels: read topography
        do nl=1,nniv
          iniv(1,nl)=-32767
        end do
        iniv(1,nniv)=itopog(5)
        iniv(2,nniv)=itopog(6)
        do np=1,npar
          ipar(1,np)=-32767
        end do
        ipar(1,nphght)=itopog(3)
        ipar(2,nphght)=itopog(4)
        ipar(3,nphght)=0
        ipar(4,nphght)=0
        ipar(5,nphght)=0
        ipar(6,nphght)=0
        ipar(7,nphght)=0
        ipar(8,nphght)=0
        parlim(1,nphght)=0.
        parlim(2,nphght)=0.
        itidx2=itid(2,1)
        itidx3=itid(3,1)
        itid(2,1)=itopog(1)
        itid(3,1)=itopog(2)
c
        ltmax=0
        do ng=1,ngrd
          iprod=igrd(1,ng)
          igrid=igrd(2,ng)
          if(info.ne.0)
     *       write(6,*) ' topo.      produsent,grid: ',iprod,igrid
c
          call flt2pos(messag,istop1,istop2,
     +                 newpos,npos,npar,nniv,1,
     +                 interp,iundef,pos,geopos(1,1,ng),gxypos(1,1,ng),
     +                 dat,ipar,iniv,itid,iprod,igrid,
     +                 parlim,maxflt,idat,felt,itime,idsave(1,1,ng),
     +                 nx,ny,igtype,grid(1,ng),jinter(1,ng),
     +                 ndimri,rinter,vturn,dwork,ltim,ierror)
          if(ierror.ne.0) write(6,*) 'FLT2POS ERROR: ',ierror
          if(ierror.ne.0) goto 980
          newpos=0
c
          if(ltmax.lt.ltim) ltmax=ltim
        end do
        itid(2,1)=itidx2
        itid(3,1)=itidx3
        if(ltmax.eq.0) then
          write(6,*) 'missing topography (for height computation)'
          goto 980
        end if
c..set a strange parameter no. for subr. comput (-1002=topography)
        inppar(nphght)=-1002
      end if
c
      if(ivcoor.eq.1 .and. npress.gt.0) then
c..trykk, p-flater
        do nt=1,ntid
          ndt=(nt-1)*npos*npar*nniv
          do nl=1,nniv
            nd=ndt+(nl-1)*npos*npar+(npress-1)*npos
            p=inpniv(nl)
            do n=1,npos
              dat(nd+n)=p
            end do
          end do
c..set parameter no. for subr. comput
          inppar(npress)=8
        end do
c
      elseif(ivcoor.ne.1 .and. (npress.gt.0 .or. isurf.ne.0)) then
c..trykk, sigma/eta-flater: leser ps (surface pressure) og evt. andre
        do np=1,npar
          ipar(1,np)=-32767
        end do
        do nl=1,nniv
          iniv(1,nl)=-32767
        end do
        do n=1,5
          np=insurf(5,n)
          if(np.gt.0) then
            iniv(1,nniv)=insurf(3,n)
            iniv(2,nniv)=insurf(4,n)
            ipar(1,np)=insurf(1,n)
            ipar(2,np)=insurf(2,n)
            ipar(3,np)=0
            ipar(4,np)=0
            ipar(5,np)=0
            ipar(6,np)=0
            ipar(7,np)=0
            ipar(8,np)=0
            parlim(1,np)=0.
            parlim(2,np)=0.
            if(np.eq.npuu) ipar(5,np)=1
            if(np.eq.npvv) ipar(5,np)=2
          end if
        end do
c
        ltmax=0
        do ng=1,ngrd
          iprod=igrd(1,ng)
          igrid=igrd(2,ng)
          if(info.ne.0)
     *       write(6,*) ' bakke.     produsent,grid: ',iprod,igrid
c
          call flt2pos(messag,istop1,istop2,
     +                 newpos,npos,npar,nniv,ntid,
     +                 interp,iundef,pos,geopos(1,1,ng),gxypos(1,1,ng),
     +                 dat,ipar,iniv,itid,iprod,igrid,
     +                 parlim,maxflt,idat,felt,itime,idsave(1,1,ng),
     +                 nx,ny,igtype,grid(1,ng),jinter(1,ng),
     +                 ndimri,rinter,vturn,dwork,ltim,ierror)
          if(ierror.ne.0) write(6,*) 'FLT2POS ERROR: ',ierror
          if(ierror.ne.0) goto 980
          newpos=0
c
          if(ltmax.lt.ltim) ltmax=ltim
c
          if(npress.gt.0) then
            do nt=1,ltim
              ptop=0.
              nidsav=npar*nniv*(nt-1)+npar*(nniv-1)+npress
              if(idsave(1,nidsav,ng).ne.0) then
                if(ivcoor.eq. 2) ptop=idsave(3,nidsav,ng)
                if(ivcoor.eq.10) ptop=idsave(3,nidsav,ng)*0.1
              end if
              pptop(nt)=ptop
            end do
          else
            do nt=1,ltim
              pptop(nt)=0.
            end do
          end if
c
          do nt=1,ltim
            do np=1,npar
              if(ipar(1,np).ne.-32767) then
                nidsav=npar*nniv*(nt-1)+npar*(nniv-1)+np
                if(idsave(1,nidsav,ng).eq.0) itimok(nt)=0
              end if
            end do
          end do
c
        end do
c
        ntid=ltmax
        if(ntid.eq.0) goto 980
c..set a strange parameter no. for subr. comput (-1001=ps)
        if(npress.gt.0) inppar(npress)=-1001
      end if
c
c        leser felt i flere nivaa
c
      do nt=1,ntid
        if(itimok(nt).eq.0) itid(1,nt)=-32767
      end do
c
      level2=0
      if(ivcoor.eq.10) level2=-32767
c
      do nl=1,nniv-isurf
        iniv(1,nl)=inpniv(nl)
        iniv(2,nl)=level2
      end do
      if(isurf.eq.1) then
        iniv(1,nniv)=-32767
        iniv(2,nniv)=-32767
      end if
      do np=1,npar
        ipar(1,np)=ivcoor
        ipar(2,np)=inppar(np)
        ipar(3,np)=0
        ipar(4,np)=0
        ipar(5,np)=0
        ipar(6,np)=0
        ipar(7,np)=0
        ipar(8,np)=0
        if(np.eq.npress) ipar(1,np)=-32767
        if(np.eq.nphght) ipar(1,np)=-32767
        if(np.eq.nphsrf) ipar(1,np)=-32767
        parlim(1,np)=0.
        parlim(2,np)=0.
      end do
c
c..check codes handled in subr. flt2pos
      do np=1,npar
        if(index(pcode(np),'u->u.ew')      .gt.0) ipar(5,np)=1
        if(index(pcode(np),'v->v.ns')      .gt.0) ipar(5,np)=2
        if(index(pcode(np),'u->dd')        .gt.0) ipar(5,np)=1
        if(index(pcode(np),'v->ff')        .gt.0) ipar(5,np)=2
        if(index(pcode(np),'direction')    .gt.0) ipar(5,np)=3
        if(index(pcode(np),'direction.180').gt.0) ipar(5,np)=4
        k=index(pcode(np),'min=')
        if(k.gt.0) then
          cipart=pcode(np)(k+4:)
          read(cipart,*,iostat=ios) parlim(1,np)
          if(ios.eq.0) ipar(6,np)=1
        end if
        k=index(pcode(np),'max=')
        if(k.gt.0) then
          cipart=pcode(np)(k+4:)
          read(cipart,*,iostat=ios) parlim(2,np)
          if(ios.eq.0) ipar(7,np)=1
        end if
      end do
c
      ltmax=0
c
      do ng=1,ngrd
c
        iprod=igrd(1,ng)
        igrid=igrd(2,ng)
        if(info.ne.0)
     *     write(6,*) ' flater.    produsent,grid: ',iprod,igrid
c
        call flt2pos(messag,istop1,istop2,
     +               newpos,npos,npar,nniv,ntid,
     +               interp,iundef,pos,geopos(1,1,ng),gxypos(1,1,ng),
     +               dat,ipar,iniv,itid,iprod,igrid,
     +               parlim,maxflt,idat,felt,itime,idsave(1,1,ng),
     +               nx,ny,igtype,grid(1,ng),jinter(1,ng),
     +               ndimri,rinter,vturn,dwork,ltim,ierror)
        if(ierror.ne.0) write(6,*) 'FLT2POS ERROR: ',ierror
        if(ierror.ne.0) goto 980
        newpos=0
c
        if(ltmax.lt.ltim) ltmax=ltim
c
        do nt=1,ltim
          do nl=1,nniv
            if(iniv(1,nl).ne.-32767) then
              do np=1,npar
                if(ipar(1,np).ne.-32767) then
                  nidsav=npar*nniv*(nt-1)+npar*(nl-1)+np
                  if(idsave(1,nidsav,ng).eq.0) itimok(nt)=0
                end if
              end do
            end if
          end do
        end do
c
      end do
c
      ntid=ltmax
      if(ntid.eq.0) goto 980
c
c
      if(ivcoor.eq.2) then
c..sigma ........................ p = pt + sigma(k) * (ps - pt)
c................................   = pt*(1-sigma(k)) + sigma(k)*ps
c................................   = aaa(k) + bbb(k)*ps
        do ng=1,ngrd
          do nt=1,ltim
            ptop=pptop(nt)
            do nl=1,nniv-isurf
              nidsav=npar*nniv*(nt-1)+npar*(nl-1)+npsigm
              if(idsave(1,nidsav,ng).ne.0) then
                nd=nniv*(nt-1)+nl
                sigma=idsave(3,nidsav,ng)*0.0001
                aaa(nd)=ptop*(1.-sigma)
                bbb(nd)=sigma
              end if
            end do
          end do
        end do
        if(isurf.eq.1) then
          do nt=1,ltim
            nd=nniv*nt
            aaa(nd)=0.
            bbb(nd)=1.
          end do
        end if
      elseif(ivcoor.eq.10) then
c..eta .......................... p = aaa(k) + bbb(k)*ps
        do ng=1,ngrd
          do nt=1,ltim
            do nl=1,nniv-isurf
              nidsav=npar*nniv*(nt-1)+npar*(nl-1)+npsigm
              if(idsave(1,nidsav,ng).ne.0) then
                nd=nniv*(nt-1)+nl
                aaa(nd)=idsave(4,nidsav,ng)*0.1
                bbb(nd)=idsave(3,nidsav,ng)*0.0001
              end if
            end do
          end do
        end do
        if(isurf.eq.1) then
          do nt=1,ltim
            nd=nniv*nt
            aaa(nd)=0.
            bbb(nd)=1.
          end do
        end if
      else
        do nt=1,ltim
          do nl=1,nniv
            nd=nniv*(nt-1)+nl
            aaa(nd)=-1.
            bbb(nd)=-1.
          end do
        end do
      end if
c
c----------------------------------------------------------------------
      if(iprpos.ne.0) call pliste(maxpos,npos,ngrd,
     +                            pos,geopos,gxypos,igrd,navn)
c----------------------------------------------------------------------
c
c..sjekk om posisjoner ikke skal legges p$ output-file
      noff=npos*2
      nkpos=0
      do n=1,npos
        k=0
        do ng=1,ngrd
          if(jinter(noff+n,ng).ne.0) k=1
        end do
        if(k.eq.1) then
          nkpos=nkpos+1
          kpos(nkpos)=n
        else
          write(6,*) ' posisjon utenfor grid(ene): ',navn(n)
        end if
      end do
c
      if(nkpos.eq.0) then
        write(6,*) ' ingen posisjoner innenfor grid(ene)'
        goto 980
      end if
c
      if(nkpos.lt.npos) then
c..fjern posisjoner fra output-liste
        call delpos(maxpos,npos,npar,nniv,ntid,ngrd,dat,
     *              geopos,gxypos,pos,idobs,ridobs,navn,
     *              nkpos,kpos,dat)
        npos=nkpos
      end if
c
c..sjekk om tidspunkt ikke skal legges p$ output-file
c..(ikke til file hvis noe mangler)
      nktid=0
      do nt=1,ntid
        if(itimok(nt).eq.1) then
          nktid=nktid+1
          ktid(nktid)=nt
        end if
      end do
c
      if(nktid.eq.0) then
        write(6,*) ' no timesteps o.k.'
        goto 980
      end if
c
      if(nktid.lt.ntid) then
c..fjern tidspunkt fra output-liste
        call deltim(npos,npar,nniv,ntid,dat,itime,aaa,bbb,pptop,
     +              nktid,ktid)
        ntid=nktid
      end if
c
c
      write(6,*) ' no. of levels:                 ',nniv
      write(6,*) ' no. of parameters:             ',npar
      write(6,*) ' no. of positions input,output: ',nposin,npos
      write(6,*) ' no. of timesteps input,output: ',ntidin,ntid
c
c-----------------------------------------------------------------------
      if(iprdin.ne.0) call dliste(1,npos,npar,nniv,ntid,itime,dat,navn)
c-----------------------------------------------------------------------
c
c
c..tilleggsbehandling av punkt-verdier
      call comput(npos,npar,nniv,ntid,dat,aaa,bbb,pptop,
     +            ivcoor,inppar,pcode,iparam,jparam,aa2,bb2,isurf,
     +                  inpniv)
c
c-----------------------------------------------------------------------
      if(iprdot.ne.0) call dliste(2,npos,npar,nniv,ntid,itime,dat,navn)
c-----------------------------------------------------------------------
c
c        antall tegn i hvert navn og total lengde for lagring
      mt=len(navn(1))
      ltnavn=0
      do n=1,npos
        kt=1
        do k=1,mt
          if(navn(n)(k:k).ne.' ') kt=k
        end do
        ntnavn(n)=kt
        ltnavn=ltnavn+(kt+1)/2
      end do
c
c        antall tegn i hver tids-tekst og total lengde for lagring
      mt=len(tidtxt(1))
      lttitx=0
      do n=1,ntid
        kt=1
        do k=1,mt
          if(tidtxt(n)(k:k).ne.' ') kt=k
        end do
        ktidtx(n)=kt
        lttitx=lttitx+(kt+1)/2
      end do
c
c..geografisk posisjon og identifikasjon for evt. 'temp'
c
c        iposit(1,n) - parameter-nr. eller identifikasjon
c                         -1 = geografisk bredde
c                         -2 = geografisk lengde
c                         -3 = wmo sone-nr. (99=bevegelig)
c                         -4 = stasjons-nr.
c                         -5 = max. avst. i g.bredde (bevegelig st.)
c                         -6 = max. avst. i g.lengde (bevegelig st.)
c        iposit(2,n) - skalering
c
      iposit(1,1)=-1
      iposit(1,2)=-2
      iposit(1,3)=-3
      iposit(1,4)=-4
      iposit(1,5)=-5
      iposit(1,6)=-6
c
      iposit(2,1)=-2
      iposit(2,2)=-2
      iposit(2,3)= 0
      iposit(2,4)= 0
      iposit(2,5)=-2
      iposit(2,6)=-2
c
      ng=1
      do n=1,npos
        iposid(1,n)=nint(geopos(1,n,ng)*100.)
        iposid(2,n)=nint(geopos(2,n,ng)*100.)
        iposid(3,n)=idobs(n)/1000
        iposid(4,n)=idobs(n)-(idobs(n)/1000)*1000
        iposid(5,n)=nint(ridobs(1,n)*100.)
        iposid(6,n)=nint(ridobs(2,n)*100.)
      end do
c
c        skalering av data
      do np=1,npar
        skalp(np)=10.**(-iskalp(np))
      end do
      i=0
      do nt=1,ntid
        do nl=1,nniv
          do np=1,npar
            do n=1,npos
              dat(i+n)=skalp(np)*dat(i+n)
            end do
            i=i+npos
          end do
        end do
      end do
c
c
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
      ihead(1)=201
      ihead(2)=2
      ihead(3)=lbuff
      ihead(4)=0
      ihead(5)=0
      ihead(6)=0
      ihead(7)=0
      ihead(8)=0
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
      idnum(5)=igrd(1,1)
      idnum(6)=igrd(2,1)
      if(ngrd.gt.1) idnum(6)=0
      idnum(7)=ivcoor
      idnum(8)=interp
      idnum(9)=isurf
      idnum(10)=0
      nidnum=10
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
      nposid=2+2+2
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
c
c--- output ---------------------------------------------------------
c
c        konvc - character:   0 = ingen
c                             1 = ascii (standard) -> ascii (pc)
c                             2 = ascii (standard) -> ebcdic (ibm)
c                            -1 = ascii (standard) -> ascii (pc),
c                                 og forhindrer byteswap p.g.a. konvd=1,
c                                 (for aa simulere gamle pc-rutiner)
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
      iunit=iunito
c
      write(6,*) ' output file: ',fileot
      write(6,*) ' output konvertering.  konvc,konvd: ',konvc,konvd
c
      call rmfile(fileot,0,ierror)
c
      open(iunit,file=fileot,
     +           access='direct',form='unformatted',
     +           recl=(lbuff*2)/lrunit,
     +           status='unknown',iostat=ios,err=910)
c
      irec=0
      ibuff=0
c
c..header
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ihead,8,ierr)
      if(ierr.ne.0) goto 980
c..contents
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,icont,ncont,ierr)
      if(ierr.ne.0) goto 980
c..'101'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,idnum,nidnum,ierr)
      if(ierr.ne.0) goto 980
c..'201'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,itime,5*ntid,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ktidtx,ntid,ierr)
      if(ierr.ne.0) goto 980
      do n=1,ntid
        l=((ktidtx(n)+1)/2)*2
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +                    tidtxt(n),1,l,ierr)
        if(ierr.ne.0) goto 980
      end do
c..'301'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ltekst,1,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +                  ktekst,ltekst*2,ierr)
      if(ierr.ne.0) goto 980
      do n=1,ltekst
        l=((ktekst(1,n)+1)/2)*2
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +                    tekst(n),1,l,ierr)
        if(ierr.ne.0) goto 980
      end do
c..'401'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnavn,npos,ierr)
      if(ierr.ne.0) goto 980
      do n=1,npos
        l=((ntnavn(n)+1)/2)*2
        call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +                    navn(n),1,l,ierr)
        if(ierr.ne.0) goto 980
      end do
c..'501'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,nposid,1,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +                  iposit,nposid*2,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +                  iposid,nposid*npos,ierr)
      if(ierr.ne.0) goto 980
c..'601'
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iparam,npar,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iskalp,npar,ierr)
      if(ierr.ne.0) goto 980
c
c..data .............. innlest:    dat(npos,npar,nniv,ntid)
c                           ut: datout(nniv,npar,ntid,npos) ... oppdelt
      ndat=nniv*npar
      do nn=1,npos
        do nt=1,ntid
          io=0
          do np=1,npar
            do nl=1,nniv
              id= npos*npar*nniv*(nt-1)
     +           +npos*npar*(nl-1)
     +           +npos*(np-1)
     +           +nn
              io=io+1
              datout(io)=dat(id)
            end do
          end do
          call bputr4(iunit,irec,buff,lbuff,ibuff,konvd,
     +                datout,ndat,ierr)
          if(ierr.ne.0) goto 980
        end do
      end do
c
      call bputnd(iunit,irec,buff,lbuff,ibuff,konvd,ierr)
      if(ierr.ne.0) goto 980
c
      close(iunit)
c
      goto 990
c
  900 write(6,*) ' *** open error.  felt file: ',filein
      write(6,*) ' ***                   unit: ',ifile
      write(6,*) ' ***                 iostat: ',ios
      goto 980
c
  905 write(6,*) ' *** read error.  felt file: ',filein
      write(6,*) ' ***                   unit: ',ifile
      write(6,*) ' ***                 record: ',irec
      write(6,*) ' ***                 iostat: ',ios
      goto 980
c
  910 write(6,*) ' *** open error.  output file: ',fileot
      write(6,*) ' ***                     unit: ',iunit
      write(6,*) ' ***                   iostat: ',ios
      stop 3
c
  980 write(6,*) ' =========== error exit ============'
      close(iunito)
      call rmfile(fileot,0,ierror)
      stop 3
c---------------------------------------------------------------------
c
  990 continue
c
      end
c
c**********************************************************************
c
      subroutine comput(npos,npar,nniv,ntid,dat,afull,bfull,pptop,
     +                  ivcoor,inppar,pcode,iparam,jparam,
     +                  ahalf,bhalf,isurf,inpniv)
c
c        tilleggsbehandling av punkt-verdier.
c
c        ivcoor: vertikal-koordinat
c                  1=p  2=sigma  10=eta (hybrid sigma/p)
c
c        isurf:  0 : ikke bakkeparametre
c                1 : bakkeparametre i nederste nivaa (nniv)
c                    (psurf,t2m,rh2m,vind10m,...)
c
c-----------------------------------------------------------------------
c pcode:
c -------------
c '='            : ingen beregning
c 'p'            : avsetter plass for trykk, leser ps (par. 8) . sigma/eta
c 'p'            : avsetter plass for trykk, p gitt som nivaa ... p
c 'th->t(c)'     : potensiell temp,   t(c) output
c 't(k)->t(c)'   : abs. temp (kelvin), temp (celsius) output
c 'q+t->td(c)'   : spesifikk fuktighet,   td(c) output .. sigma/eta-flater
c 'q+t->rh'      : spesifikk fuktighet,   rh(%) output .. sigma/eta-flater
c 'rh+t->td(c)'  : relativ fuktighet (%), td(c) output .. p-flater
c 'u->u.ew(k)'   : from m/s to knots (wind rotation already done)
c 'v->v.ns(k)'   : from m/s to knots (wind rotation already done)
c 'u->dd'        : u(e/w) to dd (direction)
c 'v->ff'        : v(n/s) to ff (speed), no scaling
c 'v->ff(k)'     : v(n/s) to ff (speed), m/s to knots
c 'height'       : store height above mean sea level,
c                  computed from topography and temperature (sigma/eta)
c 'h.above.surf' : store height above surface,
c                  computed from temperature (sigma/eta)
c 'scale=<value>': extra scaling
c 'param=<n>'    : set a new output parameter no.
c-----------------------------------------------------------------------
c
c  sigma/eta with surface data (isurf=1):
c    input temperature in deg. kelvin
c    input relative humidity
c    input wind in same form as the model level wind
c    pressure = ps
c    omega (param. 13) = 0.
c    height = topography
c    h.above.surf = 0.
c    all other get same value as in the lower level
c
c  note: the sequence for computing the parameters is crucial !!
c
c
      integer   npos,npar,nniv,ntid,ivcoor
      integer   inppar(npar),iparam(npar),jparam(npar),inpniv(nniv)
      real      dat(npos,npar,nniv,ntid)
      real      afull(nniv,ntid),bfull(nniv,ntid),pptop(ntid)
      real      ahalf(nniv+1),bhalf(nniv+1)
      character*(*) pcode(npar)
c
      character*32 text
c
      real ewt(41),tpk(131),tpi(131)
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
c        tabeller
c        tpk: (p/1000)**0.28585           (hver 10. mb, 0,10,20,...1300)
c        tpi: 1004.*(p/1000)**0.28585     (hver 10. mb, 0,10,20,...1300)
c              x=(p+10.)*0.1    i=x    --  0 < p < 1300 mb
c              pix=tpx(i)+(tpx(i+1)-tpx(i))*(x-i), tpx=tpk,tpi
c
      r=287.
      cp=1004.
      rcp=r/cp
      tpk(1)=0.
      tpi(1)=0.
      do i=2,131
        p=i*10.-10.
        tpk(i)=(p*0.001)**rcp
        tpi(i)=cp*tpk(i)
      end do
c
c..grenser for akseptabel rh
      rhmin=0.01
      rhmax=1.00
c
      do n=1,npar
        iparam(n)=inppar(n)
        jparam(n)=0
      end do
c
      lpc=len(pcode(1))
c
c..height ... in sigma/eta levels
      if(ivcoor.eq.2 .or. ivcoor.eq.10) then
        do n=1,npar
          nphght=0
          nphsrf=0
          if(index(pcode(n),'height').gt.0)       nphght=n
          if(index(pcode(n),'h.above.surf').gt.0) nphsrf=n
          if(nphght.gt.0 .or. nphsrf.gt.0) then
            nph =max(nphght,nphsrf)
            npp =0
            npth=0
            do i=1,npar
c..surface pressure (one level)
              if(iparam(i).eq.-1001) npp =i
c..potential temperature 
              if(iparam(i).eq.   18) npth=i
            end do
            if(npp.eq.0 .or. npth.eq.0) then
              write(6,*) 'not computing height'
              write(6,*) '(missing surface pressure or pot.temp.)'
              do nt=1,ntid
                do nl=1,nniv-isurf
                  do nn=1,npos
                    dat(nn,nph,nl,nt)=0.
                  end do
                end do
              end do
            else
              ginv=1./9.8
              if (nph.eq.nphght) then
c..height...sigma/eta-levels: topography stored in lower level, timestep 1
                do nt=2,ntid
                  do nn=1,npos
                    dat(nn,nph,nniv,nt)=dat(nn,nph,nniv,1)
                  end do
                end do
              else
                do nt=1,ntid
                  do nn=1,npos
                    dat(nn,nph,nniv,nt)=0.
                  end do
                end do
              end if
c..compute height in sigma_2/eta_full levels (as other parameters)
              do nt=1,ntid
c..sigma_1/eta_half (where height can be computed)
                if(inpniv(1).eq.1) then
                  ahalf(1)=pptop(nt)
                  bhalf(1)=0.
                  do nl=2,nniv-isurf
                    da=afull(nl-1,nt)-ahalf(nl-1)
                    db=bfull(nl-1,nt)-bhalf(nl-1)
                    ahalf(nl)=afull(nl-1,nt)+da
                    bhalf(nl)=bfull(nl-1,nt)+db
                  end do
                  ahalf(nniv-isurf+1)=0.
                  bhalf(nniv-isurf+1)=1.
                else
c..assuming from surface and up, but not to top (model level 1)
                  ahalf(nniv-isurf+1)=0.
                  bhalf(nniv-isurf+1)=1.
                  do nl=nniv-isurf,1,-1
                    da=ahalf(nl+1)-afull(nl,nt)
                    db=bhalf(nl+1)-bfull(nl,nt)
                    ahalf(nl)=afull(nl,nt)-da
                    bhalf(nl)=bfull(nl,nt)-db
                  end do
                end if
                do nn=1,npos
                  zs=dat(nn,nph,nniv,nt)
                  ps=dat(nn,npp,nniv,nt)
                  x=(ps+10.)*0.1
                  i=x
                  pi2=tpi(i)+(tpi(i+1)-tpi(i))*(x-i)
                  z2=zs
c..integration from surface to top
                  do nl=nniv-isurf,1,-1
                    pi1=pi2
                    z1=z2
                    th=dat(nn,npth,nl,nt)
                    p=ahalf(nl)+bhalf(nl)*ps
                    x=(p+10.)*0.1
                    i=int(x)
                    pi2=tpi(i)+(tpi(i+1)-tpi(i))*(x-real(i))
                    z2=z1+th*(pi1-pi2)*ginv
c..a very simple interpolation to full level
ccc                 dat(nn,nph,nl,nt)=(z1+z2)*0.5
c
c..linear interpolation of height is not good (=> T=const. in layer),
c..so we make a first guess of a temperature profile to comp. height
                    nlm1=max(nl-1,1)
                    nlp1=min(nl+1,nniv-isurf)
                    p=afull(nlm1,nt)+bfull(nlm1,nt)*ps
                    x=(p+10.)*0.1
                    i=int(x)
                    pix1=tpi(i)+(tpi(i+1)-tpi(i))*(x-real(i))
                    p=afull(nlp1,nt)+bfull(nlp1,nt)*ps
                    x=(p+10.)*0.1
                    i=int(x)
                    pix2=tpi(i)+(tpi(i+1)-tpi(i))*(x-real(i))
                    thx1=dat(nn,npth,nlm1,nt)
                    thx2=dat(nn,npth,nlp1,nt)
                    dthdpi=(thx1-thx2)/(pix1-pix2)
c..get temperature at half level (bottom of layer)
                    p=afull(nl,nt)+bfull(nl,nt)*ps
                    x=(p+10.)*0.1
                    i=int(x)
                    pix=tpi(i)+(tpi(i+1)-tpi(i))*(x-real(i))
                    th1=th+dthdpi*(pix-pi1)
c..thickness from half level to full level
                    dz=(th1+th)*0.5*(pi1-pix)*ginv
c..height at full level
                    dat(nn,nph,nl,nt)=z1+dz
c
                    if(dat(nn,nph,nl,nt).gt.32767.)
     +                 dat(nn,nph,nl,nt)=dat(nn,nph,nl,nt)*(-0.1)
                  end do
                end do
              end do
              iparam(nph)=1
              jparam(nph)=1
            end if
          end if
        end do
      end if
c
c..p ... in sigma/eta levels
      if(ivcoor.eq.2 .or. ivcoor.eq.10) then
        do n=1,npar
          npp=0
          if(pcode(n)(1:2)      .eq.'p ') npp=n
          if(pcode(n)(lpc-1:lpc).eq.' p') npp=n
          if(index(pcode(n),' p ').gt.0)  npp=n
          if(npp.gt.0) then
c..sigma-flater: ps lagret i nederste nivaa
            do nt=1,ntid
              do nn=1,npos
                ps=dat(nn,npp,nniv,nt)
                do nl=1,nniv
                  dat(nn,npp,nl,nt)=afull(nl,nt)+bfull(nl,nt)*ps
                end do
              end do
            end do
            iparam(npp)=8
            jparam(npp)=1
          end if
        end do
      end if
c
c..th->t(c) ... from potential temperature to temperature celsius
      do n=1,npar
        if(index(pcode(n),'th->t(c)').gt.0) then
          npth=n
          npp =0
          do i=1,npar
c..pressure
            if(iparam(i).eq.8) npp=i
          end do
          if(npp.eq.0) then
            write(6,*) 'not computing t(c) from th (pot.temp.)'
            write(6,*) '(missing pressure)'
          else
            npt=npth
            do nt=1,ntid
              do nl=1,nniv-isurf
                do nn=1,npos
                  p= dat(nn,npp, nl,nt)
                  th=dat(nn,npth,nl,nt)
                  x=(p+10.)*0.1
                  i=int(x)
                  pik=tpk(i)+(tpk(i+1)-tpk(i))*(x-real(i))
                  t=pik*th-273.15
                  dat(nn,npt,nl,nt)=t
                end do
              end do
            end do
            if(isurf.eq.1) then
c..surface temp. in deg. kelvin (not pot.temp.)
              nl=nniv
              do nt=1,ntid
                do nn=1,npos
                  dat(nn,npt,nl,nt)=dat(nn,npt,nl,nt)-273.15
                end do
              end do
            end if
            iparam(npt)=4
            jparam(npt)=1
          end if
        end if
      end do
c
c..t(k)->t(c) ... from abs. temp. in kelvin to temperature in celsius
      do n=1,npar
        if(index(pcode(n),'t(k)->t(c)').gt.0) then
          npt=n
          do nt=1,ntid
            do nl=1,nniv-isurf
              do nn=1,npos
                dat(nn,npt,nl,nt)=dat(nn,npt,nl,nt)-273.15
              end do
            end do
          end do
          iparam(npt)=4
          jparam(npt)=1
        end if
      end do
c
c..q+t->td(c) ... from specific humidity to dew point temp. (celsius)
      do n=1,npar
        if(index(pcode(n),'q+t->td(c)').gt.0) then
          npq =n
          npp =0
          npt =0
          do i=1,npar
c..pressure
            if(iparam(i).eq.8) npp=i
c..temperature (celsius)
            if(iparam(i).eq.4) npt=i
          end do
          if(npp.eq.0 .or. npt.eq.0) then
            write(6,*) 'not computing td(c) from q'
            write(6,*) '(missing pressure or temperature)'
          else
            nptd=npq
            do nt=1,ntid
              do nl=1,nniv-isurf
                do nn=1,npos
                  p=dat(nn,npp,nl,nt)
                  t=dat(nn,npt,nl,nt)
                  q=dat(nn,npq,nl,nt)
                  x=(t+105.)*0.2
                  i=int(x)
                  et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
                  qsat=0.622*et/p
                  rh=q/qsat
                  if(rh.lt.rhmin) rh=rhmin
                  if(rh.gt.rhmax) rh=rhmax
                  etd=rh*et
                  do while (ewt(i).gt.etd .and. i.gt.1)
                    i=i-1
                  end do
                  x=(etd-ewt(i))/(ewt(i+1)-ewt(i))
                  td=-105.+(real(i)+x)*5.
                  dat(nn,nptd,nl,nt)=td
                end do
              end do
            end do
            if(isurf.eq.1) then
c..surface: rel.hum. input (not q)
              nl=nniv
              do nt=1,ntid
                do nn=1,npos
                  t =dat(nn,npt,nl,nt)
                  rh=dat(nn,npq,nl,nt)*0.01
                  x=(t+105.)*0.2
                  i=int(x)
                  et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
                  if(rh.lt.rhmin) rh=rhmin
                  if(rh.gt.rhmax) rh=rhmax
                  etd=rh*et
                  do while (ewt(i).gt.etd .and. i.gt.1)
                    i=i-1
                  end do
                  x=(etd-ewt(i))/(ewt(i+1)-ewt(i))
                  td=-105.+(real(i)+x)*5.
                  dat(nn,nptd,nl,nt)=td
                end do
              end do
            end if
            iparam(nptd)=5
            jparam(nptd)=1
          end if
        end if
      end do
c
c..q+t->rh ... from specific humidity to rel.humidity (%)
      do n=1,npar
        if(index(pcode(n),'q+t->rh').gt.0) then
          npq =n
          npp =0
          npt =0
          do i=1,npar
c..pressure
            if(iparam(i).eq.8) npp=i
c..temperature (celsius)
            if(iparam(i).eq.4) npt=i
          end do
          if(npp.eq.0 .or. npt.eq.0) then
            write(6,*) 'not computing rh from q'
            write(6,*) '(missing pressure or temperature)'
          else
            nprh=npq
            do nt=1,ntid
              do nl=1,nniv-isurf
                do nn=1,npos
                  p=dat(nn,npp,nl,nt)
                  t=dat(nn,npt,nl,nt)
                  q=dat(nn,npq,nl,nt)
                  x=(t+105.)*0.2
                  i=int(x)
                  et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
                  qsat=0.622*et/p
                  rh=q/qsat
                  if(rh.lt.rhmin) rh=rhmin
                  if(rh.gt.rhmax) rh=rhmax
                  dat(nn,nprh,nl,nt)=rh*100.
                end do
              end do
            end do
            if(isurf.eq.1) then
c..surface: rel.hum. input (not q)
              nl=nniv
              do nt=1,ntid
                do nn=1,npos
                  rh=dat(nn,nprh,nl,nt)*0.01
                  if(rh.lt.rhmin) rh=rhmin
                  if(rh.gt.rhmax) rh=rhmax
                  dat(nn,nprh,nl,nt)=rh*100.
                end do
              end do
            end if
            iparam(nprh)=10
            jparam(nprh)=1
          end if
        end if
      end do
c
c..rh+t->td(c) ... from relative humidity to dew point temp. (celsius)
      do n=1,npar
        if(index(pcode(n),'rh+t->td(c)').gt.0) then
          nprh=n
          npt =0
          do i=1,npar
c..temperature (celsius)
            if(iparam(i).eq.4) npt=i
          end do
          if(npt.eq.0) then
            write(6,*) 'not computing td(c) from rh'
            write(6,*) '(missing temperature)'
          else
c..rh input in unit %
            nptd=nprh
            do nt=1,ntid
              do nl=1,nniv
                do nn=1,npos
                  t= dat(nn,npt, nl,nt)
                  rh=dat(nn,nprh,nl,nt)*0.01
                  x=(t+105.)*0.2
                  i=int(x)
                  et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
                  if(rh.lt.rhmin) rh=rhmin
                  if(rh.gt.rhmax) rh=rhmax
                  etd=rh*et
                  do while (ewt(i).gt.etd .and. i.gt.1)
                    i=i-1
                  end do
                  x=(etd-ewt(i))/(ewt(i+1)-ewt(i))
                  td=-105.+(real(i)+x)*5.
                  dat(nn,nptd,nl,nt)=td
                end do
              end do
            end do
c..change parameter no. (td(5))
            iparam(nptd)=5
            jparam(nptd)=1
          end if
        end if
      end do
c
c..wind components:
c..u->u.ew(k) ... from m/s to knots  
c..v->v.ns(k) ... from m/s to knots
c..u->dd ........ from u(e/w) to dd (direction)
c..v->ff ........ from v(n/s) to ff (speed), no scaling
c..v->ff(k) ..... from v(n/s) to ff (speed), m/s to knots
      do np=1,npar-1
        iknots=0
        iddff =0
        if(index(pcode(np),  'u->u.ew(k)').gt.0 .and.
     *     index(pcode(np+1),'v->v.ns(k)').gt.0) then
          iknots=1
        elseif(index(pcode(np),  'u->dd'   ).gt.0 .and.
     *         index(pcode(np+1),'v->ff(k)').gt.0) then
          iknots=1
          iddff =1
        elseif(index(pcode(np),  'u->dd').gt.0 .and.
     *         index(pcode(np+1),'v->ff').gt.0) then
          iddff =1
        end if
        npu=np
        npv=np+1
        if(iknots.eq.1) then
c..fra m/s til knop
          tknots=3600./1852.
          do nt=1,ntid
            do nl=1,nniv
              do nn=1,npos
                dat(nn,npu,nl,nt)=tknots*dat(nn,npu,nl,nt)
                dat(nn,npv,nl,nt)=tknots*dat(nn,npv,nl,nt)
              end do
            end do
          end do
        end if
        if(iddff.eq.1) then
c..fra u(e/w),v(n/s) til dd,ff
          deg=180./3.141592654
          do nt=1,ntid
            do nl=1,nniv
              do nn=1,npos
                u=dat(nn,npu,nl,nt)
                v=dat(nn,npv,nl,nt)
                ff=sqrt(u*u+v*v)
                if(ff.gt.1.e-10) then
                  dd=270.-deg*atan2(v,u)
                  if(dd.gt.360.) dd=dd-360.
                  if(dd.le.  0.) dd=dd+360.
                else
                  ff=0.
                  dd=0.
                end if
                dat(nn,npu,nl,nt)=dd
                dat(nn,npv,nl,nt)=ff
              end do
            end do
          end do
c..change parameter no. (dd(6) ff(7))
          iparam(npu)=6
          iparam(npv)=7
        end if
      end do
c
      do np=1,npar
        if(index(pcode(np),'u->').gt.0) jparam(np)=1
        if(index(pcode(np),'v->').gt.0) jparam(np)=1
      end do
c
c..remaining surface parameters (without input data)
      if(isurf.eq.1) then
        nl=nniv
        do np=1,npar
          if(jparam(np).eq.0.) then
            if(iparam(np).eq.13) then
c..omega
              do nt=1,ntid
                do nn=1,npos
                  dat(nn,np,nl,nt)=0.
                end do
              end do
            else
c..all other parameters as lower model sigma/eta level
              do nt=1,ntid
                do nn=1,npos
                  dat(nn,np,nl,nt)=dat(nn,np,nl-1,nt)
                end do
              end do
            end if
          end if
        end do
      end if
c
c..possibly scale data
      do np=1,npar
        k=index(pcode(np),'scale=')
        if(k.gt.0) then
          text=pcode(np)(k+6:)
          scale=0.
          read(text,*,iostat=ios) scale
          if(ios.eq.0 .and. scale.ne.0. .and. scale.ne.1.) then
            do nt=1,ntid
              do nl=1,nniv
                do nn=1,npos
                  dat(nn,np,nl,nt)=dat(nn,np,nl,nt)*scale
                end do
              end do
            end do
          end if
        end if
      end do
c
c..possibly change parameter no.
      do np=1,npar
        k=index(pcode(np),'param=')
        if(k.gt.0) then
          text=pcode(np)(k+6:)
          newpar=-99999
          read(text,*,iostat=ios) newpar
          if(ios.eq.0 .and. newpar.ge.-32767
     +       .and. newpar.le.+32767) iparam(np)=newpar
        end if
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine delpos(mpos,npos,npar,nniv,ntid,ngrd,dat,
     *                  geopos,gxypos,pos,idobs,ridobs,navn,
     *                  nkpos,kpos,datout)
c
c..fjern posisjoner fra output-liste
c
      integer   mpos,npos,npar,nniv,ntid,ngrd,nkpos
      integer   idobs(npos),kpos(nkpos)
      real      dat(npos,npar,nniv,ntid)
      real      geopos(2,mpos,ngrd),gxypos(2,mpos,ngrd),pos(3,npos)
      real      ridobs(2,npos)
      real      datout(nkpos,npar,nniv,ntid)
      character*(*) navn(npos)
c
      do nt=1,ntid
        do nl=1,nniv
          do np=1,npar
            do no=1,nkpos
              n=kpos(no)
              datout(no,np,nl,nt)=dat(n,np,nl,nt)
            end do
          end do
        end do
      end do
c
      do ng=1,ngrd
        do no=1,nkpos
          n=kpos(no)
          geopos(1,no,ng)=geopos(1,n,ng)
          geopos(2,no,ng)=geopos(2,n,ng)
          gxypos(1,no,ng)=gxypos(1,n,ng)
          gxypos(2,no,ng)=gxypos(2,n,ng)
        end do
      end do
c
      do no=1,nkpos
        n=kpos(no)
        pos(1,no)=pos(1,n)
        pos(2,no)=pos(2,n)
        pos(3,no)=pos(3,n)
        idobs(no)=idobs(n)
        ridobs(1,no)=ridobs(1,n)
        ridobs(2,no)=ridobs(2,n)
        navn(no)=navn(n)
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine deltim(npos,npar,nniv,ntid,dat,itime,aaa,bbb,pptop,
     +                  nktid,ktid)
c
c        fjern tidspunkt fra output-liste
c
      integer   npos,npar,nniv,ntid,nktid
      integer   itime(5,ntid),ktid(nktid)
      real      dat(npos,npar,nniv,ntid)
      real      aaa(nniv,ntid),bbb(nniv,ntid),pptop(ntid)
c
      do nto=1,nktid
        nt=ktid(nto)
        if(nt.ne.nto) then
          do nl=1,nniv
            do np=1,npar
              do n=1,npos
                dat(n,np,nl,nto)=dat(n,np,nl,nt)
              end do
            end do
          end do
          do i=1,5
            itime(i,nto)=itime(i,nt)
          end do
          do nl=1,nniv
            aaa(nl,nto)=aaa(nl,nt)
            bbb(nl,nto)=bbb(nl,nt)
          end do
          pptop(nto)=pptop(nt)
        end if
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine pliste(mpos,npos,ngrd,pos,geopos,gxypos,igrd,navn)
c
c        stasjons-liste
c
      integer   mpos,npos,ngrd
      integer*2 igrd(2,ngrd)
      real      pos(3,mpos),geopos(2,mpos,ngrd),gxypos(2,mpos,ngrd)
      character*(*) navn(npos)
c
      ifil=6
c
      write(ifil,*)
      write(ifil,*)
      write(ifil,*) '-------- posisjoner ------------------------------'
c
      write(ifil,*)
      do ng=1,ngrd
        write(ifil,*) '  produsent,grid: ',igrd(1,ng),igrd(2,ng)
      end do
      write(ifil,*)
c
      do n=1,npos
        itype=nint(pos(1,n))
        write(ifil,1001) navn(n)
 1001   format(1x,a30)
        do ng=1,ngrd
          write(ifil,1002) itype,(pos(i,n),i=2,3),
     *      (geopos(i,n,ng),i=1,2),(gxypos(i,n,ng),i=1,2)
 1002     format(1x,i2,2(1x,f7.2),3x,2(1x,f7.2),3x,2(1x,f7.2))
        end do
      end do
c
      write(ifil,*) '--------------------------------------------------'
c
      return
      end
c
c**********************************************************************
c
      subroutine dliste(nout,npos,npar,nniv,ntid,itime,dat,navn)
c
c        skriver ut data.
c
      integer   nout,npos,npar,nniv,ntid
      integer   itime(5,ntid)
      real      dat(npos,npar,nniv,ntid)
      character*(*) navn(npos)
c
      ifil=6
c
      write(ifil,*)
      if(nout.eq.1) then
        write(ifil,*) ' --------- input data ----------'
      elseif(nout.eq.2) then
        write(ifil,*) ' --------- output data ----------'
      end if
c
      write(ifil,*)
      write(ifil,*) ' tid-nr, tid'
      do nt=1,ntid
        write(ifil,1005) nt,(itime(i,nt),i=1,5)
 1005   format(2x,i3,':',4x,5i6)
      end do
c
      npstep=7
c
      do n=1,npos
        do nt=1,ntid
          write(ifil,*) ' '
          do np1=1,npar,npstep
            np2=np1+npstep-1
            if(np2.gt.npar) np2=npar
            write(ifil,*) navn(n)
            do nl=1,nniv
              write(ifil,1010) nt,nl,(dat(n,np,nl,nt),np=np1,np2)
 1010         format(1x,2i3,':',7(1x,f8.3))
            end do
          end do
        end do
      end do
      write(ifil,*) ' -----------------------------------------------'
c
      return
      end
