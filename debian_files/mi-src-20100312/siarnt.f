      SUBROUTINE ARNT(IMIN,IMAX,JMIN,JMAX,NRPEK,NSPEK,NPEK,IPEK,
     +                   PII,RP,RO,ICF,NOBS,XOBS,YOBS,DOBS,EOBS,
     +                            DBESSL,D,II,JJ,XM,FELT,ESTVAR)
C
C=======================================================================
C
C   STATISTISK INTERPOLASJON AV AVVIK MELLOM OBSERVASJONER OG FELT
C
C=======================================================================
C
C   Variable inn :
C
C   IMIN,IMAX,JMIN,JMAX      - NEDRE OG ×VRE GRIDKOORDINATER
C                              FOR INTERPOLASJON
C   NR,NS                    - ANTALL R/S- OG SATELLITTOBSERVASJONER
C   NPEK                     - ANTALL OBS. SOM SKAL ANALYSERES
C   IPEK(1:NPEK)             - PEKER TIL FELT MED OBSERVASJONER
C   PII                      - STANDARDAVVIK TIL PROGNOSEFEIL
C   DBESSL                   - DIST. MELLOM PKT. I BESSEL-TABELL I KM
C   IMAP  = 0                - IKKE MODIFIKASJON AV GRIDAVSTAND
C         = 1                - GRIDAVSTAND MODIFISERT MED XM(I,J) FRA
C                              COMMON/MAP/ F×R BEREGNING AV KORRELASJON
C   II,JJ                    - FELT-DIMMENSJON
C   FELT(II,JJ)             - FELT F×R INTERPOLASJON
C
C
C   I COMMON/OB/
C   NROBS                    - ANTALL OBSERVASJONER MED UKORR. FEIL
C   NSOBS                    - ANTALL OBSERVASJONER MED KORR. FEIL
C   XOBS(I),YOBS(I),I=1,NROBS- POSISJON TIL OBSERVASJONER MED
C                              UKORRELERT OBS.FEIL
C   XOBS(I),YOBS(I),I=NROBS+1,NROBS+NSOBS- POSISJON TIL OBSERVASJONER ME
C                              KORRELERT OBS.FEIL
C   DOBS(I),I=1,NT           - AVVIK MELLOM OBSERVASJON OG FELT
C   EOBS(I),I=1,NT           - STANDARDAVVIK TIL OBS.FEIL
C
C   VARIABLE UT :
C
C   FELT(II,JJ)             - FELT ETTER INTERPOLASJON
C
C
C   INTERNE VARIABLE :
C
C   NO                       - MAKS ANTALL OBS I DENNE ANALYSEN
C   MO                       - MAKS ANTALL OBS TOTALT
C   M(NO,NO)                 - P + D - MATRISE
C   PI(NO)
C   W(NO)                    - BEREGNEDE VEKTER
C
C   DATA FOR DEFINING THE GAUSS CORRELATION FUNCTION
C   RP - "INFLUENCE" RADIUS IN KILOMETERS
C   RO - GAUSS  "INFLUENCE" RADIUS IN KILOMETERS FOR OBS ERROR ( SAT )
C=======================================================================
C
C
      INTEGER IMIN,IMAX,JMIN,JMAX,NRPEK,NSPEK,NPEK,ICF,NOBS,II,JJ
      INTEGER IPEK(NPEK)
      REAL    PII,RP,RO,DBESSL,D
      REAL    XOBS(NOBS),YOBS(NOBS),DOBS(NOBS),EOBS(NOBS)
      REAL    FELT(II,JJ), XM(II,JJ), ESTVAR(II,JJ)
C
C
      PARAMETER (NO=99)
      INTEGER INDEX(NO)
      REAL   DIST(NO),RESULT(NO)
      DOUBLE PRECISION M(NO,NO),PI(NO), W(NO)
      DOUBLE PRECISION EPS,DET
C
      PARAMETER (ND=200)
      COMMON/BESS/SZZ(ND),SLON(ND),STRA(ND)
C
C
C=======================================================================
C     C = skaleringsfaktor for TOAR-funksjonen
C
C     C = 2.65/R   ==>   TOARH(C,R) = 0.5
C
      C = 2.65/RP
C
C=======================================================================
C
      DX=DBESSL
      DG=D*0.001
