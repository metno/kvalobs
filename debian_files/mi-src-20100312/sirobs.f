C
C*********************************************************************
C
      SUBROUTINE ROBS1(FILNAM,IUNIT,GRID,XYLIM,NPARAM,PARAM,
     *                 MINUT1,MINUT2,MAXOBS,MAXPAR,OBSDAT,UNDEF,
     *                                             NOBS,IUTC,OK)
C
C       Read SYNOP or DRIBU file  (obervation type 1 or 4)
C
C       Parameters possible in this version:
C         MSLP       (hPa, mean sea level pressure)
C         T.2M       (Celsius)
C         TD.2M      (Celsius, dewpoint)
C         RH.2M      (%, relative humidity)
C         T.WATER    (Celsius, TW)
C         PRECIP.12H (mm, 06 and 18 UTC)
C         TENDENCY   (hPa, 3 hours)
C
C       Quality control:
C         Add '<Q0>', '<Q1>' or '<Q2>' to the parameter name to
C         use data with quality flag equal or better than 0,1 or 2.
C         (0 is best, 3 is worst and means now checking here)
C         Currently only supported for MSLP.
C
C       UNDEF - missing value (i.e. +1.E+35)
C
C       NOBS  - input and output (may be larger than 0 input)
C
C       OBSDAT(N,1) - x position
C       OBSDAT(N,2) - y position
C       OBSDAT(N,3) - parameter no. 1
C       OBSDAT(N,4) - parameter no. 2
C       ........... - parameter no. ...
C
C       OK=.FALSE. returned if open error, read error, wrong file type,
C                           unknown parameter or MAXOBS too small
C
C
      INTEGER IUNIT,NPARAM,MAXOBS,MAXPAR,NOBS,IUTC
      REAL    UNDEF
      REAL    GRID(4),XYLIM(4),OBSDAT(MAXOBS,MAXPAR)
      CHARACTER*(*) FILNAM
      CHARACTER*(*) PARAM(NPARAM)
      LOGICAL OK
C
      INTEGER*2 INH(8,128),IDAT(2048)
C
      logical swap,swapfile
C
C---------------------------------------------------------------------
      REAL EW(41)
C
C        VANNDAMPENS METNINGSTRYKK, EW(41).
C        T I GRADER CELSIUS: -100,-95,-90,...,90,95,100.
C            TC=...    X=(TC+105)*0.2    I=X
C            ET=EWT(I)+(EWT(I+1)-EWT(I))*(X-I)
C
      DATA EW/.000034,.000089,.000220,.000517,.001155,.002472,
     *        .005080,.01005, .01921, .03553, .06356, .1111,
     *        .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *        2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *        23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *        123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *        473.67, 578.09, 701.13, 845.28, 1013.25/
C---------------------------------------------------------------------
C
      NO=NOBS
C
      OK=.FALSE.
C
C
C..get record length unit in bytes for RECL= in OPEN statements
C..(machine dependant)
      CALL RLUNIT(LRUNIT)
C
      OPEN(IUNIT,FILE=FILNAM,
     *           FORM='UNFORMATTED',ACCESS='DIRECT',RECL=2048/LRUNIT,
     *           STATUS='OLD',IOSTAT=IOS,ERR=900)
C
      swap= swapfile(-IUNIT)
C
      IREC=1
      READ(IUNIT,REC=IREC,IOSTAT=IOS,ERR=910) (IDAT(I),I=1,12)
C
      if(swap) call bswap2(12,IDAT)
C
      WRITE(*,*) ' *ROBS1* File: ',FILNAM
      WRITE(*,FMT='(8X,''* Type,Update,Time:'',I6,2(2X,3I5))')
     *                     (IDAT(I),I=1,7)
C
      ITYPE=IDAT(1)
      ITERM=IDAT(7)
      IUTC=ITERM/100
C
      IREC1=3
      IREC2=IDAT(9)
C
C..HOVEDTYPE:  1=SYNOP  4=DRIBU
      IF(ITYPE.NE.1 .AND. ITYPE.NE.4) GOTO 980
C
      NPUSE=2+NPARAM
      IF(NPUSE.GT.MAXPAR) GOTO 980
C
      JMSLP=0
      JT2M =0
      JTD2M=0
      JRH2M=0
      JTWAT=0
      JPR12=0
      JTEND=0
      JERR =0
      JNUM =0
C
      JQMSLP=3
