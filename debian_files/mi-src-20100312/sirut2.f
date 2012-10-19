C*********************************************************************
C**
C**      subroutines for SI - statistical interpolation
C**      version:  DNMI/FoU  November 1991
C**      update:   DNMI/FoU  23.03.1994  Anstein Foss
C**      update:   DNMI/FoU  26.06.1995  Anstein Foss
C**
C*********************************************************************
C**
C**  SUBRUTINER: HOBS
C**              INTERP
C**              MAPPAR
C**              FELTIN
C**              FELTUT
C**              BLXY1
C**              MVMISS
C**
C**********************************************************************
C**
C**   NB| INPUT/OUTPUT AV FELT FRA/TIL 'FELT-FILE' MED RFELT/WFELT
C**
C**********************************************************************
C
C
      SUBROUTINE HOBS(NOBS,NROBS,NSOBS,XOBS,YOBS,
     *                    XYLIM,NPEK,NRPEK,NSPEK,IPEK)
C
C=======================================================================
C
C   PLUKKER UT OBSERVASJONER SOM SKAL VÎRE MED I NESTE ANALYSE
C   INNENFOR BEGRENSNINGENE (I1,I2)*(J1,J2)
C
C=======================================================================
C
C   VARIABLE UT:
C   NRPEK               - ANTALL R/S-OBSERVASJONER
C   NSPEK               - ANTALL SATELLITTOBSERVASJONER
C   IPEK(1:NRPEK+NSPEK) - PEKERE TIL ARRAY MED OBSERVASJONER
C
C=======================================================================
C
      INTEGER NOBS,NROBS,NSOBS,NPEK,NRPEK,NSPEK
      INTEGER IPEK(NOBS)
      REAL    XOBS(NOBS),YOBS(NOBS),XYLIM(4)
C
      X1 = XYLIM(1)
      X2 = XYLIM(2)
      Y1 = XYLIM(3)
      Y2 = XYLIM(4)
      NRPEK = 0
      NSPEK = 0
C
      K = 0
      DO 10 I = 1,NROBS
         IF ( XOBS(I) .GT. X1 .AND. XOBS(I) .LT. X2 .AND.
     +        YOBS(I) .GT. Y1 .AND. YOBS(I) .LT. Y2 ) THEN
            K = K + 1
            IPEK(K) = I
         ENDIF
10    CONTINUE
      NRPEK = K
C
      DO 20 I = NROBS+1,NROBS+NSOBS
         IF ( XOBS(I) .GT. X1 .AND. XOBS(I) .LT. X2 .AND.
     +        YOBS(I) .GT. Y1 .AND. YOBS(I) .LT. Y2 ) THEN
            K = K + 1
            IPEK(K) = I
         ENDIF
20    CONTINUE
      NSPEK = K - NRPEK
      NPEK = K
C
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE INTERP(FELT,NX,NY,NOBS,XOBS,YOBS,OBSINT)
C
      INTEGER NX,NY,NOBS
      REAL    FELT(NX,NY),XOBS(NOBS),YOBS(NOBS),OBSINT(NOBS)
C
C
      WRITE(6,*)
      WRITE(6,*)' inerpolate  field ---> obspoints  '
      WRITE(6,*)
C
      IOBS = NOBS
      II   = NX
      JJ   = NY
      IIM1 = II - 1
      JJM1 = JJ - 1
C
C        INTERPOLASJON
C
      DO 100 N=1,IOBS
        X=XOBS(N)
        Y=YOBS(N)
        I=X
        J=Y
        IF(I.GT.2 .AND. I.LT.IIM1 .AND. J.GT.2 .AND. J.LT.JJM1) THEN
C..BESSEL
          X1=X-I
          X2=1.-X1
          X3=-0.25*X1*X2
          X4=-0.1666667*X1*X2*(X1-0.5)
          X5=X3-X4
          X6=X2-X3+3.*X4
          X7=X1-X3-3.*X4
          X8=X3+X4
          Y1=Y-J
          Y2=1.-Y1
          Y3=-0.25*Y1*Y2
          Y4=-0.1666667*Y1*Y2*(Y1-0.5)
          Y5=Y3-Y4
          Y6=Y2-Y3+3.*Y4
          Y7=Y1-Y3-3.*Y4
          Y8=Y3+Y4
          T1= Y5*FELT(I-1,J-1)+Y6*FELT(I-1,J)
     *       +Y7*FELT(I-1,J+1)+Y8*FELT(I-1,J+2)
          T2= Y5*FELT(I,  J-1)+Y6*FELT(I,  J)
     *       +Y7*FELT(I,  J+1)+Y8*FELT(I,  J+2)
          T3= Y5*FELT(I+1,J-1)+Y6*FELT(I+1,J)
     *       +Y7*FELT(I+1,J+1)+Y8*FELT(I+1,J+2)
          T4= Y5*FELT(I+2,J-1)+Y6*FELT(I+2,J)
     *       +Y7*FELT(I+2,J+1)+Y8*FELT(I+2,J+2)
          OBSINT(N)=X5*T1+X6*T2+X7*T3+X8*T4
