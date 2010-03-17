      program nyfelt
c
c#### MAN_BEGIN ########################################################
c
c  DNMI PROGRAM: NYFELT
c
c  PURPOSE: Create FELT file.
c           Copy fields from one FELT file to another.
c
c  Basic document on FELT files:
c
c          FILE STRUKTUR FOR "SANNTIDS" LAGRING AV GRID-DATA
c               Forskningsavdeling DNMI, oktober 1982
c
c  FELT file types:
c  ----------------
c       STANDARD - one basic date/time (analysis or reference)
c                  all fields stored with forecast lengths (positive,
c                  zero or negative) relative to this date/time
c       ARCHIVE  - several basic date/time's (and forecast lengths
c                  as 'standard')
c       CYCLIC_ARCHIVE - 'archive' where the oldest date/time will
c                        be removed when writing a new date/time to
c                        the file (NYFELT has options to do such update)
c
c  Specifications for NYFELT is given in a 'nyfelt.input' file when
c  running the program. This file is given to the program as a
c  command line argument, e.g.:
c      nyfelt nyfelt.input
c  or with a more describing file name:
c      nyfelt nyfelt.lam50s
c  or print help lines in the 'nyfelt.input' file (if such exists):
c      nyfelt nyfelt.lam50s ?
c
c  NYFELT.INPUT:
c  -------------
c  Consists of two sections (which may be repeated):
c    1) General specifications
c       (file name, date/time, grid, file type etc.)
c    2) Field identifications
c       (forecast_lengths, levels, parameters)
c
c  Comment lines are expected to be found before each section and also
c  between each 'loop' in the 'field identification' section.
c  Comment lines start with a * in column one. The last comment line
c  before 'real' program input has *> in the two first columns.
c  Help lines are identified with *=> in the three first columns.
c
c  ------------------------------
c  General specification section:
c  ------------------------------
c  Consists of keywords with and without user values.
c  Syntax is 'KEYWORD=value' or 'KEYWORD'.
c  (Keywords are always converted to uppercase letters, and is
c   everything in front of a = sign. Several keywords may be written
c   on one line separated by a ; sign. The maximum line length is
c   80 charcaters.)
c  There are two ways to use variables here, instead of
c  editing the 'user values' in the 'nyfelt.input' file:
c     - environment variables,  starting with a $ sign
c       (e.g. 'FILE=$feltfile')
c     - command line arguments, starting with a # sign and followed by
c       the argument no. (e.g. 'FILE=#2'), remember that argument no. 1
c       (#1) always is the name of the 'nyfelt.input' file.
c  There is no sequence rules for the keywords.
c
c  Keywords and user values (the second in <....>):
c
c   'FILE=<output_FELT_file>'
c        name of the output file
c   'TIME=<year,month,day,hour>'
c        date/time when creating a file, for Archive files this is
c        the first date/time on the file (but after creation the last
c        date/time will be  used as the file's date/time),
c        for Cyclic_Archive files this is the last date/time.
c   'TIME.FILE_IN'
c        date/time as the input file when creating a file
c        (as above for Archive files).
c   'FILE_IN=<input_FELT_file>'
c        input file, for date/time and/or field copy.
c   'TIME_STEP=<nstep, year_step,month_step,day_step,hour_step>'
c        used when creating Archive or Cyclic_Archive files, nstep is
c        the number of timesteps followed by the step in
c        years,months,days and hours, 3 of these must be 0.
c   'FELT.STANDARD'
c        output FELT file type is Standard.
c   'FELT.ARCHIVE'
c        output FELT file type is Archive.
c   'FELT.CYCLIC_ARCHIVE'
c        output FELT file type is Cyclic_Archive.
c   'CREATE.MACHINE.ENDIAN'
c	 create file in same endian byte order as the machine
c	 (This may become the default later...)
c   'CREATE.BIG.ENDIAN'
c	 create file in big endian byte order (SGI etc.)
c	 (This is the default in this version)
c   'CREATE.LITTLE.ENDIAN'
c	 create file in little endian byte order (Linux/Intel/Dos,...)
c   'DATA_GAPS.ON'
c        each field will occupy a whole number of records, and always
c        start in the first word of a record (default)
c        (this option is ignored if the file is not created)
c   'DATA_GAPS.OFF'
c        field data will not occupy more space than necessary, there
c        will be no unused space between two fields
c        (this option is ignored if the file is not created)
c   'MODE.CREATE'
c        the output file is created (if the file exists, and the file
c        name is not a 'link', the existing file will be removed).
c   'MODE.CREATE+COPY'
c        the output file is created and all fields in the
c        'field identification' section wil be copied from the input
c        file (if they exist).
c   'MODE.COPY'
c        copy fields from the input to the output file,
c        output file must exist (it is not created).
c   'MODE.UPDATE'
c        for Archive and Cyclic_Archive files.
c        copy fields from the (Standard type) input file,
c        no 'field identification' section is needed as nyfelt uses the
c        existing 'content list' in the file, only one date/time is
c        copied and for Cyclic_Archive files the date/time will be
c       'cycled' if necessary (removing the oldest date/time).
c   'MODE.UPDATE_ALL'
c        for Archive and Cyclic_Archive files.
c        if the input file is a Archive or Cyclic_Archive file all
c        timesteps will be copied using the existing 'content list'
c        (use this to regenerate a file after changing 'content list')
c   'GRID=<producer,grid,grid_type>'
c        the producer no., grid no. and grid type for all fields in the
c        following 'field identifications', this option may be repeated,
c        giving the same field identifications for all 'grids'.
c   'OVER_WRITE.ON'               - allow overwrite of existing fields
c        allow overwrite of existing fields when fields are copied
c        (default).
c   'OVER_WRITE.OFF'
c        prevent overwrite of existing fields when fields are copied.
c   'CHECK_TIME.ON'               - check date/time on fields before
c        check date/time of fields before copy (current version of
c        WFELT routine will as default use the output file's date/time)
c        (default)
c   'CHECK_TIME.OFF'
c        not check date/time (useful when copying fields which really
c        are independant of time).
c   'INFO.ON'
c        print messages about missing fields during copy (default).
c   'INFO.OFF'
c        not print messages about missing fields during copy.
c   'PROG_LIMIT=<min_prog_hour,max_prog_hour>'
c        used to decrease the number of forecast lengths given in the
c        following 'field identification' section (when this is made
c        to cover a maximum length forecast)
c        (default is no limit, i.e. -32767,+32767)
c   'DEFINE.LOOP= <identifier>,<value1>,<value2>,...'
c	 when the 'identifier' value is found the time/level/parameter
c	 section, the identifier element is replaced with all these
c        values, identifiers should be in the range -32767 - +32767
c	 and have a value not used anywhere else.
c   'DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>'
c	 as above, except that values are specified by the first, last
c	 and increment (step) value.
c   'CONTINUE'
c        end of this 'general specification' section, but another
c        will follow (usually after a 'field identification' section)
c   'END'
c        end of the last (and usually only) 'general specification'
c        section. for MODE.CREATE, MODE.CREATE+COPY and MODE.COPY
c        a 'field identification' section must follow.
c
c  Note: There is no default for FELT file type (FELT.xxx) and the
c        action to be done (MODE.xxx).
c        When more than one producer is defined (in GRID=...) they must
c        be given in increasing order (FELT file sequence rule).
c        If more than one 'general spec.' section is used, the previous
c        defined 'grid(s)' will be 'forgotten' if at least one grid is
c        specified.
c
c  -----------------------------
c  Field identification section:
c  -----------------------------
c  This section defines the field identifications to be created or
c  the fields to be copied. Date/time and producer,grid and grid type
c  is given in the 'general spec.' section above, as they (in almost
c  all cases) will be constant here, but also the part of field id.'s
c  which most frequently changes ('variables' are not allowed in the
c  field id. section).
c  The organization of this section is more strict than FELT file rules.
c  It defines 'loops' where the outer loop is forecast lengths, then
c  levels and with parameters as the inner loop.
c  Several of these trippel loops may be needed to store fields with
c  different forecast lengths (different time steps) and combinations
c  of levels and parameters.
c  Note that for some reason the vertical coordinate is paired with
c  parameter numbers and not levels.
c  In front of each trippel loop is one or more comment lines.
c  The easiest way to explain this is an example:
c
c    ** Comments:
c    **
c    **   4 forecast lengths
c    **      4 pairs of datatype and forecast length
c    **  11 levels
c    **     11 pairs of level_1 and level_2    (level_2 is usually 0)
c    **   5 parameters
c    **      5 pairs of vertical coordinate and parameter
c    **
c    *>
c    4
c    1,0, 3,0, 2,6, 2,12
c    11
c    100,0, 150,0, 200,0, 250,0, 300,0, 400,0, 500,0,
c           700,0, 850,0, 925,0, 1000,0
c    5
c    1,1, 1,2, 1,3, 1,10, 1,18
c
c  To terminate the sequence of trippel loops:
c
c    ** Comments:
c    **    end of forecast length / level / parameter loops
c    *>    0 forecast lengths
c    0
c
c  If more than one grid is defined (in GRID=...), there will be a
c  'grid loop' on the outside of all the trippel loops explained above.
c  For Archive and Cyclic_Archive there is an extreme outer loop (also
c  outside all field id. sections) on date/time.
c
c
c-----------------------------------------------------------------------
c EXAMPLE 1.  Create a file for LAM analysis and some model output
c=======================================================================
c *** nyfelt.lam50s  ('nyfelt.input')
c ***
c *=> Create FELT file for LAM50S.
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    nyfelt  nyfelt.lam50s  1992,8,25,0  +12  felt00.dat
c ***
c ***----------------------------------------------------------------
c **
c ** Option list:
c **-------------
c ** FILE=<output_FELT_file>
c ** TIME=<year,month,day,hour>
c ** TIME.FILE_IN
c ** TIME_OFFSET=<years,months,days,hours>
c ** FILE_IN=<input_FELT_file>
c ** TIME_STEP=<nstep, year_step,month_step,day_step,hour_step>
c ** FELT.STANDARD
c ** FELT.ARCHIVE
c ** FELT.CYCLIC_ARCHIVE
c ** CREATE.MACHINE.ENDIAN
c ** CREATE.BIG.ENDIAN ........................... (default)
c ** CREATE.LITTLE.ENDIAN
c ** DATA_GAPS.ON  ............................... (default)
c ** DATA_GAPS.OFF
c ** MODE.CREATE
c ** MODE.CREATE+COPY
c ** MODE.COPY
c ** MODE.UPDATE
c ** MODE.UPDATE_ALL
c ** GRID=<producer,grid,grid_type> .............. (repeatable)
c ** OVER_WRITE.ON  .............................. (default)
c ** OVER_WRITE.OFF
c ** CHECK_TIME.ON  .............................. (default)
c ** CHECK_TIME.OFF
c ** INFO.ON  .................................... (default)
c ** INFO.OFF
c ** PROG_LIMIT=<min_prog_hour,max_prog_hour> .... (default = no limit)
c ** DEFINE.LOOP= <identifier>,<value1>,<value2>,...
c ** DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>
c ** CONTINUE
c ** END
c **
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c ** Options (for CREATE of STANDARD file)     Remember '....' syntax.
c ** ($... = environment var. ;  #n = command line arg. no. n)
c *>
c 'FILE= #4'
c 'TIME= #2'
c 'PROG_LIMIT= -32767,#3'             ..... or 'PROG_LIMIT=-32767,+48'
c 'GRID= 88,1814,1'                   ..... or 'GRID= 88,$grid_no,1'
c 'FELT.STANDARD'
c 'MODE.CREATE'
c 'END'
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c **
c *> ..... P LEVELS: Z(1) U(2) V(3) RH(10) POT.TEMP.(18)
c 4
c 1,0, 3,0,  2,6, 2,12
c 11
c 100,0, 150,0, 200,0, 250,0, 300,0, 400,0, 500,0,
c        700,0, 850,0, 925,0, 1000,0
c 5
c 1,1, 1,2, 1,3, 1,10, 1,18
c **
c *> ..... SIGMA LEVELS: U(2) V(3) POT.TEMP.(18) Q(9) OMEGA(13)
c 3
c 3,0, 2,6, 2,12
c 18
c 1,0,  2,0,  3,0,  4,0,  5,0,  6,0,  7,0,  8,0,  9,0, 10,0, 11,0,
c      12,0, 13,0, 14,0, 15,0, 16,0, 17,0, 18,0
c 5
c 2,2, 2,3, 2,18, 2,9, 2,13
c ** .....
c ** ..... SURFACE:
c *> ..... MSLP(58) T2M(31) U10M(33) V10M(34)
c 5
c 3,0, 2,3,  2,6,  2,9,  2,12
c 1
c 1000,0
c 4
c 2,58, 2,31, 2,33, 2,34
c **
c *> ..... end of time/level/parameter section
c 0
c=======================================================================
c
c-----------------------------------------------------------------------
c EXAMPLE 2.  Create file and copy LAM analysis fields.
c=======================================================================
c *** nyfelt.copyana  ('nyfelt.input')
c ***
c *=> Create FELT file and copy LAM analysis
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    nyfelt  nyfelt.copyana  felt1.dat  1814  felt2.dat
c *=>                            <input>   <grid>  <output>
c ***
c ***----------------------------------------------------------------
c **
c ** Option list:
c **-------------
c ** FILE=<output_FELT_file>
c ** TIME=<year,month,day,hour>
c ** TIME.FILE_IN
c ** TIME_OFFSET=<years,months,days,hours>
c ** FILE_IN=<input_FELT_file>
c ** TIME_STEP=<nstep, year_step,month_step,day_step,hour_step>
c ** FELT.STANDARD
c ** FELT.ARCHIVE
c ** FELT.CYCLIC_ARCHIVE
c ** CREATE.MACHINE.ENDIAN
c ** CREATE.BIG.ENDIAN ........................... (default)
c ** CREATE.LITTLE.ENDIAN
c ** DATA_GAPS.ON  ............................... (default)
c ** DATA_GAPS.OFF
c ** MODE.CREATE
c ** MODE.CREATE+COPY
c ** MODE.COPY
c ** MODE.UPDATE
c ** MODE.UPDATE_ALL
c ** GRID=<producer,grid,grid_type> .............. (repeatable)
c ** OVER_WRITE.ON  .............................. (default)
c ** OVER_WRITE.OFF
c ** CHECK_TIME.ON  .............................. (default)
c ** CHECK_TIME.OFF
c ** INFO.ON  .................................... (default)
c ** INFO.OFF
c ** PROG_LIMIT=<min_prog_hour,max_prog_hour> .... (default = no limit)
c ** DEFINE.LOOP= <identifier>,<value1>,<value2>,...
c ** DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>
c ** CONTINUE
c ** END
c **
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c ** Options (for CREATE of STANDARD file)     Remember '....' syntax.
c ** ($... = environment var. ;  #n = command line arg. no. n)
c *>
c 'FILE= #4'
c 'FILE_IN= #2'
c 'TIME.FILE_IN'
c 'GRID= 88,#3,1'                     ..... or 'GRID= 88,1814,1'
c 'FELT.STANDARD'
c 'MODE.CREATE+COPY'
c 'END'
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c **
c ** ..... Analysis.
c *> ..... P LEVELS: Z(1) U(2) V(3) RH(10)
c 1
c 1,0
c 11
c 100,0, 150,0, 200,0, 250,0, 300,0, 400,0, 500,0,
c        700,0, 850,0, 925,0, 1000,0
c 4
c 1,1, 1,2, 1,3, 1,10
c **
c *> ..... end of time/level/parameter section
c 0
c=======================================================================
c
c**********************************************************************
c
c======================================================================
c  FILE=<output_FELT_file>
c  TIME=<year,month,day,hour>
c  TIME.FILE_IN
c  TIME_OFFSET=<years,months,days,hours>
c  FILE_IN=<input_FELT_file>
c  TIME_STEP=<nstep, year_step,month_step,day_step,hour_step>
c  FELT.STANDARD
c  FELT.ARCHIVE
c  FELT.CYCLIC_ARCHIVE
c  CREATE.MACHINE.ENDIAN
c  CREATE.BIG.ENDIAN ........................... (default)
c  CREATE.LITTLE.ENDIAN
c  DATA_GAPS.ON  ............................... (default)
c  DATA_GAPS.OFF
c  MODE.CREATE
c  MODE.CREATE+COPY
c  MODE.COPY
c  MODE.UPDATE
c  MODE.UPDATE_ALL
c  GRID=<producer,grid,grid_type> ................. (repeatable)
c  OVER_WRITE.ON  ................................. (default)
c  OVER_WRITE.OFF
c  CHECK_TIME.ON  ................................. (default)
c  CHECK_TIME.OFF
c  INFO.ON  ....................................... (default)
c  INFO.OFF
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> ....... (default = no limit)
c  DEFINE.LOOP= <identifier>,<value1>,<value2>,...
c  DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>
c  CONTINUE
c  END
c======================================================================
c
c-------------------------------------------------------------------
c  DNMI/FoU  1983 - 1990   Anstein Foss ..... IBM
c  DNMI/FoU  1983 - 1990   Rebecca Rudsar
c  DNMI/FoU   19.03.1991   Anstein Foss
c  DNMI/FoU   16.10.1992   Anstein Foss ..... Unix
c  DNMI/FoU   27.07.1993   Anstein Foss
c  DNMI/FoU   11.03.1994   Anstein Foss
c  DNMI/FoU   13.05.1994   Anstein Foss
c  DNMI/FoU   06.10.1994   Anstein Foss
c  DNMI/FoU   09.01.1995   Anstein Foss
c  DNMI/FoU   17.01.1995   Anstein Foss
c  DNMI/FoU   08.06.1995   Anstein Foss ... time_offset
c  DNMI/FoU   01.09.1997   Anstein Foss ... define.loop
c  DNMI/FoU   28.08.2001   Anstein Foss ... automatic byte swap (input)
c  DNMI/FoU   04.12.2001   Anstein Foss ... minor eta.blevel change ok
c  DNMI/FoU   22.03.2003   Anstein Foss ... auto byte swap COPY/UPDATE
c  DNMI/FoU   23.03.2003   Anstein Foss ... CREATE.xxx.ENDIAN options
c  DNMI/FoU   05.11.2003   Anstein Foss ... Default CREATE.BIG.ENDIAN
c  DNMI/FoU   07.03.2004   Anstein Foss ... fixed CREATE+COPY swap-bug
c-------------------------------------------------------------------
c
c#### MAN_END ##########################################################
c
c-------------------------------------------------------------------
c      DNMI library subroutines:  RFTURBO (fast version of rfelt)
c                                 WFTURBO (fast version of wfelt)
c				  WCFELT
c                                 RLUNIT
c                                 RCOMNT
c                                 GETVAR
c                                 KEYWRD
c                                 PRHELP
c                                 RMFILE
c                                 VTIME
c                                 DAYTIM
c                                 swapfile
c                                 bswap2
c                                 bigendian
c-------------------------------------------------------------------
c
c        nwrec  - antall ord i hver record (16 bits ord)
c        ninrec - antall innh.fort. i hver record
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for nyfelt.f
c
c..maxij - max dimension of fields (for copy and update)
c
ccc   parameter (maxij=100000)
c
ccc   parameter (maxset=50,mstore=30000,maxgrd=24,mloops=24)
c
ccc   parameter (mdefloop=100,mloopdef=2000)
c
c..don't change the following:
c
ccc   parameter (limit=20+maxij)
c
ccc   parameter (nwrec=1024,ninrec=64)
c
ccc   common/a/ntim(2,maxset),nlev(2,maxset),npar(2,maxset),
ccc  *         itimef(5),ntstep,itstep(5),ngrid,igrid(3,maxgrd),
ccc  *         imode(mloops),
ccc  *         ioverw(mloops),ictime(mloops),info(mloops),
ccc  *         numgrd(2,mloops),numset(2,mloops),
ccc  *         idrec1(1024),idrec2(2,512),idata(limit),
ccc  *         istore(2,mstore),
ccc  *         fileot,filein(mloops),filnam
ccc   integer*2 idrec1,idrec2,idata,istore
ccc   character*256 fileot,filein,filnam
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c
      include 'nyfelt.inc'
c
      integer   ioffset(4)
      integer*2 idfile(32)
c
      integer   idefloop(3,mdefloop+1)
      integer*2 iloopdef(mloopdef+1)
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*256 cinput,cipart
      character*256 finput
      character*1   tchar
c
      logical   swapfile,swap,bigendian,createswap
c
      character*16 carkiv(0:2)
c
      carkiv(0)='standard'
      carkiv(1)='archive'
      carkiv(2)='cyclic_archive'
c
      iprhlp=0
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c..termination character for free format read (machine dependant)
      call termchar(tchar)
c
c..file unit for 'nyfelt.input'
      iuinp=9
c
c..file unit for output felt file
      iunit=20
c
c..file unit for all input felt files (for field or date/time copy)
      iunitc=30
c
c-----------------------------------------------------------------------
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: nyfelt <nyfelt.input>'
        write(6,*) '      or: nyfelt <nyfelt.input> <arguments>'
        write(6,*) '      or: nyfelt <nyfelt.input> ?     (to get help)'
        write(6,*)
        goto 981
      end if
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error: ',finput(1:lenstr(finput,1))
        goto 981
      end if
c
      if(narg.eq.2) then
        call getarg(2,cinput)
        if(cinput.eq.'?') then
          iprhlp=1
          goto 241
        end if
      end if
c
c
      write(6,*) 'reading input file: ',finput(1:lenstr(finput,1))
c
      nlines = 0
c
      fileot = '*'
      inptim = 0
      ntstep = 0
      ngrid  = 0
      nstore = 0
      nsets  = 0
      iarkiv =-1
      newinh = 0
      nogap  =-1
      iendian=-1
c
      iprmin = -32767
      iprmax = +32767
      ioffset(1)=-32767
c
      lprod=1
      iprod1=99
      iprod2= 1
      ncreat=0
      ncreco=0
      ncopy =0
      nupdat=0
      nupall=0
c
      ndefloop=0
      nloopdef=0
c
      do 100 iloop=1,mloops
c
        call rcomnt(iuinp,'*>','*',nlines,ierror)
        if(ierror.ne.0) goto 210
c
        imode(iloop) = -1
c
        if(iloop.eq.1) then
          ioverw(iloop)   = 1
          ictime(iloop)   = 1
          info(iloop)     = 1
          numgrd(1,iloop) = 0
          numgrd(2,iloop) = 0
          filein(iloop)   = '*'
          numset(1,iloop) = 0
          numset(2,iloop) = 0
        else
          ioverw(iloop)   = ioverw(iloop-1)
          ictime(iloop)   = ictime(iloop-1)
          info(iloop)     = info(iloop-1)
          numgrd(1,iloop) = numgrd(1,iloop-1)
          numgrd(2,iloop) = numgrd(2,iloop-1)
          filein(iloop)   = filein(iloop-1)
          numset(1,iloop) = 0
          numset(2,iloop) = 0
        end if
c
        iend=0
        lgrid=ngrid
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
ccc           l=kwhere(1,ikey)
             k1=kwhere(2,ikey)
             k2=kwhere(3,ikey)
            kv1=kwhere(4,ikey)
            kv2=kwhere(5,ikey)
c
            if(kv1.gt.0) cipart=cinput(kv1:kv2)//tchar
c
c=====================================================================
c  file=<output_felt_file>
c  time=<year,month,day,hour>
c  time.file_in
c  time_offset=<years,months,days,hours>
c  file_in=<input_felt_file>
c  time_step=<nstep, year_step,month_step,day_step,hour_step>
c  felt.standard
c  felt.archive
c  felt.cyclic_archive
c  create.machine.endian
c  create.big.endian .............................. (default)
c  create.little.endian
c  data_gaps.on  .................................. (default)
c  data_gaps.off
c  mode.create
c  mode.create+copy
c  mode.copy
c  mode.update
c  mode.update_all
c  grid=<producer,grid,grid_type> ................. (repeatable)
c  over_write.on  ................................. (default)
c  over_write.off
c  check_time.on  ................................. (default)
c  check_time.off
c  info.on  ....................................... (default)
c  info.off
c  prog_limit=<min_prog_hour,max_prog_hour>
c  define.loop= <identifier>,<value1>,<value2>,...
c  define.loop.from.to.step= <identifier>,<first>,<last>,<step>
c  continue
c  end
c======================================================================
c
            if(cinput(k1:k2).eq.'file') then
c..file=<output_felt_file>
              if(fileot(1:1).ne.'*') goto 214
              if(kv1.lt.1) goto 213
              fileot=cinput(kv1:kv2)
            elseif(cinput(k1:k2).eq.'time') then
c..time=<year,month,day,hour>
              if(inptim.ne.0) goto 214
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) (itimef(i),i=1,4)
              itimef(5)=0
              inptim=1
            elseif(cinput(k1:k2).eq.'time.file_in') then
c..time.file_in
              if(inptim.ne.0) goto 214
              inptim=2
            elseif(cinput(k1:k2).eq.'time_offset') then
c..time_offset=<years,months,days,hours>
              if(ioffset(1).ne.-32767) goto 214
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) (ioffset(i),i=1,4)
            elseif(cinput(k1:k2).eq.'file_in') then
c..file_in=<input_felt_file>
              if(kv1.lt.1) goto 213
              filein(iloop)=cinput(kv1:kv2)
            elseif(cinput(k1:k2).eq.'time_step') then
c..time_step=<nstep, year_step,month_step,day_step,hour_step>
              if(ntstep.ne.0) goto 214
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) ntstep,(itstep(i),i=1,4)
              if(ntstep.lt.1) goto 213
              if(ntstep.gt.1) then
                i=0
                j=0
                do n=1,4
                  if(itstep(n).ne.0) i=i+1
                  if(itstep(n).lt.0) j=1
                end do
                if(itstep(2).gt.12) j=1
                if(i.ne.1 .or. j.ne.0) goto 213
              end if
            elseif(cinput(k1:k2).eq.'felt.standard') then
c..felt.standard
              if(iarkiv.ne.-1 .and. iarkiv.ne.0) goto 214
              iarkiv=0
            elseif(cinput(k1:k2).eq.'felt.archive') then
c..felt.archive
              if(iarkiv.ne.-1 .and. iarkiv.ne.1) goto 214
              iarkiv=1
            elseif(cinput(k1:k2).eq.'felt.cyclic_archive') then
c..felt.cyclic_archive
              if(iarkiv.ne.-1 .and. iarkiv.ne.2) goto 214
              iarkiv=2
            elseif(cinput(k1:k2).eq.'create.machine.endian') then
c..create.machine.endian
	      if(iendian.ne.-1) goto 214
	      iendian=0
            elseif(cinput(k1:k2).eq.'create.big.endian') then
c..create.big.endian
	      if(iendian.ne.-1) goto 214
	      iendian=1
            elseif(cinput(k1:k2).eq.'create.little.endian') then
c..create.little.endian
	      if(iendian.ne.-1) goto 214
	      iendian=2
            elseif(cinput(k1:k2).eq.'data_gaps.on') then
c..data_gaps.on
              if(nogap.ne.-1 .and. nogap.ne.0) goto 214
              nogap=0
            elseif(cinput(k1:k2).eq.'data_gaps.off') then
c..data_gaps.off
              if(nogap.ne.-1 .and. nogap.ne.1) goto 214
              nogap=1
            elseif(cinput(k1:k2).eq.'mode.create') then
c..mode.create
              if(imode(iloop).ne.-1 .and. imode(iloop).ne.0) goto 214
              imode(iloop)=0
            elseif(cinput(k1:k2).eq.'mode.create+copy') then
c..mode.create+copy
              if(imode(iloop).ne.-1 .and. imode(iloop).ne.1) goto 214
              imode(iloop)=1
            elseif(cinput(k1:k2).eq.'mode.copy') then
c..mode.copy
              if(imode(iloop).ne.-1 .and. imode(iloop).ne.2) goto 214
              imode(iloop)=2
            elseif(cinput(k1:k2).eq.'mode.update') then
c..mode.update
              if(imode(iloop).ne.-1 .and. imode(iloop).ne.3) goto 214
              imode(iloop)=3
            elseif(cinput(k1:k2).eq.'mode.update_all') then
c..mode.update_all
              if(imode(iloop).ne.-1 .and. imode(iloop).ne.4) goto 214
              imode(iloop)=4
            elseif(cinput(k1:k2).eq.'grid') then
c..grid=<producer,grid,grid_type>
              if(kv1.lt.1) goto 213
              ngrid=ngrid+1
              if(ngrid.gt.maxgrd) goto 215
              read(cipart,*,err=213) (igrid(i,ngrid),i=1,3)
            elseif(cinput(k1:k2).eq.'over_write.on') then
c..over_write.on
              ioverw(iloop)=1
            elseif(cinput(k1:k2).eq.'over_write.off') then
c..over_write.off
              ioverw(iloop)=0
            elseif(cinput(k1:k2).eq.'check_time.on') then
c..check_time.on
              ictime(iloop)=1
            elseif(cinput(k1:k2).eq.'check_time.off') then
c..check_time.off
              ictime(iloop)=0
            elseif(cinput(k1:k2).eq.'info.on') then
c..info.on
              info(iloop)=1
            elseif(cinput(k1:k2).eq.'info.off') then
c..info.off
              info(iloop)=0
            elseif(cinput(k1:k2).eq.'prog_limit') then
c..prog_limit=<min_prog_hour,max_prog_hour>
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) iprmin,iprmax
            elseif(cinput(k1:k2).eq.'define.loop.from.to.step') then
c..define.loop.from.to.step= <identifier>,<first>,<last>,<step>
              if(iloop.ne.1) goto 233
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) id,i1,i2,i3
              if(id.lt.-32768 .or. id.gt.32767) goto 213
              if(i3.eq.0) i3=1
              if(i1.lt.i2 .and. i3.lt.0) goto 213
              if(i1.gt.i2 .and. i3.gt.0) goto 213
              i=0
	      if(ndefloop.gt.0) then
                if(idefloop(1,ndefloop).eq.id) i=ndefloop
	      end if
              if(i.eq.0) then
		ndefloop=ndefloop+1
                if(ndefloop.gt.mdefloop) goto 234
		idefloop(1,ndefloop)=id
		idefloop(2,ndefloop)=0
		idefloop(3,ndefloop)=nloopdef+1
	      end if
              j=nloopdef
              do i=i1,i2,i3
                nloopdef=nloopdef+1
                if(nloopdef.gt.mloopdef) goto 234
                iloopdef(nloopdef)=i
              end do
              idefloop(2,ndefloop)=idefloop(2,ndefloop)+nloopdef-j
            elseif(cinput(k1:k2).eq.'define.loop') then
