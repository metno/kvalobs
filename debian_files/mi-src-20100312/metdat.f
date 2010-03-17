      program metdat
c
c        ***** dnmi og ecmwf meteogram-data *****
c
c        *****  input: felt fra felt-file(r) ****
c        *****         polarstereografisk/geografiske grid *******
c        ***** output: punkt-data for meteogram, for ibm host ****
c        *****         evt. konvertering til pc-/nd-format    ****
c
c        ***** nb| kan benyttes til $ ta ut 'ikke standard' data *
c
c
c        punkt-verdier tas fra felt i polarstereografisk eller
c        geografisk(e) grid.       vind plottes relativt nord.
c
c        NB! Denne versjonen av metdat kan p.g.a. den nye flt2pos
c            ikke haandtere flere input-grid samtidig.
c
c
c metdat.input
c----------------------------------------------------------------------
c EXAMPLE 1 : DNMI meteogram
c======================================================================
C *** metdat.input  ('metdat.input')
C ***
C *=> Meteogram data from LAM50S.
C *=>
C *=> Environment var:
C *=>    none
C *=> Command format:
C *=>    metdat  metdat.input  fltsXX.dat  +0,+48  met.dat
C *=>                          <input>     <prog>  <output>
C ***
C ***-----------------------------------------------------------------
C **
C ** Options:
C ** --------
C ** FILE=<output_file>
C ** DATA.METEOGRAM ................................. (default)
C ** DATA.WAVE
C ** DATA.SEA
C ** FORMAT.STANDARD ................................ (default)
C ** FORMAT.SWAP
C ** FORMAT.PC
C ** FORMAT.IBM
C ** POSITIONS.HERE ................................. (default)
C ** POSITIONS.FILE=<file>
C ** GRID=<producer,grid>
C ** PROG_LIMIT=<min_prog_hour,max_prog_hour> ..... (default = no limit)
C ** INTERP.BESSEL
C ** INTERP.BILINEAR ................................ (default)
C ** INTERP.NEAREST
C ** INTERP.OCEAN
C ** INTERP.MEAN.4X4
C ** CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
C ** CHECK.UNDEF.IN.EACH.FIELD
C ** PRINT_POS
C ** PRINT_DATA_IN
C ** PRINT_DATA_OUT
C ** PRINT_DATA_EXTRA
C ** INFO.OFF
C ** INFO.ON ........................................ (default)
C ** INFO.MAX
C ** END
C **
C **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C ** Options    Remember '......' syntax.
C ** ($... = environment var. ; #n = coomand line arg. no. n)
C *>
C 'FILE= #4'               <<< 'FILE= metg.dat'
C 'FORMAT.STANDARD'
C 'POSITIONS.HERE'
C 'GRID= 88,1814'
C 'PROG_LIMIT= #3'         <<< 'PROG_LIMIT= +0,+48'
C 'INTERP.BILINEAR'
C 'END'
C **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C **
C ** 'file': 'file.dat' -  file name
C **         '$.....'   -  environment var.
C **         '#n'       -  command line argument no. n
C **         '='        -  same file as previous timestep
C **
C ** no. of timesteps
C *> each timestep: 'file',data_type,forecast_length_hour
C 17
C '#2',3,0, '=',2,3,  '=',2,6,  '=',2,9,  '=',2,12, '=',2,15, '=',2,18,
C           '=',2,21, '=',2,24, '=',2,27, '=',2,30, '=',2,33, '=',2,36,
C           '=',2,39, '=',2,42, '=',2,45, '=',2,48
C **
C ** 'code': '='            : no special treatment
C **         '0.AT+0'       : not reading field at +0 forecast, value=0.
C **         '0.UNTIL.X'    : not reading fields until +X forecast, value=0.
C **         'PRECIP.MI'    : (old) the same as 'PRECIP.STEP'
C **         'PRECIP.EC'    : (old) the same as 'PRECIP.ACCUM'
C **         'T(K)->T(C)'   : Temperature from Kelvin to Celsius
C **         'TD(K)->TD(C)' : Dew point from Kelvin to Celsius
C **         'RH+T->TD(C)'  : relative humidity (%) to Td(Celsius)
C **         'RH+T->TD.ICE(C)': relative humidity (%) to Td(Celsius) ICE
C **         'TD+T->RH'     : Temp. and dew point to relative humidity (%)
C **         'U->U.EW(K)'   : U(x) or U(e/w) in m/s to U(e/w) in knots
C **         'V->V.NS(K)'   : V(y) or V(n/s) in m/s to V(n/s) in knots
C **         'U->U.EW'      : U(x) or U(e/w) to U(e/w) (no scaling)
C **         'V->V.NS'      : V(y) or V(n/s) to V(n/s) (no scaling)
C **         'U->DD'        : U(x) or U(e/w) to DD (direction)
C **         'V->FF'        : V(y) or V(n/s) to FF (speed), no scaling
C **         'V->FF(K)'     : V(y) or V(n/s) to FF (speed), m/s to knots
C **         'FF->FF(K)'    : wind speed from m/s to knots
C **         'ZERO'         : Make a dummy parameter, always 0.
C **         'PRECIP.ACCUM' : Precipitation accumulated from +0 hours
C **         'PRECIP.STEP'  : Precipitation accumulated between the
C **                          input timesteps
C **         'MIN=<value>'  : Check minimum value after interpolation
C **         'MAX=<value>'  : Check maximum value after interpolation
C **         'DIRECTION'    : True north direction (dd)
C **         'DIRECTION.180': True north direction (dd), turned 180 deg.
C **         'MAKE.INCREMENTS' : compute increments (between timesteps)
C **         'SCALE=<value>': Extra scaling
C **         'PARAM=<n>'    : Set parameter no. for output
C **
C ** No. of parameters          (scale=-32767 => automatic best scaling)
C *> each parameter: vertical_coord,pamater,lvl_1,lvl_2,'code(s)', scale
C 10
C 2,58,1000,0, '=',                  -1    <-- TRYKK (MSLP)
C 2,31,1000,0, 'T(K)->T(C)',         -1    <-- TEMPERATUR (2M)
C 2,15,1000,0, '0.At+0 Precip.Step', -2    <-- FRONTAL NEDBOR
C 2,16,1000,0, '0.At+0 Precip.Step', -2    <-- KONVEKTIV NEDBOR
C 2,33,1000,0, 'U->U.EW(K)',         -2    <-- U(X)
C 2,34,1000,0, 'V->V.NS(K)',         -2    <-- V(Y)
C 2,39,1000,0, '=',                   0    <-- SKYDEKKE: TAAKE
C 2,39, 850,0, '=',                   0    <--           CL (LAVE)
C 2,39, 500,0, '=',                   0    <--           CM (MIDDELS HOYE)
C 2,39, 300,0, '=',                   0    <--           CH (HOYE)
C **
C ** For extra parameters (not time dependant):
C ** 'code':  '='          : no special treatment (field is read)
C **          'G.LAT'      : compute and store geographic latitude
C **          'G.LONG'     : compute and store geographic longitude
C **
C ** No. of extra parameters (not time dependant)
C ** each parameter: 'file(*)', data_type,forecast_length,
C **                 vertical_coord,parameter,level_1,level_2,code,scale
C *>                             (scale=-32767 => automatic best scaling)
C 3                                   <-- Antall 'ekstra' parametre
C '*', 0,0,0, -1,0000,0,'G.LAT', -2   <-- Lagrer geografisk bredde
C '*', 0,0,0, -2,0000,0,'G.LONG',-2   <-- Lagrer geografisk lengde
C '#2',4,0,2,101,1000,0,'=',      0   <-- Interpolerer og lagrer topografi
C **
C ** No. of text lines     .... max 10 lines
C *> 'text_line(s)'        .... may include '$...' or '#n' variables
C 2
C '--- METEOGRAM --- DNMI LAM50S --- :S:'
C 'Temperatur-prog. ikke korrigert :S:'
C **
C ** When 'POSITIONS.HERE'
C **
C ** pos_type:  1 : pos_1,pos_2  - grid  i,j  (integer)
C **            2 : pos_1,pos_2  - grid  x,y  (real)
C **            3 : pos_1,pos_2  - geographic latitude,longitude (real)
C **            4 : pos_1,pos_2  - geographic latitude,longitude
C **                                          as degrees*100+minutes
C **            0 : pos_1,pos_2  - end_of_list
C **
C ** pos_type, pos_1, pos_2, 'name'
C *>
C 4, 5957, 1043, 'OSLO'
C 4, 6023, 0520, 'BERGEN'
C 4, 6331, 1008, 'TRONDHEIM'
C 4, 6941, 1855, 'TROMSO'
C 4, 6106, 1029, 'LILLEHAMMER'
C 0, 0000, 0000, '*'                  <<< end_of_list
C **------------------------------------------------------------------
c======================================================================
c
c----------------------------------------------------------------------
c EXAMPLE 2 : ECMWF meteogram
c======================================================================
C *** metdat.ecmwf  ('metdat.input')
C ***
C *=> Meteogram data from ECMWF model (geographic grids input).
C *=>
C *=> Environment var:
C *=>    none
C *=> Command format:
C *=>    metdat  metdat.ecmwf  grecXX.dat  +0,+168  ecmet.dat
C *=>                          <input>     <prog>   <output>
C ***
C ***------------------------------------------------------------------
C **
C ** Options:
C ** --------
C ** FILE=<output_file>
C ** DATA.METEOGRAM ................................. (default)
C ** DATA.WAVE
C ** DATA.SEA
C ** FORMAT.STANDARD ................................ (default)
C ** FORMAT.SWAP
C ** FORMAT.PC
C ** FORMAT.IBM
C ** POSITIONS.HERE ................................. (default)
C ** POSITIONS.FILE=<file>
C ** GRID=<producer,grid>
C ** PROG_LIMIT=<min_prog_hour,max_prog_hour> ..... (default = no limit)
C ** INTERP.BESSEL
C ** INTERP.BILINEAR ................................ (default)
C ** INTERP.NEAREST
C ** INTERP.OCEAN
C ** INTERP.MEAN.4X4
C ** CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
C ** CHECK.UNDEF.IN.EACH.FIELD
C ** PRINT_POS
C ** PRINT_DATA_IN
C ** PRINT_DATA_OUT
C ** PRINT_DATA_EXTRA
C ** INFO.OFF
C ** INFO.ON ........................................ (default)
C ** INFO.MAX
C ** END
C **
C **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C ** Options    Remember '......' syntax.
C ** ($... = environment var. ; #n = coomand line arg. no. n)
C *>
C 'FILE= #4'               <<< 'FILE= metg.dat'
C 'FORMAT.STANDARD'
C 'POSITIONS.HERE'
C 'GRID= 98,102'
C 'PROG_LIMIT= #3'         <<< 'PROG_LIMIT= +0,+168'
C 'INTERP.BILINEAR'
C 'END'
C **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C ** NOT ALLOWED IN THIS VERSION:
C ** 'GRID= 98,101'           <<< low  priority grid (first in list)
C ** 'GRID= 98,102'
C ** 'GRID= 98,103'           <<< high priority grid (last in list)
C **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C **
C ** 'file': 'file.dat' -  file name
C **         '$.....'   -  environment var.
C **         '#n'       -  command line argument no. n
C **         '='        -  same file as previous timestep
C **
C ** no. of timesteps
C *> each timestep: 'file',data_type,forecast_length_hour
C 23
C '#2'1,0,'='2,6,   '='2,12   '='2,18,  '='2,24,  '='2,30,
C         '='2,36,  '='2,42,  '='2,48,  '='2,54,  '='2,60,
C         '='2,66,  '='2,72,  '='2,78,  '='2,84,  '='2,90,
C         '='2,96,  '='2,108, '='2,120, '='2,132, '='2,144,
C         '='2,156, '='2,168
C **
C ** 'code': '='            : no special treatment
C **         '0.AT+0'       : not reading field at +0 forecast, value=0.
C **         '0.UNTIL.X'    : not reading fields until +X forecast, value=0.
C **         'PRECIP.MI'    : (old) the same as 'PRECIP.STEP'
C **         'PRECIP.EC'    : (old) the same as 'PRECIP.ACCUM'
C **         'T(K)->T(C)'   : Temperature from Kelvin to Celsius
C **         'TD(K)->TD(C)' : Dew point from Kelvin to Celsius
C **         'RH+T->TD(C)'  : relative humidity (%) to Td(Celsius)
C **         'RH+T->TD.ICE(C)': relative humidity (%) to Td(Celsius) ICE
C **         'TD+T->RH'     : Temp. and dew point to relative humidity (%)
C **         'U->U.EW(K)'   : U(x) or U(e/w) in m/s to U(e/w) in knots
C **         'V->V.NS(K)'   : V(y) or V(n/s) in m/s to V(n/s) in knots
C **         'U->U.EW'      : U(x) or U(e/w) to U(e/w) (no scaling)
C **         'V->V.NS'      : V(y) or V(n/s) to V(n/s) (no scaling)
C **         'U->DD'        : U(x) or U(e/w) to DD (direction)
C **         'V->FF'        : V(y) or V(n/s) to FF (speed), no scaling
C **         'V->FF(K)'     : V(y) or V(n/s) to FF (speed), m/s to knots
C **         'FF->FF(K)'    : wind speed from m/s to knots
C **         'ZERO'         : Make a dummy parameter, always 0.
C **         'PRECIP.ACCUM' : Precipitation accumulated from +0 hours
C **         'PRECIP.STEP'  : Precipitation accumulated between the
C **                          input timesteps
C **         'MIN=<value>'  : Check minimum value after interpolation
C **         'MAX=<value>'  : Check maximum value after interpolation
C **         'DIRECTION'    : True north direction (dd)
C **         'DIRECTION.180': True north direction (dd), turned 180 deg.
C **         'MAKE.INCREMENTS' : compute increments (between timesteps)
C **         'SCALE=<value>': Extra scaling
C **         'PARAM=<n>'    : Set parameter no. for output
C **
C ** No. of parameters
C *> each parameter: vertical_coord,pamater,lvl_1,lvl_2,'code(s)', scale
C 6
C 2,58,1000,0, '=',                   -1    <-- TRYKK (MSLP)
C 2,31,1000,0, 'T(K)->T(C)',          -1    <-- TEMPERATUR (2M)
C 2,17,1000,0, '0.At+0 Precip.Accum', -2    <-- TOTAL NEDBOR
C 2,33,1000,0, 'U->U.EW(K)',          -2    <-- U(X)
C 2,34,1000,0, 'V->V.NS(K)',          -2    <-- V(Y)
C 2,25,1000,0, '=',                    0    <-- SKYDEKKE: TOTALT
C **
C ** For extra parameters (not time dependant):
C ** 'code':  '='          : no special treatment (field is read)
C **          'G.LAT'      : compute and store geographic latitude
C **          'G.LONG'     : compute and store geographic longitude
C **
C ** No. of extra parameters (not time dependant)
C ** each parameter: 'file(*)', data_type,forecast_length,
C *>                 vertical_coord,parameter,level_1,level_2,code,scale
C 2                            <-- Antall 'ekstra' parametre
C '*', 0,0,0, -1,0000,0,'G.LAT', -2  <-- Lagrer geografisk bredde
C '*', 0,0,0, -2,0000,0,'G.LONG',-2  <-- Lagrer geografisk lengde
C **
C ** No. of text lines     .... max 10 lines
C *> 'text_line(s)'        .... may include '$...' or '#n' variables
C 1
C '--- METEOGRAM --- ECMWF --- :S:'
C **
C ** When 'POSITIONS.HERE'
C **
C ** pos_type:  1 : pos_1,pos_2  - grid  i,j  (integer)
C **            2 : pos_1,pos_2  - grid  x,y  (real)
C **            3 : pos_1,pos_2  - geographic latitude,longitude (real)
C **            4 : pos_1,pos_2  - geographic latitude,longitude
C **                                          as degrees*100+minutes
C **            0 : pos_1,pos_2  - end_of_list
C **
C ** pos_type, pos_1, pos_2, 'name'
C *>
C 4, 5957, 1043, 'OSLO'
C 4, 6023, 0520, 'BERGEN'
C 4, 6331, 1008, 'TRONDHEIM'
C 4, 6941, 1855, 'TROMSO'
C 4, 6106, 1029, 'LILLEHAMMER'
C 0, 0000, 0000, '*'                  <<< end_of_list
C **-------------------------------------------------------------------
c======================================================================
c
c POSITIONS.FILE=....
c File format:
c -------------------------
c======================================================================
C 4, 5957, 1043, 'OSLO'
C 4, 6023, 0520, 'BERGEN'
C 4, 6331, 1008, 'TRONDHEIM'
C 4, 6941, 1855, 'TROMSO'
C 4, 6106, 1029, 'LILLEHAMMER'
C 0, 0000, 0000, '*'                  <<< end_of_list
c=======================================================================
c
c
c------------------------------------------------------
c DNMI:  parameter 1: trykk
c -----            2: temperatur
c                  3: frontal nedb@r
c                  4: konvektiv nedb@r
c                  5: u
c                  6: v
c                  7: skydekke  -  t$ke (stratus/fog)
c                  8: skydekke  -  cl (lave)
c                  9: skydekke  -  cm (midlere)
c                 10: skydekke  -  ch (hoye)
c------------------------------------------------------
c ECMWF: parameter 1: trykk
c ------           2: temperatur
c                  3: nedb@r
c                  4: u
c                  5: v
c                  6: skydekke  -  totalt
c------------------------------------------------------
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
C  FILE=<output_file>
C  DATA.METEOGRAM ................................. (default)
C  DATA.WAVE
C  DATA.SEA
C  FORMAT.STANDARD ................................ (default)
C  FORMAT.SWAP
C  FORMAT.PC
C  FORMAT.IBM
C  POSITIONS.HERE ................................. (default)
C  POSITIONS.FILE=<file>
C  GRID=<producer,grid>
C  PROG_LIMIT=<min_prog_hour,max_prog_hour> ..... (default = no limit)
C  INTERP.BESSEL
C  INTERP.BILINEAR ................................ (default)
C  INTERP.NEAREST
C  INTERP.OCEAN
C  INTERP.MEAN.4X4
C  CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
C  CHECK.UNDEF.IN.EACH.FIELD
C  PRINT_POS
C  PRINT_DATA_IN
C  PRINT_DATA_OUT
C  PRINT_DATA_EXTRA
C  INFO.OFF
C  INFO.ON ........................................ (default)
C  INFO.MAX
C  END
c=======================================================================
c
c-----------------------------------------------------------------------
c  DNMI/FoU  xx.xx.198x  Anstein Foss ... IBM
c  DNMI/FoU  25.10.1989  Anstein Foss
c  DNMI/FoU  23.11.1990  Anstein Foss
c  DNMI/FoU  24.10.1991  Anstein Foss
c  DNMI/FoU  18.12.1992  Anstein Foss ... Unix
c  DNMI/FoU  03.09.1993  Anstein Foss
c  DNMI/FoU  07.10.1993  Anstein Foss
c  DNMI/FoU  15.12.1993  Anstein Foss
c  DNMI/FoU  29.03.1995  Anstein Foss ... call flt2pos (also for Hirlam)
c  DNMI/FoU  07.04.1995  Anstein Foss ... bput* routines
c  DNMI/FoU  29.12.1995  Anstein Foss
c  DNMI/FoU  14.06.1996  Anstein Foss ... minor flt2pos update
c  DNMI/FoU  03.06.1997  Anstein Foss ... interp.mean.4x4 (interp=44)
c  DNMI/FoU  05.02.1998  Anstein Foss ... TD(K)->TD(C)  and  TD+T->RH
c  DNMI/FoU  22.09.1998  Anstein Foss ... interp.ocean (interp=5)
c  DNMI/FoU  11.02.1999  Anstein Foss ... "ipcwave"
c  DNMI/FoU  18.02.1999  Anstein Foss ... removed bug, missing timestep...
c  DNMI/FoU  16.03.1999  Anstein Foss ... RH+T->TD.ICE(C) Td with ICE table
c  DNMI/FoU  29.05.2000  Anstein Foss ... automatic scaling ("-32767")
c  DNMI/FoU  30.06.2001  Anstein Foss ... automatic byte swap (input)
c  DNMI/FoU  07.03.2003  Anstein Foss ... 'rh+t->td' using first 't(k)->t(c)'
c  DNMI/FoU  03.06.2003  Anstein Foss ... format.standard always bigendian
c  DNMI/FoU  04.06.2003  Anstein Foss ... file signature 211,212 (first word)
c  DNMI/FoU  28.01.2004  Anstein Foss ... 'make.increments' (p tendency etc.)
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c-----------------------------------------------------------------------
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for metdat.f
c
c        maxpos - posisjoner/stasjoner
c        maxpar - parametre
c        maxtid - tidspunkt
c        maxdat - total datamengde (npos*npar*ntid)
c        maxflt - st@rrelse av felt fra felt-file (nx*ny)
c        maxgrd - antall grid/sektorer input
c        mxpar  - ekstra parametre (ikke tids-avhengig,
c                                   posisjon og/eller 'parameter'-felt)
c
ccc   parameter (maxpos=1500,maxtid=50,maxpar=30)
c
ccc   parameter (mxpar=6)
c
c..mi_meteogram, +0 - +48  dt=2:    25 tidspunkt  10 parametre
c..ec_meteogram, +0 - +240 dt=6/12: 31 tidspunkt   6 parametre
c
ccc   parameter (maxdat=maxpos*30*15)
c
ccccc parameter (maxflt=64000,maxgrd=3)
ccc   parameter (maxflt=64000,maxgrd=1)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'metdat.inc'
c
c..buffer and file record length
      parameter (lbuff=512)
c
      parameter (undef=+1.e+35)
c
      integer*2 itid(3,maxtid),ilev(2),ipar(8,maxpar)
      integer*2 igrd(2,maxgrd),idsave(4,maxpar*maxtid,maxgrd)
c
      integer   iskal(maxpar),konpar(maxpar),itime(5,maxtid,maxgrd)
      integer   ipcwave,ivtime(5,maxtid)
      integer   numprog(maxtid)
c
      real      pos(3,maxpos)
      real      geopos(2,maxpos,maxgrd),gxypos(2,maxpos,maxgrd)
      real      parlim(2,maxpar)
      real      dat(maxdat),dwork(maxpos)
      real      grid(6,maxgrd),skal(maxpar)
c
      integer   jinter(maxpos*3,maxgrd)
      real      rinter(16*maxpos),vturn(4,maxpos)
c
      integer      ltekst,ktekst(2,10),ntnavn(maxpos)
      integer      npudef(maxpos),ipudef(maxpos)
      character*80 tekst(10)
      character*30 navn(maxpos)
c
      integer*2 idat(20+maxflt),buff(lbuff)
      real      felt(maxflt,2)
c
      integer   idato(5),iheader(11)
      integer   kpos(maxpos),ktid(maxtid),kgrd(maxgrd)
      integer   idupl(maxpos)
      integer   iufelt(maxtid+mxpar)
      integer*2 idfile(32)
c
      integer*2 ixtid(3,mxpar),ixpar(7,mxpar)
      integer   ixskal(mxpar),konprx(mxpar)
      real      xparlim(2,mxpar)
      real      xdat(maxpos*mxpar)
c
      character*64 pcode(maxpar)
      character*64 pcodex(mxpar)
c
      character*256 filepo,fltfil(maxtid+mxpar),filein,fileot
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*256 cinput,cipart
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
c..file unit for 'metdat.input'
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
        write(6,*) '   usage: metdat <metdat.input>'
        write(6,*) '      or: metdat <metdat.input> <arguments>'
        write(6,*) '      or: metdat <metdat.input> ?     (to get help)'
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
      interp = 0
      iprpos = 0
      iprdin = 0
      iprdot = 0
      iprdex = 0
      info   = 1
      iundef =-1
      lheader= 0
c
      ipcwave=0
c
      fileot ='*'
      filepo ='*'
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
C  FILE=<output_file>
C  DATA.METEOGRAM ................................. (default)
C  DATA.WAVE
C  DATA.SEA
C  FORMAT.STANDARD ................................ (default)
C  FORMAT.SWAP
C  FORMAT.PC
C  FORMAT.IBM
C  POSITIONS.HERE ................................. (default)
C  POSITIONS.FILE=<file>
C  GRID=<producer,grid>
C  PROG_LIMIT=<min_prog_hour,max_prog_hour> ..... (default = no limit)
C  INTERP.BESSEL
C  INTERP.BILINEAR ................................ (default)
C  INTERP.NEAREST
C  INTERP.OCEAN
C  INTERP.MEAN.4X4
C  CHECK.UNDEF.IN.FIRST.FIELD ..................... (default)
C  CHECK.UNDEF.IN.EACH.FIELD
C  PRINT_POS
C  PRINT_DATA_IN
C  PRINT_DATA_OUT
C  PRINT_DATA_EXTRA
C  INFO.OFF
C  INFO.ON ........................................ (default)
C  INFO.MAX
C  END
c======================================================================
c
          if(cinput(k1:k2).eq.'file') then
c..file=<output_felt_file>
            if(fileot(1:1).ne.'*') goto 214
            if(kv1.lt.1) goto 213
            fileot=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'data.meteogram') then
c..data.meteogram
	    if(lheader.ne.0) goto 214
	    lheader=11
          elseif(cinput(k1:k2).eq.'data.wave') then
c..data.wave
	    if(lheader.ne.0) goto 214
	    lheader=8
	    ipcwave=1
          elseif(cinput(k1:k2).eq.'data.sea') then
c..data.sea
	    if(lheader.ne.0) goto 214
	    lheader=8
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
ccccc       if(ngrd.gt.maxgrd) goto 233
ccccc
            if(ngrd.gt.1) goto 233
ccccc
            read(cipart,*,err=213) (igrd(i,ngrd),i=1,2)
          elseif(cinput(k1:k2).eq.'prog_limit') then
c..prog_limit=<min_prog_hour,max_prog_hour>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) iprmin,iprmax
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
          elseif(cinput(k1:k2).eq.'interp.ocean') then
