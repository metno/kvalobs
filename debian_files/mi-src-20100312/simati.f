      SUBROUTINE MATINV
     +   (A,IDIM,NDIM,INDEX,NERROR,DETERM)
C
C     Original code:
C     MATRIX INVERSION WITH ACCOMPANYING SOLUTION OF LINEAR EQUATIONS
C     CERN LIBRARY
C
C     DNMI/FoU  November 1991  Anstein Foss
C        - Not computing DETERM (gives too much overflow on IBM)
C        - Modernized code
C        - Changed indexing of A from one to two dimensions
C          (leaving index computations to the computer)
C
C     A(1:IDIM,1:IDIM) - size of A
C     A(1:NDIM,1:NDIM) - part of A used
C
      INTEGER IDIM,NDIM,NERROR
      INTEGER INDEX(IDIM)
      REAL    DETERM
      DOUBLE PRECISION A(IDIM,IDIM)
C
      DOUBLE PRECISION DETER,PIVOT,SWAP,ZERO,APIVOT
C
      ZERO =0.0D0
      DETER=1.0D0
C
      N=NDIM
C
C..Main loop to invert the matrix
      DO 11 J=1,N
C..Search for next pivot in column J ('main')
        LPIV=0
        APIVOT=ZERO
        DO 2 I=J,N
          IF(DABS(A(I,J)).GT.APIVOT) THEN
            APIVOT=DABS(A(I,J))
            LPIV=I
          ENDIF
    2   CONTINUE
C..Is pivot different from zero
        IF(LPIV.EQ.0) GOTO 15
        PIVOT=A(LPIV,J)
C..Get the pivot-line indicator and swap lines if necessary
        INDEX(J)=LPIV
C
        IF(LPIV.GT.J) THEN
C..Complement the determinant
C-->>     DETER=-DETER
          DO 5 JN=1,N
            SWAP=A(J,JN)
            A(J,JN)=A(LPIV,JN)
            A(LPIV,JN)=SWAP
    5     CONTINUE
        ENDIF
C..Compute determinant
C-->>   DETER=DETER*PIVOT
        PIVOT=1./PIVOT
C..Transform pivot column
        APIVOT=-PIVOT
        DO 7 I=1,N
          A(I,J)=APIVOT*A(I,J)
    7   CONTINUE
        A(J,J)=PIVOT
C..Pivot element transformed
C
C..Now convert rest of the matrix
        DO 10 JN=1,N
          IF(JN.NE.J) THEN
C..Pivot column excluded
            SWAP=A(J,JN)
            DO 9 I=1,N
              A(I,JN)=A(I,JN)+SWAP*A(I,J)
    9       CONTINUE
            A(J,JN)=SWAP*PIVOT
          ENDIF
   10   CONTINUE
   11 CONTINUE
C
C..Now rearrange the matrix to get right invers
      DO 14 J=N,1,-1
        LPIV=INDEX(J)
        IF(LPIV.NE.J) THEN
          DO 13 I=1,N
            SWAP=A(I,J)
            A(I,J)=A(I,LPIV)
            A(I,LPIV)=SWAP
   13     CONTINUE
        ENDIF
   14 CONTINUE
C
      NERROR=0
      DETERM=DETER
      RETURN
C
   15 NERROR=J
      DETERM=DETER
      RETURN
      END
