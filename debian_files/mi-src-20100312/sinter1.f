      PROGRAM SINTER1
C
C*********************************************************************
C**
C**      Main program for statistical interpolation (SI) in
C**      two dimensions of a scalar field.
C**      Version:  D N M I  October  1990  M. Homleid
C**      Revised            July     1991  M. Homleid and J.E. Haugen
C**      Revised            November 1991  A. Foss ... (not SI method)
C**      Revised            February 1993  A. Foss ... (unix)
C**      Revised            November 1993  A. Foss ... (added RLIST1)
C**      Revised               23.03.1994  A. Foss ... (new libmi.a)
C**      Revised               27.10.2003  A. Foss ... SGI+Linux, auto swap
C**
C*********************************************************************
C**
C**
C**  Subroutines in SIARNT :
C**
C**  SUBROUTINE ARNT
C**  FUNCTION   TOARH
C**
C**
C**  Subroutines in SIRUT1 :
C**
C**  SUBROUTINE BESSEL
C**  SUBROUTINE Q8QST4
C**  SUBROUTINE ULIBER
C**  SUBROUTINE BESIR
C**  SUBROUTINE BESJR
C**  SUBROUTINE B2SLRI
C**
C**
C**  Subroutines in SIRUT2 :
C**
C**  SUBROUTINE HOBS
C**  SUBROUTINE INTERP
C**  SUBROUTINE MAPPAR
C**  SUBROUTINE FELTIN
C**  SUBROUTINE FELTUT
C**  SUBROUTINE BLXY1
C**  SUBROUTINE MVMISS
C**
C**
C**  Subroutines in SIMATI :
C**
C**  SUBROUTINE MATIN2
C**
C**********************************************************************
C
C-----------------------------------------------------------------------
C       Keyword   Specifications  -  Description
C -------------   -------------------------------------------------
C       VERIFY             - print 'commands' on the screen
C       COMMENT   .......  - comment line
C       INFO      'text'   - information to be printed on the screen
C       GRID      NX,NY,XP,YP,AN,FI  - grid spec. (if no input field)
C       BOX       <number> - box size (grid units)
C       OVERLAP   <number> - overlapping between boxes (grid units)
C       R.FELT    'file', IN(1),IN(2),(IN(I),I=9,14),'code'
C                        - 'code': '=' : field = input_field
C                                  '+' : field = field + input_field
C                                  '-' : field = field - input_field
C       FELT.INT  NX,NY,XP,YP,AN,FI,'method'  - field interpolation
C                     - make new field by interpolation in existing
C                       'method' : 'BILINEAR' - 2x2 position interp.
C                                  'BESSEL'   - 4x4 position interp.
C                       field (after R.FELT or possibly before W.FELT)
C       TIME.LIM  -30,+30    - max. allowed time difference in miutes
C       PARAM     NPAR,'MSLP','T.2M','TD.2M','RH.2M','T.WATER',
C                      'PRECIP.12H','TENDENCY'
C                     - Using quality control flag:
C                       'MSLP<Qn>', n=0,1,2, to use data with quality
C                       flag equal or better than 'n'
C                       (0 is best, 3 is worst and means no control).
C                       Currently only supported for MSLP.
C                     - parameters listed above can be read from
C                       DNMI standard observation files (SYNO,DRAU).
C		      - File name: see R.OBS below
C       R.OBS     'file'
C       PARAM     NPAR,'PAR.A','PAR.B','PAR.C','PAR.D',....
C                     - parameters to be read from formatted file
C                       (reading NPAR parameters after latitude
C                        and longitude)
C		      - File name: see R.LIST below
C       R.LIST    'file'
C       PARAM     NPAR,'SST'
C                     - parameter to be read from 'line' file
C                       (latitude, longitude and one parameter)
C		      - File name: see R.LINE below
C       R.LINE    'file'
C       CORRFUNC  'BESSEL'      - correlation function
C                 'GAUSS'
C                 'TOAR'
C       RP        <value>    - "influence" radius in km
C       RO        <value>    - "influence" radius in km for correlated
C                                          observations from sat.
C       STD.P     <value>    - standardavvik for "prognosefeil"
C       STD.O     <value>    - standardavvik for observasjonsfeil
C       STD.O.INP 'PAR_3'    - standardavvik for observasjonsfeil,
C                              innlest fra 'liste-file'
C                              (evt. korrelerte observasjons-feil)
C       EV.LIMIT  <value>    - missing values i posisjoner med
C                                       estimeringsvarians > EV.LIMIT
C       SI.TYPE   'SI.TYPE.1'   - INDIK=1
C                 'SI.TYPE.2'   - INDIK=2
C                 'SI.TYPE.3'   - INDIK=3
C                 'SI.TYPE.4'   - INDIK=4
C                 'SI.TYPE.5'   - INDIK=5
C       SI        'parameter_1' - run SI with current specifications
C       W.FELT    'file', IN(1),IN(2),(IN(I),I=9,14),ISKAL - SI result
C       SI        'parameter_2'
C       W.FELT    'file', IN(1),IN(2),(IN(I),I=9,14),ISKAL - SI result
C       W.FELT.EV 'file', IN(1),IN(2),(IN(I),I=9,14),ISKAL - ESTVAR
C       BESSEL.R  <value>  -  Bessel 'influence' radius in km
C       BESSEL.D  <value>  -  Distance between table points in km
C       TEST.FELT               - test field, print min,max etc.
C       TEST.OBS                - test observations, print min,max etc.
C       FELT.COMP 'T.C->T.K'    - field: temp. from Celsius to Kelvin.
C                 'T.K->T.C'    - field: temp. from Kelvin to Celsius.
C       END                     - end of SI job
C-----------------------------------------------------------------------
C
C-----------------------------------------------------------------------
C
C  INDIK - bestemmer om det er beregnede eller innleste differanser,
C          avvik mellom innleste observasjoner og felt som skal
C          interpoleres, eller om det skal utf×res tradisjonell optimal
C          interpolasjon.
C
C   INDIK = 1 : leser OPAR og FPAR fra datafil, beregner differansen, DO
C               og interpolerer differansene. Omrâder uten data fâr 0.
C   INDIK = 2 : leser OBS fra datafil, FELT fra FELT-fil. Beregner
C               differansen i observasjonsposisjonene og interpolerer.
C               Omrâder uten data fâr verdi 0.
C   INDIK = 3 : leser OBS fra datafil. Interpolerer. Omrâder uten data
C               fâr verdi OBSMEAN.
C   INDIK = 4 : leser OBS fra datafil, FELT fra FELT-fil. Utf×rer
C               tradisjonell optimal interpolasjon.
C   INDIK = 5 : som 4, men observasjonen kan gis ulik vekt.
C               EOO - standardavvikene til observasjonsfeilene leses
C               inn i RDATA sammen med observasjonene.
C-----------------------------------------------------------------------
C
C**********************************************************************
C
C   Variabel-definisjoner :
C
C   IBOX  - antall grid-ruter i x- og y-retning for hver SI
C   IOLAP - antall grid-ruter utenfor omr}det hvor obs. benyttes
C
C   NB| Disse m} v{re i samsvar med influens-radius
C       til korrelasjonsfunksjonen og gridavstanden.
C
C
C   Flere variabel-definisjoner i SUBROUTINE ARNT
C
C**********************************************************************
C**
C**   NB| INPUT/OUTPUT AV FELT FRA/TIL 'FELT-FILE' MED RFELT/WFELT
C**
C**********************************************************************
C
C  DNMI library routines: RFELT
C                         WFELT
C                         PRHELP
C                         RCOMNT
C                         GETVAR
C                         CHCASE
C                         RLUNIT
C                         DAYTIM
C                         GCTOXY
C                         GINTER
C
C----------------------------------------------------------------------
C
C..... Legg inn advarsel hvis:
C        1) IOLAP(km) < RP
C        2) d.o. ved minsking av omr}de n}r NOBS > NO (99)
C-----------------------------------------------------------------
C
C
C        MAXDIM = max field size
C        MAXPAR = max no. of parameters read and stored at one time
C        MAXOBS = max no. of observations
C        MAXDAT = max length of observation buffer
C
      PARAMETER (MAXDIM=100000,MAXPAR=20,MAXOBS=20000,MAXDAT=100000)