c..interp.ocean
            if(interp.ne.0) goto 214
            interp=5
          elseif(cinput(k1:k2).eq.'interp.mean.4x4') then
c..interp.mean.4x4
            if(interp.ne.0) goto 214
            interp=44
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
          elseif(cinput(k1:k2).eq.'print_data_extra') then
c..print_data_extra
            iprdex=1
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
c..file=
      if(fileot(1:1).eq.'*') goto 227
c
c..default 'data.meteogram'
      if(lheader.le.0) lheader=11
c
c..default reading positions from the 'metdat.input' file
      if(inppos.lt.0) inppos=0
c
c..default output file format
      if(iformt.lt.0) iformt=0
c
c.."valid time" on file only if DATA.WAVE and FORMAT.PC
      if(iformt.ne.2) ipcwave=0
c
c..default interpolation type is bilinear
      if(interp.eq.0) interp=1
c
c..default checking undefined field values only in first field
      if(iundef.eq.-1) iundef=1
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
c        hvert tidspunkt: 'file',data-type,prognosetid
c                         'file' = '=' => som forrige
      read(iuinp,*,iostat=ios,err=211,end=212)
     *                        (fltfil(n),(itid(i,n),i=2,3),n=1,ntid)
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
c        ipar(1,n) - vertical coordinate
c            (2,n) - parameter no.
c            (3,n) - level_1
c            (4,n) - level_2
c            (5,n) - code, see subr. flt2pos
c            (6,n) - 0=no min. value,  1=min value=parlim(1,n)
c            (7,n) - 0=no max. value,  1=max value=parlim(2,n)
c            (8,n) - time limit for setting zero values
c         pcode(n) - misc. codes
c         iskal(n) - output scaling  (ivalue=value*10.**(-iskal))
c
c     pcode: '='            : no special treatment
c            '0.at+0'       : not reading field at +0 forecast, value=0.
c            '0.until.x'    : not reading fields until +x forecast, value=0.
c            'precip.mi'    : (old) the same as 'precip.step'
c            'precip.ec'    : (old) the same as 'precip.accum'
c            't(k)->t(c)'   : temperature from kelvin to celsius
c            'td(k)->td(c)' : dew point from kelvin to celsius
c	     't.for.td'     : use this temp for rh+t->td and td+t->rh
c            'rh+t->td(c)'  : relativ fuktighet (%), td(c) output
c            'rh+t->td.ice(c)': relative humidity (%) to td(celsius) ice
c            'td+t->rh'     : temp. and dew point to relative humidity (%)
c            'u->u.ew(k)'   : u(x) or u(e/w) in m/s to u(e/w) in knots
c            'v->v.ns(k)'   : v(y) or v(n/s) in m/s to v(n/s) in knots
c            'u->u.ew'      : u(x) or u(e/w) to u(e/w) (no scaling)
c            'v->v.ns'      : v(y) or v(n/s) to v(n/s) (no scaling)
c            'u->dd'        : u(x) or u(e/w) to dd (direction)
c            'v->ff'        : v(y) or v(n/s) to ff (speed), no scaling
c            'v->ff(k)'     : v(y) or v(n/s) to ff (speed), m/s to knots
c            'ff->ff(k)'    : wind speed from m/s to knots
c            'zero'         : make a dummy parameter, always 0.
c            'precip.accum' : precipitation accumulated from +0 hours
c            'precip.step'  : precipitation accumulated between the
c                             input timesteps
c            'min=<value>'  : check minimum value after interpolation
c            'max=<value>'  : check maximum value after interpolation
c            'direction'    : true north direction (dd)
c            'direction.180': true north direction (dd), turned 180 deg.
c            'make.increments' : compute increments (between timesteps)
c            'scale=<value>': extra scaling
c            'param=<n>'    : set a new output parameter no.
c-----------------------------------------------------------------------
c
      do n=1,npar
        read(iuinp,*,iostat=ios,err=211,end=212)
     *                          (ipar(i,n),i=1,4),pcode(n),iskal(n)
        ipar(5,n)=0
        ipar(6,n)=0
        ipar(7,n)=0
        ipar(8,n)=0
        parlim(1,n)=0.
        parlim(2,n)=0.