C
      DO 10 NP=1,NPARAM
        JNUM=JNUM+1
        IF(JMSLP.EQ.0 .AND. PARAM(NP).EQ.'MSLP') THEN
          JMSLP=2+NP
          JQMSLP=3
        ELSEIF(JMSLP.EQ.0 .AND. PARAM(NP).EQ.'MSLP<Q2>') THEN
          JMSLP=2+NP
          JQMSLP=2
        ELSEIF(JMSLP.EQ.0 .AND. PARAM(NP).EQ.'MSLP<Q1>') THEN
          JMSLP=2+NP
          JQMSLP=1
        ELSEIF(JMSLP.EQ.0 .AND. PARAM(NP).EQ.'MSLP<Q0>') THEN
          JMSLP=2+NP
          JQMSLP=0
        ELSEIF(JT2M.EQ.0 .AND. PARAM(NP).EQ.'T.2M') THEN
          JT2M =2+NP
        ELSEIF(JTD2M.EQ.0 .AND. PARAM(NP).EQ.'TD.2M') THEN
          JTD2M=2+NP
        ELSEIF(JRH2M.EQ.0 .AND. PARAM(NP).EQ.'RH.2M') THEN
          JRH2M=2+NP
        ELSEIF(JTWAT.EQ.0 .AND. PARAM(NP).EQ.'T.WATER') THEN
          JTWAT=2+NP
        ELSEIF(JPR12.EQ.0 .AND. PARAM(NP).EQ.'PRECIP.12H') THEN
          JPR12=2+NP
          IF(ITYPE.NE.1 .OR. (IUTC.NE.06 .AND. IUTC.NE.18)) THEN
            JPR12=0
            JNUM =JNUM-1
          ENDIF
        ELSEIF(JTEND.EQ.0 .AND. PARAM(NP).EQ.'TENDENCY') THEN
          JTEND=2+NP
          IF(ITYPE.NE.1) THEN
            JTEND=0
            JNUM =JNUM-1
          ENDIF
        ELSE
          WRITE(*,*) ' **ROBS1** Unknown or more than once: ',PARAM(NP)
          JERR =JERR+1
          JNUM =JNUM-1
        ENDIF
   10 CONTINUE
C
      IF(JERR.GT.0 .OR. JNUM.EQ.0) GOTO 980
C
      XP=GRID(1)
      YP=GRID(2)
      AN=GRID(3)
      FI=GRID(4)
      RAD=3.1415927/180.
      VXR=(90.+FI)*RAD
      BETA=SIN(VXR)
      ALFA=COS(VXR)
      RADG=0.01*RAD
CCC   FKON=100.*1852./3600.
      X1=XYLIM(1)
      X2=XYLIM(2)
      Y1=XYLIM(3)
      Y2=XYLIM(4)
C
      IF(MINUT1.GT.MINUT2 .OR. MINUT2-MINUT1.GE.24*60) THEN
        IH1=0000
        IH2=2359
        IH3=0000
        IH4=2359
      ELSEIF(MINUT1.EQ.MINUT2) THEN
        IH1=ITERM
        IH2=ITERM
        IH3=ITERM
        IH4=ITERM
      ELSE
        IH=ITERM/100
        IM=IH*60+(ITERM-IH*100)
        IM1=IM+MINUT1
        IM2=IM+MINUT2
        IF(IM1.LT.0)     IM1=IM1+24*60
        IF(IM1.GE.24*60) IM1=IM1-24*60
        IF(IM2.LT.0)     IM2=IM2+24*60
        IF(IM2.GE.24*60) IM2=IM2-24*60
        IH1=IM1/60
        IH1=IH1*100+(IM1-IH1*60)
        IH2=IM2/60
        IH2=IH2*100+(IM2-IH2*60)
        IF(IH1.LT.IH2) THEN
          IH3=IH1
          IH4=IH2
        ELSE
          IH4=IH2
          IH2=2359
          IH3=0000
        ENDIF
      ENDIF
C
C---------------------------------------------------------------------
      I17B   = 2**4-1
CCC   I377B  = 2**8-1
      I7777B = 2**12-1
C---------------------------------------------------------------------
C
      LRECD1=-1
      LRECD2=-1
C
      DO 100 IRECI=IREC1,IREC2
C
        IREC=IRECI
        READ(IUNIT,REC=IREC,IOSTAT=IOS,ERR=910) INH
C
        if(swap) call bswap2(1024,INH)