C
C
      PARAMETER (LIFELT=20+MAXDIM)
C
      COMMON/F/XM(MAXDIM),FELT(MAXDIM),ESTVAR(MAXDIM),IFELT(LIFELT)
      REAL       XM,FELT,ESTVAR
      INTEGER*2  IFELT
C
      COMMON/O/OBSDAT(MAXDAT),IPEK(MAXOBS)
      REAL       OBSDAT
      INTEGER    IPEK
C
      INTEGER   ITIME(5),IADRP(MAXPAR)
      INTEGER*2 IN(16)
      REAL      UNDEF,GRID(4),XYLIM(4),GRIDEX(4)
      CHARACTER*16 PARAM(MAXPAR)
C
      LOGICAL VERIFY,OK
C
      PARAMETER (NKWORD=30)
C
      INTEGER      LKWORD(NKWORD)
      CHARACTER*12  KWORD(NKWORD)
C
      CHARACTER*16 CFUNC,EOOPAR,SITYPE,SIPAR,WRPAR,RFCODE,METHOD
      CHARACTER*78 CMD,COMAND,SPEC,SPECU,TEXT
      CHARACTER*60 FINPUT,FILNAM
C
C.................1..............2..............3............
      DATA KWORD/'VERIFY      ','COMMENT     ','INFO        ',
C.................4..............5..............6............
     *           'GRID        ','BOX         ','OVERLAP     ',
C.................7..............8..............9............
     *           'TIME.LIM    ','PARAM       ','R.OBS       ',
C.................10.............11.............12...........
     *           'R.LIST      ','R.LINE      ','CORRFUNC    ',
C.................13.............14.............15...........
     *           'RP          ','RO          ','STD.P       ',
C.................16.............17.............18...........
     *           'STD.O       ','STD.O.INP   ','EV.LIMIT    ',
C.................19.............20.............21...........
     *           'SI.TYPE     ','SI          ','R.FELT      ',
C.................22.............23.............24...........
     *           'W.FELT      ','W.FELT.EV   ','BESSEL.R    ',
C.................25.............26.............27...........
     *           'BESSEL.D    ','TEST.FELT   ','TEST.OBS    ',
C.................28.............29.............30...........
     *           'FELT.COMP   ','FELT.INT    ','END         '/
C
      DATA UNDEF/+1.E+35/
C
C
      IPRHLP=0
C
C------------------------------------------------------------------
      NARG=IARGC()
      IF(NARG.LT.1) THEN
        WRITE(6,*)
        WRITE(6,*) 'usage: sinter1 <sinter1.input>'
        WRITE(6,*) '   or: sinter1 <sinter1.input> <arguments>'
        WRITE(6,*) '   or: sinter1 <sinter1.input> ?    (to get help)'
        WRITE(6,*)
        STOP 1
      ENDIF
      CALL GETARG(1,FINPUT)
