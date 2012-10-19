      PROGRAM OBSREAD
c
c     program som leser obs-filer for data p} Unix-anlegget.
c     antall filer bestemmes i 'obsread.input' sammen med det griddet
c     det skal velges data i.
c     utputt er ei fil med 16-bits pakka tall og symboler
c     som er sekvensiell og leses av subrutine ROBSFIL
c     i analyseprogrammet versjon av 26.6.90
c     formatet er definert fºrst i subrutine ANIN i herv{rende
c     program
c     recordlengen for output bestemmes av parameter  LREC og LENREC
c     som m} v{re like
C
C-----------------------------------------------------------------------
C obsread.input, example:
C=======================================================================
C *** obsread.lam50s (input to the obsread program)
C ***
C *=> Observations for the LAM50S analysis.
C *=>
C *=> Environment var:
C *=>    $obsdir
C *=> Command format:
C *=>    obsread  obsread.lam50s  1992,8,25,0  obs.dat
C ***
C ***-----------------------------------------------------------------
C **
C *> year,month,day,hour   ($... = env.var. ; #n = command line arg.)
C '#2'    >>>>> '1992,8,25,0'
C **
C *> max. hours difference ($... = env.var. ; #n = command line arg.)
C '-3,+3'
C **
C *> Output_file ($... = env.var.)
C '#3'    >>>>> 'obs.dat'
C **
C *> status file ($... = env.var.)
C 'status.run'
C **
C *> producer,grid, xp,yp,an,fi
C 88,1814, 59.,109.,237.,-5.
C **
C *> part of grid for data selection
C 1,121,1,97
C **
C *> Input observation files, used if time is ok. ($... = env.var.)
C '$obsdir/temp00.dat'
C '$obsdir/pilo00.dat'
C '$obsdir/syno00.dat'
C '$obsdir/drau00.dat'
C '$obsdir/arep00.dat'
C '$obsdir/temp06.dat'
C '$obsdir/pilo06.dat'
C '$obsdir/syno06.dat'
C '$obsdir/drau06.dat'
C '$obsdir/arep06.dat'
C '$obsdir/temp12.dat'
C '$obsdir/pilo12.dat'
C '$obsdir/syno12.dat'
C '$obsdir/drau12.dat'
C '$obsdir/arep12.dat'
C '$obsdir/temp18.dat'
C '$obsdir/pilo18.dat'
C '$obsdir/syno18.dat'
C '$obsdir/drau18.dat'
C '$obsdir/arep18.dat'
C '*' ........ end_of_observation_file_list
C ***-----------------------------------------------------------------
C=======================================================================
C
C  Note:  *    - comment line
C         *>   - last comment line before 'real' program input
C         *=>  - help
C-----------------------------------------------------------------------
C
C      DNMI library subroutines:  RLUNIT
C                                 RCOMNT
C                                 PRHELP
C                                 GETVAR
C                                 RMFILE
C                                 HRDIFF
C
C-----------------------------------------------------------------------
C  DNMI/FoU  26.06.1990  Knut Helge Midtbº
C  DNMI/FoU  13.04.1992  Anstein Foss ..... IBM .. Unix f77 output format
C  DNMI/FoU  02.09.1992  Anstein Foss ..... Unix . Date/time input
C  DNMI/FoU  20.04.1993  Anstein Foss
c  DNMI/FoU  17.01.1994  Anstein Foss ..... (xp,yp,... on 'obs.dat')
c  DNMI/FoU  20.02.1997  Anstein Foss ..... skip new Aireps/Andar data
c  DNMI/FoU  01.09.2003  Anstein Foss ..... automatic byteswap (input)
C-----------------------------------------------------------------------
C
C
      PARAMETER (MAXSTA=6000,MAXOBS=12000)
C
      PARAMETER (MAXFIL=200)
C
      COMMON/TID/IDAG,MND,IAAR,KL,LSI
C
      COMMON/STATION/NSTA,ISTA(16,MAXSTA),CSTA(MAXSTA)
      INTEGER*2 ISTA
      CHARACTER*6 CSTA
      COMMON/OBS/NOBS,IOBS(8,MAXOBS)
      INTEGER*2 IOBS
C
      DIMENSION ITIMEA(5),ITIME(5)
      DIMENSION NTYPE(11)
C
      INTEGER*2 NBOA(1024)
C
      DIMENSION IDSIA(10)
      DIMENSION IJLIM(4)
      DIMENSION GRID(4)
C
      CHARACTER*80 RUNTXT
      CHARACTER*1  CHSTAT
C
C
      PARAMETER (NCINP=4)
C
      CHARACTER*60 FINPUT,FISTAT,FILEOT,FILEIN(MAXFIL),FILEIX
      CHARACTER*60 CINPUT(NCINP)
C
      LOGICAL swap,swapfile
C
C-----------------------------------------------------------------------
C        HOVEDTYPER:  1 - SYNOP     (SHIP: INSTRUMENT=10)
C                     2 - AIREPS
C                     3 - SATOB
C                     4 - DRIBU
C                     5 - TEMP
C                     6 - PILOT
C                     7 - SATEM
C
C---------------------------------------------------------------------
C
C
C..get record length unit in bytes for RECL= in OPEN statements
C..(machine dependant)
      CALL RLUNIT(LRUNIT)
C
C..file unit for the 'obsread.input' file
      IUINP  = 9
C..file unit for the status file
      IUSTAT = 10
C..file unit for all input observation files
      IFILIN = 20
C..file unit for the output data file
      IFILOT = 30
C
C---------------------------------------------------------------------
      NARG=IARGC()
      IF(NARG.LT.1) THEN
        WRITE(6,*)
        WRITE(6,*) '  usage: obsread <obsread.input>'
        WRITE(6,*) '     or: obsread <obsread.input> <arguments>'
        WRITE(6,*) '     or: obsread <obsread.input> ?    (to get help)'
        WRITE(6,*)
        GOTO 981
      ENDIF
      CALL GETARG(1,FINPUT)
C
      OPEN(IUINP,FILE=FINPUT,
     *           ACCESS='SEQUENTIAL',FORM='FORMATTED',
     *           STATUS='OLD',IOSTAT=IOS)
      IF(IOS.NE.0) THEN
        WRITE(6,*) 'Open error:'
        WRITE(6,*) FINPUT
        GOTO 981
      ENDIF
C
      IF(NARG.EQ.2) THEN
        CALL GETARG(2,CINPUT(1))
        IF(CINPUT(1).EQ.'?') GOTO 19
      ENDIF
C
C
      WRITE(6,*) 'Reading input file:'
      WRITE(6,*) FINPUT
C
      NLINES=0
C
      DO N=1,NCINP
        CALL RCOMNT(IUINP,'*>','*',NLINES,IERROR)
        IF(IERROR.NE.0) GOTO 12
        NLINES=NLINES+1
        READ(IUINP,*,ERR=12) CINPUT(N)
      END DO
C
C        GRID SPEC.
C
      CALL RCOMNT(IUINP,'*>','*',NLINES,IERROR)
      IF(IERROR.NE.0) GOTO 12
      NLINES=NLINES+1
      READ(IUINP,*,ERR=12) IPROD,NRGRID, XP,YP,AN,FI
C
C        LIMITS FOR DATA SELECTION IN GRID COORDINATES (I1,I2,J1,J2)
C
      CALL RCOMNT(IUINP,'*>','*',NLINES,IERROR)
      IF(IERROR.NE.0) GOTO 12
      NLINES=NLINES+1
      READ(IUINP,*,ERR=12) IJLIM(1),IJLIM(2),IJLIM(3),IJLIM(4)
C
      CALL RCOMNT(IUINP,'*>','*',NLINES,IERROR)
      IF(IERROR.NE.0) GOTO 12
C
      DO 10 N=1,MAXFIL
        NLINES=NLINES+1
        READ(IUINP,*,ERR=12) FILEIN(N)
        IF(FILEIN(N)(1:1).EQ.'*') GOTO 11
   10 CONTINUE
      N=MAXFIL+1
      NLINES=NLINES+1
      READ(IUINP,*,ERR=12) FILEIX
      IF(FILEIX(1:1).EQ.'*') GOTO 11
      WRITE(6,*) ' **** FOR MANGE FILER ****'
      WRITE(6,*) ' **** MAX ANTALL ("MAXFIL") = ',MAXFIL
      GOTO 981
   11 NFIL=N-1
C
      GOTO 13
C
   12 WRITE(6,*) '** Read error. ''obsread.input'' file:'
      WRITE(6,*) '** ',FINPUT
      WRITE(6,*) '** At approx. line no. ',NLINES
      CLOSE(IUINP)
      GOTO 981
C
   13 CONTINUE
C
C..check if input as environment variables, command line arguments
C                    or possibly as 'user questions'.
C
      CALL GETVAR(NCINP,CINPUT,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 19
C
      CALL GETVAR(NFIL,FILEIN,1,1,1,IERROR)
      IF(IERROR.NE.0) GOTO 19
C
      N=1
      READ(CINPUT(N),*,ERR=18) IYYYY,IMM,IDD,IHH
      N=2
      READ(CINPUT(N),*,ERR=18) IHDIF1,IHDIF2
      N=3
      FILEOT=CINPUT(N)
      N=4
      FISTAT=CINPUT(N)
      GOTO 20
C
   18 WRITE(6,*) '*** Error in input: '
      WRITE(6,*) CINPUT(N)
      CLOSE(IUINP)
      GOTO 981
C
   19 L=INDEX(FINPUT,' ')-1
      IF(L.LT.1) L=LEN(FINPUT)
      WRITE(6,*) 'Help from ',FINPUT(1:L),' :'
      CALL PRHELP(IUINP,'*=>')
      CLOSE(IUINP)
      GOTO 981
C
   20 CLOSE(IUINP)
      WRITE(6,*) 'Input o.k.'
C---------------------------------------------------------------------
C
      WRITE(6,*) ' Read observations at'
      WRITE(6,*) ' year,month,day,hour: ',IYYYY,IMM,IDD,IHH
C
      IFGAT=0
      IF(IHDIF1.NE.IHDIF2) THEN
        IFGAT=1
        WRITE(6,*) ' FGAT hour interval: ',IHDIF1,IHDIF2
      ENDIF
C
      ITIMEA(1)=IYYYY
      ITIMEA(2)=IMM
      ITIMEA(3)=IDD
      ITIMEA(4)=IHH
      ITIMEA(5)=0
C
      IMMDD=IMM*100+IDD
      IHHHH=IHH*100
C
      IAAR=IYYYY
      MND =IMM
      IDAG=IDD
      KL  =IHH*100
C
      WRITE(6,*)
      WRITE(6,FMT='('' LESER OBSERVASJONER I GITTER NR:'',I7)') NRGRID
      WRITE(6,FMT='('' XP,YP,AN og FI er:'',4F10.2)') XP,YP,AN,FI
      WRITE(6,*)
      GRID(1)=XP
      GRID(2)=YP
      GRID(3)=AN
      GRID(4)=FI
C
C
      WRITE(6,FMT='('' leser fra X='',I5,'' til X='',i5)')
     *             IJLIM(1),IJLIM(2)
      WRITE(6,FMT='('' og    fra Y='',I5,'' til Y='',i5)')
     *             IJLIM(3),IJLIM(4)
      WRITE(6,*)
      IGRID1=IJLIM(1)
      IGRID2=IJLIM(2)
      IGRID3=IJLIM(3)
      IGRID4=IJLIM(4)
C
C
      NTRMOK=0
      NSTA=0
      NOBS=0
      DO 80 I=1,11
   80 NTYPE(I)=0
C
      DO 100 N=1,NFIL
C
C..OPEN FILE
        OPEN(IFILIN,FILE=FILEIN(N),
     *              FORM='UNFORMATTED',ACCESS='DIRECT',RECL=2048/LRUNIT,
     *              STATUS='OLD',IOSTAT=IOS,ERR=110)
	swap=swapfile(-IFILIN)
C..READ FIRST RECORD
        READ(IFILIN,REC=1,IOSTAT=IOS,ERR=115) NBOA
	if(swap) call bswap2(1024,NBOA)
C
        ITIME(1)=NBOA(5)
        ITIME(2)=NBOA(6)/100
        ITIME(3)=NBOA(6)-(NBOA(6)/100)*100
        ITIME(4)=NBOA(7)/100
        ITIME(5)=0
        CALL HRDIFF(0,0,ITIMEA,ITIME,IHDIFF,IERR1,IERR2)
        IF(IERR1.NE.0 .OR. IERR2.NE.0) IHDIFF=-32767
C
C..HVIS OPEN OG LESING AV F@RSTE RECORD OK SJEKKES TERMIN-TID
Cold    IF(NBOA(5).NE.IYYYY .OR.
Cold *     NBOA(6).NE.IMMDD .OR.
Cold *     NBOA(7).NE.IHHHH) THEN
        IF(IHDIFF.LT.IHDIF1 .OR. IHDIFF.GT.IHDIF2) THEN
          CLOSE(IFILIN)
          GO TO 100
        ENDIF
        GO TO 120
C
  110   WRITE(6,*) ' OPEN ERROR OBS FILE.  IOSTAT=',IOS
        WRITE(6,*) ' FILE: ',FILEIN(N)
        WRITE(6,*) ' FORTSETTER UTEN DATA FRA FILEN'
        CLOSE(IFILIN)
        GO TO 100
  115   WRITE(6,*) ' READ ERROR (REC. 1) OBS FILE.  IOSTAT=',IOS
        WRITE(6,*) ' FILE: ',FILEIN(N)
        WRITE(6,*) ' FORTSETTER UTEN DATA FRA FILEN'
        CLOSE(IFILIN)
        GO TO 100
C
C  OPEN OG LESING AV F@RSTE RECORD OK
  120   NTRMOK=NTRMOK+1
        WRITE(6,*) ' File: ',FILEIN(N)
C
C..for FGAT: set time cut at beginning or end of time interval
        IHCUT1=0
        IHCUT2=0
        IF(IFGAT.EQ.1) THEN
          IHCUT1=IHDIF1-IHDIFF
          IHCUT2=IHDIF2-IHDIFF
          WRITE(6,*) ' FGAT time cut (hours): ',IHCUT1,IHCUT2
        ENDIF
C
        CALL ROBSFIL(IFILIN,GRID,IGRID1,IGRID2,IGRID3,IGRID4,
     *               IFGAT,IHCUT1,IHCUT2,IHTYPE,NTYPE,ITIMEA,
     *		     IREADERR,IMAXUT)
C
        CLOSE(IFILIN)
C
        IF(IREADERR.EQ.1) THEN
          WRITE(6,*) ' READ ERROR (REC.GT.1) OBS FILE.  IOSTAT=',IOS
        ENDIF
        IF(IMAXUT.EQ.1) THEN
          WRITE(6,*) ' SLUTTER $ LESE PGA FOR MYE DATA FOR PROGRAM'
          GO TO 200
        ENDIF
C
        IF(NSTA.EQ.0 .AND. NOBS.EQ.0) NTRMOK=0
C
  100 CONTINUE
C
  200 CONTINUE
C
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
      WRITE(6,*)' *** OPPSUMMERING ***'
      WRITE(6,*)
      WRITE(6,*)' fordelt paa typer har programmet lagt ut:'
      WRITE(6,*) 'Hovedtype 1, instru 10: SYNOP (SHIP):  ',NTYPE(10)
      WRITE(6,*) 'Hovedtype 1, instru  1: SYNOP (LAND):  ',NTYPE(11)
      WRITE(6,*) 'Totalt hovedtype 1:     SYNOP:         ',NTYPE(1)
      WRITE(6,*) 'Hovedtype 2             AIREPS:        ',NTYPE(2)
      WRITE(6,*) 'Hovedtype 3             ERS1-data:     ',NTYPE(3)
      WRITE(6,*) 'Hovedtype 4             DRIBU          ',NTYPE(4)
      WRITE(6,*) 'Hovedtype 5             TEMP:          ',NTYPE(5)
      WRITE(6,*) 'Hovedtype 6             PILOT:         ',NTYPE(6)
      WRITE(6,*) 'Hovedtype 7             SATEM:         ',NTYPE(7)
      WRITE(6,*)
      WRITE(6,*) 'Totalt antall alle typer observasjoner: ',NSTA
      WRITE(6,*) 'Totalt antall datasett (p,z,u,v,rh):    ',NOBS
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
c     WRITE(6,*)' *** TEST AV OUTPUT ***'
c     WRITE(6,*)
c     DO 544 N=1,NSTA
c     WRITE(6,FMT='('' STA ID:'',8I7,/,7X,8I7)') (ISTA(I,N),I=1,16)
c     WRITE(6,FMT='('' KJENNETEGN: '',A6)') CSTA(N)
c     DO 544 NX=ISTA(5,N),ISTA(6,N)
c     WRITE(6,FMT='(''    OBS: '',8I7)') (IOBS(I,NX),I=1,8)
c 544 CONTINUE
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C
      WRITE(6,*) ' Output.'
      WRITE(6,*) ' File: ',FILEOT
C
C..OPEN OUTPUT FILE
C
      CALL RMFILE(FILEOT,0,IERROR)
C
      OPEN(IFILOT,FILE=FILEOT,
     *            FORM='UNFORMATTED',ACCESS='SEQUENTIAL',
     *            STATUS='UNKNOWN',IOSTAT=IOS)
      IF(IOS.NE.0) THEN
        WRITE(6,*) ' Open error.  Output file.'
        GOTO 982
      ENDIF
C
C..LEGGER UT P$ BIN#RFIL
C
C  DNMI "STANDARD": ("150 KM" => AN=79.)
      DXY=(79./AN)*150000.
C
      IF(XP.GE.-327. .AND. XP.LE.+327. .AND.
     +   YP.GE.-327. .AND. YP.LE.+327.) THEN
        IGID1=NINT(XP*100.)
        IGID2=NINT(YP*100.)
        IGID3=NINT(DXY*0.01)
        IGID4=NINT(FI)
      ELSE
        IGID1= NINT(XP)
        IGID2= NINT(YP)
        IGID3=-NINT(DXY*0.01)
        IGID4= NINT(FI)
      END IF
C
      CALL ANIN(IFILOT,NRGRID,IGID1,IGID2,IGID3,IGID4,IMAXSTR)
      IF(IMAXSTR.EQ.1) THEN
        WRITE(6,*)
        WRITE(6,*) ' PROGRAMMET HAR BARE LAGT UT EN DEL AV DATAENE'
        WRITE(6,*) ' PGA PLASSPROBLEMER I SUBR. ANIN'
        WRITE(6,*)
      ENDIF
C
C..Close output file
      CLOSE(IFILOT)
C
C-------------------------------------------------------------------
C  Write info on 'status.run' file
C
      OPEN(IUSTAT,FILE=FISTAT,
     *            ACCESS='SEQUENTIAL',FORM='FORMATTED',
     *            STATUS='UNKNOWN')
      LL=1000000
      DO 710 L=1,LL
        READ(IUSTAT,FMT='(A1)',ERR=711,END=711) CHSTAT
  710 CONTINUE
Cxxxx L=LL+1
Cx711 L=L-1
Cxxxx IF(L.GT.0) BACKSPACE(IUSTAT)
  711 CONTINUE
C
      RUNTXT=' OBSERVATIONS:         Synop  Airep  Satob  Dribu'//
     *                            '   Temp  Pilot  Satem'
      WRITE(IUSTAT,FMT='(A75)') RUNTXT(1:75)
C
      WRITE(RUNTXT,FMT='(1X,I4,1X,I2,1X,I2,1X,I4,5X,7(1X,I6))')
     *                  (ITIMEA(I),I=1,4),(NTYPE(I),I=1,7)
      WRITE(IUSTAT,FMT='(A75)') RUNTXT(1:75)
C
      CLOSE(IUSTAT)
C-------------------------------------------------------------------
      GOTO 990
C
  981 CONTINUE
CCC   STOP 1
      WRITE(6,*) 'obsread ***** STOP 1 *****'
      CALL EXIT(1)
C
  982 CONTINUE
CCC   STOP 2
      WRITE(6,*) 'obsread ***** STOP 2 *****'
      CALL EXIT(2)
C
  990 CONTINUE
      END
C
C***********************************************************************
C***********************************************************************
      SUBROUTINE ANIN(IFILOT,NRGRID,IGRID1,IGRID2,IGRID3,IGRID4,IMAXSTR)
C     DEFINISJON AV HEADER som er lagt inn i idout
c     IDOUT( 1)=NREC
c     IDOUT( 2)=LREC
c     IDOUT( 3)=NSTA
c     IDOUT( 4)=NOBS
c     IDOUT( 5)=LNIVA
c     IDOUT( 6)=LLAST
c     IDOUT( 7)=IAAR
c     IDOUT( 8)=MND
c     IDOUT( 9)=IDAG
c     IDOUT(10)=KL/100
c     IDOUT(11)=0
c     IDOUT(12)=NRGRID
c     IDOUT(13)=IGRID1
c     IDOUT(14)=IGRID2
c     IDOUT(15)=IGRID3
c     IDOUT(16)=IGRID4
c     datene ligger s} sekvensielt med 16 ord for hver observasjon
c     fulgt av ett antall av niv}er med lniva=8 tall
c     de 16 ordene er som ISTA-ARRAYEN lest inn i subrutinen ROBSFIL
c     med unntak av 5,6 og 7 som er pekere definert p} en annen m}te
c     for } unng} problemer med 16-bits apkking
c
C       16 ord for hver observasjon:
C       IDOUT( 1): HOVEDTYPE (1-7)
C            ( 2): INSTRUMENTTYPE (SE FILSTRUKTUR)
C            ( 3): TID (TIMER*100+MINUTTER)
C            ( 4): TIMEDIFFERENSE ( TIMER*100 + MINUTTER )
C   cc-ikke brukt direkte ISTA( 5): PEKER TIL F@RSTE DATANIV$ I  'IOBS'
C   cc-ikke brukt direkte ISTA( 6): PEKER TIL SISTE  DATANIV$ I  'IOBS'
C   cc-ikke brukt direkte ISTA( 7)--
C   cc-regnet om etter fºlgende formel
c         NR=ISTA(5,N)/LREC
c         IP1=I+16+1-NR*LREC
c         IP2=I+16+LNIVA*(1+ISTA(6,N)-ISTA(5,N))-NR*LREC
c        IDOUT(5): NR
c        IDOUT(6): IP1
c        IDOUT(7): IP2
C            ( 8): KJENNETEGN I KARAKTERFORMAT
C            ( 9):          "
C            (10):          "
C            (11): BREDDE*100
C            (12): LENGDE*100
C            (13): X*100
C            (14): Y*100
C            (15)--
C            (16)--
C             -- ER SATT LIK 0
c
c      tilslutt et antall av 8 ord for hver p
c      tatt direkte fra IOBS-ARRAYEN FRA subrutine ROBSFIL
C          8 ord for hvert p-nivaa
C        IOBS( 1,N): P (1/10 MB) .......
C            ( 2,N): Z  (M) ...................
C            ( 3,N): U  (1/100 M/S) ...........
C            ( 4,N): V  (1/100 M/S) ...........
C            ( 5,N): RH (PROSENT) .............
C            ( 6,N): FLAGG ORD 1
C            ( 7,N): FLAGG ORD 2
C            ( 8,N): FLAGG ORD 3
C            MANGLENDE VERDI =  -32767 FOR 1,,,8
C                N = 1,...NOBS
C
 
 
      PARAMETER (MAXSTA=6000,MAXOBS=12000)
      PARAMETER (MAXOUT=16+16*MAXSTA+8*MAXOBS)
      PARAMETER (LREC=1024)