c..define.loop= <identifier>,<value1>,<value2>,...
              if(iloop.ne.1) goto 233
              if(kv1.lt.1) goto 213
              read(cipart,*,end=213) id
              if(id.lt.-32767 .or. id.gt.32767) goto 213
              i=0
              if(ndefloop.gt.0) then
                if(idefloop(1,ndefloop).eq.id) i=ndefloop
              end if
              if(i.eq.0) then
                ndefloop=ndefloop+1
                if(ndefloop.gt.mdefloop) goto 234
                idefloop(1,ndefloop)=id
                idefloop(2,ndefloop)=0
                idefloop(3,ndefloop)=nloopdef+1
              end if
              i1=nloopdef+1
              i2=i1-1
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mloopdef) goto 234
                i2=i2+1
                read(cipart,*,iostat=ios) id,(iloopdef(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              nloopdef=i2
              idefloop(2,ndefloop)=idefloop(2,ndefloop)+i2-i1+1
            elseif(cinput(k1:k2).eq.'continue') then
c..continue
              iend=-1
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
        if(ngrid.gt.lgrid) then
          numgrd(1,iloop)=lgrid+1
          numgrd(2,iloop)=ngrid
        end if
c
        if(iarkiv.eq.-1) goto 228
c
        if(imode(iloop).eq.-1) then
          if(iloop.eq.1) goto 229
          imode(iloop)=imode(iloop-1)
        end if
c
        if(imode(iloop).eq.1 .and. iarkiv.ne.0) goto 230
        if(imode(iloop).ge.3 .and. iarkiv.eq.0) goto 231
c
c..file=
        if(fileot(1:1).eq.'*') goto 227
c
c..time= or time.file_in
        if(imode(iloop).le.1 .and. inptim.eq.0) goto 225
c
c..file_in= if not create
        if(imode(iloop).gt.0 .and. filein(iloop)(1:1).eq.'*') goto 220
c
c..copy or update before create or create+copy not accepted
        if(imode(iloop).le.1 .and. ncopy+nupdat.gt.0) goto 221
c
c..producer no. (1-99)
c..producer sequence, create mode
c..one producer on archive and cyclic_archive files, create mode
c..time= or time.file_in for create and create+copy
        if(imode(iloop).le.2) then
          if(ngrid.lt.1) goto 219
          do n=numgrd(1,iloop),numgrd(2,iloop)
            if(iprod1.gt.igrid(1,n)) iprod1=igrid(1,n)
            if(iprod2.lt.igrid(1,n)) iprod2=igrid(1,n)
            if(igrid(1,n).lt.1 .or. igrid(1,n).gt.99) goto 222
          end do
          if(imode(iloop).le.1) then
            do n=numgrd(1,iloop),numgrd(2,iloop)
              if(lprod.gt.igrid(1,n)) ierror=1
              if(lprod.lt.igrid(1,n)) lprod=igrid(1,n)
            end do
            if(ierror.ne.0) goto 223
          end if
          if(iarkiv.gt.0 .and. iprod1.ne.iprod2) goto 224
          if(iarkiv.gt.0 .and. ntstep.lt.1) goto 226
        end if
c
        if(imode(iloop).eq.0) ncreat=ncreat+1
        if(imode(iloop).eq.1) ncreco=ncreco+1
        if(imode(iloop).eq.2) ncopy =ncopy +1
        if(imode(iloop).eq.3) nupdat=nupdat+1
        if(imode(iloop).eq.4) nupall=nupall+1
c
        if(imode(iloop).ge.3) goto 180
c
        nsets1=nsets+1
        nih=0
c
        do 150 n=nsets1,maxset
c
          call rcomnt(iuinp,'*>','*',nlines,ierror)
          if(ierror.ne.0) goto 210
c
          lstore=nstore
c
c..time (forecast)
          nlines=nlines+1
          read(iuinp,*,err=211) nt
          if(nt.lt.1) goto 160
          ntim(1,n)=nt
          ntim(2,n)=nstore+1
          n1=nstore+1
          nstore=nstore+nt
          if(nstore.gt.mstore) goto 216
          nlines=nlines+1
          read(iuinp,*,err=211) ((istore(i,j),i=1,2),j=n1,nstore)
c
c..check defined loops (value=identifier)
	  if(ndefloop.gt.0) then
	    call defloop(ntim(1,n),nstore,mstore,istore,
     +			 ndefloop,mdefloop,idefloop,
     +			 nloopdef,mloopdef,iloopdef)
            if(nstore.gt.mstore) goto 235
	  end if
c
          if(iprmin.gt.-32767 .or. iprmax.lt.+32767) then
            n0=n1-1
            nt=0
            do j=n1,nstore
              if(istore(2,j).ge.iprmin .and. istore(2,j).le.iprmax) then
                nt=nt+1
                istore(1,n0+nt)=istore(1,j)
                istore(2,n0+nt)=istore(2,j)
              end if
            end do
            ntim(1,n)=nt
            nstore=n0+nt
          end if
c
c..level
          nlines=nlines+1
          read(iuinp,*,err=211) nl
          if(nl.lt.1) goto 217
          nlev(1,n)=nl
          nlev(2,n)=nstore+1
          n1=nstore+1
          nstore=nstore+nl
          if(nstore.gt.mstore) goto 216
          nlines=nlines+1
          read(iuinp,*,err=211) ((istore(i,j),i=1,2),j=n1,nstore)
c
c..check defined loops (value=identifier)
	  if(ndefloop.gt.0) then
	    call defloop(nlev(1,n),nstore,mstore,istore,
     +			 ndefloop,mdefloop,idefloop,
     +			 nloopdef,mloopdef,iloopdef)
            if(nstore.gt.mstore) goto 235
	  end if
c
c..parameter
          nlines=nlines+1
          read(iuinp,*,err=211) np
          if(np.lt.1) goto 217
          npar(1,n)=np
          npar(2,n)=nstore+1
          n1=nstore+1
          nstore=nstore+np
          if(nstore.gt.mstore) goto 216
          nlines=nlines+1
          read(iuinp,*,err=211) ((istore(i,j),i=1,2),j=n1,nstore)
c
c..check defined loops (value=identifier)
	  if(ndefloop.gt.0) then
	    call defloop(npar(1,n),nstore,mstore,istore,
     +			 ndefloop,mdefloop,idefloop,
     +			 nloopdef,mloopdef,iloopdef)
            if(nstore.gt.mstore) goto 235
	  end if
c
          if(ntim(1,n).eq.0 .or. nlev(1,n).eq.0
     +		            .or. npar(1,n).eq.0) then
            ntim(1,n)=0
            nlev(1,n)=0
            npar(1,n)=0
            nstore=lstore
          end if
c
          nih=nih+ntim(1,n)*nlev(1,n)*npar(1,n)
c
  150   continue
c
        n=maxset+1
        nlines=nlines+1
        read(iuinp,*,err=211) nt
        if(nt.gt.0) goto 218
c
  160   nsets=n-1
        numset(1,iloop)=nsets1
        numset(2,iloop)=nsets
        if(imode(iloop).le.1) then
          ng=numgrd(2,iloop)-numgrd(1,iloop)+1
          newinh=newinh+nih*ng
        end if
c
  180   if(iend.eq.+1) goto 300
c
  100 continue
c
      write(6,*) 'too many description sections.'
      write(6,*) 'max. no. (mloops): ',mloops
      goto 240
  210 write(6,*) 'error reading comment lines.'
      goto 240
  211 write(6,*) 'error reading input.'
      goto 240
  212 write(6,*) 'end of file not o.k.'
      goto 240
  213 write(6,*) 'error in input.  input text:'
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  214 write(6,*) 'option already set.  input text:'
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  215 write(6,*) 'too many grids.   input text:'
      write(6,*) cinput(1:lenstr(cinput,1))
      write(6,*) 'max. no. of grids (maxgrd): ',maxgrd
      goto 240
  216 write(6,*) 'too many time/level/parameter specifications'
      write(6,*) 'max. no. of specifications (mstore): ',mstore
      goto 240
  217 write(6,*) 'no. of levels or parameters less than 1.'
      write(6,*) 'approx. line no. ',nlines
      goto 240
  218 write(6,*) 'too many sets of time/level/parameter specifications'
      write(6,*) 'max. no. of sets (maxset): ',maxset
      goto 240
  219 write(6,*) 'no grids specified.'
      goto 240
  220 write(6,*) 'no input file for create+copy / copy / update'
      goto 240
  221 write(6,*) 'copy or update before create or create+copy is not ok'
      goto 240
  222 write(6,*) 'illegal producer no. found, (legal range is 1 - 99)'
      goto 240
  223 write(6,*) 'create mode. not ok producer sequence, not increasing'
      goto 240
  224 write(6,*) 'archive or cyclic_archive files: only one producer.'
      goto 240
  225 write(6,*) 'create. no time specified.'
      goto 240
  226 write(6,*) 'create. no time step specified for archive',
     *           ' or cyclic_archive.'
      goto 240
  227 write(6,*) 'no output file specified.'
      goto 240
  228 write(6,*) 'felt file type not specified.'
      goto 240
  229 write(6,*) 'mode not specified.'
      goto 240
  230 write(6,*) 'create+copy not allowed for file type archive',
     *           ' and cyclic_archive.'
      goto 240
  231 write(6,*) 'update and update_all not allowed for',
     *           ' file type standard.'
      goto 240
  232 iprhlp=1
      goto 240
  233 write(6,*) 'option only allowed in first ''specification',
     *           ' section'': '
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  234 write(6,*) 'too many specifications of the following type: '
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  235 write(6,*) 'too many time/level/parameter specifications'
      write(6,*) '         when expanding defined loops'
      write(6,*) 'max. no. of specifications (mstore): ',mstore
      goto 240
c
  240 write(6,*) 'error at line no. ',nlines,'   (or below)'
  241 if(iprhlp.eq.1) then
        write(6,*) 'help from ',finput(1:lenstr(finput,1)),' :'
        call prhelp(iuinp,'*=>')
      end if
      close(iuinp)
      goto 981
c
c..finished reading input file
c
  300 continue
      close(iuinp)
      nloops=iloop
c
      if(newinh.gt.32767) then
        write(6,*) 'too many fields specified: ',newinh
        write(6,*) '             max is 32767'
        goto 981
      end if
c
      if(ncreat+ncreco.eq.0) inptim=0
c
      if(nogap.eq.-1) nogap=0
c
c..default is big.endian, yet...
      if(iendian.lt.0) iendian=1
      createswap=.false.
      if(iendian.eq.1 .and. .not.bigendian()) createswap=.true.
      if(iendian.eq.2 .and.      bigendian()) createswap=.true.
c
      write(6,*) 'input o.k.'
c
c-----------------------------------------------------------------------
c
c..open output felt-file
c
      write(6,*) 'output file: ',fileot(1:lenstr(fileot,1))
c
      if(ncreat+ncreco.gt.0) then
c
        call rmfile(fileot,0,ierror)
        open(iunit,file=fileot,
     *             form='unformatted',access='direct',
     *             recl=2048/lrunit,
     *             status='unknown',iostat=ios,err=900)
c
      else
c
c..file exists, check if it was properly updated last time
        call wcfelt(fileot,iunit,iretur)
c
        open(iunit,file=fileot,
     *             form='unformatted',access='direct',
     *             recl=2048/lrunit,
     *             status='old',iostat=ios,err=900)
        swap= swapfile(-iunit)
        read(iunit,rec=1,iostat=ios,err=910) idrec1
        read(iunit,rec=2,iostat=ios,err=910) idrec2
c
        if(swap) then
	  call bswap2(1024,idrec1)
	  call bswap2(1024,idrec2)
        end if
c
        ia=-1
        if(idrec1(1).eq.999) ia=0
        if(idrec1(1).eq.998) ia=1
        if(idrec1(1).eq.997) ia=2
        write(6,*) 'record 1, word  1 - 11 :'
        write(6,fmt='(2x,11i6)') (idrec1(i),i=1,11)
        if(ia.gt.0) then
          write(6,*) 'record 1, word 20 - 28 :'
          write(6,fmt='(2x,11i6)') (idrec1(i),i=20,28)
        end if
        if(ia.eq.-1) then
          write(6,*) 'output felt file type is unknown.'
          write(6,*) 'continue as if type is standard'
          ia=0
        else
          write(6,*) 'output felt file type: ',carkiv(ia)
        end if
        if(ia.ne.iarkiv) then
          write(6,*) 'requested felt file type: ',carkiv(iarkiv)
          close(iunit)
          goto 982
        end if
c
      end if
c
      if(inptim.eq.2) then
        filnam=filein(1)
        write(6,*) 'date/time from: ',filnam(1:lenstr(filnam,1))
        open(iunitc,file=filnam,
     *              form='unformatted',access='direct',
     *              recl=2048/lrunit,
     *              status='old',iostat=ios,err=920)
        read(iunitc,rec=1,iostat=ios,err=930) idfile
	if (swapfile(-iunitc)) call bswap2(32,idfile)
        close(iunitc)
        itimef(1)=idfile(5)
        itimef(2)=idfile(6)/100
        itimef(3)=idfile(6)-(idfile(6)/100)*100
        itimef(4)=idfile(7)/100
        itimef(5)=0
      end if
c
      if(ioffset(1).ne.-32767) then
	itimef(1)=itimef(1)+ioffset(1)
	itimef(2)=itimef(2)+ioffset(2)
	do while (itimef(2).lt.1)
	  itimef(1)=itimef(1)-1
	  itimef(2)=itimef(2)+12
	end do
	do while (itimef(2).gt.12)
	  itimef(1)=itimef(1)+1
	  itimef(2)=itimef(2)-12
	end do
	itimef(5)=ioffset(3)*24+ioffset(4)
	call vtime(itimef,ierror)
      end if
c
      write(6,*) 'date/time:      ',(itimef(i),i=1,4)
c
      if(ncreat+ncreco.gt.0) then
        write(6,*) 'create'
        call create(iarkiv,iunit,nloops,newinh,nogap,lrunit,
     +		    createswap)
      end if
c
      if(ncreco+ncopy.gt.0) then
        write(6,*) 'copy'
        call copdat(iarkiv,iunit,iunitc,nloops,lrunit)
      end if
c
      if(nupdat+nupall.gt.0) then
        write(6,*) 'update'
        call aupdat(iarkiv,iunit,iunitc,nloops,lrunit)
      end if
c
      write(6,*) 'finished.'
      write(6,*) 'record 1, word  1 - 11 :'
      write(6,fmt='(2x,11i6)') (idrec1(i),i=1,11)
      if(iarkiv.ne.0) then
        write(6,*) 'record 1, word 20 - 30 :'
        write(6,fmt='(2x,11i6)') (idrec1(i),i=20,30)
      end if
c
      close(iunit)
      goto 990
c
  900 write(6,*) 'open error.  iostat=',ios,'   output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      goto 982
  910 write(6,*) 'read error.  iostat=',ios,'   output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      close(iunit)
      goto 982
  920 write(6,*) 'open error.  iostat=',ios,'   input file:'
      write(6,*) filnam(1:lenstr(filnam,1))
      close(iunit)
      goto 982
  930 write(6,*) 'read error.  iostat=',ios,'   input file:'
      write(6,*) filnam(1:lenstr(filnam,1))
      close(iunit)
      close(iunitc)
      goto 982
c
  981 continue
ccc   stop 1
      write(6,*) 'nyfelt ***** stop 1 *****'
      call exit(1)
c
  982 continue
ccc   stop 2
      write(6,*) 'nyfelt ***** stop 2 *****'
      call exit(2)
c
  990 continue
c
      end
c
c***********************************************************************
c
      subroutine defloop(num,nstore,mstore,istore,
     +			 ndefloop,mdefloop,idefloop,
     +			 nloopdef,mloopdef,iloopdef)
c
c	add defined loop to time/level/parameter specifications.
c	failure if returning with nstore>mstore.
c
      integer   nstore,mstore,ndefloop,mdefloop,nloopdef,mloopdef
      integer   num(2),idefloop(3,mdefloop+1)
      integer*2 istore(2,mstore),iloopdef(mloopdef+1)
c
      idefloop(1,mdefloop+1)=0
      idefloop(2,mdefloop+1)=1
      idefloop(3,mdefloop+1)=mloopdef+1
c
      i=num(2)-1
c
      do while (i.lt.nstore)
c
	i=i+1
c
	nd1=0
	nd2=0
c..check if value = loop identifier
	do j=1,ndefloop
	  if(istore(1,i).eq.idefloop(1,j)) nd1=j
	  if(istore(2,i).eq.idefloop(1,j)) nd2=j
	end do
c
	if(nd1.gt.0 .or. nd2.gt.0) then
c
c..add dummy if one loop
	  if(nd1.eq.0) then
	    nd1=mdefloop+1
	    iloopdef(mloopdef+1)=istore(1,i)
	  elseif(nd2.eq.0) then
	    nd2=mdefloop+1
	    iloopdef(mloopdef+1)=istore(2,i)
	  end if
c
	  nadd=idefloop(2,nd1)*idefloop(2,nd2)-1
	  ns=nstore
	  nstore=nstore+nadd
c
	  if(nstore.lt.mstore) then
	    if(nadd.gt.0) then
	      do j=ns,i+1,-1
		istore(1,j+nadd)=istore(1,j)
		istore(2,j+nadd)=istore(2,j)
	      end do
	    end if
	    ids1=idefloop(3,nd1)-1
	    ids2=idefloop(3,nd2)-1
	    i=i-1
	    do id1=1,idefloop(2,nd1)
	      do id2=1,idefloop(2,nd2)
		i=i+1
		istore(1,i)=iloopdef(ids1+id1)
		istore(2,i)=iloopdef(ids2+id2)
	      end do
	    end do
	    num(1)=num(1)+nadd
	  end if
c
	end if
c
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine create(iarkiv,iunit,nloops,newinh,nogap,lrunit,
     +			createswap)
c
c  create felt file (without data)
c
      include 'nyfelt.inc'
c
      integer iarkiv,iunit,nloops,newinh,nogap,lrunit
      logical createswap
c
      integer   itimew(5),itime1(5)
      integer*2 iterm(3)
      integer*2 inh(16,ninrec)
c
c
      do i=1,nwrec
        idrec1(i)=0
      end do
      n=nwrec/2
      do i=1,n
        idrec2(1,i)=0
        idrec2(2,i)=0
      end do
c
c..vanlig felt-file (1 dato/termin)
      idrec1(1)=999
c
c..normal arkiv felt-file (flere datoer/terminer)
      if(iarkiv.eq.1) idrec1(1)=998
c
c..sirkul#r arkiv felt-file (flere datoer/terminer)
      if(iarkiv.eq.2) idrec1(1)=997
c
c..store fields with or without data gaps (set switch for wfelt)
      if(nogap.eq.1) idrec1(14)=1
c
      if(createswap) then
        call bswap2(1024,idrec1)
        call bswap2(1024,idrec2)
      end if
c
c..skriver record 1 og 2   ("blank")
      irec=1
      write(iunit,rec=irec,err=910,iostat=ios) idrec1
      irec=2
      write(iunit,rec=irec,err=910,iostat=ios) idrec2
c
      if(createswap) then
        call bswap2(1024,idrec1)
        call bswap2(1024,idrec2)
      end if
c
      do i=1,4
        itimew(i)=itimef(i)
      end do
      itimew(5)=0
c
      ktresol=0
      itresol=0
c
      if(iarkiv.eq.0) then
c..standard file
        ntstep=1
      else
c..archive or cyclic_archive file
        call vtime(itimew,ierror)
        if(ierror.ne.0) then
          write(6,*) 'not correct start date/time:'
          write(6,*) (itimew(i),i=1,4)
          goto 970
        end if
	do i=4,1,-1
	  if(itstep(i).gt.0) then
	    ktresol=i
	    itresol=itstep(i)
	  end if
	end do
        if(iarkiv.eq.2) then
c..cyclic_archive file
c..time series ending at specified date/time.
c..time step cannot be a combination of year, month, day and hour !
c..(see tests in the main program)
          if(itstep(1).gt.0) then
            itimew(1)=itimew(1)-itstep(1)*(ntstep-1)
          elseif(itstep(2).gt.0) then
            do n=2,ntstep
              itimew(2)=itimew(2)-itstep(2)
              if(itimew(2).lt.1) then
                itimew(2)=itimew(2)+12
                itimew(1)=itimew(1)-1
              end if
            end do
          elseif(itstep(3).gt.0) then
            itimew(5)=-24*itstep(3)*(ntstep-1)
            call vtime(itimew,ierror)
          elseif(itstep(4).gt.0) then
            itimew(5)=-itstep(4)*(ntstep-1)
            call vtime(itimew,ierror)
          end if
        end if
      end if
c
      nrec=2+(newinh*ntstep+ninrec-1)/ninrec
      idrec1( 8)=nrec
      idrec1( 9)=nrec
      idrec1(10)=0
      idrec1(11)=newinh*ntstep
      idrec1(12)=0
      idrec1(13)=nwrec
c
      lprod=0
      ireci=2
      ni=0
c
      nwih=0
c
      do i=1,5
        itime1(i)=itimew(i)
      end do
c
      if(iarkiv.ne.0)
     *  write(6,*) 'first date/time: ',(itimew(i),i=1,4)
c
c
c        dato/termin-l@kke
      do 170 ntstp=1,ntstep
c
      if(ntstp.gt.1) then
        if(itstep(1).gt.0) then
          itimew(1)=itimew(1)+itstep(1)
        elseif(itstep(2).gt.0) then
          itimew(2)=itimew(2)+itstep(2)
          if(itimew(2).gt.12) then
            itimew(2)=itimew(2)-12
            itimew(1)=itimew(1)+1
          end if
        elseif(itstep(3).gt.0) then
          itimew(5)=+24*itstep(3)
          call vtime(itimew,ierror)
        elseif(itstep(4).gt.0) then
          itimew(5)=+itstep(4)
          call vtime(itimew,ierror)
        end if
      end if
c
      iterm(1)=itimew(1)
      iterm(2)=itimew(2)*100+itimew(3)
      iterm(3)=itimew(4)*100
c
      do 180 iloop=1,nloops
c
c..jump if not create (0) or create+copy (1)
      if(imode(iloop).gt.1) goto 180
c
      do 185 ng=numgrd(1,iloop),numgrd(2,iloop)
c
      if(igrid(1,ng).ne.lprod) then
        lprod=igrid(1,ng)
        idrec2(1,lprod)=ireci+1
        idrec2(2,lprod)=16*ni+1
      end if
c
c        "sett"-lokke
      do 190 ks=numset(1,iloop),numset(2,iloop)
c
      if(ntim(1,ks).lt.1 .or. nlev(1,ks).lt.1
     *                   .or. npar(1,ks).lt.1) goto 190
c
      nt1=ntim(2,ks)
      nt2=ntim(2,ks)+ntim(1,ks)-1
      nl1=nlev(2,ks)
      nl2=nlev(2,ks)+nlev(1,ks)-1
      np1=npar(2,ks)
      np2=npar(2,ks)+npar(1,ks)-1
c
c        tids-lokke
      do 200 nt=nt1,nt2
c        nivaa-lokke
      do 200 nl=nl1,nl2
c        parameter-lokke
      do 200 np=np1,np2
c
      idrec1(100+lprod)=idrec1(100+lprod)+1
      ni=ni+1
c
      inh( 1,ni)=igrid(1,ng)
      inh( 2,ni)=igrid(2,ng)
      inh( 3,ni)=iterm(1)
      inh( 4,ni)=iterm(2)
      inh( 5,ni)=iterm(3)
      inh( 6,ni)=-32767
      inh( 7,ni)=-32767
      inh( 8,ni)=-32767
      inh( 9,ni)=istore(1,nt)
      inh(10,ni)=istore(2,nt)
      inh(11,ni)=istore(1,np)
      inh(12,ni)=istore(2,np)
      inh(13,ni)=istore(1,nl)
      inh(14,ni)=istore(2,nl)
      inh(15,ni)=igrid(3,ng)
      inh(16,ni)=-32767
c
      if(ni.eq.ninrec) then
c..skriver en record med innh.fort.
	if(createswap) call bswap2(16*ninrec,inh)
        ireci=ireci+1
        irec=ireci
        write(iunit,rec=irec,iostat=ios,err=910) inh
        ni=0
        nwih=nwih+ninrec
      end if
c
  200 continue
c
  190 continue
c
  185 continue
c
  180 continue
c
  170 continue
c
      if(ni.gt.0) then
        do 250 j=ni+1,ninrec
        do 250 i=1,16
          inh(i,j)=-1
  250   continue
	if(createswap) call bswap2(16*ninrec,inh)
        ireci=ireci+1
        irec=ireci
        write(iunit,rec=irec,iostat=ios,err=910) inh
        nwih=nwih+ni
	if(createswap) call bswap2(16*ninrec,inh)
      end if
c
      if(iarkiv.ne.0)
     *  write(6,*) 'last  date/time: ',(itimew(i),i=1,4)
c
      if(ireci.ne.idrec1(9) .or. nwih.ne.idrec1(11)) goto 960
c
c        skriver record 1 og 2
      idrec1(5)=itimew(1)
      idrec1(6)=itimew(2)*100+itimew(3)
      idrec1(7)=itimew(4)*100
c
      if(iarkiv.ne.0) then
        idrec1(20)=itime1(1)
        idrec1(21)=itime1(2)*100+itime1(3)
        idrec1(22)=itime1(4)*100
        idrec1(23)=itimew(1)
        idrec1(24)=itimew(2)*100+itimew(3)
        idrec1(25)=itimew(4)*100
        idrec1(26)=ntstep
        idrec1(27)=newinh
        idrec1(28)=lprod
        idrec1(29)=ktresol
        idrec1(30)=itresol
      end if
c
      if(createswap) then
        call bswap2(1024,idrec1)
        call bswap2(1024,idrec2)
      end if
c
      irec=2
      write(iunit,rec=irec,err=910,iostat=ios) idrec2
      irec=1
      write(iunit,rec=irec,err=910,iostat=ios) idrec1
c
      if(createswap) then
        call bswap2(1024,idrec1)
        call bswap2(1024,idrec2)
      end if
c
      goto 990
c
  910 write(6,*) 'write error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '             record,iostat: ',irec,ios
      goto 970
c
  960 write(6,*) '***************************************************'
      write(6,*) 'PROGRAM ERROR. ireci,idrec1( 9): ',ireci,idrec1( 9)
      write(6,*) '               nwih, idrec1(11): ',nwih, idrec1(11)
      write(6,*) '***************************************************'
      goto 970
c
  970 close(iunit)
ccc   stop 2
      write(6,*) 'nyfelt ***** stop 2 *****'
      call exit(2)
c
  990 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine copdat(iarkiv,iunit,iunitc,nloops,lrunit)
c
c       copy (to existing file)
c
c   notes:
c   1) copy from standard to standard:
c           no problem.
c   2) copy from archive or cyclic_archove to standard:
c           date/time as on the output standard file.
c   3) copy from standard to archive:
c           date/time as on the input standard file.
c   4) copy from standard to cyclic_archive:
c           date/time as on the input standard file,
c           possibly update date/time on the output cyclic_archive file.
c   5) copy from archive or cyclic_archive to archive:
c           date/time as last date/time on the input file.
c   6) copy from archive or cyclic_archive to cyclic_archive:
c           date/time as last date/time on the input file,
c           possibly update date/time on the output cyclic_archive file.
c----------------------------------------------------------------
c
      include 'nyfelt.inc'