C
        ELSE
C..BILINE#R
          Y1=Y-J
          X1=X-I
          X5=1.-X1-Y1+X1*Y1
          X6=X1-X1*Y1
          X7=Y1-X1*Y1
          X8=X1*Y1
          OBSINT(N)= X5*FELT(I,J)+X6*FELT(I+1,J)
     *              +X7*FELT(I,J+1)+X8*FELT(I+1,J+1)
        ENDIF
  100 CONTINUE
C
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE MAPPAR(PAR,GRID,NX,NY,FELT)
C
C       PAR:          'M' = map factor
C                     'F' = coriolis parameter
C       GRID(4):      grid definition: XP,YP,AN,FI
C       NX:           field x dimension
C       NY:           field y dimension
C       FELT(NX,NY):  output field
C
C
C
      CHARACTER*1 PAR
      INTEGER NX,NY
      REAL    GRID(4),FELT(NX,NY)
C
      XP=GRID(1)
      YP=GRID(2)
      AN=GRID(3)
      FI=GRID(4)
C
      IF(PAR.EQ.'M' .OR. PAR.EQ.'m') THEN
C..Map factor
        AN2=AN*AN
        XM1=0.933/AN2
        DO 10 J=1,NY
        DO 10 I=1,NX
          RPOL=(I-XP)*(I-XP)+(J-YP)*(J-YP)
          FELT(I,J)=XM1*(AN2+RPOL)
  10    CONTINUE
C
      ELSEIF(PAR.EQ.'F' .OR. PAR.EQ.'f') THEN
C..Coriolis parameter
        AN2=AN*AN
        FP=1.458E-4
        DO 20 J=1,NY
        DO 20 I=1,NX
          RPOL=(I-XP)*(I-XP)+(J-YP)*(J-YP)
          FELT(I,J)=FP*(AN2-RPOL)/(AN2+RPOL)
  20    CONTINUE
C
      ELSE
        WRITE(6,*) '***MAPPAR*** UNKNOWN PARAMETER: ',PAR
        STOP 117
      ENDIF
C
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE FELTIN(FILNAM,IUNIT,IN,LDATA,IDATA,CODE,LFELT,FELT,
     *                                          NX,NY,GRID,ITIME,OK)
C
C       Read field from FELT file.
C
C       code: '=' field = input_field
C             '+' field = field + input_field
C             '-' field = field - input_field
C
      INTEGER   IUNIT,LDATA,LFELT,NX,NY,ITIME(5)
      INTEGER*2 IN(16),IDATA(LDATA)
      REAL      FELT(LFELT),GRID(4)
      CHARACTER*(*) FILNAM
      CHARACTER*(*) CODE
      LOGICAL OK
C
      integer   ierror,igtype
      real      gparam(6),felt2(lfelt)
C
      OK=.FALSE.
C
      WRITE(6,*) ' -----  FELTIN - READ FELT  -----'
C
      LCODE=LEN(CODE)
      DO 10 K=LCODE,1,-1
        IF(CODE(K:K).NE.' ') GOTO 11
   10 CONTINUE
      K=1
   11 ICODE=-1
      IF(K.EQ.1 .AND. CODE(1:1).EQ.'=') ICODE=0
      IF(K.EQ.1 .AND. CODE(1:1).EQ.'+') ICODE=1
      IF(K.EQ.1 .AND. CODE(1:1).EQ.'-') ICODE=2
      IF(ICODE.LT.0) THEN
        WRITE(6,*) '*** Unknown code: ',CODE
        GOTO 990
      ENDIF
C
      call mrfelt(0,filnam,iunit,in,2,lfelt,felt2,1.0,
     *            ldata,idata,ierror)
c
      if(ierror.ne.0) goto 900
c
      call gridpar(+1,ldata,idata,igtype,ix,iy,gparam,ierror)
c
      if(ierror.ne.0) write(6,*) 'FELTIN: GRIDPAR ERROR !!!'
      if(ierror.ne.0) goto 900
c
      if(igtype.ne.1) then
	write(6,*) 'ERROR: INPUT FIELD HAS GRIDTYPE= ',igtype
	write(6,*) '       ONLY GRIDTYPE=1 (POLARSTEREOGRAPHIC) OK'
	goto 900
      end if