C      antall tall pr niva
      PARAMETER (LNIVA=8)
 
      COMMON/TID/IDAG,MND,IAAR,KL,LSI
      COMMON/STATION/NSTA,ISTA(16,MAXSTA),CSTA(MAXSTA)
      INTEGER*2 ISTA
      CHARACTER*6 CSTA
      COMMON/OBS/NOBS,IOBS(8,MAXOBS)
      INTEGER*2 IOBS
 
      DIMENSION IDOUT(MAXOUT)
      INTEGER*2 IDOUT
 
      IMAXSTR=0
      WRITE(6,*)
 
      NOUT=16+16*NSTA+LNIVA*NOBS
 
      IF(NOUT.GT.MAXOUT) THEN
      WRITE(6,*)' PLASSPROBLEMER VED UTPUTTING TIL FIL,'
      WRITE(6,*)' PR@VER $ LEGGE UT DET DET ER PLASS TIL'
      WRITE(6,FMT='('' innlest antall obser '',I10)')NSTA
      WRITE(6,FMT='('' innlest antall niv}er'',I10)')NOBS
C M$ FINNE HVOR MANGE OBSER SOM KAN LEGGES UT
      NOBSL=0
      DO 100 L=1,NSTA
      LX=L
      NOBSL=NOBSL+ISTA(5,L)
      NOUTL=16+16*L+LNIVA*NOBSL
      IF(NOUTL.GT.MAXOUT) GO TO 200
  100 CONTINUE