C
        DO 110 N=1,128
C..Check if any data
          IF(INH(6,N).LT.IREC2 .OR. INH(7,N).LT.1 .OR.
     *       INH(7,N).GT.1024  .OR. INH(8,N).LT.24) GOTO 110
          IF(INH(7,N)+INH(8,N)-1.GT.2048) GOTO 110
C..Check time
          IF((INH(5,N).LT.IH1 .OR. INH(5,N).GT.IH2) .AND.
     *       (INH(5,N).LT.IH3 .OR. INH(5,N).GT.IH4)) GOTO 110
C..Check position
          GLAT=RADG*INH(3,N)
          GLON=RADG*INH(4,N)
          RR=AN*COS(GLAT)/(1.+SIN(GLAT))
          YR=-RR*COS(GLON)
          XR=+RR*SIN(GLON)
          X=XR*BETA-YR*ALFA+XP
          Y=YR*BETA+XR*ALFA+YP
          IF(X.LT.X1 .OR. X.GT.X2 .OR. Y.LT.Y1 .OR. Y.GT.Y2) GOTO 110
C
          IW0=INH(7,N)-1
          IW2=IW0+INH(8,N)
CCC       IF(IW2.GT.2048) IW2=2048
          IREC=INH(6,N)
          IF(IREC.EQ.LRECD1) THEN
            CONTINUE
          ELSEIF(IREC.EQ.LRECD2) THEN
            LRECD1=LRECD2
            DO 130 I=1,1024
  130       IDAT(I)=IDAT(1024+I)
          ELSE
            LRECD1=IREC
            READ(IUNIT,REC=IREC,IOSTAT=IOS,ERR=910)(IDAT(I),I=1,1024)
            if(swap) call bswap2(1024,IDAT(1))
          ENDIF
          IF(IW2.GT.1024) THEN
            IREC=IREC+1
            LRECD2=IREC
            READ(IUNIT,REC=IREC,IOSTAT=IOS,ERR=910)(IDAT(I),I=1025,2048)
            if(swap) call bswap2(1024,IDAT(1025))
          ENDIF
C
C..LDATA = length of data, including 'end'
          LDATA=INH(8,N)
C
C---------------------------------------------------------------------
C
          IF(NO.GT.MAXOBS) GOTO 920
          NO=NO+1
C
          OBSDAT(NO,1)=X
          OBSDAT(NO,2)=Y
          DO 200 NP=3,NPUSE
            OBSDAT(NO,NP)=UNDEF
  200     CONTINUE
          ND=0
C
C..MSLP (PPP)
          IF(JMSLP.GT.0 .AND. LDATA.GT.21
     *                  .AND. IDAT(IW0+21).NE.-32767) THEN
C..Quality..OBSDAT(NO,JMSLP)=IDAT(IW0+21)*0.1
C..Quality..ND=1
            IF(JQMSLP.GT.2 .OR. LDATA.LT.28) THEN
              OBSDAT(NO,JMSLP)=IDAT(IW0+21)*0.1
              ND=1
            ELSE
              IFLAG=IDAT(IW0+27)
C............ IFLAG=IAND(ISHFT(IFLAG,-8),255)
              IFLAG=IAND(ISHFT(IFLAG,-8),3)
              IF(IFLAG.LE.JQMSLP) THEN
                OBSDAT(NO,JMSLP)=IDAT(IW0+21)*0.1
                ND=1
              ENDIF
            ENDIF
          ENDIF
C
C..T.2M (T)
          IF(JT2M.GT.0 .AND. LDATA.GT.24
     *                 .AND. IDAT(IW0+24).NE.-32767) THEN
            OBSDAT(NO,JT2M)=IDAT(IW0+24)*0.1
            ND=1
          ENDIF
C
C..TD.2M (Td)
          IF(JTD2M.GT.0 .AND. LDATA.GT.25
     *                  .AND. IDAT(IW0+25).NE.-32767) THEN
            OBSDAT(NO,JTD2M)=IDAT(IW0+25)*0.1
            ND=1
          ENDIF