C
C
C
C   SKAL ALLE OBSERVASJONER ANALYSERES ?
C   I DE TILFELLET SETTES PEKER-FELTET OPP HER
CCC   NPEK = NRPEK + NSPEK
CCC   NOBS = NROBS + NSOBS
CCC   IF ( NPEK .EQ. NOBS ) THEN
CCC      DO 10 I = 1,NPEK
C10      IPEK(I) = I
CCC   ENDIF
C
      IF ( NPEK .GT. NO ) THEN
         WRITE(6,*)' '
         WRITE(6,*)'NPEK.GT.NO I ARNT'
         WRITE(6,*)'NPEK,NO = ',NPEK,NO
         RETURN
      ENDIF
C
C=======================================================================
C
C   BEREGNER VERDIER I M-MATRISE
C
C
      PII2 = PII*PII
      DO 60 I = 1,NRPEK
         IX = XOBS(IPEK(I))
         IY = YOBS(IPEK(I))
         DFAC = DG/XM(IX,IY)
         M(I,I) = PII2 + EOBS(IPEK(I))*EOBS(IPEK(I))
         DO 40 J = I+1,NRPEK
            XD = XOBS(IPEK(I)) - XOBS(IPEK(J))
            YD = YOBS(IPEK(I)) - YOBS(IPEK(J))
            DIST(J) = SQRT( XD**2 + YD**2 ) * DFAC
40       CONTINUE
         IF ( ICF .EQ. 1 ) THEN
            DO 41 J = I+1,NRPEK
               RBES=DIST(J)/DX
               RBES=RBES+1.
               IRBES=RBES
               PIJ =SZZ(IRBES)+(SZZ(IRBES+1)-SZZ(IRBES))*(RBES-IRBES)
               M(I,J) = PIJ*PII2
41         CONTINUE
         ELSEIF ( ICF .EQ. 2 ) THEN
            DO 42 J = I+1,NRPEK
               PIJ = EXP(-.5 * (DIST(J)/RP)**2)
               M(I,J) = PIJ*PII2
42         CONTINUE
         ELSE
            CALL TOARHV(C,NRPEK-I,DIST(I+1),RESULT(I+1))
            DO 43 J = I+1,NRPEK
               PIJ = RESULT(J)
C->>           PIJ = TOARH(C,DIST(J))
               M(I,J) = PIJ*PII2
43         CONTINUE
         ENDIF
C
         DO 50 J = NRPEK+1,NPEK
            DIST(J) = SQRT( (XOBS(IPEK(I))-XOBS(IPEK(J)))**2
     +                    + (YOBS(IPEK(I))-YOBS(IPEK(J)))**2 ) * DFAC
50       CONTINUE
         IF ( ICF .EQ. 1 ) THEN
            DO 51 J = NRPEK+1,NPEK
               RBES=DIST(J)/DX
               RBES=RBES+1.
               IRBES=RBES
               PIJ =SZZ(IRBES)+(SZZ(IRBES+1)-SZZ(IRBES))*(RBES-IRBES)
               M(I,J) = PIJ*PII2
51          CONTINUE
         ELSEIF ( ICF .EQ. 2 ) THEN
            DO 52 J = NRPEK+1,NPEK
               PIJ = EXP(-.5 * (DIST(J)/RP)**2)
               M(I,J) = PIJ*PII2
52          CONTINUE
         ELSE
            CALL TOARHV(C,NPEK-NRPEK,DIST(NRPEK+1),RESULT(NRPEK+1))
            DO 53 J = NRPEK+1,NPEK
               PIJ = RESULT(J)
C->>           PIJ = TOARH(C,DIST(J))
               M(I,J) = PIJ*PII2
53          CONTINUE
         ENDIF
C
60    CONTINUE
C
      DO 80 I = NRPEK+1,NPEK
         IX = XOBS(IPEK(I))
         IY = YOBS(IPEK(I))
         DFAC = DG/XM(IX,IY)
         M(I,I) = PII2 + EOBS(IPEK(I))*EOBS(IPEK(I))
         DO 70 J = I+1,NPEK
            DIST(J) = SQRT( (XOBS(IPEK(I))-XOBS(IPEK(J)))**2
     +                    + (YOBS(IPEK(I))-YOBS(IPEK(J)))**2 ) * DFAC