C  LEGGER UT DET DET ER PLASS TIL
  200 CONTINUE
      NSTA=LX-1
      NOBS=NOBSL-ISTA(5,LX)
      WRITE(6,FMT='('' antall til sekvensiell fil:'')')
      WRITE(6,FMT='(''         antall obser '',I10)')NSTA
      WRITE(6,FMT='(''         antall niv}er'',I10)')NOBS
      WRITE(6,*)
      IMAXSTR=1
      ENDIF
 
      DO 10 L=1,16
      IDOUT(L)=0
   10 CONTINUE
 
      IDOUT( 5)=LNIVA
      IDOUT( 7)=IAAR
      IDOUT( 8)=MND
      IDOUT( 9)=IDAG
      IDOUT(10)=KL/100
      IDOUT(12)=NRGRID
      IDOUT(13)=IGRID1
      IDOUT(14)=IGRID2
      IDOUT(15)=IGRID3
      IDOUT(16)=IGRID4
 
 
      NX=0
      I=16
 
      DO 544 N=1,NSTA
      IDOUT(I+ 1)=ISTA( 1,N)
      IDOUT(I+ 2)=ISTA( 2,N)
      IDOUT(I+ 3)=ISTA( 3,N)
      IDOUT(I+ 4)=ISTA( 4,N)