c..parameter no., for output
        konpar(n)=ipar(2,n)
      end do
c
c..convert to lowercase
      call chcase(1,npar,pcode)
c
c..check codes handled in subr. flt2pos
      do n=1,npar
        if(index(pcode(n),'u->u.ew')      .gt.0) ipar(5,n)=1
        if(index(pcode(n),'v->v.ns')      .gt.0) ipar(5,n)=2
        if(index(pcode(n),'u->dd')        .gt.0) ipar(5,n)=1
        if(index(pcode(n),'v->ff')        .gt.0) ipar(5,n)=2
        if(index(pcode(n),'direction')    .gt.0) ipar(5,n)=3
        if(index(pcode(n),'direction.180').gt.0) ipar(5,n)=4
        if(index(pcode(n),'0.at+0')       .gt.0) ipar(5,n)=5
        if(index(pcode(n),'zero')         .gt.0) ipar(5,n)=6
c       more general method for setting zero values
        k=index(pcode(n),'0.until.')
        if(k.gt.0) then
          cipart=pcode(n)(k+8:)
          read(cipart,*,iostat=ios) ipar(8,n)
          if(ios.eq.0) ipar(5,n)=5
c          write(6,*) ' time limit for precip=0: ',ipar(8,n)
        end if
c       min/max values
        k=index(pcode(n),'min=')
        if(k.gt.0) then
          cipart=pcode(n)(k+4:)
          read(cipart,*,iostat=ios) parlim(1,n)
          if(ios.eq.0) ipar(6,n)=1
        end if
        k=index(pcode(n),'max=')
        if(k.gt.0) then
          cipart=pcode(n)(k+4:)
          read(cipart,*,iostat=ios) parlim(2,n)
          if(ios.eq.0) ipar(7,n)=1
        end if
      end do