C
      IF(ICODE.EQ.0) THEN
        NX=IX
        NY=IY
        GRID(1)=gparam(1)
        GRID(2)=gparam(2)
        GRID(3)=gparam(3)
        GRID(4)=gparam(4)
      ELSEIF(IX.NE.NX .OR. IY.NE.NY .OR.
     *       gparam(1).NE.GRID(1) .OR. gparam(2).NE.GRID(2) .OR.
     *       gparam(3).NE.GRID(3) .OR. gparam(4).NE.GRID(4)) THEN
        WRITE(6,*) '*** Code: ',CODE
        WRITE(6,*) '*** Existing field:'
        WRITE(6,*) '*** NX,NY:       ',NX,NY
        WRITE(6,*) '*** XP,YP,AN,FI: ',(GRID(I),I=1,4)
        WRITE(6,*) '*** Input field:'
        WRITE(6,*) '*** NX,NY:       ',IX,IY
        WRITE(6,*) '*** XP,YP,AN,FI: ',(gparam(I),I=1,4)
        WRITE(6,*) '*** Grid no:     ',IDATA(1)
        WRITE(6,*) '*** Can''t combine these fields'
        GOTO 990
      ENDIF
C
      NXY=NX*NY
C
      IF(NXY.GT.LFELT) GOTO 930
C
      ITIME(1)=IDATA(12)
      ITIME(2)=IDATA(13)/100
      ITIME(3)=IDATA(13)-(IDATA(13)/100)*100
      ITIME(4)=IDATA(14)/100
      ITIME(5)=IDATA(4)
C
      IF(ICODE.EQ.0) THEN
        DO 50 I=1,NXY
   50   FELT(I)=FELT2(I)
      ELSEIF(ICODE.EQ.1) THEN
        DO 51 I=1,NXY
   51   FELT(I)=FELT(I)+FELT2(I)
      ELSEIF(ICODE.EQ.2) THEN
        DO 52 I=1,NXY
   52   FELT(I)=FELT(I)-FELT2(I)
      ELSE
        WRITE(6,*) '**** Program error in FELTIN ****'
        STOP 8888
      ENDIF
C
      CLOSE(IUNIT)
      OK=.TRUE.
      GOTO 990
C
  900 WRITE(6,*) ' *********************'
      WRITE(6,*) ' ******* ERROR *******'
      WRITE(6,*) ' *********************'
      GOTO 990
  930 WRITE(6,*) ' INPUT.  NX,NY,NX*NY: ',NX,NY,NXY
      WRITE(6,*) '          MAX(NX*NY): ',LFELT
CCC   GOTO 990
C
  990 CONTINUE
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE FELTUT(FILNAM,IUNIT,IN,LDATA,IDATA,NX,NY,FELT,ISKAL,
     *                             IZLIM,FLTDEF,ZLIMIT,GRID,ITIME,OK)
C
C       Write field to FELT file.
C
      INTEGER   IUNIT,LDATA,NX,NY,ISKAL,IZLIM,ITIME(5)
      INTEGER*2 IN(16),IDATA(LDATA)
      REAL      FELT(NX*NY),FLTDEF(NX*NY),ZLIMIT,GRID(4)
      CHARACTER*(*) FILNAM
      LOGICAL OK
C
      integer   ierror,igtype
      real      undef,gparam(6),FELT2(NX*NY)
C
      OK=.FALSE.
C
      WRITE(6,*) ' -----  FELTUT - WRITE FELT  -----'
C
      NXY=NX*NY
C
      IDATA(1)=IN( 1)
      IDATA(2)=IN( 2)
      IDATA(3)=IN( 9)
      IDATA(4)=IN(10)
      IDATA(5)=IN(11)
      IDATA(6)=IN(12)
      IDATA(7)=IN(13)
      IDATA(8)=IN(14)
c
      IDATA(12)=IN(3)
      IDATA(13)=IN(4)
      IDATA(14)=IN(5)
c
CC    IDATA(12)=ITIME(1)
CC    IDATA(13)=ITIME(2)*100+ITIME(3)
CC    IDATA(14)=ITIME(4)*100
c
      IDATA(19)=0
      IDATA(20)=ISKAL
c
      igtype=1
      gparam(1)=GRID(1)
      gparam(2)=GRID(2)
      gparam(3)=GRID(3)
      gparam(4)=GRID(4)
      gparam(5)=0.0
      gparam(6)=0.0
c
      call gridpar(-1,ldata,idata,igtype,nx,ny,gparam,ierror)
c
      if(ierror.ne.0) write(6,*) 'FELTUT: GRIDPAR ERROR !!!'
      if(ierror.ne.0) goto 990
C
      DO 50 I=1,NXY
	 FELT2(I)= FELT(I)
   50 CONTINUE
C
      undef= +1.e+35
C
C**********************************************************************
C-JEH
C
      IF(IZLIM.NE.0) THEN