C
      IUINP=9
      OPEN(IUINP,FILE=FINPUT,
     *           ACCESS='SEQUENTIAL',FORM='FORMATTED',
     *           STATUS='OLD',IOSTAT=IOS)
      IF(IOS.NE.0) THEN
        WRITE(6,*) 'Open error:'
        WRITE(6,*)  FINPUT
        STOP 1
      ENDIF
C
      IF(NARG.EQ.2) THEN
        CALL GETARG(2,COMAND)
        IF(COMAND(1:1).EQ.'?') THEN
          CALL PRHELP(IUINP,'*=>')
          CLOSE(IUINP)
          STOP 1
        ENDIF
      ENDIF
C
      INPLIN=0
C
C..read comment lines
      CALL RCOMNT(IUINP,'*>','*',INPLIN,IERROR)
      IF(IERROR.NE.0) THEN
        WRITE(6,*) 'ERROR reading comment lines, top of file:'
        WRITE(6,*)  FINPUT
        CLOSE(IUINP)
        STOP 1
      ENDIF
C------------------------------------------------------------------
C
C
      L=LEN(KWORD(1))
      DO 10 N=1,NKWORD
        DO 11 K=L,1,-1
          IF(KWORD(N)(K:K).NE.' ') GOTO 12
   11   CONTINUE
        K=0
   12   LKWORD(N)=K
   10 CONTINUE
C
C
C..Default values (most will not work)
C
      DO 20 I=1,MAXDIM
        FELT(I)=UNDEF
        ESTVAR(I)=UNDEF
        XM(I)=UNDEF
   20 CONTINUE
C
      NX      = 1
      NY      = 1
      GRID(1) = 0.
      GRID(2) = 0.
      GRID(3) = 0.
      GRID(4) = 0.
      XYLIM(1)= 0.
      XYLIM(2)= 0.
      XYLIM(3)= 0.
      XYLIM(4)= 0.
      ISKAL = 0
      IBOX  = 0
      IOLAP = 0
      MINUT1= 0
      MINUT2= 0
      INDIK = 0
      ICF   = 0
      RP    = 0.
      RO    = 0.
      PII   = 0.
      EOO   = 0.
      ZLIMIT=100.
      IEOTYP= 0
      IEOPAR= 0
      NPARAM= 0
      NPDATA= 0
      NPUSED= 0
      NOBS  = 0
C
C  RBESSL = BESSEL "influence" radius in km
      RBESSL = 1300.
C  DBESSL = Distance between points in the BESSEL table in km
      DBESSL = 20.
C
      LCMD =LEN(CMD)
      LSPEC=LEN(SPEC)
      VERIFY=.FALSE.
      NRUN  =0
      NSIOBS=0
C
C..File unit used for all files
      IUNIT=20
C
C..Do forever loop
C
      DO 60 LOOP=1,10000
C
      CMD=' '
      INPLIN=INPLIN+1
      READ(IUINP,FMT='(A)',ERR=910,END=910) CMD
C
      IF(VERIFY) WRITE(6,*) CMD
C
      K1=0
      DO 70 K=1,LCMD
        IF(CMD(K:K).NE.' ' .AND. K1.EQ.0) K1=K
        IF(CMD(K:K).EQ.' ' .AND. K1.GT.0) GOTO 71
   70 CONTINUE
      IF(K1.EQ.0) GOTO 60
      K=LCMD+1
   71 K2=K-1
      COMAND=CMD(K1:K2)
      SPEC=CMD(K2+1:LCMD)
      SPECU=CMD(K2+1:LCMD)
C..convert to uppercase letters
      CALL CHCASE(2,1,COMAND)
      CALL CHCASE(2,1,SPECU)
C
      K=K2-K1+1
      DO 75 N=1,NKWORD
        IF(K.EQ.LKWORD(N) .AND. COMAND(1:K).EQ.KWORD(N)(1:K)) GOTO 76
   75 CONTINUE
      GOTO 910
   76 KEY=N
C
      GOTO (101,102,103,104,105,106,107,108,109,110,
     *      111,112,113,114,115,116,117,118,119,120,
     *      121,122,123,124,125,126,127,128,129,130) KEY
C
      WRITE(6,*) '***** Program Error - Keywords *****'
      STOP 255
C
C-----------------------------------------------------------------------
C.................1..............2..............3............
C     DATA KWORD/'VERIFY      ','COMMENT     ','INFO        ',
C.................4..............5..............6............
C    *           'GRID        ','BOX         ','OVERLAP     ',
C.................7..............8..............9............
C    *           'TIME.LIM    ','PARAM       ','R.OBS       ',
C.................10.............11.............12...........
C    *           'R.LIST      ','R.LINE      ','CORRFUNC    ',
C.................13.............14.............15...........
C    *           'RP          ','RO          ','STD.P       ',
C.................16.............17.............18...........
C    *           'STD.O       ','STD.O.INP   ','EV.LIMIT    ',
C.................19.............20.............21...........
C    *           'SI.TYPE     ','SI          ','R.FELT      ',
C.................22.............23.............24...........
C    *           'W.FELT      ','W.FELT.EV   ','BESSEL.R    ',
C.................25.............26.............27...........
C    *           'BESSEL.D    ','TEST.FELT   ','TEST.OBS    ',
C.................28.............29.............30...........
C    *           'FELT.COMP   ','FELT.INT    ','END         '/
C-----------------------------------------------------------------------
C
C..VERIFY (1)
  101 VERIFY=.TRUE.
      GOTO 60
C
C..COMMENT (2)
  102 GOTO 60
C
C..INFO (3)
  103 READ(SPEC,*,ERR=910,END=910) TEXT
      WRITE(6,*) TEXT
      GOTO 60