C-OLD IDOUT(I+ 5)=I+1+16
C-OLD IDOUT(I+ 6)=I+16+LNIVA*(1+ISTA(6,N)-ISTA(5,N))
C-OLD IDOUT(I+ 7)=ISTA( 7,N)
 
C-OLD NR=ISTA(5,N)/LREC
C-OLD IP1=I+16+1-NR*LREC
C-OLD IP2=I+16+LNIVA*(1+ISTA(6,N)-ISTA(5,N))-NR*LREC
 
      IDOUT(I+ 5)=ISTA( 5,N)
      IDOUT(I+ 6)=0
      IDOUT(I+ 7)=0
 
      IDOUT(I+ 8)=ISTA( 8,N)
      IDOUT(I+ 9)=ISTA( 9,N)
      IDOUT(I+10)=ISTA(10,N)
      IDOUT(I+11)=ISTA(11,N)
      IDOUT(I+12)=ISTA(12,N)
      IDOUT(I+13)=ISTA(13,N)
      IDOUT(I+14)=ISTA(14,N)
      IDOUT(I+15)=ISTA(15,N)
      IDOUT(I+16)=ISTA(16,N)
      I=I+16
      DO 544 NXX=1,ISTA(5,N)
      NX=NX+1
      DO 543 L=1,LNIVA
      I=I+1
      IDOUT(I)=IOBS(L,NX)
  543 CONTINUE
  544 CONTINUE
 
      IMAX=I
 
C   HEADER OPPDATERES MED ANTALL RECORDS, ANTALL OBSERVASJONER
C   (N SETT MED p,u,v,rh,z for en X OG Y)
      NREC=(IMAX+LREC-1)/LREC
      LLAST=IMAX-LREC*(NREC-1)
      IDOUT( 1)=NREC
      IDOUT( 2)=LREC
      IDOUT( 3)=NSTA
      IDOUT( 4)=NOBS
      IDOUT( 6)=LLAST