c
      integer   ierr(3),infoa(8,3),infoup(8)
      integer   iterm(3)
      integer*2 in(16),idfile(32)
c
c.rfelt/wfelt.......................................................
cc    integer   ihelpr(6),ihelpw(6)
c.rfelt/wfelt.......................................................
c.rfturbo/wfturbo...................................................
      integer   ihelpr(6),ihelpw(10)
      integer*2 idrec1r(nwrec),idrec2r(nwrec),innhr(16,ninrec)
      integer*2 innhw(16,ninrec)
c.rfturbo/wfturbo...................................................
c
      logical   swapfile,swap
c
c
      filnam='*'
c
      do i=1,6
        ihelpr(i)=0
      end do
      do i=1,10
        ihelpw(i)=0
      end do
c
c..swap for output file
      swap= swapfile(-iunit)
c
c        leser record 1 og 2
      irec=1
      read(iunit,rec=irec,iostat=ios,err=910) idrec1
      irec=2
      read(iunit,rec=irec,iostat=ios,err=910) idrec2
c
      if(swap) then
        call bswap2(1024,idrec1)
        call bswap2(1024,idrec2)
      end if
c
c..set update flag (record 1 not updated until file updated)
      idrec1(15)=1
      iwuflag=0
c
      ireci1=3
      ireci2=idrec1(9)