C
C..RH.2M (Td,T -> RH)
          IF(JRH2M.GT.0 .AND. LDATA.GT.25
     *                  .AND. IDAT(IW0+24).NE.-32767
     *                  .AND. IDAT(IW0+25).NE.-32767) THEN
            T2M =IDAT(IW0+24)*0.1
            TD2M=IDAT(IW0+25)*0.1
            IF(TD2M.GT.T2M+2. .OR. TD2M.LT.-95. .OR. T2M.GT.+95.) THEN
              CONTINUE
            ELSEIF(TD2M.GE.T2M) THEN
              OBSDAT(NO,JRH2M)=100.
              ND=1
            ELSE
              XL=(TD2M+105.)*0.2
              LX=XL
              EWTD=EW(LX)+(EW(LX+1)-EW(LX))*(XL-LX)
              XL=(T2M+105.)*0.2
              LX=XL
              EWT=EW(LX)+(EW(LX+1)-EW(LX))*(XL-LX)
              OBSDAT(NO,JRH2M)=100.*EWTD/EWT
              ND=1
            ENDIF
          ENDIF
C
C..T.WATER (TW)
          IF(JTWAT.GT.0 .AND. LDATA.GT.30
     *                  .AND. IDAT(IW0+30).NE.-32767) THEN
            OBSDAT(NO,JTWAT)=IDAT(IW0+30)*0.1
            ND=1
          ENDIF
C
C..PRECIP.12H (RRR, 12 hours precipitation, 06 an 18 UTC)
          IF(JPR12.GT.0 .AND. LDATA.GT.31) THEN
            LTR=I17B
            LRR=I7777B
            IF(IDAT(IW0+31).NE.-32767) THEN
              LRR=IDAT(IW0+31)
              LTR=IAND(LRR,I17B)
              LRR=IAND(ISHFT(LRR,-4),I7777B)
            ENDIF
            IF(LTR.EQ.2 .AND. LRR.LE.999) THEN
              IF(LRR.GE.990) THEN
                OBSDAT(NO,JPR12)=(LRR-990)*0.1
              ELSE
                OBSDAT(NO,JPR12)=LRR
              ENDIF
              ND=1
            ELSEIF(LDATA.GT.36 .AND. IDAT(IW0+36).NE.-32767) THEN
              LIR=IDAT(IW0+36)
              LIR=IAND(ISHFT(LIR,-4),I17B)
              IF(LIR.EQ.3) THEN
                OBSDAT(NO,JPR12)=0.
                ND=1
              ELSEIF(LIR.EQ.4 .AND.
     *               IDAT(IW0+2).EQ.1 .AND. IDAT(IW0+3).EQ.1 .AND.
     *               LTR.EQ.I17B .AND. LRR.EQ.I7777B) THEN
C..Warning| IR=4 and instrument type 1 .................... ?????
                OBSDAT(NO,JPR12)=0.
                ND=1
              ENDIF
            ENDIF
          ENDIF
C
C..TENDENCY (PP)
          IF(JTEND.GT.0 .AND. LDATA.GT.32
     *                  .AND. IDAT(IW0+32).NE.-32767) THEN
            LPP=IDAT(IW0+32)
            LA=IAND(ISHFT(LPP,-12),I17B)
            LPP=IAND(LPP,I7777B)
            IF(LA.LE.8 .AND. LPP.LE.999) THEN
              IF(LA.GT.4) LPP=-LPP
              OBSDAT(NO,JTEND)=LPP*0.1
              ND=1
            ENDIF
          ENDIF
C
          IF(ND.EQ.0) NO=NO-1
C
C---------------------------------------------------------------------
C
  110   CONTINUE
C
  100 CONTINUE
C
      OK=.TRUE.
      GOTO 980
C
  900 WRITE(*,*) ' **ROBS1** OPEN ERROR.  FILE: ',FILNAM
      WRITE(*,*) ' **                     UNIT: ',IUNIT
      WRITE(*,*) ' **                   IOSTAT: ',IOS
      GOTO 990
C
  910 WRITE(*,*) ' **ROBS1** READ ERROR.  FILE: ',FILNAM
      WRITE(*,*) ' **                     UNIT: ',IUNIT
      WRITE(*,*) ' **                   RECORD: ',IREC
      WRITE(*,*) ' **                   IOSTAT: ',IOS
      GOTO 980
C
  920 N=MAXOBS-NOBS
      WRITE(*,*) ' **ROBS1** Reading file: ',FILNAM
      WRITE(*,*) ' **        Too many observations. Max allowed: ',N
CCC   GOTO 980
C
  980 CLOSE(IUNIT)
C
  990 CONTINUE
      N=NO-NOBS
      WRITE(*,*) '       * No. of observations: ',N
      NOBS=NO