c
c..'extra' parameters (not time dependant)
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
c..no. of 'extra' parameters (not time dependant)
      read(iuinp,*,iostat=ios,err=211,end=212) nxpar
c
      if(nxpar.lt.0) nxpar=0
      if(nxpar.gt.mxpar) then
        write(6,*) ' no. of extra parameters: ',nxpar
        write(6,*) '        max allowed:      ',mxpar
        goto 240
      end if
c
c-----------------------------------------------------------------------
c
c        ixtid(1,n): file unit
c             (2,n): data-type          (in( 9))
c             (3,n): prognose-tid       (in(10))
c        ixpar(1,n): vertikal-koordinat (in(11))
c             (2,n): parameter          (in(12))
c             (3,n): niv}_1             (in(13))
c             (4,n): niv}_2             (in(14))
c        pcodex(n) - kode for evt. beregning m.m.
c        ixskal(n):  output skalering
c
c        spesial:
c        ixpar(1,n)  ixpar(2,n)  ixskal(n)
c        0           -1          -2        : geografisk bredde
c        0           -2          -2        : geografisk lengde
c
c    pcodex:  '='          : no special treatment (field is read)
c             'g.lat'      : compute and store geographic latitude
c             'g.long'     : compute and store geographic longitude
c
c-----------------------------------------------------------------------
c
      if(nxpar.gt.0) then
        nflast=ntid
        do n=1,nxpar
          read(iuinp,*,iostat=ios,err=211,end=212)
     *                 fltfil(ntid+n),(ixtid(i,n),i=2,3),
     *                 (ixpar(i,n),i=1,4),pcodex(n),ixskal(n)
          ixpar(5,n)=0
          ixpar(6,n)=0
          ixpar(7,n)=0
          xparlim(1,n)=0.
          xparlim(2,n)=0.
          konprx(n)=ixpar(2,n)
          if(fltfil(ntid+n)(1:1).eq.'=') then
            if(nflast.lt.1) then
              write(6,*) ' no felt file name for first extra parameter.'
              goto 240
            end if
            fltfil(ntid+n)=fltfil(nflast)
            nflast=ntid+n
          elseif(fltfil(ntid+n)(1:1).ne.'*') then
            call getvar(1,fltfil(ntid+n),1,1,1,ierror)
            if(ierror.ne.0) goto 232
            nflast=ntid+n
          end if
        end do
c..convert to lowercase
        call chcase(1,nxpar,pcodex)
      end if
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
c..text for plot
c                 nb| teksten kan plasseres ved en "kommando"
c                     bakerst paa linjen(e):
c                     :v: => til venstre (vanlig)
c                     :s: => sentrert
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
c..positions .... in 'metdat.input' or separate file
c
      if(inppos.eq.0) then
c..positions in 'metdat.input'
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
c        posisjons-type,pos1,pos2,'navn'
c
      itype=999
      n=0
      do while (itype.gt.0 .and. n.lt.maxpos)
        n=n+1
        read(iu,*,iostat=ios,err=211,end=212)
     *                       itype,(pos(i,n),i=2,3),navn(n)
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
  227 write(6,*) 'no output file specified.'
      goto 240
  232 iprhlp=1
      goto 240
  233 write(6,*) 'too many specifications of the type below:'
      write(6,*) cinput
ccccc
      write(6,*) '(this version of metdat can''t handle multiple grids)'
ccccc
      goto 240