c
      nwrite=0
      iarkin=0
      iupdat=0
      newtrm=0
c
      do 180 iloop=1,nloops
c
c..jump if not creat+copy (1) or  copy (2)
      if(imode(iloop).ne.1 .and. imode(iloop).ne.2) goto 180
c
      if(filnam.ne.filein(iloop)) then
        if(filnam(1:1).ne.'*') close(iunitc)
        ihelpr(1)=0
        filnam=filein(iloop)
        write(6,*) 'copy from file:'
        write(6,*) filnam(1:lenstr(filnam,1))
        open(iunitc,file=filnam,
     *              form='unformatted',access='direct',
     *              recl=2048/lrunit,
     *              status='old',iostat=ios)
        if(ios.eq.0) read(iunitc,rec=1,iostat=ios) idfile
        if(ios.ne.0) then
          write(6,*) 'open/read error. iostat= ',ios,'    no copy.'
          filnam='*'
          goto 180
        end if
	if (swapfile(-iunitc)) call bswap2(32,idfile)
        iarkin=0
        if(idfile(1).eq.998) iarkin=1
        if(idfile(1).eq.997) iarkin=2
        iterm(1)=idfile(5)
        iterm(2)=idfile(6)
        iterm(3)=idfile(7)
        iupdat=0
        insert=0
        if(iarkiv.eq.0) then
          if(iarkin.ne.0) then
            iterm(1)=idrec1(5)
            iterm(2)=idrec1(6)
            iterm(3)=idrec1(7)
          end if
        else
          leave=0
          call ainfo2(iunit,ireci1,ireci2,infoa)
          igt1=0
          igt2=0
          igt3=0
          do i=1,3
            if(igt1.eq.0 .and. iterm(i).lt.infoa(i,1)) igt1=-1
            if(igt1.eq.0 .and. iterm(i).gt.infoa(i,1)) igt1=+1
            if(igt2.eq.0 .and. iterm(i).lt.infoa(i,2)) igt2=-1
            if(igt2.eq.0 .and. iterm(i).gt.infoa(i,2)) igt2=+1
            if(igt3.eq.0 .and. iterm(i).lt.infoa(i,3)) igt3=-1
            if(igt3.eq.0 .and. iterm(i).gt.infoa(i,3)) igt3=+1
          end do
          if(igt1.lt.0) then
            leave=1
          elseif(igt3.le.0) then
            infoup(1)=iterm(1)
            infoup(2)=iterm(2)
            infoup(3)=iterm(3)
            call ainfo1(iunit,ireci1,ireci2,infoup)
            if(infoup(8).eq.0 .and. iarkiv.eq.1) then
              leave=1
            elseif(infoup(8).eq.0) then
              iupdat=1
              insert=1
              if(igt2.lt.0) insert=2
            end if
          elseif(iarkiv.eq.1) then
            leave=1
          else
            iupdat=1
            insert=0
          end if
          if(leave.ne.0) then
            write(6,*) 'input date/time not accepted.'
            write(6,*) '    input file:       ',(iterm(i),i=1,3)
            write(6,*) '    output file from: ',(infoa(i,1),i=1,3)
            write(6,*) '                  to: ',(infoa(i,3),i=1,3)
            goto 180
          end if
          if(iupdat.eq.1) then
            infoup(1)=iterm(1)
            infoup(2)=iterm(2)
            infoup(3)=iterm(3)
            infoup(4)=infoa(4,1)
            infoup(5)=infoa(5,1)
            infoup(6)=infoa(6,1)
            infoup(7)=infoa(7,1)
          end if
        end if
      end if