C
C..First record must be complete
      IF(NREC.EQ.1) THEN
        DO I=IMAX+1,LREC
          IDOUT(I)=-32767
        END DO
        IMAX=LREC
      ENDIF
C
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     TEST OF OUTPUT
C     DO 100 L=1,I,8
C     WRITE(6,FMT='(''  TELLER'',8I7)') L
C     WRITE(6,FMT='(''  IDOUT:'',8I7)') (IDOUT(IT),IT=L,L+7)
C 100 CONTINUE
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
 
C     OUTPUT
 
      CALL OUTDAT(IFILOT,IDOUT(1),IMAX,NREC,LREC,LLAST)
 
      RETURN
C
      END
C
C***********************************************************************
      SUBROUTINE OUTDAT(IFILE,IDATA,IMAX,NREC,LREC,LLAST)
C
C        VARIABEL RECORD-LENGDE
C
 
      INTEGER*2 IDATA(IMAX)
C
 
C++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C     TEST OF HEADER OUTPUT
      WRITE(6,*)' SKRIVER UT HEADER P$ F@RSTE RECORD'
      WRITE(6,FMT='(''  IDATA:'',8I8,/,8X,8I8)') (IDATA(I),I=1,16)
      WRITE(6,*)
C++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 
      WRITE(6,*)' **DATAENE LEGGES UT P$ SEKVENSIELL FIL'
      WRITE(6,FMT='(''   ANTALL RECORDS:'',I7,'' LREC='',I6)') NREC,LREC
      WRITE(6,FMT='(''   ANTALL 16-BITS I ALT          :'',I6)') IMAX
      WRITE(6,FMT='(''   ANTALL 16-BITS P$ SISTE RECORD:'',I6)') LLAST
 
      I2=0
 
      DO 10 IREC=1,NREC
        I1=I2+1
        I2=I2+LREC
        IF(I2.GT.IMAX) I2=IMAX
C
        WRITE(IFILE,IOSTAT=IOS,ERR=90) (IDATA(I),I=I1,I2)
C
   10 CONTINUE
C
      RETURN
C
   90 WRITE(6,*) 'WRITE ERROR ON FILE: ',IFILE
      WRITE(6,*) '             IOSTAT: ',IOS
      WRITE(6,*) '            (RECORD: ',IREC,' )'
CCC   STOP 3
      WRITE(6,*) 'obsread ***** STOP 3 *****'
      CALL EXIT(3)
C
      END
C
C********************************************************************
C
      SUBROUTINE ROBSFIL(IFILE,GRID,I1,I2,J1,J2,IFGAT,IHCUT1,IHCUT2,
     *                   IHTYPE,NTYPE,ITIMEA,IREADERR,IMAXUT)
C
C        LES OBSERVASJONER FRA  OBS -FILE (NB| IKKE SATEM)
C
C------------------------------------------------------------
c        16 ord HEADER:
c     IDOUT( 1)=NREC
c     IDOUT( 2)=LREC
c     IDOUT( 3)=NSTA
c     IDOUT( 4)=NOBS
c     IDOUT( 5)=LNIVA
c     IDOUT( 6)=LLAST
c     IDOUT( 7)=IAAR
c     IDOUT( 8)=MND
c     IDOUT( 9)=IDAG
c     IDOUT(10)=KL/100
c     IDOUT(11)=0
c     IDOUT(12)=NRGRID
c     IDOUT(13)=IGRID1
c     IDOUT(14)=IGRID2
c     IDOUT(15)=IGRID3
c     IDOUT(16)=IGRID4
C------------------------------------------------------------
C       16 ord for hver observasjon
C        ISTA( 1,N): HOVEDTYPE (1-7)
C            ( 2,N): INSTRUMENTTYPE (SE FILSTRUKTUR)
C            ( 3,N): TID (TIMER*100+MINUTTER)
C            ( 4,N): TIMEDIFFERENSE ( TIMER*100 + MINUTTER )
C            ( 5,N): ANTALL NIV$ER I STASJONEN
C            ( 6,N): 0
C            ( 7,N)--0
C            ( 8,N): KJENNETEGN I KARAKTERFORMAT
C            ( 9,N):          "
C            (10,N):          "
C            (11,N): BREDDE*100
C            (12,N): LENGDE*100
C            (13,N): X*100
C            (14,N): Y*100
C-----------------
C            (15,N)--
C            (16,N)--
C             -- ER SATT LIK 0
C
C------------------------------------------------------------
C          8 ord for hvert p-nivaa
C        IOBS( 1,N): P (1/10 MB) .......
C            ( 2,N): Z  (M) ...................
C            ( 3,N): U  (1/100 M/S) ...........
C            ( 4,N): V  (1/100 M/S) ...........
C            ( 5,N): RH (PROSENT) .............
C            ( 6,N): FLAGG ORD 1
C            ( 7,N): FLAGG ORD 2
C            ( 8,N): FLAGG ORD 3
C            MANGLENDE VERDI =  -32767 FOR 1,,,8
C                N = 1,...NOBS
C------------------------------------------------------------
c      kjennetegn legges ut fra ISTA men ligger ogs} p}:
C------------------------------------------------------------
C        CSTA(N):    KJENNETEGN ....
C             N = 1,...NSTA
C
      PARAMETER (LENREC=1024)
      PARAMETER (MAXDAT=LENREC*2)
C
      PARAMETER (MAXSTA=6000,MAXOBS=12000)
C
      COMMON/STATION/NSTA,ISTA(16,MAXSTA),CSTA(MAXSTA)
      INTEGER*2 ISTA
      CHARACTER*6 CSTA
      COMMON/OBS/NOBS,IOBS(8,MAXOBS)
      INTEGER*2 IOBS
C
      DIMENSION GRID(4)
C
      DIMENSION NTYPE(11),ITIMEA(5)
C
      DIMENSION JF1(5),JF2(5),JF3(5),JF4(5),JF5(5),JF6(5)
C
      INTEGER   JTIMD1(10),JTIMD2(10)
      INTEGER   ITIMD1(10),ITIMD2(10)
C
      INTEGER*2 NLEV(11)
C
      INTEGER*2 NBO(1024),INH(8,128),IDATA(MAXDAT)
      CHARACTER*6 CID
C
      LOGICAL swap,swapfile
C
      DIMENSION EWT(41)
C
C        VANNDAMPENS METNINGSTRYKK, EWT(41).
C        T I GRADER CELSIUS: -100,-95,-90,...,90,95,100.
C            TC=...    X=(TC+105)*0.2    L=X
C            ET=EWT(L)+(EWT(L+1)-EWT(L))*(X-L)
      DATA EWT/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
C
C         TIDSGRENSER FOR DATAHOVEDTYPER I MINUTTER (hvis ikke FGAT)
      DATA JTIMD1/  0, 90,  0, 120, 120, 120,  0,  0,  0,  0/
C
      DATA JTIMD2/  0,-90,  0,-120,-120,-120,  0,  0,  0,  0/
C
      DATA NLEV/10000,9250,8500,7000,5000,4000,3000,2500,
     *          2000,1500,1000/