C       WRITE(6,*) ' '
C       WRITE(6,*) '**************************************************'
C       WRITE(6,*) '*'
C       WRITE(6,*) '* FROM FELTUT :'
C       WRITE(6,*) '* MISSING VALUE INSERTED FOR FLTDEF(I,J) > ',ZLIMIT
C       WRITE(6,*) '*'
C       WRITE(6,*) '**************************************************'
C       WRITE(6,*) ' '
C
C          INSERT UNDEFINED VALUE -32767 ACCORDING TO FLTDEF-VALUE
        DO 60 I=1,NXY
           IF (FLTDEF(I).GT.ZLIMIT) FELT2(I) = UNDEF
   60   CONTINUE
      ENDIF
C
C-JEH
C**********************************************************************
C
      call mwfelt(0,filnam,iunit,2,nx*ny,felt2,1.0,
     *            ldata,idata,ierror)
C
      if(ierror.ne.0) goto 990
C
      OK=.TRUE.
      GOTO 990
C
  990 CONTINUE
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE BLXY1(NOBS,NROBS,NSOBS,XOBS,YOBS,DOBS,EOBS,
     *                                     GRID,ICLIP,XYLIM)
C
C        POSISJONER FRA B,L TIL X,Y
C        OG FJERNER OBSERVASJONER UTENFOR GRIDET
C
C
      INTEGER NOBS,NROBS,NSOBS,ICLIP
      REAL    XOBS(NOBS),YOBS(NOBS),DOBS(NOBS),EOBS(NOBS)
      REAL    GRID(4),XYLIM(4)
C
      IOBS=NROBS+NSOBS
      IROBS=NROBS
      ISOBS=NSOBS
C
      CALL GCTOXY(IOBS,XOBS,YOBS,GRID)
C
C        FJERNER POSISJONER UTENFOR
C
      IF(ICLIP.NE.0) THEN
        X1=XYLIM(1)
        X2=XYLIM(2)
        Y1=XYLIM(3)
        Y2=XYLIM(4)
        K=0
        DO 120 N=1,IROBS
          IF(XOBS(N).LE.X1 .AND. XOBS(N).GE.X2 .AND.
     *       YOBS(N).LE.Y1 .AND. YOBS(N).GE.Y2) THEN
            K=K+1
            XOBS(K)=XOBS(N)
            YOBS(K)=YOBS(N)
            DOBS(K)=DOBS(N)
            EOBS(K)=EOBS(N)
          ENDIF
  120   CONTINUE
        NROBS=K
        DO 130 N=IROBS+1,IOBS
          IF(XOBS(N).LE.X1 .AND. XOBS(N).GE.X2 .AND.
     *       YOBS(N).LE.Y1 .AND. YOBS(N).GE.Y2) THEN
            K=K+1
            XOBS(K)=XOBS(N)
            YOBS(K)=YOBS(N)
            DOBS(K)=DOBS(N)
            EOBS(K)=EOBS(N)
          ENDIF
  130   CONTINUE
        NSOBS=K-NROBS
        NOBS=K
      ENDIF
C
      WRITE(6,*)
      WRITE(6,*) ' ANTALL OBSERVASJONER INPUT:          ',IROBS,ISOBS
      WRITE(6,*) ' ANTALL OBSERVASJONER INNENFOR GRIDET:',NROBS,NSOBS
      WRITE(6,*)
C
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE MVMISS(MOBS,MPAR,OBSDAT,UNDEF,NPAR,NOBSIN,NOBS)
C
C       Move positions with missing (undefined) values in
C       parameter NPAR to bottom of 'arrays'
C
C       UNDEF - must be a positive, high value (i.e. +1.E+35)
C
      INTEGER MOBS,MPAR,NPAR,NOBSIN,NOBS
      REAL    OBSDAT(MOBS,MPAR),UNDEF
C
      NOBS=0
      IF(NPAR.LT.1 .OR. NPAR.GT.MPAR) RETURN
C
      UDEF=0.9*UNDEF
      NBOT=NOBSIN
C
      DO 10 N=1,NOBSIN
C
        IF(OBSDAT(N,NPAR).GE.UDEF) THEN
C..undefined value here ... search from bottom for existing value
          DO 20 I=NBOT,N+1,-1
            IF(OBSDAT(I,NPAR).LT.UDEF) GOTO 25
   20     CONTINUE
C..no more existing values
          GOTO 90
C..swap data
   25     NBOT=I
          DO 30 I=1,MPAR
            SWAP=OBSDAT(N,I)
            OBSDAT(N,I)=OBSDAT(NBOT,I)
            OBSDAT(NBOT,I)=SWAP
   30     CONTINUE
          NBOT=NBOT-1
        ENDIF
   10 CONTINUE
      N=NOBSIN+1
C
   90 NOBS=N-1
C
      RETURN
      END