c
      idater=999
      idatew=999
      if(iarkiv.eq.2) idatew=997
      if(ictime(iloop).eq.0 .and. iarkin.eq.0 .and. iarkiv.eq.0) then
        idater=0
        idatew=0
      end if
c
      iow=ioverw(iloop)
      iut=info(iloop)
c
      do 200 ng=numgrd(1,iloop),numgrd(2,iloop)
c
c        "sett"-lokke
      do 200 ks=numset(1,iloop),numset(2,iloop)
c
      nt1=ntim(2,ks)
      nt2=ntim(2,ks)+ntim(1,ks)-1
      nl1=nlev(2,ks)
      nl2=nlev(2,ks)+nlev(1,ks)-1
      np1=npar(2,ks)
      np2=npar(2,ks)+npar(1,ks)-1
c
c        tids-lokke
      do 200 nt=nt1,nt2
c        nivaa-lokke
      do 200 nl=nl1,nl2
c        parameter-lokke
      do 200 np=np1,np2
c
        in( 1)=igrid(1,ng)
        in( 2)=igrid(2,ng)
        in( 3)=iterm(1)
        in( 4)=iterm(2)
        in( 5)=iterm(3)
        in( 6)=-32767
        in( 7)=-32767
        in( 8)=-32767
        in( 9)=istore(1,nt)
        in(10)=istore(2,nt)
        in(11)=istore(1,np)
        in(12)=istore(2,np)
        in(13)=istore(1,nl)
        in(14)=istore(2,nl)
        in(15)=igrid(3,ng)
        in(16)=-32767