c
  240 write(6,*) 'error at line no. ',nlines,'   (or below)'
      if(iprhlp.eq.1) then
        write(6,*) 'help from ''metdat.input'':'
        call prhelp(iuinp,'*=>')
      end if
  249 close(iuinp)
      stop 1
c
  250 close(iuinp)
c
      write(6,*) 'input o.k.'
c
      ndimri=1
      if(interp.eq.1) ndimri= 4
      if(interp.eq.2) ndimri= 9
      if(interp.eq.3) ndimri=16
      if(interp.eq.4) ndimri= 1
      if(interp.eq.5) ndimri= 6
      if(interp.eq.44) ndimri= 1
c
c-----------------------------------------------------------------------
c        nb| gi gridene i @kende prioritet (f.eks. grovt 'globalt' grid
c            foran finere 'begrensede' sektorer)
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
     *         tekst(l)(kt-2:kt).eq.':V:') then
          tekst(l)(kt-2:kt)='   '
          ktekst(2,l)=0
          kt=-1
        elseif(tekst(l)(kt-2:kt).eq.':s:' .or.
     *         tekst(l)(kt-2:kt).eq.':S:') then
          tekst(l)(kt-2:kt)='   '
          ktekst(2,l)=1
          kt=-1
        elseif(tekst(l)(kt-2:kt).eq.':h:' .or.
     *         tekst(l)(kt-2:kt).eq.':H:') then
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
      numprog(1)=nfltfil
      do n=2,ntid
        if(fltfil(n).ne.fltfil(n-1)) nfltfil=nfltfil+1
	numprog(n)=nfltfil
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
            end if
          end if
        end do
        if(nt.lt.1) then
          write(6,*) ' ingen tidspunkt etter prognose-begrensning:'
          write(6,*) ' prog_limit = ',iprmin,iprmax
          goto 980
        end if
        if(nt.lt.ntid .and. nxpar.gt.0) then
          do n=1,nxpar
            fltfil(nt+n)=fltfil(ntid+n)
          end do
        end if
        ntid=nt
      end if
c
      ndat=npos*npar*ntid
c
      if(ndat.gt.maxdat) then
        write(6,*) ' for mye data,      npos*npar*ntid = ',ndat
        write(6,*) '            max tillatt ("maxdat") = ',maxdat
        write(6,*) '                              npos = ',npos
        write(6,*) '                              npar = ',npar
        write(6,*) '                              ntid = ',ntid
        ntid=maxdat/(npos*npar)
        write(6,*) ' minker antall tidspunkt til: ntid = ',ntid
        if(ntid.lt.1) goto 980
        ndat=npos*npar*ntid
      end if
c
      newpos=1
c......................................................
c
c        aapner felt-file(r) og skriver ut dato,termin
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
      if(nxpar.gt.0) then
        do n=1,nxpar
          if(fltfil(ntid+n)(1:1).eq.'*') then
            iufelt(ntid+n)=0
            ixtid(1,n)=-1
          else
            nf=0
            do i=1,ntid+n-1
              if(fltfil(ntid+n).eq.fltfil(i) .and. iufelt(i).gt.0) nf=i
            end do
            if(nf.gt.0) then
              iufelt(ntid+n)=0
              ixtid(1,n)=iufelt(nf)
            else
              ifile=ifile+1
              iufelt(ntid+n)=ifile
              ixtid(1,n)=ifile
            end if
          end if
        end do
      end if
c
      do n=1,ntid+nxpar
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
      nlev=1
      ilev(1)=0
      ilev(2)=0
c
      ndat=npos*npar*ntid
      do n=1,ndat
	dat(n)=undef
      end do
      ndat=npos*nxpar
      do n=1,ndat
	xdat(n)=undef
      end do
      do ng=1,ngrd
	do n=1,npos
	  geopos(1,n,ng)=undef
	  geopos(2,n,ng)=undef
	  gxypos(1,n,ng)=undef
	  gxypos(2,n,ng)=undef
	end do
      end do
c
c--------------------------------------------------------------------
c
      messag=info
      istop1=0
      istop2=1
c
      do np=1,nxpar
c
      if(index(pcodex(np),'g.lat') .lt.1 .and.
     *   index(pcodex(np),'g.long').lt.1) then
c
      ndat=(np-1)*npos+1
c
      do ng=1,ngrd
        iprod=igrd(1,ng)
        igrid=igrd(2,ng)
        if(info.ne.0)
     *     write(6,*) ' ekstra.    produsent,grid: ',iprod,igrid
c
        call flt2pos(messag,istop1,istop2,
     +               newpos,npos,1,nlev,1,
     +               interp,iundef,pos,geopos(1,1,ng),gxypos(1,1,ng),
     +               xdat(ndat),ixpar(1,np),ilev,ixtid(1,np),
     +               iprod,igrid,xparlim,maxflt,idat,felt,
     +               itime(1,1,ng),idsave,nx,ny,igtype,grid,
     +               jinter(1,ng),ndimri,rinter,vturn,dwork,ltim,ierror)
        if(ierror.ne.0) write(6,*) 'FLT2POS ERROR: ',ierror
        if(ierror.ne.0) goto 980
        newpos=0
c
        if(ltim.eq.0) goto 980
      end do
c
      end if
c
      end do
c
c
      messag=info
      istop1=0
      istop2=2
c..hvis 'mi': stopp innlesning hvis et felt mangler
      if(iprod.eq.88 .and. nfltfil.eq.1) istop2=1
c
c..leser felt og interpolerer til posisjoner
c
      do ng=1,ngrd
c
        iprod=igrd(1,ng)
        igrid=igrd(2,ng)
c
        if(info.ne.0)
     +     write(6,*) ' prognose.  produsent,grid: ',iprod,igrid
c
        call flt2pos(messag,istop1,istop2,
     +               newpos,npos,npar,nlev,ntid,
     +               interp,iundef,pos,geopos(1,1,ng),gxypos(1,1,ng),
     +               dat(1),ipar,ilev,itid(1,1),iprod,igrid,
     +               parlim,maxflt,idat,felt,itime(1,1,ng),
     +               idsave(1,1,ng),nx,ny,igtype,grid(1,ng),
     +               jinter(1,ng),ndimri,rinter,vturn,dwork,ltim,ierror)
        if(ierror.ne.0) write(6,*) 'FLT2POS ERROR: ',ierror
        if(ierror.ne.0) goto 980
        newpos=0
c
        if(ltim.eq.0) goto 980
c
      end do
c
c-----------------------------------------------------------------------
      if(iprpos.eq.1) call pliste(6,maxpos,npos,ngrd,
     +                            pos,geopos,gxypos,igrd,navn)
c-----------------------------------------------------------------------
      nposin=npos
      ntidin=ntid
c
      udef=undef*0.9
c
c..for lagring av geografisk bredde og lengde (lagres skalert desimalt)
c
c........geografisk bredde: vertikal-koordinat,parameter = 0,-1
c........geografisk lengde: vertikal-koordinat,parameter = 0,-2
c
      do np=1,nxpar
        ip=0
        if(index(pcodex(np),'g.lat') .gt.0) ip=1
        if(index(pcodex(np),'g.long').gt.0) ip=2
        if(ip.gt.0) then
          ndat=(np-1)*npos
	  ng=1
	  do n=1,npos
	    xdat(ndat+n)=geopos(ip,n,ng)
	  end do
	  do ng=2,ngrd
	    do n=1,npos
	      if(xdat(ndat+n).gt.udef .and. geopos(ip,n,ng).lt.udef)
     +				xdat(ndat+n)=geopos(ip,n,ng)
	    end do
	  end do
        end if
      end do
c
c#######################################################################
c     call pliste(91,maxpos,npos,ngrd,pos,geopos,gxypos,igrd,navn)
c     call dliste(91,1,npos,npar,ntid,itime,dat,navn)
c     call dlistx(91,npos,nxpar,xdat,navn)
c#######################################################################
c
c..sjekk om posisjoner ikke skal legges paa output-file,
c..posisjon fjernes hvis den mangler noe ved alle tidspunkt
c..(i denne versjonen maa EGENTLIG alle output-data eksistere...)
      do n=1,npos
	npudef(n)=0
      end do
      nd=0
      do nt=1,ntid
	do n=1,npos
	  ipudef(n)=0
	end do
	do np=1,npar
	  do n=1,npos
	    if(dat(nd+n).gt.udef) ipudef(n)=1
	  end do
	  nd=nd+npos
	end do
	do n=1,npos
	  npudef(n)=npudef(n)+ipudef(n)
	end do
      end do
c
      nkpos=0
      do n=1,npos
	if(npudef(n).lt.ntid) then
          nkpos=nkpos+1
          kpos(nkpos)=n
        else
          write(6,*) ' posisjon utenfor eller uten data: ',navn(n)
        end if
      end do
c
      if(nkpos.eq.0) then
        write(6,*) ' ingen posisjoner med data'
        goto 980
      end if
c
      if(nkpos.lt.npos) then