70       CONTINUE
         IF ( ICF .EQ. 1 ) THEN
            DO 71 J = I+1,NPEK
               RBES=DIST(J)/DX
               RBES=RBES+1.
               IRBES=RBES
               PIJ =SZZ(IRBES)+(SZZ(IRBES+1)-SZZ(IRBES))*(RBES-IRBES)
               M(I,J) = PIJ*PII2
71          CONTINUE
         ELSEIF ( ICF .EQ. 2 ) THEN
            DO 72 J = I+1,NPEK
               PIJ = EXP(-.5 * (DIST(J)/RP)**2)
               M(I,J) = PIJ*PII2
72          CONTINUE
         ELSE
            CALL TOARHV(C,NPEK-I,DIST(I+1),RESULT(I+1))
            DO 73 J = I+1,NPEK
               PIJ = RESULT(J)
C->>           PIJ = TOARH(C,DIST(J))
               M(I,J) = PIJ*PII2
73          CONTINUE
         ENDIF
         DO 79 J = I+1,NPEK
            DIJ = EXP(-.5 * (DIST(J)/RO)**2)
            M(I,J) = M(I,J) + DIJ*EOBS(IPEK(I))*EOBS(IPEK(J))
79       CONTINUE
80    CONTINUE
C
C   M(I,J) ER SYMMETRISK
C
      DO 90 I = 2,NPEK
      DO 90 J = 1,I-1
         M(I,J) = M(J,I)
90    CONTINUE
C
C
C   INVERTERER M(I,J)
C
C.... WRITE(6,*) 'MATINV : MAXdim,  dim = ',NO,' ',NPEK
C
      CALL MATINV(M,NO,NPEK,INDEX,IERR,DET)
C
C...  WRITE(6, *)'ETTER MATIN2: DET = ',DET,' IERR = ',IERR
C
CC    IF(IERR.NE.0 .OR. DET.GT.1.E+60)
CC   *  WRITE(6,FMT='(''  << MATINV >> err,det,npek,i1,i2,j1,j2: '',
CC   *          I3,E11.3,I3,2X,4I4)') IERR,DET,NPEK,IMIN,IMAX,JMIN,JMAX
C
      IF(IERR.NE.0)
     *  WRITE(6,FMT='(''  << MATINV >> err,npek,i1,i2,j1,j2: '',
     *                2I4,2X,4I4)') IERR,NPEK,IMIN,IMAX,JMIN,JMAX
C
C
C
C=======================================================================
C   L×KKE FOR HVERT ANALYSE-PUNKT
C
      DO 500 IA = IMIN,IMAX
      DO 500 JA = JMIN,JMAX
         DFAC = DG/XM(IA,JA)
C
C   BEREGNE PI(NT)
C
         DO 200 I = 1,NPEK
            XD = XOBS(IPEK(I)) - IA
            YD = YOBS(IPEK(I)) - JA
            DIST(I) = SQRT( XD**2 + YD**2 ) * DFAC
200      CONTINUE
         IF ( ICF .EQ. 1 ) THEN
            DO 201 I = 1,NPEK
               RBES=DIST(I)/DX
               RBES=RBES+1.
               IRBES=RBES
               PIJ =SZZ(IRBES)+(SZZ(IRBES+1)-SZZ(IRBES))*(RBES-IRBES)
               PI(I) = PIJ*PII2
201         CONTINUE
         ELSEIF ( ICF .EQ. 2 ) THEN
            DO 202 I = 1,NPEK
               PIJ = EXP(-.5 * (DIST(I)/RP)**2)
               PI(I) = PIJ*PII2
202         CONTINUE
         ELSE
            CALL TOARHV(C,NPEK,DIST(1),RESULT(1))
            DO 203 I = 1,NPEK
               PIJ = RESULT(I)
C->>           PIJ = TOARH(C,DIST(I))
               PI(I) = PIJ*PII2
203         CONTINUE
         ENDIF
C
C   BEREGNE VEKTENE W(NPEK)
C-->> WSUM = 0.
      DO 220 I = 1,NPEK
         W(I) = 0.
         DO 210 J = 1,NPEK
           W(I) = W(I) + PI(J)*M(I,J)
210      CONTINUE
C-->>    WSUM = WSUM + W(I)
220   CONTINUE
C
C   BEREGNER NY ANALYSEVERDI OG ESTIMERINGSVARIANS
      EST = 0.
      FLT = FELT(IA,JA)
      DO 230 I = 1,NPEK
         FLT = FLT + W(I)*DOBS(IPEK(I))
         EST = EST +W(I)*PI(I)