c
c        felt skal kopieres (hvis det finnes)
        ierr(2)=idater
c.rfelt................................................................
cc      call rfelt(iunitc,ip,in,idata,limit,ierr,ihelpr)
c.rfelt................................................................
c.rfturbo..............................................................
        call rfturbo(iunitc,ip,in,idata,limit,ierr,ihelpr,
     *               idrec1r,idrec2r,innhr)
c.rfturbo..............................................................
        if(ip.ne.1 .and. iut.ne.0) then
          if(idater.eq.0) then
            write(6,1020) ip,ierr,(in(i),i=1,2),(in(i),i=9,14)
 1020       format(' field not found.  ip=',i4,'  ierr=',3i9,
     *             /,6x,'in(1-2,9-14):',8i7)
          else
            write(6,1021) ip,ierr,(in(i),i=1,5),(in(i),i=9,14)
 1021       format(' field not found.  ip=',i4,'  ierr=',3i9,
     *             /,6x,'in(1-5): ',5i7,/,6x,'in(9-14):',6i7)
          end if
        elseif(ip.eq.1) then
          if(iupdat.eq.1) then
c.wfturbo..............................................................
            if(ihelpw(1).gt.0) then
c..write last innh back to file (if it is updated)
              ihelpw(7)=1
              call wfturbo(iunit,ip,in,idrec1,idrec2,ldata,idata,
     *                     ierr,ihelpw,innhw)
              if(ip.ne.1) then
                write(6,1025) ip,ierr
 1025           format(' file update error. ip=',i4,'  ierr=',3i9)
              end if
            end if
c.wfturbo..............................................................
            call arcset(iunit,infoup)
c..update parts of record 1. will be updated even if no fields written
            if(insert.eq.1) then
c..insert date/time between the existing second and last
              idrec1(20)=infoa(1,2)
              idrec1(21)=infoa(2,2)
              idrec1(22)=infoa(3,2)
            elseif(insert.eq.2) then
c..insert date/time between the existing first and second
              idrec1(20)=infoup(1)
              idrec1(21)=infoup(2)
              idrec1(22)=infoup(3)
            else
c..new date/time after the existing last
              idrec1( 5)=infoup(1)
              idrec1( 6)=infoup(2)
              idrec1( 7)=infoup(3)
              idrec1(20)=infoa(1,2)
              idrec1(21)=infoa(2,2)
              idrec1(22)=infoa(3,2)
              idrec1(23)=infoup(1)
              idrec1(24)=infoup(2)
              idrec1(25)=infoup(3)
            end if
c..update record 1
	    if(swap) call bswap2(1024,idrec1)
            irec=1
            write(iunit,rec=irec,iostat=ios,err=920) idrec1
	    if(swap) call bswap2(1024,idrec1)
            iwuflag=1
            newtrm=1
            iupdat=0
          end if
          ierr(1)=iow
          ierr(2)=idatew
          ierr(3)=0
          ldata1=in( 8)
          ldata2=in(16)/100
          ldata=ldata1+ldata2*32767
c.wfelt................................................................
cc        call wfelt(iunit,ip,in,idrec1,idrec2,ldata,idata,ierr,ihelpw)
c.wfelt................................................................
c.wfturbo..............................................................
          call wfturbo(iunit,ip,in,idrec1,idrec2,ldata,idata,
     *                 ierr,ihelpw,innhw)
c.wfturbo..............................................................
          if(ip.eq.1) then
            nwrite=nwrite+1
            if(iwuflag.eq.0) then
c..update record 1 (set update flag)
	      if(swap) call bswap2(1024,idrec1)
              irec=1
              write(iunit,rec=irec,iostat=ios,err=920) idrec1
	      if(swap) call bswap2(1024,idrec1)
              iwuflag=1
            end if
          elseif(iut.ne.0) then
            if(idatew.eq.0) then
              write(6,1030) ip,ierr,(in(i),i=1,2),(in(i),i=9,15)
 1030         format(' field not written.  ip=',i4,'  ierr=',3i9,
     *               /,4x,'in(1-2,9-15):',9i6)
            else
              write(6,1031) ip,ierr,(in(i),i=1,5),(in(i),i=9,15)
 1031         format(' field not written.  ip=',i4,'  ierr=',3i9,
     *               /,4x,'in(1-5): ',5i7,/,4x,'in(9-15):',7i7)
            end if
          end if
        end if
c
  200 continue
c
  180 continue
c
      if(filnam(1:1).ne.'*') close(iunitc)
      filnam='*'
c
      if(iut.eq.1) then
        if(nwrite.eq.0) write(6,*) 'no new data written. '
        if(newtrm.eq.1) write(6,*) 'date/time is updated.'
      end if
c
c.wfturbo..............................................................
      if(ihelpw(1).gt.0) then
c..write last innh back to file (if it is updated)
        ihelpw(7)=1
        call wfturbo(iunit,ip,in,idrec1,idrec2,ldata,idata,
     *               ierr,ihelpw,innhw)
        if(ip.ne.1) then
          write(6,1035) ip,ierr
 1035     format(' file update error. ip=',i4,'  ierr=',3i9)
        end if
      end if