c..fjern posisjoner fra output-liste
        call delpos(maxpos,npos,npar,ntid,ngrd,dat,geopos,gxypos,
     *              pos,navn,nkpos,kpos,dat,nxpar,xdat,xdat)
        npos=nkpos
      end if
c
c..sjekk om tidspunkt ikke skal legges paa output-file,
c..tidspunkt fjernes hvis alle posisjoner mangler noe
c..(i denne versjonen maa EGENTLIG alle output-data eksistere...)
      nktid=0
      nd=0
      do nt=1,ntid
	do n=1,npos
	  ipudef(n)=0
	end do
	do np=1,npar
	  do n=1,npos
	    if(dat(nd+n).gt.udef) ipudef(n)=1
	  end do
	  nd=nd+npos
	end do
	nud=0
	do n=1,npos
	  nud=nud+ipudef(n)
	end do
	if(nud.lt.npos) then
          nktid=nktid+1
          ktid(nktid)=nt
	  if(ngrd.gt.1) then
	    do ng=2,ngrd
	      if(itime(1,nt, 1).eq.-32767 .and.
     +		 itime(1,nt,ng).ne.-32767) then
		do i=1,5
		  itime(i,nt,1)=itime(i,nt,ng)
		end do
	      end if
	    end do
	  end if
	end if
      end do
c
      if(nktid.eq.0) then
        write(6,*) ' ingen tidspunkt o.k.'
        goto 980
      end if
c
      if(nktid.lt.ntid) then
c..fjern tidspunkt fra output-liste
        call deltim(npos,npar,ntid,dat,itime,numprog,nktid,ktid)
        ntid=nktid
      end if
c
c..i tilfelle en og samme prognose er lagt paa flere feltfiler
c..(tilsynelatende!) ... for nedboer.
      do nt=2,ntid
	if(numprog(nt).ne.numprog(nt-1)) then
	  if(itime(1,nt,1).eq.itime(1,nt-1,1) .and.
     +       itime(2,nt,1).eq.itime(2,nt-1,1) .and.
     +       itime(3,nt,1).eq.itime(3,nt-1,1) .and.
     +       itime(4,nt,1).eq.itime(4,nt-1,1) .and.
     +       itime(5,nt,1).gt.itime(5,nt-1,1)) then
	    n=numprog(nt)
	    do it=nt,ntid
	      if(numprog(it).eq.n) numprog(it)=numprog(nt-1)
	    end do
	  end if
	end if
      end do
c
c
      write(6,*) ' no. of parameters:             ',npar
      write(6,*) ' no. of positions input,output: ',nposin,npos
      write(6,*) ' no. of timesteps input,output: ',ntidin,ntid
      write(6,*) ' no. of extra parameters:       ',nxpar
c
c#######################################################################
c     call pliste(92,maxpos,npos,ngrd,pos,geopos,gxypos,igrd,navn)
c     call dliste(92,1,npos,npar,ntid,itime,dat,navn)
c     call dlistx(92,npos,nxpar,xdat,navn)
c#######################################################################
c---------------------------------------------------------------------
      if(iprdin.eq.1) call dliste(6,1,npos,npar,ntid,itime,dat,navn)
c---------------------------------------------------------------------
c
c..tilleggsbehandling av punkt-verdier
      call comput(npos,npar,ntid,dat,pcode,konpar,numprog)
c
c---------------------------------------------------------------------
      if(iprdot.eq.1) call dliste(6,2,npos,npar,ntid,itime,dat,navn)
      if(iprdex.eq.1) call dlistx(6,npos,nxpar,xdat,navn)
c---------------------------------------------------------------------
c
c
c..max antall tegn i fast tekst
      ntt=len(tekst(1))
c..max antall tegn i hvert navn
      ntn=len(navn(1))
c
      ndat=npos*npar*ntid
c
c..max antall tegn benyttet i navn
      do n=1,npos
        kt=1
        do k=1,ntn
          if(navn(n)(k:k).ne.' ') kt=k
        end do
        ntnavn(n)=kt
      end do
c
c..skalering
      do np=1,npar
	if(iskal(np).eq.-32767) then
	  fmax=0.
          do nt=1,ntid
	    nd=npos*npar*(nt-1)+npos*(np-1)
            do n=1,npos
	      fmax=max(fmax,abs(dat(nd+n)))
            end do
          end do
	  if(fmax.gt.0.) then
	    iscale=log10(fmax)-4.
	    ifmax=nint(fmax*10.**(-iscale))
	    if(ifmax.lt.3278) then
	      iscale=iscale-1
	      ifmax=nint(fmax*10.**(-iscale))
	    end if
	    if(ifmax.gt.32766) iscale=iscale+1
	    iscale=max(iscale,-30)
	  else
	    iscale=0
	  end if
	  iskal(np)=iscale
	end if
        skal(np)=10.**(-iskal(np))
      end do
c
      nd=0
      do nt=1,ntid
        do np=1,npar
          do n=1,npos
            dat(nd+n)=skal(np)*dat(nd+n)
          end do
          nd=nd+npos
        end do
      end do
c
c..skalering av ekstra parametre
      do np=1,nxpar
	nd=npos*(np-1)
	if(ixskal(np).eq.-32767) then
	  fmax=0.
          do n=1,npos
	    fmax=max(fmax,abs(xdat(nd+n)))
          end do
	  if(fmax.gt.0.) then
	    iscale=log10(fmax)-4.
	    ifmax=nint(fmax*10.**(-iscale))
	    if(ifmax.lt.3278) then
	      iscale=iscale-1
	      ifmax=nint(fmax*10.**(-iscale))
	    end if
	    if(ifmax.gt.32766) iscale=iscale+1
	    iscale=max(iscale,-30)
	  else
	    iscale=0
	  end if
	  ixskal(np)=iscale
	end if
        xskal=10.**(-ixskal(np))
        do n=1,npos
          xdat(nd+n)=xskal*xdat(nd+n)
        end do
      end do
c
c--------------------------------------------------------------------
c..antall 16 bits ord ut.
c..nb! character*nn tekst(.) .... nn=2*n, n=1,2,3,...
c      character*nn navn(..) .... nn=2*n, n=1,2,3,...
c
      nword= lheader
     *      +ltekst*(ntt/2)+ltekst*2
     *      +npos*(ntn/2)+npos
     *      +5*ntid
     *      +npar+npar
     *      +ndat
     *      +1
c
      nxdat=npos*nxpar
      if(nxpar.gt.0) nword = nword + 1 + nxpar*2 + nxdat
c
      nrec=(nword+lbuff-1)/lbuff
c
      write(6,*) '   antall ord:     ',nword
      write(6,*) '   antall records: ',nrec
      write(6,*) '   record-lengde:  ',lbuff
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
     *           access='direct',form='unformatted',
     *           recl=(lbuff*2)/lrunit,
     *           status='unknown',iostat=ios,err=910)
c
      irec=0
      ibuff=0
c
ccc   iheader( 1)=nrec
      iheader( 1)=211
      if(lheader.eq.8) iheader( 1)=212
      iheader( 2)=npos
      iheader( 3)=npar
      iheader( 4)=ntid
      iheader( 5)=ntidin
      iheader( 6)=ltekst
      iheader( 7)=ntt
      iheader( 8)=ntn
c..following not written if 'data.wave' or 'data.sea' !!!!
      iheader( 9)=igrd(1,1)
      iheader(10)=igrd(2,1)
      iheader(11)=interp
c
      if(ngrd.gt.1) iheader(10)=0
c
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		  iheader,lheader,ierr)
      if(ierr.ne.0) goto 980
c
      call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +            tekst,ltekst,ntt,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +            ktekst,2*ltekst,ierr)
      if(ierr.ne.0) goto 980
      call bputch(iunit,irec,buff,lbuff,ibuff,konvd,konvc,
     +            navn,npos,ntn,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ntnavn,npos,ierr)
      if(ierr.ne.0) goto 980
      if(ipcwave.ne.1) then
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		    itime,5*ntid,ierr)
      else
	do n=1,ntid
	  do i=1,5
	    ivtime(i,n)=itime(i,n,1)
	  end do
	  call vtime(ivtime(1,n),ierr)
	end do
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,
     +		    ivtime,5*ntid,ierr)
      end if
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,konpar,npar,ierr)
      if(ierr.ne.0) goto 980
      call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,iskal,npar,ierr)
      if(ierr.ne.0) goto 980
c
      call bputr4(iunit,irec,buff,lbuff,ibuff,konvd,dat,ndat,ierr)
      if(ierr.ne.0) goto 980
c
c..ekstra parametre (ikke tidsserier)
c.........f.eks: bredde(-1),lengde(-2),topografi(101)
      if(nxpar.le.0) then
        i=0
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,i,1,ierr)
      else
        i=1001
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,i,1,ierr)
        if(ierr.ne.0) goto 980
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,nxpar,1,ierr)
        if(ierr.ne.0) goto 980
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,konprx,nxpar,ierr)
        if(ierr.ne.0) goto 980
        call bputi4(iunit,irec,buff,lbuff,ibuff,konvd,ixskal,nxpar,ierr)
        if(ierr.ne.0) goto 980
        call bputr4(iunit,irec,buff,lbuff,ibuff,konvd,xdat,nxdat,ierr)
        if(ierr.ne.0) goto 980
      end if