C
      RETURN
      END
C
C*********************************************************************
C
      SUBROUTINE RLIST1(FILNAM,IUNIT,GRID,XYLIM,NPARAM,PARAM,
     *                  MAXOBS,MAXPAR,OBSDAT,NOBS,OK)
     *                                             
C
C       Read formatted file.
C	Each line: latitude,longitude,(value(n),n=1,NPARAM)
C
C	Missing (undefined) values not allowed in this version.
C
C       NOBS  - input and output (may be larger than 0 input)
C
C       OBSDAT(N,1) - x position
C       OBSDAT(N,2) - y position
C       OBSDAT(N,3) - parameter no. 1
C       OBSDAT(N,4) - parameter no. 2
C       ........... - parameter no. ...
C
C       OK=.FALSE. returned if open error, read error, wrong file type,
C                           unknown parameter or MAXOBS too small
C
C
      INTEGER IUNIT,NPARAM,MAXOBS,MAXPAR,NOBS
      REAL    GRID(4),XYLIM(4),OBSDAT(MAXOBS,MAXPAR)
      CHARACTER*(*) FILNAM
      CHARACTER*(*) PARAM(NPARAM)
      LOGICAL OK
C
      NO=NOBS
C
      OK=.FALSE.
C
      OPEN(IUNIT,FILE=FILNAM,
     *           FORM='FORMATTED',ACCESS='SEQUENTIAL',
     *           STATUS='OLD',IOSTAT=IOS,ERR=900)
C
      WRITE(*,*) ' *RLIST1* File: ',FILNAM
C
      NPUSE=2+NPARAM
      IF(NPUSE.GT.MAXPAR) GOTO 980
C
      XP=GRID(1)
      YP=GRID(2)
      AN=GRID(3)
      FI=GRID(4)
      RAD=3.1415927/180.
      VXR=(90.+FI)*RAD
      BETA=SIN(VXR)
      ALFA=COS(VXR)
CCC   RADG=0.01*RAD
CCC   FKON=100.*1852./3600.
      X1=XYLIM(1)
      X2=XYLIM(2)
      Y1=XYLIM(3)
      Y2=XYLIM(4)
C
      LINE=0
C
      DO WHILE (NO.LT.MAXOBS)
C
        NO=NO+1
	LINE=LINE+1
	READ(IUNIT,*,IOSTAT=IOS,ERR=910,END=500)
     *		     (OBSDAT(NO,NP),NP=1,NPUSE)
C
C..Check position
        GLAT=RAD*OBSDAT(NO,1)
        GLON=RAD*OBSDAT(NO,2)
        RR=AN*COS(GLAT)/(1.+SIN(GLAT))
        YR=-RR*COS(GLON)
        XR=+RR*SIN(GLON)
        X=XR*BETA-YR*ALFA+XP
        Y=YR*BETA+XR*ALFA+YP
	OBSDAT(NO,1)=X
	OBSDAT(NO,2)=Y
        IF(X.LT.X1 .OR. X.GT.X2 .OR. Y.LT.Y1 .OR. Y.GT.Y2) NO=NO-1
C
      END DO
C
      LINE=LINE+1
      READ(IUNIT,*,IOSTAT=IOS,ERR=910,END=500) GLAT,GLON
      GOTO 920
C
  500 OK=.TRUE.
      GOTO 980
C
  900 WRITE(*,*) ' **RLIST1** OPEN ERROR.  FILE: ',FILNAM
      WRITE(*,*) ' **                      UNIT: ',IUNIT
      WRITE(*,*) ' **                    IOSTAT: ',IOS
      GOTO 990
C
  910 WRITE(*,*) ' **RLIST1** READ ERROR.  FILE: ',FILNAM
      WRITE(*,*) ' **                      UNIT: ',IUNIT
      WRITE(*,*) ' **                      LINE: ',LINE
      WRITE(*,*) ' **                    IOSTAT: ',IOS
      GOTO 980
C
  920 N=MAXOBS-NOBS
      WRITE(*,*) ' **RLIST1** Reading file: ',FILNAM
      WRITE(*,*) ' **         Too many observations. Max allowed: ',N
CCC   GOTO 980
C
  980 CLOSE(IUNIT)
C
  990 CONTINUE
      N=NO-NOBS
      WRITE(*,*) '        * No. of observations: ',N
      NOBS=NO
C
      RETURN
      END