c.wfturbo..............................................................
c
      if(nwrite.gt.0 .or. newtrm.eq.1) then
c..update record 1.
        call daytim(iyear,month,iday,ihour,minut,isecn)
        idrec1(2)=iyear
        idrec1(3)=month*100+iday
        idrec1(4)=ihour*100+minut
c..reset update flag
        idrec1(15)=0
	if(swap) call bswap2(1024,idrec1)
        irec=1
        write(iunit,rec=irec,iostat=ios,err=920) idrec1
	if(swap) call bswap2(1024,idrec1)
      end if
c
      goto 990
c
  910 write(6,*) 'read error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '            record,iostat: ',irec,ios
      goto 970
c
  920 write(6,*) 'write error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '             record,iostat: ',irec,ios
      goto 970
c
  970 close(iunit)
      if(filnam(1:1).ne.'*') close(iunitc)
ccc   stop 2
      write(6,*) 'nyfelt ***** stop 2 *****'
      call exit(2)
c
  990 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine aupdat(iarkiv,iunit,iunitc,nloops,lrunit)
c
c     update and update_all
c
c        normal arkiv og sirkul{r arkiv felt-file:
c        benytter eksisterende innholds-fortegnelse for } kopiere
c        inn data fra annen felt-file.
c
c        update:     en dato/termin
c        update_all: alle datoer/terminer (f.eks. ved forandring
c                    av innholds-fortegnelse => m} fºrst lage ny
c                    file uten data)
c                    nb! for sirkul{r arkiv felt-file forandres ikke
c                        datoer/terminer i innholds-fortegnelsen.
c
c        nb! forutsetter at det bare er en produsent p} filen
c            og at hver dato/termin ligger samlet i
c            innholds-fortegnelsen.
c----------------------------------------------------------------
c
      include 'nyfelt.inc'
c
      integer   ierr(3),infoa(8,3),infoup(8)
      integer   iterm(3)
      integer*2 in(16),idfile(32),inh(16,ninrec)
c
c.rfelt/wfelt.......................................................
cc    integer   ihelpr(6),ihelpw(6)
c.rfelt/wfelt.......................................................
c.rfturbo/wfturbo...................................................
      integer   ihelpr(6),ihelpw(10)
      integer*2 idrec1r(nwrec),idrec2r(nwrec),innhr(16,ninrec)
      integer*2 innhw(16,ninrec)
c.rfturbo/wfturbo...................................................
c
      logical   swapfile,swap
c
c
      filnam='*'
c
      do i=1,6
        ihelpr(i)=0
      end do
      do i=1,10
        ihelpw(i)=0
      end do
c
c..swap for output file
      swap= swapfile(iunit)
c
c..leser record 1 og 2
      irec=1
      read(iunit,rec=irec,iostat=ios,err=910) idrec1
      irec=2
      read(iunit,rec=irec,iostat=ios,err=910) idrec2
c
      if(swap) then
        call bswap2(1024,idrec1)
        call bswap2(1024,idrec2)
      end if
c
c..set update flag (record 1 not updated until file updated)
      idrec1(15)=1
      iwuflag=0
c
      irec1a=3
      irec2a=idrec1(9)
      ninh1a=1
      ninh2a=idrec1(11)-(irec2a-irec1a)*ninrec
c
      nwrite=0
      iupdat=0
      newtrm=0
c
      do 180 iloop=1,nloops
c
c..jump if not update (3) or update_all (4)
      if(imode(iloop).ne.3 .and. imode(iloop).ne.4) goto 180
c
      if(filnam.ne.filein(iloop)) then
        if(filnam(1:1).ne.'*') close(iunitc)
        ihelpr(1)=0
        filnam=filein(iloop)
        write(6,*) 'update. copy from file:'
        write(6,*) filnam(1:lenstr(filnam,1))
        open(iunitc,file=filnam,
     *              form='unformatted',access='direct',
     *              recl=2048/lrunit,
     *              status='old',iostat=ios)
        if(ios.eq.0) read(iunitc,rec=1,iostat=ios) idfile
        if(ios.ne.0) then
          write(6,*) 'open/read error. iostat= ',ios,'    no update.'
          filnam='*'
          goto 180
        end if
	if (swapfile(-iunitc)) call bswap2(32,idfile)
        iterm(1)=idfile(5)
        iterm(2)=idfile(6)
        iterm(3)=idfile(7)
      end if
c
      iupdat=0
      insert=0
c
      if(imode(iloop).eq.4) then
        irec1=irec1a
        ninh1=ninh1a
        irec2=irec2a
        ninh2=ninh2a
      else
        leave=0
        call ainfo2(iunit,irec1a,irec2a,infoa)
        igt1=0
        igt2=0
        igt3=0
        do i=1,3
          if(igt1.eq.0 .and. iterm(i).lt.infoa(i,1)) igt1=-1
          if(igt1.eq.0 .and. iterm(i).gt.infoa(i,1)) igt1=+1
          if(igt2.eq.0 .and. iterm(i).lt.infoa(i,2)) igt2=-1
          if(igt2.eq.0 .and. iterm(i).gt.infoa(i,2)) igt2=+1
          if(igt3.eq.0 .and. iterm(i).lt.infoa(i,3)) igt3=-1
          if(igt3.eq.0 .and. iterm(i).gt.infoa(i,3)) igt3=+1
        end do
        if(igt1.lt.0) then
          leave=1
        elseif(igt3.le.0) then
          infoup(1)=iterm(1)
          infoup(2)=iterm(2)
          infoup(3)=iterm(3)
          call ainfo1(iunit,irec1a,irec2a,infoup)
          if(infoup(8).eq.0 .and. iarkiv.eq.1) then
            leave=1
          elseif(infoup(8).eq.0) then
            iupdat=1
            insert=1
            if(igt2.lt.0) insert=2
          end if
        elseif(iarkiv.eq.1) then
          leave=1
        else
          iupdat=1
          insert=0
        end if
        if(leave.ne.0) then
          write(6,*) 'input date/time not accepted.'
          write(6,*) '    input file:       ',(iterm(i),i=1,3)
          write(6,*) '    output file from: ',(infoa(i,1),i=1,3)
          write(6,*) '                  to: ',(infoa(i,3),i=1,3)
          goto 180
        end if
        if(iupdat.eq.1) then
          infoup(1)=iterm(1)
          infoup(2)=iterm(2)
          infoup(3)=iterm(3)
          infoup(4)=infoa(4,1)
          infoup(5)=infoa(5,1)
          infoup(6)=infoa(6,1)
          infoup(7)=infoa(7,1)
        end if
        irec1=infoup(4)
        ninh1=infoup(5)
        irec2=infoup(6)
        ninh2=infoup(7)
      end if
c
      idater=999
      idatew=999
      if(iarkiv.eq.2) idatew=997
c
      iow=ioverw(iloop)
      iut=info(iloop)
c
      do 200 ireci=irec1,irec2
c
	irec=ireci
        read(iunit,rec=irec,iostat=ios,err=910) inh
c
	if(swap) call bswap2(16*ninrec,inh)
c
        n1=1
        n2=ninrec
        if(ireci.eq.irec1) n1=ninh1
        if(ireci.eq.irec2) n2=ninh2
c
        if(iupdat.eq.1) then
c..update date/time of input until arcset has updated the file
          do n=n1,n2
            inh(3,n)=iterm(1)
            inh(4,n)=iterm(2)
            inh(5,n)=iterm(3)
          end do
        end if
c
        do 210 n=n1,n2
c
c..felt skal kopieres (hvis det finnes)
          do i=1,16
            in(i)=inh(i,n)
          end do
c
c..solve a problem with (minor) changes in forecast model, eta coord.
	  if(in(11).eq.10) in(14)=-32767
c
          ierr(2)=idater
c.rfelt................................................................
cc        call rfelt(iunitc,ip,in,idata,limit,ierr,ihelpr)
c.rfelt................................................................
c.rfturbo..............................................................
          call rfturbo(iunitc,ip,in,idata,limit,ierr,ihelpr,
     *                 idrec1r,idrec2r,innhr)
c.rfturbo..............................................................
          if(ip.ne.1 .and. iut.ne.0) then
            if(idater.eq.0) then
              write(6,1020) ip,ierr,(in(i),i=1,2),(in(i),i=9,14)
 1020         format(' field not found.  ip=',i4,'  ierr=',3i9,
     *               /,6x,'in(1-2,9-14):',8i7)
            else
              write(6,1021) ip,ierr,(in(i),i=1,5),(in(i),i=9,14)
 1021         format(' field not found.  ip=',i4,'  ierr=',3i9,
     *               /,6x,'in(1-5): ',5i7,/,6x,'in(9-14):',6i7)
            end if
          elseif(ip.eq.1) then
            if(iupdat.eq.1) then
c.wfturbo..............................................................
              if(ihelpw(1).gt.0) then
c..write last innh back to file (if it is updated)
                ihelpw(7)=1
                call wfturbo(iunit,ip,in,idrec1,idrec2,ldata,idata,
     *                       ierr,ihelpw,innhw)
                if(ip.ne.1) then
                  write(6,1025) ip,ierr
 1025             format(' file update error. ip=',i4,'  ierr=',3i9)
                end if
              end if
c.wfturbo..............................................................
              call arcset(iunit,infoup)
c..update parts of record 1. will be updated even if no fields written
              if(insert.eq.1) then
c..insert date/time between the existing second and last
                idrec1(20)=infoa(1,2)
                idrec1(21)=infoa(2,2)
                idrec1(22)=infoa(3,2)
              elseif(insert.eq.2) then
c..insert date/time between the existing first and second
                idrec1(20)=infoup(1)
                idrec1(21)=infoup(2)
                idrec1(22)=infoup(3)
              else
c..new date/time after the existing last
                idrec1( 5)=infoup(1)
                idrec1( 6)=infoup(2)
                idrec1( 7)=infoup(3)
                idrec1(20)=infoa(1,2)
                idrec1(21)=infoa(2,2)
                idrec1(22)=infoa(3,2)
                idrec1(23)=infoup(1)
                idrec1(24)=infoup(2)
                idrec1(25)=infoup(3)
              end if
c..update record 1
	      if(swap) call bswap2(1024,idrec1)
              irec=1
              write(iunit,rec=irec,iostat=ios,err=920) idrec1
	      if(swap) call bswap2(1024,idrec1)
              iwuflag=1
              newtrm=1
              iupdat=0
            end if
            ierr(1)=iow
            ierr(2)=idatew
            ierr(3)=0
            ldata1=in( 8)
            ldata2=in(16)/100
            ldata=ldata1+ldata2*32767
c.wfelt................................................................
cc          call wfelt(iunit,ip,in,idrec1,idrec2,ldata,idata,
cc   *                 ierr,ihelpw)
c.wfelt................................................................
c.wfturbo..............................................................
            call wfturbo(iunit,ip,in,idrec1,idrec2,ldata,idata,
     *                   ierr,ihelpw,innhw)
c.wfturbo..............................................................
            if(ip.eq.1) then
              nwrite=nwrite+1
              if(iwuflag.eq.0) then
c..update record 1 (set update flag)
	        if(swap) call bswap2(1024,idrec1)
                irec=1
                write(iunit,rec=irec,iostat=ios,err=920) idrec1
	        if(swap) call bswap2(1024,idrec1)
                iwuflag=1
              end if
            elseif(iut.ne.0) then
              if(idatew.eq.0) then
                write(6,1030) ip,ierr,(in(i),i=1,2),(in(i),i=9,15)
 1030           format(' field not written.  ip=',i4,'  ierr=',3i9,
     *                 /,4x,'in(1-2,9-15):',9i6)
              else
                write(6,1031) ip,ierr,(in(i),i=1,5),(in(i),i=9,15)
 1031            format(' field not written.  ip=',i4,'  ierr=',3i9,
     *                  /,4x,'in(1-5): ',5i7,/,4x,'in(9-15):',7i7)
              end if
            end if
          end if