C
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     WRITE(6,*) '========= ROBSFIL ==========='
C     WRITE(6,*) ' IFILE= ',IFILE
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C
      IF(IFGAT.EQ.0) THEN
        DO N=1,10
          ITIMD1(N)=JTIMD1(N)
          ITIMD2(N)=JTIMD2(N)
        END DO
      ELSE
        DO N=1,10
          ITIMD2(N)=IHCUT1*60
          ITIMD1(N)=IHCUT2*60
        END DO
      END IF
C
      IREADERR=0
      IMAXUT=0
      IFIL=IFILE
      MINTERM=ITIMEA(4)*60
      MIN12H=12*60
      MINDAY=24*60
C
      DO I=1,5
        JF1(I)=0
        JF2(I)=0
        JF3(I)=0
        JF4(I)=0
        JF5(I)=0
        JF6(I)=0
      END DO
C
      XP=GRID(1)
      YP=GRID(2)
      AN=GRID(3)
      FI=GRID(4)
C
      RAD=3.1415927/180.
      VXR=(90.+FI)*RAD
      BETA=SIN(VXR)
      ALFA=COS(VXR)
      RADG=0.01*RAD
C
      FKON=100.*1852./3600.
C
      IX1=I1*100
      IX2=I2*100
      IY1=J1*100
      IY2=J2*100
c
      swap=swapfile(IFIL)
C
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     KRECXX=0
C     IRECXX=1
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
      READ(IFIL,REC=1,ERR=900,IOSTAT=IOS) NBO
      if(swap) call bswap2(1024,NBO)
      IHTYPE=NBO(1)
      IF(IHTYPE.EQ.7) GOTO 940
      IF(IHTYPE.LT.1 .OR. IHTYPE.GT.7) GOTO 456
      WRITE(6,*)'  HOVEDTYPEN SOM LESES FRA IDSI=',IFILE,' ER:',IHTYPE
      NINH=NBO(10)
      NRECI1=3
      NRECI2=NBO(9)
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     WRITE(6,*) ' NBO(1-12):'
C     WRITE(6,FMT='(12I6)') (NBO(I),I=1,12)
C     WRITE(6,*) 'NINH,NRECI1,NRECI2: ',NINH,NRECI1,NRECI2
C     NREAD=0
C     NCOPY=0
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C
C     LESER FRA OBS-FILER; EN HOVEDTYPE FOR HVER FIL
C
C        HOVEDTYPER:  1 - SYNOP     (SHIP: INSTRUMENT=10)
C                     2 - AIREPS
C                     3 - SATOB
C                     4 - DRIBU
C                     5 - TEMP
C                     6 - PILOT
C                     7 - SATEM
C
C
      ITYPE=IHTYPE
C
 
      LRECD2=0
      LWORD2=0
C
C
      DO 100 IRECI=NRECI1,NRECI2
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     WRITE(6,*) '   --- IRECI= ',IRECI
C     KRECXX=1
C     IRECXX=IRECI
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
      READ(IFIL,REC=IRECI,ERR=900,IOSTAT=IOS) INH
      if(swap) call bswap2(1024,INH)
      NINH=128
C
C     WRITE(6,*)'NY REC:',IRECI
C     WRITE(6,*)'NINH  :',NINH
CKHM
C
      DO 110 N=1,NINH
C
      IF(INH(6,N).LT.1) GOTO 110
C
      IHR=INH(5,N)/100
      IMN=INH(5,N)-IHR*100
C  LEGGER UT MINUTTER
      KLDIF=IHR*60+IMN-MINTERM
      IF(KLDIF.LT.-MIN12H) KLDIF=KLDIF+MINDAY
      IF(KLDIF.GT.+MIN12H) KLDIF=KLDIF-MINDAY
      IF(KLDIF.GT.ITIMD1(IHTYPE).OR.KLDIF.LT.ITIMD2(IHTYPE))GO TO 110
C
C
      GLAT=RADG*INH(3,N)
      GLON=RADG*INH(4,N)
      RR=AN*COS(GLAT)/(1. +SIN(GLAT))
      YR=-RR*COS(GLON)
      XR=+RR*SIN(GLON)
      IX=(XR*BETA-YR*ALFA+XP)*100.+0.5
      IY=(YR*BETA+XR*ALFA+YP)*100.+0.5
      IF(IX.LT.IX1 .OR. IX.GT.IX2 .OR.
     *   IY.LT.IY1 .OR. IY.GT.IY2) GOTO 110
C
      IRECD1=INH(6,N)
      IW0=   INH(7,N)-1
      IW2=   INH(8,N)+IW0
      IF(IW2.GT.MAXDAT) IW2=MAXDAT
      IRECD2=IRECD1-1+(IW2+LENREC-1)/LENREC
C
      L2=0
      IF(IRECD1.EQ.LRECD2) THEN
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     NCOPY=NCOPY+1
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
      IF(LWORD2.GT.0) THEN
      DO 130 L=1,LENREC
  130 IDATA(L)=IDATA(LWORD2+L)
      ENDIF
      IRECD1=IRECD1+1
      L2=LENREC
      ENDIF
      DO 140 IRECD=IRECD1,IRECD2
        L1=L2+1
        L2=L2+LENREC
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     NREAD=NREAD+1
C     KRECXX=2
C     IRECXX=IRECD
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
        READ(IFIL,REC=IRECD,ERR=900,IOSTAT=IOS) (IDATA(L),L=L1,L2)
        if(swap) call bswap2(1024,IDATA(L1))
  140 CONTINUE
      LRECD2=IRECD2
      LWORD2=L2-LENREC
C
      NLVL=1
      LLVL=9
C
      IF(IHTYPE.EQ.1) THEN
        ITYPE=11
        IF(IDATA(IW0+3).EQ.10) ITYPE=10
      ELSEIF(IHTYPE.EQ.5 .OR. IHTYPE.EQ.6) THEN
        NLVL=(IW2-IW0-20)/9
        LLVL=9
      ENDIF
C
      IWD1=IW0+21
      IWD2=IW0+20+LLVL*NLVL
C
      IF(IDATA(IW0+7).NE.INH(3,N) .OR.
     *   IDATA(IW0+8).NE.INH(4,N)) THEN
        WRITE(6,FMT='('' FEIL.  INH: '',8I6)') (INH(I,N),I=1,8)
        WRITE(6,FMT='(''       DATA: '',8I6)') (IDATA(I),I=1,8)
        GOTO 110
      ENDIF
C
      ID1=IDATA(IW0+4)
      ID2=IDATA(IW0+5)
      ID3=IDATA(IW0+6)
      ID11=IAND(ISHFT(ID1,-8),255)
      ID12=IAND(ID1,255)
      ID21=IAND(ISHFT(ID2,-8),255)
      ID22=IAND(ID2,255)
      ID31=IAND(ISHFT(ID3,-8),255)
      ID32=IAND(ID3,255)
      CID=CHAR(ID11)//CHAR(ID12)//CHAR(ID21)//CHAR(ID22)//
     *                            CHAR(ID31)//CHAR(ID32)
C
CKHM
C     WRITE(6,*)
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     WRITE(6,*)
C     WRITE(6,FMT='('' kjennetegn,pos: '',4X,''('',A6,2I6,'')    '')')
C    *CID,INH(3,N)
C    *,INH(4,N)
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
CKHM
C
      KL=IDATA(IW0+14)