C
C..GRID (4)
  104 READ(SPEC,*,ERR=910,END=910) NX,NY,XP,YP,AN,FI
      IF(NX.LT.2 .OR. NY.LT.2 .OR. NX*NY.GT.MAXDIM .OR. AN.LE.0.)
     *                                                       GOTO 910
      GRID(1)=XP
      GRID(2)=YP
      GRID(3)=AN
      GRID(4)=FI
      XYLIM(1)=1
      XYLIM(2)=NX
      XYLIM(3)=1
      XYLIM(4)=NY
      GOTO 60
C
C..BOX (5)
  105 READ(SPEC,*,ERR=910,END=910) IBOX
      IF(IBOX.LT.1) GOTO 910
      GOTO 60
C
C..OVERLAP (6)
  106 READ(SPEC,*,ERR=910,END=910) IOLAP
C..Due to observation 'sorting' and 'box' method: IOLAP > 0
      IF(IOLAP.LT.1) GOTO 910
      GOTO 60
C
C..TIME.LIM (7)  .... used when reading standard obs. files
  107 READ(SPEC,*,ERR=910,END=910) MINUT1,MINUT2
      IF(MINUT1.GT.MINUT2) GOTO 910
      GOTO 60
C
C..PARAM (8)  .... also clearing observation buffer (all parameters)
  108 READ(SPECU,*,ERR=910,END=910) NPARAM,(PARAM(N),N=1,NPARAM)
      IF(NPARAM.LT.1 .OR. NPARAM.GT.MAXPAR) GOTO 910
C..allocate space for position (x/y)
      NPDATA=2+NPARAM
C..allocate space for SI (error and field interpolation)
      NPUSED=NPDATA+2
C..max no. of observations (with current parameter allocation)
      MOBS=MAXDAT/NPUSED
      IF(MOBS.GT.MAXOBS) MOBS=MAXOBS
C..addresses in OBSDAT to X, Y, parameters, error and interpolation
      IADRX=0*MOBS+1
      IADRY=1*MOBS+1
      DO 2008 N=1,NPARAM
        IADRP(N)=(N+1)*MOBS+1
 2008 CONTINUE
      IADRE=(NPARAM+2)*MOBS+1
      IADRI=(NPARAM+3)*MOBS+1
      NOBS=0
      GOTO 60
C
C..R.OBS (9)
  109 READ(SPEC,*,ERR=910,END=910) FILNAM