c
  210   continue
c
  200 continue
c
  180 continue
c
      if(filnam(1:1).ne.'*') close(iunitc)
      filnam='*'
c
      if(iut.eq.1) then
        if(nwrite.eq.0) write(6,*) 'no new data written. '
        if(newtrm.eq.1) write(6,*) 'date/time is updated.'
      end if
c
c.wfturbo..............................................................
      if(ihelpw(1).gt.0) then
c..write last innh back to file (if it is updated)
        ihelpw(7)=1
        call wfturbo(iunit,ip,in,idrec1,idrec2,ldata,idata,
     *               ierr,ihelpw,innhw)
        if(ip.ne.1) then
          write(6,1035) ip,ierr
 1035     format(' file update error. ip=',i4,'  ierr=',3i9)
        end if
      end if
c.wfturbo..............................................................
c
      if(nwrite.gt.0 .or. newtrm.eq.1) then
c..update record 1.
        call daytim(iyear,month,iday,ihour,minut,isecn)
        idrec1(2)=iyear
        idrec1(3)=month*100+iday
        idrec1(4)=ihour*100+minut
c..reset update flag
        idrec1(15)=0
	if(swap) call bswap2(1024,idrec1)
        irec=1
        write(iunit,rec=irec,iostat=ios,err=920) idrec1
	if(swap) call bswap2(1024,idrec1)
      end if
c
      goto 990
c
  910 write(6,*) 'read error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '            record,iostat: ',irec,ios
      goto 970
c
  920 write(6,*) 'write error. output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
      write(6,*) '             record,iostat: ',irec,ios
      goto 970
c
  970 close(iunit)
      if(filnam(1:1).ne.'*') close(iunitc)
ccc   stop 2
      write(6,*) 'nyfelt ***** stop 2 *****'
      call exit(2)
c
  990 continue
c
      return
      end
c
c***********************************************************************
c
      subroutine ainfo1(iunit,irec1,irec2,infoup)
c
c        normal arkiv og sirkul{r arkiv felt-file:
c        finn peker til innholds-fortegnelse for en dato/termin.
c
c        nb! forutsetter at det bare er en produsent p} filen
c            og at hver dato/termin ligger samlet i
c            innholds-fortegnelsen.
c
c
      include 'nyfelt.inc'
c
      integer   iunit,irec1,irec2,infoup(8)
c
      integer*2 iyy,imd,ihm,lyy,lmd,lhm,inh(16,ninrec)
      logical   swapfile,swap
c
      swap= swapfile(iunit)
c
      iyy=infoup(1)
      imd=infoup(2)
      ihm=infoup(3)
      infoup(4)=0
      infoup(5)=0
      infoup(6)=0
      infoup(7)=0
      infoup(8)=0
      lyy=-32767
      lmd=-32767
      lhm=-32767
      nr=0
      nn=0
      ni=0
      nf=0
c
      do 100 irec=irec1,irec2
c
        read(iunit,rec=irec,iostat=ios,err=900) inh
c
	if(swap) call bswap2(16*ninrec,inh)
c
        do 110 n=1,ninrec
          if(inh(3,n).eq.lyy .and. inh(4,n).eq.lmd
     *                       .and. inh(5,n).eq.lhm) then
            nr=irec
            nn=n
            ni=ni+1
          else
            lyy=inh(3,n)
            lmd=inh(4,n)
            lhm=inh(5,n)
            if(nf.eq.0 .and.
     *         inh(1,n).gt.0   .and. inh(1,n).lt.100 .and.
     *         inh(3,n).eq.iyy .and. inh(4,n).eq.imd
     *                         .and. inh(5,n).eq.ihm) then
              infoup(4)=irec
              infoup(5)=n
	      nr=irec
	      nn=n
              ni=1
              nf=1
            elseif(nf.eq.1) then
              infoup(6)=nr
              infoup(7)=nn
              infoup(8)=ni
              nf=2
            end if
          end if
  110   continue
c
        if(nf.eq.2) goto 180
c
  100 continue
c
  180 if(nf.eq.1) then
        infoup(6)=nr
        infoup(7)=nn
        infoup(8)=ni
      end if
c
      return
c
  900 write(6,*) ' ** ainfo1 ** read error. output file.'
      write(6,*) ' **           record,iostat: ',irec,ios
      close(iunit)
ccc   stop 3
      write(6,*) 'nyfelt ***** stop 3 *****'
      call exit(3)
c
      end
c
c***********************************************************************
c
      subroutine ainfo2(iunit,irec1,irec2,infoa)
c
c        sirkul{r arkiv felt-file:
c        finn fºrste, andre og siste dato/termin og pekere til
c        innholds-fortegnelse for disse.
c
c        nb! forutsetter at det bare er en produsent p} filen
c            og at hver dato/termin ligger samlet i
c            innholds-fortegnelsen.
c
c
      include 'nyfelt.inc'
c
      integer   iunit,irec1,irec2,infoa(8,3)
c
      integer   it(8,4),icount(4)
      integer*2 inh(16,ninrec)
      logical   swapfile,swap
c
      swap= swapfile(iunit)
c
      do 20 i=1,3
        it(i,1)=+32767
        it(i,2)=+32767
        it(i,3)=-32767
        it(i,4)=-32767
   20 continue
      do 25 n=1,4
      icount(n)=0
      do 25 i=4,8
        it(i,n)=0
   25 continue
c
      lyy=-32767
      lmd=-32767
      lhm=-32767
      nt=4
c
      do 100 irec=irec1,irec2
c
        read(iunit,rec=irec,iostat=ios,err=900) inh
c
	if(swap) call bswap2(16*ninrec,inh)
c
        do 110 n=1,ninrec
          if(inh(3,n).eq.lyy .and. inh(4,n).eq.lmd
     *                       .and. inh(5,n).eq.lhm) then
            it(6,nt)=irec
            it(7,nt)=n
            it(8,nt)=it(8,nt)+1
          elseif(inh(1,n).gt.0 .and. inh(1,n).lt.100) then
            lyy=inh(3,n)
            lmd=inh(4,n)
            lhm=inh(5,n)
            if( lyy.lt.it(1,1) .or.
     *         (lyy.eq.it(1,1) .and. lmd.lt.it(2,1)) .or.
     *         (lyy.eq.it(1,1) .and. lmd.eq.it(2,1)
     *                         .and. lhm.lt.it(3,1)) ) then
              nt=1
              if(icount(1).gt.0) then
                if(icount(2).gt.0 .and. icount(3).eq.0) then
                  do 121 i=1,8
                    it(i,3)=it(i,2)
  121             continue
                  icount(3)=icount(3)+1
                end if
                do 122 i=1,8
                  it(i,2)=it(i,1)
  122           continue
                icount(2)=icount(2)+1
              end if
            elseif( lyy.lt.it(1,2) .or.
     *             (lyy.eq.it(1,2) .and. lmd.lt.it(2,2)) .or.
     *             (lyy.eq.it(1,2) .and. lmd.eq.it(2,2)
     *                             .and. lhm.lt.it(3,2)) ) then
              nt=2
              if(icount(2).gt.0 .and. icount(3).eq.0) then
                do 123 i=1,8
                  it(i,3)=it(i,2)
  123           continue
                icount(3)=icount(3)+1
              end if
            elseif( lyy.gt.it(1,3) .or.
     *             (lyy.eq.it(1,3) .and. lmd.gt.it(2,3)) .or.
     *             (lyy.eq.it(1,3) .and. lmd.eq.it(2,3)
     *                             .and. lhm.gt.it(3,3)) ) then
              nt=3
            else
              nt=4
            end if
            it(1,nt)=lyy
            it(2,nt)=lmd
            it(3,nt)=lhm
            it(4,nt)=irec
            it(5,nt)=n
            it(6,nt)=irec
            it(7,nt)=n
            it(8,nt)=1
            icount(nt)=icount(nt)+1
          end if
c
  110   continue
c
  100 continue
c
      do 180 n=1,3
      k=n
      if(icount(n).eq.0 .and. n.gt.1) k=n-1
      do 180 i=1,8
        infoa(i,n)=it(i,k)
  180 continue
c
      return
c
  900 write(6,*) ' ** ainfo2 ** read error. output file.'
      write(6,*) ' **           record,iostat: ',irec,ios
      close(iunit)
ccc   stop 3
      write(6,*) 'nyfelt ***** stop 3 *****'
      call exit(3)
c
      end
c
c***********************************************************************
c
      subroutine arcset(iunit,infoup)
c
c        sirkul{r arkiv felt-file:
c        oppdater innholds-fortegnelse med ny dato/termin.
c        setter data-pekere slik at wfelt kan benytte
c        evt. avsatt plass. record 1 oppdateres ikke her.
c
c        nb! forutsetter at det bare er en produsent p} filen
c            og at hver dato/termin ligger samlet i
c            innholds-fortegnelsen.
c
c
      include 'nyfelt.inc'
c
      integer   iunit,infoup(7)
c
      integer*2 iyy,imd,ihm,inh(16,ninrec)
      logical   swapfile,swap
c
      swap= swapfile(iunit)
c
      iyy  =infoup(1)
      imd  =infoup(2)
      ihm  =infoup(3)
      irec1=infoup(4)
      ninh1=infoup(5)
      irec2=infoup(6)
      ninh2=infoup(7)
c
      do 100 irec=irec1,irec2
c
        read(iunit,rec=irec,iostat=ios,err=900) inh
c
        if(swap) call bswap2(16*ninrec,inh)
c
        n1=1
        n2=ninrec
        if(irec.eq.irec1) n1=ninh1
        if(irec.eq.irec2) n2=ninh2
c
        do 110 n=n1,n2
          inh(3,n)=iyy
          inh(4,n)=imd
          inh(5,n)=ihm
          if(inh(6,n).gt.0 .and. inh(8,n).gt.0) then
c..handle 'old' files well.......................................
            if(inh( 7,n).lt.1) inh( 7,n)=1
            if(inh(16,n).lt.0) inh(16,n)=0
c................................................................
            inh( 6,n)=-inh( 6,n)
            inh( 7,n)=-inh( 7,n)
            inh( 8,n)=-inh( 8,n)
            inh(16,n)=-inh(16,n)
c..handle 'old' files well.......................................
          elseif(inh(6,n).lt.0 .and. inh(6,n).ne.-32767 .and.
     *           inh(8,n).lt.0 .and. inh(8,n).ne.-32767) then
            if(inh( 7,n).ge.0) inh( 7,n)=-1
            if(inh(16,n).gt.0) inh(16,n)= 0
          else
            inh( 6,n)=-32767
            inh( 7,n)=-32767
            inh( 8,n)=-32767
            inh(16,n)=-32767
c................................................................
          end if
c
c..solve a problem with (minor) changes in forecast model, eta coord.
	  if (inh(11,n).eq.10) inh(14,n)=-32767
c
  110   continue
c
        if(swap) call bswap2(16*ninrec,inh)
c
        write(iunit,rec=irec,iostat=ios,err=910) inh
c
  100 continue
c
      return
c
  900 write(6,*) ' ** arcset ** read error. output file.'
      write(6,*) ' **           record,iostat: ',irec,ios
      goto 980
c
  910 write(6,*) ' ** arcset ** write error. output file.'
      write(6,*) ' **           record,iostat: ',irec,ios
      goto 980
c
  980 continue
      close(iunit)
ccc   stop 3
      write(6,*) 'nyfelt ***** stop 3 *****'
      call exit(3)
c
      end