230   CONTINUE
C
      ESTVAR(IA,JA) = (PII2-EST) * 100. / PII2
C
      FELT(IA,JA) = FLT
C
CCCCCCCCCCCC
C     DO 250 I = 1,NPEK
C      WRITE(66,246)XOBS(IPEK(I)),YOBS(IPEK(I)),EOBS(IPEK(I)),
C    +              W(I),DOBS(IPEK(I)),PI(I)
C246   FORMAT(3F7.1,F10.4,2F10.2)
C250  CONTINUE
CCCCCCCCCCCC
C
500   CONTINUE
      RETURN
      END
C
C***********************************************************************
C
      FUNCTION TOARH(C,R)
C-----------------------------------------------------------------------
C
C   Calculate third-order autoregressive function, which is used as
C   functional representation of observed height residuals inoperational
C   data assimilation at the Canadian Meteorological Centre.
C
C   ( See Mitcell et al. : Revised Interpolation Statistics for the
C   Canadian Data Assimilation Procedure : Their Derivation and
C   Application. )
C
C-----------------------------------------------------------------------
C
C   Functional form :
C
C   F(c,r) = ( f(c,r) + a*f(c/N,r) )/(1+a)
C
C   where f(c,r) = ( 1 + cr + ((cr)**2)/3 ) exp(-cr)
C
C-----------------------------------------------------------------------
C
C   Variables :
C
C   R   - distance in km
C   C   - scaling factor
C   A,N - constants
C
C-----------------------------------------------------------------------
C
      INTEGER N
      REAL TOARH,R,C,A
      REAL CR,CR2,CRN,CRN2,F1,F2
      DATA A/0.2/, N/3/
C
      CR = C*R
      CR2 = CR**2
      F1 = ( 1. + CR + CR2/3 ) * EXP(-CR)
C
      CRN = CR/N
      CRN2 = CRN**2
      F2 = ( 1. + CRN + CRN2/3 ) * EXP(-CRN)
C
      F1 = F1/(1.+A)
      F2 = F2*A/(1.+A)
      TOARH = F1 + F2
CCCCCCCCCCCCCC
CCCCCC
C     WRITE(66,66)R,F1,F2,TOARH
66    FORMAT(F6.1,3F7.3)
CCCCCC
CCCCCCCCCCCCCC
C
      RETURN
      END
C
C***********************************************************************
C
      SUBROUTINE TOARHV(C,NTOARH,R,TOARH)
C
C       TOARH Vector version
C
C-----------------------------------------------------------------------
C
C   Calculate third-order autoregressive function, which is used as
C   functional representation of observed height residuals inoperational
C   data assimilation at the Canadian Meteorological Centre.
C
C   ( See Mitcell et al. : Revised Interpolation Statistics for the
C   Canadian Data Assimilation Procedure : Their Derivation and
C   Application. )
C
C-----------------------------------------------------------------------
C
C   Functional form :
C
C   F(c,r) = ( f(c,r) + a*f(c/N,r) )/(1+a)
C
C   where f(c,r) = ( 1 + cr + ((cr)**2)/3 ) exp(-cr)
C
C-----------------------------------------------------------------------
C
C   Variables :
C
C   R   - distance in km
C   C   - scaling factor
C   A,N - constants
C
C-----------------------------------------------------------------------
C
      INTEGER NTOARH
      REAL    R(NTOARH),TOARH(NTOARH)
C
      INTEGER N
C->>  REAL TOARH,R,C,A
      REAL C,A
      REAL CR,CR2,CRN,CRN2,F1,F2
      DATA A/0.2/, N/3/
C
      DO 10 I=1,NTOARH
C
C->>  CR = C*R
      CR = C*R(I)
      CR2 = CR**2
      F1 = ( 1. + CR + CR2/3 ) * EXP(-CR)
C
      CRN = CR/N
      CRN2 = CRN**2
      F2 = ( 1. + CRN + CRN2/3 ) * EXP(-CRN)
C
      F1 = F1/(1.+A)
      F2 = F2*A/(1.+A)
C->>  TOARH = F1 + F2
      TOARH(I) = F1 + F2
C
   10 CONTINUE
C
      RETURN
      END