c
      i=ibuff
      call bputnd(iunit,irec,buff,lbuff,ibuff,konvd,ierr)
      if(ierr.ne.0) goto 980
c
      close(iunit)
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
  980 continue
      write(6,*) ' =========== error exit ============'
c
      close(iunito)
c
      call rmfile(fileot,0,ierror)
c
      stop 2
c---------------------------------------------------------------------
c
  990 continue
c
      end
c
c**********************************************************************
c
      subroutine comput(npos,npar,ntid,dat,pcode,konpar,numprog)
c
c        tilleggsbehandling av punkt-verdier.
c
c-----------------------------------------------------------------------
c pcode:
c ------
c '='            : no special treatment
c '0.at+0'       : not reading field at +0 forecast, value=0.
c '0.until.x'    : not reading fields until +x forecast, value=0.
c 'precip.mi     : mi precipitation, accum. between timesteps
c 'precip.ec'    : ec precipitation, accum from +0.
c 'precip.step   : precipitation, accum. between timesteps
c 'precip.accum' : precipitation, accum from +0.
c 't(k)->t(c)'   : temperature from kelvin to celsius
c 'td(k)->td(c)' : dew point from kelvin to celsius
c 't.for.td'     : use this temp for rh+t->td and td+t->rh
c 'rh+t->td(c)'  : relativ fuktighet (%), td(c) output
c 'rh+t->td.ice(c)': relative humidity (%) to td(celsius) ice table
c 'td+t->rh'     : temp. and dew point to relative humidity (%)
c 'u->u.ew(k)'   : from m/s to knots (wind rotation already done)
c 'v->v.ns(k)'   : from m/s to knots (wind rotation already done)
c 'u->dd'        : u(e/w) to dd (direction)
c 'v->ff'        : v(n/s) to ff (speed), no scaling
c 'v->ff(k)'     : v(n/s) to ff (speed), m/s to knots
c 'ff->ff(k)'    : wind speed from m/s to knots
c 'zero'         : make a dummy parameter, always 0.
c 'make.increments' : compute increments (between timesteps)
c 'scale=<value>': extra scaling
c 'param=<n>'    : set a new output parameter no.
c-----------------------------------------------------------------------
c
c
      integer   npos,npar,ntid
      integer   konpar(npar),numprog(ntid)
      real      dat(npos,npar,ntid)
      character*(*) pcode(npar)
c
      character*32 text
c
      real ewt(41)
      real eit1(18),eit2(14),eit(201)
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
c  saturation ice vapor pressure -100,-95....-20,-15 deg. Celsius:
      data eit1/.000014,.000039,.000099,.000242,.000562,.001251,
     +          .002679,.005531,.011037,.021336,.040045,.073115,
     +          .1285,  .2236,  .3802,  .633,   1.0328,  1.6532/
c
c  saturation ice vapor pressure -14, -13, .... -1 deg. Celsius:
      data eit2/1.838997, 2.030072, 2.236050, 2.457655, 2.695604,
     +          2.950596, 3.223306, 3.514377, 3.824411, 4.153955,
     +          4.503493, 4.873430, 5.264080, 5.675648/
c
c
c..grenser for akseptabel rh
      rhmin=0.01
      rhmax=1.00
c
c..precipitation type step (and mi) ... nothing to do
cc    do np=1,npar
cc      if(index(pcode(np),'precip.step').gt.0 .or.
cc   +     index(pcode(np),'precip.mi')  .gt.0) then
cc        do nt=1,ntid
cc          do n=1,npos
cc            dat(n,np,nt)=dat(n,np,nt)
cc          end do
cc        end do
cc      end if
cc    end do
c
c..precipitation type accum (and ec) .... input: total from +0
      do np=1,npar
        if(index(pcode(np),'precip.accum').gt.0 .or.
     +     index(pcode(np),'precip.ec')   .gt.0) then
          do nt=ntid,2,-1
	    if(numprog(nt).eq.numprog(nt-1)) then
c..bare hvis resultater fra samme prognose
c..(for foerste tidspunkt i hver prognose gjoeres ikke noe,
c.. kan bety problemer hvis foerste tid i hver prognose ikke er +0)
              do n=1,npos
                dat(n,np,nt)=dat(n,np,nt)-dat(n,np,nt-1)
c..p.g.a. mulig interpolasjon i forskjellige grid:
                dat(n,np,nt)=max(dat(n,np,nt),0.)
              end do
	    end if
          end do
        end if
      end do
c
c..t(k)->t(c)   : temperatur(kelvin) -> temperatur(celsius)
c..td(k)->td(c) : dew point temp. from kelvin to celsius
      do np=1,npar
        if(index(pcode(np),'t(k)->t(c)')  .gt.0 .or.
     +     index(pcode(np),'td(k)->td(c)').gt.0) then
          t0=-273.16
          do nt=1,ntid
            do n=1,npos
              dat(n,np,nt)=t0+dat(n,np,nt)
            end do
          end do
        end if
      end do
c
c..td+t->rh ... temp. and dew point to relative humidity (%)
      do np=1,npar
        if(index(pcode(np),'td+t->rh') .gt.0) then
          nptd=np
          npt =0
          nptt=0
          do i=1,npar
c..temperature (celsius)
            if(npt.eq.0 .and. index(pcode(i),'t(k)->t(c)').gt.0) npt=i
            if(nptt.eq.0 .and .index(pcode(i),'t.for.td').gt.0) nptt=i
          end do
	  if(nptt.gt.0) npt=nptt
          if(npt.eq.0) then
            write(6,*) 'not computing rh from td(c)'
            write(6,*) '(missing temperature.celsius)'
          else
c..rh output in unit %
            nprh=nptd
            do nt=1,ntid
              do n=1,npos
                t= dat(n,npt, nt)
                td=dat(n,nptd,nt)-273.16
                x=(t+105.)*0.2
                i=int(x)
                et=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
                x=(td+105.)*0.2
                i=int(x)
                etd=ewt(i)+(ewt(i+1)-ewt(i))*(x-real(i))
		rh=etd/et
                dat(n,nprh,nt)=rh*100.
              end do
            end do
c..change parameter no. (rh(32))
            konpar(nprh)=32
          end if
        end if
      end do
c
c..rh+t->td(c) ... from relative humidity to dew point temp. (celsius)
      do np=1,npar
        if(index(pcode(np),'rh+t->td(c)') .gt.0) then
          nprh=np
          npt =0
	  nptt=0
          do i=1,npar
c..temperature (celsius)
            if(npt.eq.0 .and .index(pcode(i),'t(k)->t(c)').gt.0) npt=i
            if(nptt.eq.0 .and .index(pcode(i),'t.for.td').gt.0) nptt=i
          end do
	  if(nptt.gt.0) npt=nptt
          if(npt.eq.0) then
            write(6,*) 'not computing td(c) from rh'
            write(6,*) '(missing temperature.celsius)'
          else
c..rh input in unit %
            nptd=nprh
            do nt=1,ntid
              do n=1,npos
                t= dat(n,npt, nt)
                rh=dat(n,nprh,nt)*0.01
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
                dat(n,nptd,nt)=td
              end do
            end do
c..change parameter no. (td(5))
            konpar(nptd)=5
          end if
        end if
      end do
c
c..rh+t->td.ice(c) ... from relative humidity to dew point temp. (celsius)
c..with ice phase saturation tables
      do np=1,npar
        if(index(pcode(np),'rh+t->td.ice(c)') .gt.0) then
c
          nprh=np
          npt =0
          nptt=0
          do i=1,npar
c..temperature (celsius)
            if(npt.eq.0 .and. index(pcode(i),'t(k)->t(c)').gt.0) npt=i
            if(nptt.eq.0 .and .index(pcode(i),'t.for.td').gt.0) nptt=i
          end do
	  if(nptt.gt.0) npt=nptt
          if(npt.eq.0) then
            write(6,*) 'not computing td.ice(c) from rh'
            write(6,*) '(missing temperature.celsius)'
          else
c
c..make one table with one degree resoluiton, -100,...+100 deg. Celsius
c..-100,-15
	    eit(1)=eit1(1)
	    ntb=1
	    do i=1,17
	      eit(ntb+1)=eit1(i)+(eit1(i+1)-eit1(i))*0.2
	      eit(ntb+2)=eit1(i)+(eit1(i+1)-eit1(i))*0.4
	      eit(ntb+3)=eit1(i)+(eit1(i+1)-eit1(i))*0.6
	      eit(ntb+4)=eit1(i)+(eit1(i+1)-eit1(i))*0.8
	      eit(ntb+5)=eit1(i+1)
	      ntb=ntb+5
	    end do
c..-14,-1
	    do i=1,14
	      ntb=ntb+1
	      eit(ntb)=eit2(i)
	    end do