C..replace environment variables, command line arguments
      IPRHLP=1
      CALL GETVAR(1,FILNAM,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 910
      IPRHLP=0
C.......................................................
      IF(NPARAM.LT.1) GOTO 910
      CALL ROBS1(FILNAM,IUNIT,GRID,XYLIM,NPARAM,PARAM,
     *                 MINUT1,MINUT2,MOBS,NPDATA,OBSDAT,UNDEF,
     *                                           NOBS,IUTC,OK)
      IF(.NOT.OK) GOTO 930
      GOTO 60
C
C..R.LIST (10)
  110 READ(SPEC,*,ERR=910,END=910) FILNAM
C..replace environment variables, command line arguments
      IPRHLP=1
      CALL GETVAR(1,FILNAM,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 910
      IPRHLP=0
C.......................................................
      IF(NPARAM.LT.1) GOTO 910
      CALL RLIST1(FILNAM,IUNIT,GRID,XYLIM,NPARAM,PARAM,
     *                      MOBS,NPDATA,OBSDAT,NOBS,OK)
      IF(.NOT.OK) GOTO 930
      GOTO 60
C
C..R.LINE (11)
  111 READ(SPEC,*,ERR=910,END=910) FILNAM
C..replace environment variables, command line arguments
      IPRHLP=1
      CALL GETVAR(1,FILNAM,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 910
      IPRHLP=0
C.......................................................
      IF(NPARAM.NE.1) GOTO 910
CCCC  CALL RLINE1(FILNAM,IUNIT,GRID,XYLIM,NPARAM,PARAM,
CCCC *                      MOBS,NPDATA,OBSDAT,NOBS,OK)
CCCC  IF(.NOT.OK) GOTO 930
CCCC  GOTO 60
      WRITE(6,*) '**** R.LINE not working in this version ****'
      GOTO 930
C
C..CORRFUNC (12)
  112 READ(SPECU,*,ERR=910,END=910) CFUNC
      ICF=0
      IF(CFUNC.EQ.'BESSEL') ICF=1
      IF(CFUNC.EQ.'GAUSS' ) ICF=2
      IF(CFUNC.EQ.'TOAR'  ) ICF=3
      IF(ICF.EQ.0) GOTO 910
      GOTO 60
C
C..RP (13)
  113 READ(SPEC,*,ERR=910,END=910) RP
C.?   IF(RP........) GOTO 910
      GOTO 60
C
C..RO (14)
  114 READ(SPEC,*,ERR=910,END=910) RO
C.?   IF(RO........) GOTO 910
      GOTO 60
C
C..STD.P (15)
  115 READ(SPEC,*,ERR=910,END=910) PII
C.?   IF(PII.......) GOTO 910
      GOTO 60
C
C..STD.O (16)
  116 READ(SPEC,*,ERR=910,END=910) EOO
C.?   IF(EOO.......) GOTO 910
      IEOTYP=1
      GOTO 60
C
C..STD.O.INP (17)
  117 READ(SPECU,*,ERR=910,END=910) EOOPAR
      IEOPAR=0
      DO 2017 N=1,NPARAM
        IF(EOOPAR.EQ.PARAM(N)) IEOPAR=N
 2017 CONTINUE
      IF(IEOPAR.EQ.0) GOTO 910
      IEOTYP=2
      GOTO 60
C
C..EV.LIMIT (18)
  118 READ(SPEC,*,ERR=910,END=910) ZLIMIT
      GOTO 60
C
C..SI.TYPE (19)
  119 READ(SPECU,*,ERR=910,END=910) SITYPE
      INDIK=0
      IF(SITYPE.EQ.'SI.TYPE.1') INDIK=1
      IF(SITYPE.EQ.'SI.TYPE.2') INDIK=2
      IF(SITYPE.EQ.'SI.TYPE.3') INDIK=3
      IF(SITYPE.EQ.'SI.TYPE.4') INDIK=4
      IF(SITYPE.EQ.'SI.TYPE.5') INDIK=5
      GOTO 60
C
C..SI (20)
  120 NSIOBS=0
      READ(SPEC,*,ERR=910,END=910) SIPAR
      NP=0
      DO 2020 N=1,NPARAM
        IF(SIPAR.EQ.PARAM(N)) NP=N
 2020 CONTINUE
      IF(NP.EQ.0) GOTO 910
C..If more than one parameter, some data may be missing.
C..Move positions with missing data to bottom of 'arrays'.
      NOBSIN=NOBS
      IF(NPARAM.GT.1 .AND. NOBS.GT.0) THEN
        NPD=2+NP
        CALL MVMISS(MOBS,NPDATA,OBSDAT,UNDEF,NPD,NOBSIN,NOBS)
      ENDIF
C..Don't start SI if no observations.
      IF(NOBS.LT.1) THEN
        WRITE(6,*) '>>>> No observations.  No SI of ',PARAM(NP)
        NOBS=NOBSIN
        GOTO 60
      ENDIF
      IADRPP=IADRP(NP)
      IF(IEOTYP.EQ.1) THEN
C..Standardavvik for observasjonsfeil er konstant
        IADREE=IADRE
        I=IADRE-1
        DO 2120 N=1,NOBS
          OBSDAT(I+N)=EOO
 2120   CONTINUE
      ELSE
C..Standardavvik for observasjonsfeil er innlest
        IADREE=IADRP(IEOPAR)
      ENDIF
      WRITE(6,*) '>>>> Statistical Interpolation of ',PARAM(NP)
      WRITE(6,*) '>>>>         No. of observations: ',NOBS
      NSIOBS=NOBS
      CALL SIRUN(INDIK,ICF,RP,RO,PII,RBESSL,DBESSL,
     *           NOBS,OBSDAT(IADRX),OBSDAT(IADRY),OBSDAT(IADRPP),
     *           OBSDAT(IADREE),OBSDAT(IADRI),IPEK,
     *           NX,NY,GRID,IBOX,IOLAP,FELT,ESTVAR,XM)
      NRUN=NRUN+1
      NOBS=NOBSIN
      GOTO 60
C
C..R.FELT (21)
  121 continue
      do i=1,16
	IN(i)=-32767
      end do
      READ(SPEC,*,ERR=910,END=910) FILNAM,IN(1),IN(2),(IN(I),I=9,14),
     *                             RFCODE
C..replace environment variables, command line arguments
      IPRHLP=1
      CALL GETVAR(1,FILNAM,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 910
      IPRHLP=0
C.......................................................
      LIMI=LIFELT
      LIMF=MAXDIM
      CALL FELTIN(FILNAM,IUNIT,IN,LIMI,IFELT,RFCODE,LIMF,FELT,
     *                                    NX,NY,GRID,ITIME,OK)
      IF(.NOT.OK) GOTO 930
      XYLIM(1)=1
      XYLIM(2)=NX
      XYLIM(3)=1
      XYLIM(4)=NY
      GOTO 60
C
C..W.FELT (22)
  122 continue
      do i=1,16
	IN(i)=-32767
      end do
      READ(SPEC,*,ERR=910,END=910) FILNAM,IN(1),IN(2),(IN(I),I=9,14),
     *                             ISKAL
C..Don't write field if no observations in last SI
      IF(NSIOBS.LT.1) THEN
        WRITE(6,*) '>>>> No observations.  No Write.FELT'
        GOTO 60
      ENDIF
C..replace environment variables, command line arguments
      IPRHLP=1
      CALL GETVAR(1,FILNAM,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 910
      IPRHLP=0
C.......................................................
      LIMI=LIFELT
      CALL FELTUT(FILNAM,IUNIT,IN,LIMI,IFELT,NX,NY,FELT,ISKAL,
     *                          1,ESTVAR,ZLIMIT,GRID,ITIME,OK)
 
CCC   IF(.NOT.OK) GOTO 930
      IF(.NOT.OK) WRITE(6,*) ' ****** Execution continues ******'
      GOTO 60
C
C..W.FELT.EV (23)
  123 continue
      do i=1,16
	IN(i)=-32767
      end do
      READ(SPEC,*,ERR=910,END=910) FILNAM,IN(1),IN(2),(IN(I),I=9,14),
     *                             ISKAL
C..Don't write field if no observations in last SI
      IF(NSIOBS.LT.1) THEN
        WRITE(6,*) '>>>> No observations.  No Write.FELT.EV'
        GOTO 60
      ENDIF
C..replace environment variables, command line arguments
      IPRHLP=1
      CALL GETVAR(1,FILNAM,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 910
      IPRHLP=0
C.......................................................
      LIMI=LIFELT
      CALL FELTUT(FILNAM,IUNIT,IN,LIMI,IFELT,NX,NY,ESTVAR,ISKAL,
     *                                0,ESTVAR,0.,GRID,ITIME,OK)
 
CCC   IF(.NOT.OK) GOTO 980
      IF(.NOT.OK) WRITE(6,*) ' ****** Execution continues ******'
      GOTO 60
C
C..BESSEL.R (24)
  124 READ(SPEC,*,ERR=910,END=910) RBESSL
      IF(RBESSL.LE.0.) GOTO 910
      GOTO 60
C
C..BESSEL.D (25)
  125 READ(SPEC,*,ERR=910,END=910) DBESSL
      IF(DBESSL.LE.0.) GOTO 910
      GOTO 60
C
C..TEST.FELT (26)
  126 WRITE(6,*) '--------------- TEST FELT -------------------'
      WRITE(6,*) 'NX,NY:        ',NX,NY
      WRITE(6,*) 'XP,YP,AN,FI:  ',(GRID(I),I=1,4)
      NXY=NX*NY
      FMIN=+UNDEF
      FMAX=-UNDEF
      DO 2026 I=1,NXY
        IF(FMIN.GT.FELT(I)) FMIN=FELT(I)
        IF(FMAX.LT.FELT(I)) FMAX=FELT(I)
 2026 CONTINUE
      WRITE(6,*) 'FELT MIN,MAX: ',FMIN,FMAX
      WRITE(6,*) '---------------------------------------------'
      GOTO 60
C
C..TEST.OBS (27)
  127 WRITE(6,*) '--------------- TEST OBS -------------------'
      WRITE(6,*) 'NOBS,NPARAM:  ',NOBS,NPARAM
      DO 2027 NP=1,NPDATA
        IF(NP.EQ.1) THEN
          WRPAR='X position'
          IADR=IADRX
        ELSEIF(NP.EQ.2) THEN
          WRPAR='Y position'
          IADR=IADRY
        ELSE
          WRPAR=PARAM(NP-2)
          IADR=IADRP(NP-2)
        ENDIF
        FMIN=+UNDEF
        FMAX=-UNDEF
        IADR=IADR-1
        UDEF=0.9*UNDEF
        NUDEF=0
        DO 2127 I=1,NOBS
          IF(OBSDAT(IADR+I).LE.UDEF) THEN
            IF(FMIN.GT.OBSDAT(IADR+I)) FMIN=OBSDAT(IADR+I)
            IF(FMAX.LT.OBSDAT(IADR+I)) FMAX=OBSDAT(IADR+I)
          ELSE
            NUDEF=NUDEF+1
          ENDIF
 2127   CONTINUE
        WRITE(6,*) WRPAR,'  MIN,MAX,NUDEF: ',FMIN,FMAX,NUDEF
 2027 CONTINUE
      WRITE(6,*) '--------------------------------------------'
      GOTO 60
C
C..FELT.COMP (28)
  128 READ(SPECU,*,ERR=910,END=910) TEXT
      IF(TEXT(1:8).EQ.'T.C->T.K') THEN
        CONST1=+273.15
      ELSEIF(TEXT(1:8).EQ.'T.K->T.C') THEN
        CONST1=-273.15
      ELSE
        WRITE(6,*) ' **FELT.COMP** Unknown type: ',TEXT(1:40)
        GOTO 910
      ENDIF
      NXY=NX*NY
      DO 2028 I=1,NXY
        FELT(I)=FELT(I)+CONST1
 2028 CONTINUE
      GOTO 60
C
C..FELT.INT (29)
  129 GRIDEX(1)=GRID(1)
      GRIDEX(2)=GRID(2)
      GRIDEX(3)=GRID(3)
      GRIDEX(4)=GRID(4)
      IX=NX
      IY=NY
      NXY=NX*NY
      DO 2029 I=1,NXY
        ESTVAR(I)=FELT(I)
 2029 CONTINUE
      READ(SPECU,*,ERR=910,END=910) NX,NY,XP,YP,AN,FI,METHOD
      IF(NX.LT.2 .OR. NY.LT.2 .OR. NX*NY.GT.MAXDIM .OR. AN.LE.0.)
     *                                                       GOTO 910
      GRID(1)=XP
      GRID(2)=YP
      GRID(3)=AN
      GRID(4)=FI
      XYLIM(1)=1
      XYLIM(2)=NX
      XYLIM(3)=1
      XYLIM(4)=NY
      IF(METHOD.EQ.'BILINEAR') THEN
	INTERG=1
      ELSE
	INTERG=2
      END IF
      CALL GINTER(INTERG,0,+1.E+35,
     *                   IX,IY,ESTVAR,GRIDEX,
     *                   NX,NY,FELT,GRID,NUNDEF)
      IF(NUNDEF.GT.0) THEN
	WRITE(6,*) '** GINTER ERROR'
	WRITE(6,*) '**   input  grid: ',IX,IY,GRIDEX
	WRITE(6,*) '**   output grid: ',NX,NY,GRID
	WRITE(6,*) '**   no. of undefined values in output: ',NUNDEF
        GOTO 930
      END IF
      GOTO 60
C
C..END (30)
  130 GOTO 990
C
   60 CONTINUE
C
      WRITE(6,*) '********* Too much ***********'
      STOP 1
C
C-----------------------------------------------------------------------
C
C  READ OBSERVATIONS, TRANSFORM POSITIONS FROM B,L -> X,Y
C
C.old CALL RDATA(IDSDAT,INI,IPAR,INDIK)
C
C.old NOBS = NR + NS
C
C
C-----------------------------------------------------------------------
C
C
  910 WRITE(6,*) '*** Error in specifications.  Line ',INPLIN,' :'
      WRITE(6,*) 'File: '
      WRITE(6,*)  FINPUT
      WRITE(6,*) 'Command:'
      WRITE(6,*) CMD
      IF(IPRHLP.EQ.1) THEN
        L=INDEX(FINPUT,' ')-1
        IF(L.LT.1) L=LEN(FINPUT)
        WRITE(6,*)
        WRITE(6,*) 'Help from ',FINPUT(1:L),' :'
        CALL PRHELP(IUINP,'*=>')
      ENDIF
      CLOSE(IUINP)
      STOP 1
C
  930 WRITE(6,*) '********** Error exit **********'
      CLOSE(IUINP)
      STOP 1
C
  990 CONTINUE
      CLOSE(IUINP)
      WRITE(6,*) 'Finished after ',NRUN,' Statistical Interpolation(s)'
      STOP
      END
C
C********************************************************************
C
      SUBROUTINE SIRUN(INDIK,ICF,RP,RO,PII,RBESSL,DBESSL,
     *                 NOBS,XO,YO,DO,EO,OBSINT,IPEK,
     *                 NX,NY,GRID,IBOX,IOLAP,FELT,ESTVAR,XM)
C
C       Statistical Interpolation
C
      INTEGER INDIK,ICF,NOBS,NX,NY,IBOX,IOLAP
      INTEGER IPEK(NOBS)
      REAL    RP,RO,PII,RBESSL,DBESSL
      REAL    XO(NOBS),YO(NOBS),DO(NOBS),EO(NOBS),OBSINT(NOBS)
      REAL    GRID(4),FELT(NX,NY),ESTVAR(NX,NY),XM(NX,NY)
C
C
C..Max no. of observations in one box
      PARAMETER (NO=99)
C
C..Bessel tables
      PARAMETER (ND=200)
C
      COMMON/BESS/SZZ(ND),SLON(ND),STRA(ND)
C
      REAL   EMISS,OBSMEAN
C
      DOUBLE PRECISION OBSSUM
C
C-----------------------------------------------------------------------
C
C     EMISS - Nâr det ikke er noen observasjoner innenfor et omrâde
C             som kriges, fâr alle gridpunktene samme verdi. St×rrelsen
C             avhenger av INDIK. Estimeringsvariansen settes til
C             EMISS = 100.
C
      DATA EMISS /100.0/
C
      INTEGER IDIMXM(2)
      REAL    GRIDXM(4)
      REAL    DGRIDO,RBESSO,DBESSO
      REAL    XYLIM(4)
C
      DATA IDIMXM/2*0/
      DATA GRIDXM/4*0./
      DATA DGRIDO,RBESSO,DBESSO/3*0./
C
C/////////////////////////////////////////////////////////////////////
C     WRITE(6,*) '------------------ SIRUN -----------------------'
C     WRITE(6,*) 'INDIK,ICF:  ',INDIK,ICF
C     WRITE(6,*) 'RP,RO,PII:  ',RP,RO,PII
C     WRITE(6,*) 'IBOX,IOLAP: ',IBOX,IOLAP
C     WRITE(6,*) 'NOBS:       ',NOBS
C     WRITE(6,*) 'x,y,d,e:'
C     DO 1 N=1,NOBS
C       WRITE(6,*) XO(N),YO(N),DO(N),EO(N)
C   1 CONTINUE
C     WRITE(6,*) '------------------------------------------------'
C/////////////////////////////////////////////////////////////////////
C
C------------------------------------------------------------
C
      NR=NOBS
      NS=0
C
      II=NX
      JJ=NY
      XP=GRID(1)
      YP=GRID(2)
      AN=GRID(3)
      FI=GRID(4)
      D=6.368E6*1.866/AN
CCC   G0=9.8
C
      ROLAP=D*0.001*IOLAP
      IF(ROLAP.LT.RP) THEN
        WRITE(6,*) '*************************************************'
        WRITE(6,*) '************** W A R N I N G ********************'
        WRITE(6,*) '** IOLAP (overlap): ',IOLAP
        WRITE(6,*) '**        Overlap:  ',ROLAP,' km'
        WRITE(6,*) '**             RP:  ',RP,   ' km'
        WRITE(6,*) '*************************************************'
      ENDIF
C
C  Calculate mapfactor (polarstereographic grid)
C
      IF(IDIMXM(1).NE.II .OR. IDIMXM(2).NE.JJ .OR.
     *   GRIDXM(1).NE.XP .OR. GRIDXM(2).NE.YP .OR.
     *   GRIDXM(3).NE.AN .OR. GRIDXM(4).NE.FI) THEN
C
         CALL MAPPAR('M',GRID,II,JJ,XM)
C
         IDIMXM(1)=II
         IDIMXM(2)=JJ
         GRIDXM(1)=XP
         GRIDXM(2)=YP
         GRIDXM(3)=AN
         GRIDXM(4)=FI
      ENDIF
C
C   Besselfunksjonens form spesifiseres her :
C
      IF ( ICF .EQ. 1 .AND.
     *     ( DGRIDO.NE.D .OR. RBESSO.NE.RBESSL
     *                   .OR. DBESSO.NE.DBESSL) ) THEN
C
C  RADIUS= BESSEL "influence" radius in metres
         RADIUS = RBESSL*1000.
C
C  BEREGNER INFLBE FRA D OG RADIUS
C
C  D     = GRID DISTANCE IN METERS
C  INFLBE= MAX NUMBER OF GRIDPOINTS COVERED BY THE BESSEL TABLE
C  (BESSEL FUNCTIONS ARE ZERO OUTSIDE A CIRCLE WITH THIS RADIUS)
C  DXX = DISTANCE BETWEEN POINTS IN THE BESSEL TABLE IN M
C
         DXX = DBESSL*1000.
C
         INFLBE = RADIUS*1.15/D
C
C  TABULATE BESSELFUNCTIONS FOR ERROR CORRELATIONS
C
         CALL BESSEL(RADIUS,DXX,INFLBE,D,ND,SZZ,SLON,STRA,BETA,SBETA)
      ENDIF
C
C------------------------------------------------------------
C
C
      IF ( INDIK .EQ. 1 ) THEN
C
         DO 6 J=1,JJ
         DO 6 I=1,II
            FELT(I,J) = 0.0
6        CONTINUE
C
      ELSEIF ( INDIK .EQ. 2 ) THEN
C
C    Interpoaltion of field --> obs.points
         CALL INTERP(FELT,II,JJ,NOBS,XO,YO,OBSINT)
C
C    Calculate differences between observation and field
         DO 14 I = 1,NOBS
            DO(I) = DO(I) - OBSINT(I)
14       CONTINUE
         DO 16 J=1,JJ
         DO 16 I=1,II
            FELT(I,J) = 0.0
16       CONTINUE
C
      ELSEIF ( INDIK .EQ. 3 ) THEN
C
C  Calculate OBSMEAN -  mean of observations
         OBSSUM = 0.0D0
         DO 22 I = 1,NOBS
            OBSSUM = OBSSUM + DO(I)
22       CONTINUE
         OBSMEAN = OBSSUM / NOBS
C
C  Calculate differences between observations and OBSMEAN
         DO 24 I = 1,NOBS
           DO(I) = DO(I) - OBSMEAN
24       CONTINUE
         DO 26 J=1,JJ
         DO 26 I=1,II
            FELT(I,J) = OBSMEAN
26       CONTINUE
C
      ELSEIF ( INDIK .EQ. 4 .OR. INDIK .EQ. 5 ) THEN
C
C     Interpolation of field --> obs.points
CCCCCCC  CALL INTERP(FELT,II,JJ,NOBS,XO,YO,OBSINT)
C     Calculate differences between observation and field
CCCCCCC  DO 34 I = 1,NOBS
C           DO(I) = DO(I) - OBSINT(I)
C34      CONTINUE
C
         CONTINUE
C
      ELSE
         WRITE(*,*) ' STOPP fordi INDIK =', INDIK
         STOP 117
      ENDIF
C
C-----------------------------------------------------------------------
C
C  Modellering av korrelasjonsfunksjon for prognosefeil
C
C     ICF = 1 ==> BESSEL
C     ICF = 2 ==> GAUSS
C     ICF = 3 ==> TOAR
C
C-----------------------------------------------------------------------
C
C-----------------------------------------------------------------------
C     RP - "influence" radius in km
C     RO - "influence" radius in km for correlated observations from sat
C
C     PII er standardavvik for "prognosefeil"
C     EOO er standardavvik for observasjonsfeil
C-----------------------------------------------------------------------
C
C=======================================================================
C
C  L×KKE FOR HVERT ANALYSE-OMR˜DE
C
      DO 500 JB1=1,JJ,IBOX
         JB2 = MIN0( JB1+IBOX-1, JJ)
         J1  = MAX0( JB1-IOLAP,  1)
         J2  = MIN0( JB2+IOLAP, JJ)
         DO 500 IB1=1,II,IBOX
            IB2 = MIN0( IB1+IBOX-1, II)
            I1  = MAX0( IB1-IOLAP,  1)
            I2  = MIN0( IB2+IOLAP, II)
            IREDUC = 0
 100        CONTINUE
C
C  Plukker ut observasjoner som skal vÎre med i neste analyse
C  innenfor begrensningene (I1,I2)*(J1,J2)
C
            XYLIM(1)=I1
            XYLIM(2)=I2
            XYLIM(3)=J1
            XYLIM(4)=J2
            CALL HOBS(NOBS,NR,NS,XO,YO,XYLIM,
     *                 NPEK,NRPEK,NSPEK,IPEK)
C
            NTOT = NRPEK + NSPEK
CCC
C........   WRITE(66,106) IB1,IB2,JB1,JB2,I1,I2,J1,J2,NTOT
C........   WRITE(6, 106) IB1,IB2,JB1,JB2,I1,I2,J1,J2,NTOT
C106        FORMAT('  BOX:',4I4,'  OBS_BOX:',4I4,'  NO=',I3)
CCC
            IF ( NTOT .GT. NO  ) THEN
               IF ( I1 .EQ. IB1 .AND. I2 .EQ. IB2 .AND.
     +              J1 .EQ. JB1 .AND. J2 .EQ. JB2 ) GO TO 110
               I1 = MIN0( I1 + 1, IB1)
               I2 = MAX0( I2 - 1, IB2)
               J1 = MIN0( J1 + 1, JB1)
               J2 = MAX0( J2 - 1, JB2)
C
               IREDUC = IREDUC + 1
               GO TO 100
C
            ENDIF
C
 110        CONTINUE
 
            IF(IREDUC.GT.0) THEN
              WRITE(6,*) 'Area reduction due to too many obs.'
              WRITE(6,*) '     Field box: ',IB1,IB2,JB1,JB2
              WRITE(6,*) '     Obs.  box: ',I1,I2,J1,J2
              WRITE(6,*) '    No. of obs: ',NOBS
              WRITE(6,*) '    Max allowed:',NO
            ENDIF
 
            IF ( NTOT .EQ. 0 .OR. NTOT .GT. NO ) THEN
C
                DO 200 JA = JB1,JB2
                   DO 200 IA = IB1,IB2
                      ESTVAR(IA,JA) = EMISS
 200            CONTINUE
C
            ELSE
C
               CALL ARNT(IB1,IB2,JB1,JB2,NRPEK,NSPEK,NPEK,IPEK,
     +                          PII,RP,RO,ICF,NOBS,XO,YO,DO,EO,
     +                           DBESSL,D,II,JJ,XM,FELT,ESTVAR)
C
            ENDIF
C
 500  CONTINUE
C
      RETURN
      END