C
      IF(NOBS+NLVL.GT.MAXOBS) GOTO 920
      IF(NSTA+1.GT.MAXSTA) GOTO 930
C
      NOBS1=NOBS+1
C
      INSTRU=IDATA(IW0+3)
C
C+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C..20.02.1997: skip Aireps/Andar (new aireps data crashed Hirlam)
      IF(IHTYPE.EQ.2 .AND. INSTRU.EQ.23) GOTO 110
C+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
C
C        DATADELEN LESES, INSTRUMENT HOVEDTYPE BRUKES
      DO 160 L=IWD1,IWD2,LLVL
CXX   sjekker ut trykk over 100 hPa
CXX   IF(IDATA(L).LT.1 .OR. IDATA(L).GT.11500) GOTO 160
      IF(IDATA(L).LT.1000 .OR. IDATA(L).GT.11500) GOTO 160
C
C..AF..BARE STANDARD-NIV$ FOR TEMP OG PILOT |||
C
      IF(ITYPE.EQ.5 .OR. ITYPE.EQ.6) THEN
        DO 161 K=1,11
          IF(IDATA(L).EQ.NLEV(K)) GOTO 162
  161   CONTINUE
        GOTO 160
  162   CONTINUE
      ENDIF
C
C****FLAGGUTPAKKING*******************
C        FLAG-KALLET KAN ERSTATTES AV BIT-FIKLING I KODEN
C        TRENGER JO BARE NOEN AV FLAGGENE
C     WRITE(6,FMT='('' data:'',6I8)')IP,IDD,IFF,IT,ITD,IZ
      IF7=IDATA(L+6)
      IF8=IDATA(L+7)
      IF9=IDATA(L+8)
CC    CALL FLAG(IF7,1,JF1)
CC    CALL FLAG(IF7,2,JF2)
CC    CALL FLAG(IF8,1,JF3)
CC    CALL FLAG(IF8,2,JF4)
CC    CALL FLAG(IF9,1,JF5)
CC    CALL FLAG(IF9,2,JF6)
C..unpack the flags that are used
      JF1(5)=IAND(ISHFT(IF7,-8),3)
      JF2(5)=IAND(IF7,3)
      JF3(5)=IAND(ISHFT(IF8,-8),3)
      JF4(5)=IAND(IF8,3)
      JF5(5)=IAND(ISHFT(IF9,-8),3)
      JF6(5)=IAND(IF9,3)
CC    JF6(3)=IAND(ISHFT(IF9,-3),3)
C*************************************
C   KUTTER UT HVIS KVALITETSFLAGGET ER 2 ELLER MER
      IP =IDATA(L)
      IDD=IDATA(L+1)
      IFF=IDATA(L+2)
      IT =IDATA(L+3)
      ITD=IDATA(L+4)
      IZ =IDATA(L+5)
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
c     IF(JF1(5).GT.1) WRITE(6,FMT='('' P  FLAGG>1: '',A6,8I6)')
c    *CID,IP,IDD,IFF,IT,ITD,IZ
c    *,IHTYPE,INSTRU
c     IF(JF2(5).GT.1) WRITE(6,FMT='('' DD FLAGG>1: '',A6,8I6)')
c    *CID,IP,IDD,IFF,IT,ITD,IZ
c    *,IHTYPE,INSTRU
c     IF(JF3(5).GT.1) WRITE(6,FMT='('' FF FLAGG>1: '',A6,8I6)')
c    *CID,IP,IDD,IFF,IT,ITD,IZ
c    *,IHTYPE,INSTRU
c     IF(JF4(5).GT.1) WRITE(6,FMT='('' T  FLAGG>1: '',A6,8I6)')
c    *CID,IP,IDD,IFF,IT,ITD,IZ
c    *,IHTYPE,INSTRU
c     IF(JF5(5).GT.1) WRITE(6,FMT='('' TD FLAGG>1: '',A6,8I6)')
c    *CID,IP,IDD,IFF,IT,ITD,IZ
c    *,IHTYPE,INSTRU
c     IF(JF6(5).GT.1) WRITE(6,FMT='('' Z  FLAGG>1: '',A6,8I6)')
c    *CID,IP,IDD,IFF,IT,ITD,IZ
c    *,IHTYPE,INSTRU
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
CXXX TRYKKFLAGGENE ER ANTAKELIG BARE S@PPEL, MEN DETTE B@R
CXXX SJEKKES SKIKKELIG P$ NORD AV TEKNISK DIVISJON
CXXX  IF(JF1(5).GT.1) IP =-32767
      IF(JF2(5).GT.1) IDD=-32767
      IF(JF3(5).GT.1) IFF=-32767
      IF(JF4(5).GT.1) IT =-32767
      IF(JF5(5).GT.1) ITD=-32767
      IF(JF6(5).GT.1) IZ =-32767
C     WRITE(6,FMT='('' data:'',6I8)')IP,IDD,IFF,IT,ITD,IZ
C|||| IF(IHTYPE.EQ.1.AND.JF6(3).GT.1.) TIDSKONTROLL GJORT FOR SYNOP
C|||| DETTE F@RES OVER I OVCRAY-PROGRAMMET
C|||| M$ VURDERE $ GJ@RE DET HER
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     IF(IHTYPE.EQ.2) WRITE(6,FMT='('' AIREPS '',2I7,4X,3I7)')
C    *                IDATA(IW0+7),IDATA(IW0+8),IP,IDD,IFF
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C
      ND=0
C
      IF(ITYPE.EQ.2 .OR. ITYPE.EQ.3) THEN
C..AIREPS OG SATOB(?): Z BARE IFLG. 'STANDARD-ATMOSF#RE'
        IZ=-32767
      ELSEIF(IZ.NE.-32767) THEN
        ND=1
      ENDIF
C
      IF(ITYPE.EQ.11 .OR. ITYPE.EQ.4) THEN
C..FJERNER VIND FRA LAND-SYNOP OG DRIBU
        IU=-32767
        IV=-32767
      ELSEIF(IDD.GT.0 .AND. IDD.LE.360 .AND. IFF.GT.0) THEN
        GAM=GLON-RAD*IDD-VXR
        IU=IFF*COS(GAM)*FKON
        IV=IFF*SIN(GAM)*FKON
        ND=1
      ELSEIF(IDD.EQ.0 .AND. IFF.EQ.0) THEN
        IU=0
        IV=0
        ND=1
      ELSE
        IU=-32767
        IV=-32767
      ENDIF
C
C..AF..FUKTIGHET BARE FRA TEMP'ER
      IF(ITYPE.NE.5) THEN
        IRH=-32767
CCC   ELSEIF(IT.LT.+999 .AND. IT.GE.ITD   .AND. ITD.GT.-999) THEN
      ELSEIF(IT.LT.+999 .AND. IT+6.GT.ITD .AND. ITD.GT.-999) THEN
        X=(IT*0.1+105.)*0.2
        LX=X
        ET=EWT(LX)+(EWT(LX+1)-EWT(LX))*(X-LX)
        X=(ITD*0.1+105.)*0.2
        LX=X
        ETD=EWT(LX)+(EWT(LX+1)-EWT(LX))*(X-LX)
        IRH=100.*ETD/ET +0.5
        ND=1
      ELSE
        IRH=-32767
      ENDIF