c..0
	    ntb=ntb+1
	    eit(ntb)=ewt(21)
c..+1,+100
	    do i=21,40
	      eit(ntb+1)=ewt(i)+(ewt(i+1)-ewt(i))*0.2
	      eit(ntb+2)=ewt(i)+(ewt(i+1)-ewt(i))*0.4
	      eit(ntb+3)=ewt(i)+(ewt(i+1)-ewt(i))*0.6
	      eit(ntb+4)=ewt(i)+(ewt(i+1)-ewt(i))*0.8
	      eit(ntb+5)=ewt(i+1)
	      ntb=ntb+5
	    end do
c
c..rh input in unit %
            nptd=nprh
            do nt=1,ntid
              do n=1,npos
                t= dat(n,npt, nt)
                rh=dat(n,nprh,nt)*0.01
                x=t+101.
                i=int(x)
                et=eit(i)+(eit(i+1)-eit(i))*(x-real(i))
                if(rh.lt.rhmin) rh=rhmin
                if(rh.gt.rhmax) rh=rhmax
                etd=rh*et
                do while (eit(i).gt.etd .and. i.gt.1)
                  i=i-1
                end do
                x=(etd-eit(i))/(eit(i+1)-eit(i))
                td=-101.+real(i)+x
                dat(n,nptd,nt)=td
              end do
            end do
c..change parameter no. (td.ice(1005))
            konpar(nptd)=1005
          end if
c
        end if
      end do
c
c..wind components (rotation to east/west and north/south already done)
      do np=1,npar-1
        iknots=0
        iddff =0
        if(index(pcode(np),  'u->u.ew(k)').gt.0 .and.
     *     index(pcode(np+1),'v->v.ns(k)').gt.0) then
          iknots=1
        elseif(index(pcode(np),  'u->dd')   .gt.0 .and.
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
            do n=1,npos
              dat(n,npu,nt)=tknots*dat(n,npu,nt)
              dat(n,npv,nt)=tknots*dat(n,npv,nt)
            end do
          end do
        end if
        if(iddff.eq.1) then
c..fra u(e/w),v(n/s) til dd,ff
          deg=180./3.141592654
          do nt=1,ntid
            do n=1,npos
              u=dat(n,npu,nt)
              v=dat(n,npv,nt)
              ff=sqrt(u*u+v*v)
              if(ff.gt.1.e-10) then
                dd=270.-deg*atan2(v,u)
                if(dd.gt.360.) dd=dd-360.
                if(dd.le.  0.) dd=dd+360.
              else
                ff=0.
                dd=0.
              end if
              dat(n,npu,nt)=dd
              dat(n,npv,nt)=ff
            end do
          end do
c..change parameter no. (dd(6) ff(7))
          konpar(npu)=6
          konpar(npv)=7
        end if
      end do
c
c..wind speed m/s to knots
      do np=1,npar
        k=index(pcode(np),'ff->ff(k)')
        if(k.gt.0) then
          tknots=3600./1852.
          do nt=1,ntid
            do n=1,npos
              dat(n,np,nt)=tknots*dat(n,np,nt)
            end do
          end do
        end if
      end do
c
c..make.increments: compute increments (between timesteps)
      do np=1,npar
        if(index(pcode(np),'make.increments').gt.0) then
          do nt=ntid,2,-1
            do n=1,npos
              dat(n,np,nt)=dat(n,np,nt)-dat(n,np,nt-1)
            end do
          end do
          do n=1,npos
            dat(n,np,1)=0.
          end do
        end if
      end do
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
              do n=1,npos
                dat(n,np,nt)=dat(n,np,nt)*scale
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
     +                .and. newpar.le.+32767) konpar(np)=newpar
        end if
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine delpos(mpos,npos,npar,ntid,ngrd,dat,geopos,gxypos,
     +                  pos,navn,nkpos,kpos,datout,
     +                  nxpar,xdat,xdatot)
c
c        fjern posisjoner fra output-liste
c
      integer   mpos,npos,npar,ntid,ngrd,nkpos,nxpar
      integer   kpos(nkpos)
      real      dat(npos,npar,ntid)
      real      geopos(2,mpos,ngrd),gxypos(2,mpos,ngrd),pos(3,npos)
      real      datout(nkpos,npar,ntid)
      real      xdat(npos,nxpar),xdatot(nkpos,nxpar)
      character*(*) navn(npos)
c
      do nt=1,ntid
        do np=1,npar
          do no=1,nkpos
            n=kpos(no)
            datout(no,np,nt)=dat(n,np,nt)
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
        navn(no)=navn(n)
      end do
c
      if(nxpar.gt.0) then
        do np=1,nxpar
          do no=1,nkpos
            n=kpos(no)
            xdatot(no,np)=xdat(n,np)
	  end do
	end do
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine deltim(npos,npar,ntid,dat,itime,numprog,nktid,ktid)
c
c        fjern tidspunkt fra output-liste
c
      integer   npos,npar,ntid,nktid
      integer   itime(5,ntid),numprog(ntid),ktid(nktid)
      real      dat(npos,npar,ntid)
c
      do nto=1,nktid
        nt=ktid(nto)
        if(nt.ne.nto) then
          do np=1,npar
            do n=1,npos
              dat(n,np,nto)=dat(n,np,nt)
            end do
          end do
          do i=1,5
            itime(i,nto)=itime(i,nt)
          end do
	  numprog(nto)=numprog(nt)
        end if
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine pliste(iunit,mpos,npos,ngrd,
     +			pos,geopos,gxypos,igrd,navn)
c
c        stasjons-liste
c
      integer   iunit,mpos,npos,ngrd
      integer*2 igrd(2,ngrd)
      real      pos(3,mpos),geopos(2,mpos,ngrd),gxypos(2,mpos,ngrd)
      character*(*) navn(npos)
c
      write(iunit,*)
      write(iunit,*)
      write(iunit,*) '-------- posisjoner ----------------------------'
c
      write(iunit,*)
      do ng=1,ngrd
        write(iunit,*) '  produsent,grid: ',igrd(1,ng),igrd(2,ng)
      end do
      write(iunit,*)
c
      do n=1,npos
        itype=nint(pos(1,n))
        write(iunit,1001) navn(n)
 1001   format(1x,a30)
        do ng=1,ngrd
          write(iunit,1002) itype,(pos(i,n),i=2,3),
     *                     (geopos(i,n,ng),i=1,2),(gxypos(i,n,ng),i=1,2),
 1002     format(1x,i2,2(1x,f7.2),3x,2(1x,f7.2),3x,2(1x,f7.2))
        end do
      end do
c
      write(iunit,*) '------------------------------------------------'
c
      return
      end
c
c**********************************************************************
c
      subroutine dliste(iunit,nout,npos,npar,ntid,itime,dat,navn)
c
c        skriver ut data.
c
      integer   iunit,nout,npos,npar,ntid
      integer   itime(5,ntid)
      real      dat(npos,npar,ntid)
      character*30 navn(npos)
c
      write(iunit,*)
      if(nout.eq.1) then
        write(iunit,*) ' --------- input data ----------'
      elseif(nout.eq.2) then
        write(iunit,*) ' --------- output data ----------'
      end if
c
      write(iunit,*)
      write(iunit,*) ' tid-nr, tid'
      do nt=1,ntid
        write(iunit,1005) nt,(itime(i,nt),i=1,5)
 1005   format(2x,i3,':',4x,5i6)
      end do
c
      npstep=10
c
      do n=1,npos
        write(iunit,*)
        do np1=1,npar,npstep
          np2=np1+npstep-1
          if(np2.gt.npar) np2=npar
          write(iunit,*) navn(n)
          do nt=1,ntid
            write(iunit,1010) nt,(dat(n,np,nt),np=np1,np2)
 1010       format(1x,i3,': ',10(1x,f6.1))
          end do
        end do
      end do
      write(iunit,*) ' ---------------------------------------------'
c
      return
      end
c
c**********************************************************************
c
      subroutine dlistx(iunit,npos,npar,dat,navn)
c
c        skriver ut data ... (ekstra parametre)
c
      integer   iunit,npos,npar
      real      dat(npos,npar)
      character*30 navn(npos)
c
      write(iunit,*)
      write(iunit,*) ' --------- ekstra parametre ----------'
c
      if(npar.le.5) then
        write(iunit,*)
        do n=1,npos
          write(iunit,1010) navn(n),(dat(n,np),np=1,npar)
 1010     format(1x,a30,2x,5(1x,f8.2))
        end do
      else
        npstep=8
        do n=1,npos
          write(iunit,*)
          do np1=1,npar,npstep
            np2=np1+npstep-1
            if(np2.gt.npar) np2=npar
            write(iunit,*) navn(n)
            write(iunit,1020) (dat(n,np),np=np1,np2)
 1020       format(8(1x,f8.2))
          end do
        end do
      end if
c
      write(iunit,*) ' ---------------------------------------------'
c
      return
      end