C
C..AF   IF(IHTYPE.EQ.1.)THEN
C        SETTER INN P=1000 for synoper med ok z
C..AF    IF(IZ.NE.-32767.AND.IP.GT.0) THEN
C..AF    IP=NLEV(1)
C..AF    ENDIF
C..AF   ENDIF
C
C..SETTER INN P=1000 FOR SYNOP OG B@YER HVIS Z ER OK
C.khm   IF(IHTYPE.EQ.1.) IP=NLEV(1)
        IF(IHTYPE.EQ.1.OR.IHTYPE.EQ.4) THEN
        IF(IZ.GT.-32000) IP=NLEV(1)
        ENDIF
C
C..AF   IF(IHTYPE.EQ.5.)THEN
C        TESTER ETTER STANDARDFLATER
C..AF    ISX=0
C..AF    DO 10 IS=1,11
C..AF    IF(IP.EQ.NLEV(IS)) ISX=1
C        WRITE(6,*)IP,NLEV(IS),ISX
C..10    CONTINUE
C..AF    IF(ISX.NE.1) ND=0
C..AF   ENDIF
C
      IF(ND.NE.0) THEN
C..AF   IF(IHTYPE.EQ.1.AND.INSTRU.NE.10)THEN
C..AF     IU=-32767
C..AF     IV=-32767
C..AF   ENDIF
C..AF   IF(IHTYPE.EQ.4.)THEN
C..AF     IU=-32767
C..AF     IV=-32767
C..AF   ENDIF
C..AF   IF(IP.LT.-32000) GO TO 830
C..AF   IF(IZ.LT.-32000.AND.IU.LT.-32000.AND.IV.LT.-32000) GO TO 830
        NOBS=NOBS+1
        IOBS(1,NOBS)=IP
        IOBS(2,NOBS)=IZ
        IOBS(3,NOBS)=IU
        IOBS(4,NOBS)=IV
        IOBS(5,NOBS)=IRH
        IOBS(6,NOBS)=IDATA(L+6)
        IOBS(7,NOBS)=IDATA(L+7)
        IOBS(8,NOBS)=IDATA(L+8)
C
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C       WRITE(99,FMT='(1X,''T'',I3,2I7,4X,5I7)')
C    *                    ITYPE,IX,IY,IP,IZ,IU,IV,IRH
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C.830   CONTINUE
C
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     IF(JF1(5).GT.1.OR.
C    *JF2(5).GT.1.OR.
C    *JF3(5).GT.1.OR.
C    *JF4(5).GT.1.OR.
C    *JF5(5).GT.1) THEN
C     WRITE(6,FMT='('' FLAGGORD SOM INTEGER:'',3I12)')IF7,IF8,IF9
C     WRITE(6,FMT='('' FLAGG: '',5I8,'' p dd ff t td z'')')JF1
C     WRITE(6,FMT='('' FLAGG: '',5I8,'' p dd ff t td z'')')JF2
C     WRITE(6,FMT='('' FLAGG: '',5I8,'' p dd ff t td z'')')JF3
C     WRITE(6,FMT='('' FLAGG: '',5I8,'' p dd ff t td z'')')JF4
C     WRITE(6,FMT='('' FLAGG: '',5I8,'' p dd ff t td z'')')JF5
C     WRITE(6,FMT='('' FLAGG: '',5I8,'' p dd ff t td z'')')JF6
C     ENDIF
C     WRITE(6,FMT='(''    OBS: '',5I7)') (IOBS(I,NOBS),I=1,5)
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
      ENDIF
C
  160 CONTINUE
C
      IF(NOBS.LT.NOBS1) GOTO 110
C
      NSTA=NSTA+1
      ISTA( 1,NSTA)=IHTYPE
      ISTA( 2,NSTA)=IDATA(IW0+3)
      ISTA( 3,NSTA)=KL
      ISTA( 4,NSTA)=KLDIF
      ISTA( 5,NSTA)=1+NOBS-NOBS1
      ISTA( 6,NSTA)=0
      ISTA( 7,NSTA)=0
      ISTA( 8,NSTA)=IDATA(IW0+4)
      ISTA( 9,NSTA)=IDATA(IW0+5)
      ISTA(10,NSTA)=IDATA(IW0+6)
      ISTA(11,NSTA)=INH(3,N)
      ISTA(12,NSTA)=INH(4,N)
      ISTA(13,NSTA)=IX
      ISTA(14,NSTA)=IY
      ISTA(15,NSTA)=0
      ISTA(16,NSTA)=0
      CSTA(NSTA)=CID
      NTYPE(ITYPE)=NTYPE(ITYPE)+1
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
c     WRITE(6,*)
c     WRITE(6,FMT='('' -- STA: '',6I7,/,1X,10I7,/,4X,A6)')
c    *(ISTA(I,NSTA),I=1,16),CID
c     DO 444 NX=NOBS1,NOBS
c     WRITE(6,FMT='(''    OBS: '',5I7)') (IOBS(I,NX),I=1,5)
c 444 CONTINUE
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C
  110 CONTINUE
C
  100 CONTINUE
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     WRITE(6,*) ' NREAD,NCOPY:   ',NREAD,NCOPY
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C
      GOTO 950
C
  900 WRITE(6,*) ' **** READ ERROR.    OBS-FILE: ',IFIL
C   READ ERROR : RETURNERER MED FEILMELDING
      IREADERR=1
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
C     WRITE(6,*) ' KRECXX,IRECXX: ',KRECXX,IRECXX
C     WRITE(6,*) ' NREAD,NCOPY:   ',NREAD,NCOPY
C›››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››››
      WRITE(6,*)
      GOTO 950
  920 WRITE(6,*) ' **** FOR MANGE OBSERVASJONER ("MAXOBS")'
      WRITE(6,*) ' LEGGER UT DET ANTALL DET ER PLASS TIL'
      WRITE(6,*) ' SLIK AT ANALYSEN KAN KJ@RES          '
      IMAXUT=1
      CLOSE(IFIL)
      WRITE(6,*)
      GOTO 950
  930 WRITE(6,*) ' **** FOR MANGE STASJONER ("MAXSTA")'
      WRITE(6,*) ' LEGGER UT DET ANTALL DET ER PLASS TIL'
      WRITE(6,*) ' SLIK AT ANALYSEN KAN KJ@RES          '
      IMAXUT=1
      CLOSE(IFIL)
      WRITE(6,*)
      GOTO 950
  940 WRITE(6,*) ' **** SATEM LESES IKKE, PROGRAMMET KAN IKKE'
      WRITE(6,*)
CCC   GOTO 950
C
  950 NTYPE(1)=NTYPE(10)+NTYPE(11)
      WRITE(6,*)'    FERDIG MED $ LESE HOVEDTYPE:',IHTYPE,' FRA OBSFIL'
      WRITE(6,*)'    DU HAR N$ ',NSTA,' OBSERVASJONER'
      WRITE(6,*)'    OG DU HAR ',NOBS,' DATASETT '
      WRITE(6,*)
      CLOSE(IFIL)
      RETURN
C
  456 CONTINUE
      WRITE(6,*)'    FEIL HOVEDTYPE'
      WRITE(6,*)
      RETURN
      END
