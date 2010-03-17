      program copflt
c
c  COPFLT is based on NYFELT (with the option 'CREATE+COPY').
c
c  Extensions: Selection of subgrid and/or rotation (n*90 degrees).
c              Interpolation (bilinear or bessel) to a more different
c              grid, undefined values input and/or output (part of
c              output grid can be outside input grid).
c              A limited number of computations.
c              Possibility to change field identification.
c              Packing routines for field distribution.
c              Options for rescaling fields (lower data precision).
c
c  Warning:    The program uses "open(....,form='system',...)" when
c              opening packed output files (binary files without any
c              extra bytes before and after each record).
c
c  Specifications for COPFLT is given in a 'copflt.input' file when
c  running the program. This file is given to the program as a
c  command line argument, e.g.:
c      copflt copflt.input
c  or with a more describing file name:
c      copflt copflt.lam50s
c  or print help lines in the 'copflt.input' file (if such exists):
c      copflt copflt.lam50s ?
c
c  COPFLT.INPUT:
c  -------------
c  Consists of two sections (which may be repeated):
c    1) General specifications
c       (file name, input grid, output grid etc.)
c    2) Field identifications
c       (forecast_lengths, levels, parameters)
c
c  Comment lines are expected to be found before each section and also
c  between each 'loop' in the 'field identification' section.
c  Comment lines start with a * in column one. The last comment line
c  before 'real' program input has *> in the two first columns.
c  Help lines are identified with *=> in the three first columns.
c
c  General specification section:
c     Consists of keywords with and without options.
c     There are three ways to use variables here, instead of
c     editing the 'copflt.input' file:
c        - environment variables,  starting with a $ sign
c        - command line arguments, starting with a # sign and followed
c          the argument no., remember that argument no. 1 (#1) always
c          is the name of 'copflt.input'
c
c++++++++ more documentation later +++++++++++++++++++++++
c
c  WARNING: New packing (packing.version.2) added here and in the
c           unix unpacking program funpack. The new features are
c	    large fields, extra geometry identification and
c	    bitmap of undefined values (used if it is space saving).
c	    The PC unpacking (DNMI unpack.exe) is howerver
c	    not updated (so the version 2 packing is default off for
c	    format.pack_pc). 
c
c-----------------------------------------------------------------------
c EXAMPLE.  Copy part (the old LAM50 area) of some LAM50S fields
c=======================================================================
c *** copflt.lam50  ('copflt.input')
c ***
c *=> Copy LAM50S fields to the old LAM50 grid (1501)
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    copflt  copflt.lam50  fltsXX.dat flt1501.dat
c *=>                          <input>    <output>
c ***
c ***----------------------------------------------------------------
c **
c ** Option list:
c **-------------
c ** FILE=<output_FELT_file>
c ** FORMAT.STANDARD .............................. (default)
c ** FORMAT.PC .................................... (PC/DOS)
c ** FORMAT.SWAP .................................. (swap bytes)
c ** FORMAT.NORD .................................. (old NORD format)
c ** FORMAT.PACK .................................. (packed format)
c ** FORMAT.PACK_PC ............................... (PC/DOS packed format)
c ** FORMAT.PACK_SWAP ............................. (packed+byte_swap)
c ** FORMAT.SEQUENTIAL ............................ (ident+ifelt+ident+)
c ** CREATE.MACHINE.ENDIAN ........................ (default)
c ** CREATE.BIG.ENDIAN ............................ (only STANDARD/SWAP)
c ** CREATE.LITTLE.ENDIAN
c ** DATA_GAPS.ON ................................. (default)
c ** DATA_GAPS.OFF ................................ (only for STANDARD)
c ** FILE_IN=<input_FELT_file>
c ** GRID=<producer,grid,grid_type> ............... (input)
c ** GRID.UNCHANGED_OUT ........................... (default)
c ** GRID_OUT=<grid,ix1,ix2,iy1,iy2,istep,irot>
c ** GRID.INTERP_OUT=<grid,nx,ny,xp,yp,an,fi>
c ** GRID.MEAN_OUT=<grid,ix1,ix2,iy1,iy2,istep,irot>
c ** GEO.GRID_OUT=<grid,nlong,nlat,glong1,glat1,delta_long,delta_lat>
c ** PACKING.VERSION.2.ON ...................... (default for pack, pack_swap)
c ** PACKING.VERSION.2.OFF ..................... (default for pack_pc)
c ** MAX.OUTPUT.PACK.FIELD.SIZE= 32767 ............ (default as first field)
c ** CHANGE_IDENT=<ident_no,input_value,output_value> (ident_no 1-20)
c ** CHANGE_IDENT_A=<v.coord,param,lvl1,lvl2, v.coord,param,lvl1,lvl2>
c ** INTERP.BILINEAR .............................. (default)
c ** INTERP.BESSEL
c ** INTERP.NEAREST.GRID.POINT
c ** INTERP.BILINEAR_PARAM=<parmeter_number(s)> ... (not as standard)
c ** INTERP.BESSEL_PARAM=<parmeter_number(s)>   ... (not as standard)
c ** INTERP.NEAREST_PARAM=<parmeter_number(s)>  ... (not as standard)
c ** X.VECTOR_PARAM=<parmeter_number(s)> ..... u(x); wind,current etc.
c ** Y.VECTOR_PARAM=<parmeter_number(s)> ..... v(y); wind,current etc.
c ** COMPUTE_T(K)->T(C)=<param_in,param_out>
c ** COMPUTE_T(C)->T(K)=<param_in,param_out>
c ** COMPUTE_PRESSURE.TH->T(K)=<param_in,param_out>
c ** COMPUTE_PRESSURE.TH->T(C)=<param_in,param_out>
c ** SIGMA.PS=<vert.coord.,parameter,level_1,level_2>
c ** COMPUTE_SIGMA.TH->T(K)=<param_in,param_out>
c ** COMPUTE_SIGMA.TH->T(C)=<param_in,param_out>
c ** ADD_PARAM/REL.HOUR=<param, nparam,param(s)_in, nhours,rel.hour(s)>
c ** TOT.ACCUM.TO.STEP=<param, param_in, hours.step>
c ** PACK_RESCALE=<v.coord,param,lvl1_min,lvl1_max,iscale,idiv>
c ** PACK_DIRECTION_PARAM=<param(s)>
c ** PACK_NOT_PARAM=<param(s)>
c ** INFO.ON  .................................... (default)
c ** INFO.OFF
c ** CHECK_TIME.ON  .............................. (default)
c ** CHECK_TIME.OFF
c ** PROG_LIMIT=<min_prog_hour,max_prog_hour> .... (default = no limit)
c ** CONTROL.FILE=<output_control_file> .......... (default = no file)
c ** CONTROL.IDENT.OFF ........................... (default)
c ** CONTROL.IDENT(19)=<ident(19)>
c ** FELT.STANDARD ................................... (default)
c ** FELT.ARCHIVE
c ** FELT.CYCLIC_ARCHIVE
c ** ARCHIVE_DATE.TIME.ALL ........................... (default)
c ** ARCHIVE_FIRST.DATE.TIME= <year,month,day,hour>
c ** ARCHIVE_LAST.DATE.TIME=  <year,month,day,hour>
c ** ARCHIVE_STEP.DATE.TIME.ALL ...................... (default)
c ** ARCHIVE_STEP.DATE.TIME=  <year,month,day,hour>
c ** INDEX.WITHOUT.DATA.ON ........................... (default)
c ** INDEX.WITHOUT.DATA.OFF
c ** NOT.COPY.PARAM=<param(s)>
c ** NOT.COPY.LEVEL=<level(s)>
c ** NOT.COPY.VERTICAL.COORD=<vertical_coordinate(s)>
c ** SMOOTH.SHAP2.ALL.PARAM
c ** SMOOTH.SHAP2.PARAM=<param(s)>
c ** SMOOTH.SHAP4.ALL.PARAM
c ** SMOOTH.SHAP4.PARAM=<param(s)>
c ** SMOOTH.LOWBAND.ITERATIONS=<no_of_iterations_for_following_parameters>
c ** SMOOTH.LOWBAND.ALL.PARAM
c ** SMOOTH.LOWBAND.PARAM=<param(s)>
c ** NOT.SMOOTH.PARAM=<param(s)>
c ** AFTER.SMOOTH.CHANGE.PARAM.SCALE=<param,iscale,param,iscale,...>
c ** RESCALE.ON ........................... (default for format.pack...)
c ** RESCALE.OFF .......................... (default for other formats)
c ** DEFINE.LOOP= <identifier>,<value1>,<value2>,...
c ** DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>
c ** CONTINUE
c ** END
c **
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c ** Options    Remember '......' syntax.
c ** ($... = environment var. ; #n = coomand line arg. no. n)
c *>
c 'FILE= #3'
c 'FILE_IN= #2'
c 'GRID= 88,1814,1'
c 'GRID.INTERP_OUT= 1501, 61,49, 73.,55.,237.,+58'
c 'INTERP.BESSEL'                <<< standard:      bessel interp.
c 'INTERP.BILINEAR_PARAM=15,16'  <<< precipitation: bilinear interp.
c 'X.VECTOR_PARAM=2,33'          <<< u(x) parameters for vector rotation
c 'Y.VECTOR_PARAM=3,34'          <<< v(y) parameters for vector rotation
c 'END'
c **+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
c **
c ** -------------- LAM50S -> old LAM50 area -------------------
c *> ..... Z(1),U(2),V(3)
c 4
c 2,6, 2,12, 2,18, 2,24
c 3
c 500,0, 850,0, 1000,0
c 3
c 1,1, 1,2, 1,3
c **
c *> ..... MSLP(58),U10M(33),V10M(34)
c 4
c 2,6, 2,12, 2,18, 2,24
c 1
c 1000,0
c 3
c 2,58, 2,33, 2,34
c **
c *> ..... L.PRECIPITATION(15),C.PRECIPITATION(16)
c 8
c 2,3, 2,6, 2,9, 2,12, 2,15, 2,18, 2,21, 2,24
c 1
c 1000,0
c 2
c 2,15, 2,16
c **
c *> ..... end of time/level/parameter section
c 0
c=======================================================================
c
c**********************************************************************
c
c
c
c======================================================================
c  FILE=<output_FELT_file>
c  FORMAT.STANDARD .............................. (default)
c  FORMAT.PC .................................... (swap bytes)
c  FORMAT.SWAP .................................. (swap bytes)
c  FORMAT.NORD .................................. (old NORD format)
c  FORMAT.PACK .................................. (packed format)
c  FORMAT.PACK_PC ............................... (PC/DOS packed format)
c  FORMAT.PACK_SWAP ............................. (packed+byte_swap)
c  FORMAT.SEQUENTIAL ............................ (ident+ifelt+ident+)
c  CREATE.MACHINE.ENDIAN ........................ (default)
c  CREATE.BIG.ENDIAN ............................ (only STANDARD/SWAP)
c  CREATE.LITTLE.ENDIAN
c  DATA_GAPS.ON ................................. (default)
c  DATA_GAPS.OFF ................................ (only for STANDARD)
c  FILE_IN=<input_FELT_file>
c  GRID=<producer,grid,grid_type> ................. (input)
c  GRID.UNCHANGED_OUT ............................. (default)
c  GRID_OUT=<grid,ix1,ix2,iy1,iy2,istep,irot>
c  GRID.INTERP_OUT=<grid,nx,ny,xp,yp,an,fi>
c  GRID.MEAN_OUT=<grid,ix1,ix2,iy1,iy2,istep,irot>
c  GEO.GRID_OUT=<grid,nlong,nlat,glong1,glat1,delta_long,delta_lat>
c  PACKING.VERSION.2.ON ...................... (default for pack, pack_swap)
c  PACKING.VERSION.2.OFF ..................... (default for pack_pc)
c  MAX.OUTPUT.PACK.FIELD.SIZE= 32767 ......... (default as first field)
c  CHANGE_IDENT=<ident_no,input_value,output_value> ... ident_no 1-20 ?
c  CHANGE_IDENT_A=<v.coord,param,lvl1,lvl2, v.coord,param,lvl1,lvl2>
c  INTERP.BILINEAR ................................ (default)
c  INTERP.BESSEL
c  INTERP.NEAREST.GRID.POINT
c  INTERP.BILINEAR_PARAM=<parmeter_number(s)> ..... (not as standard)
c  INTERP.BESSEL_PARAM=<parmeter_number(s)>   ..... (not as standard)
c  INTERP.NEAREST_PARAM=<parmeter_number(s)>  ..... (not as standard)
c  X.VECTOR_PARAM=<parmeter_number(s)> ..... u(x); wind,current etc.
c  Y.VECTOR_PARAM=<parmeter_number(s)> ..... v(y); wind,current etc.
c  COMPUTE_T(K)->T(C)=<param_in,param_out>
c  COMPUTE_T(C)->T(K)=<param_in,param_out>
c  COMPUTE_PRESSURE.TH->T(K)=<param_in,param_out>
c  COMPUTE_PRESSURE.TH->T(C)=<param_in,param_out>
c  SIGMA.PS=<vert.coord.,parameter,level_1,level_2>
c  COMPUTE_SIGMA.TH->T(K)=<param_in,param_out>
c  COMPUTE_SIGMA.TH->T(C)=<param_in,param_out>
c  ADD_PARAM/REL.HOUR=<param, nparam,param(s)_in, nhours,rel.hour(s)>
c  TOT.ACCUM.TO.STEP=<param, param_in, hours.step>
c  PACK_RESCALE=<v.coord,param,lvl1_min,lvl1_max,iscale,idiv>
c  PACK_DIRECTION_PARAM=<param(s)>
c  PACK_NOT_PARAM=<param(s)>
c  INFO.ON  ....................................... (default)
c  INFO.OFF
c  CHECK_TIME.ON  ................................. (default)
c  CHECK_TIME.OFF
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> ....... (default = no limit)
c  CONTROL.FILE=<output_control_file> ............. (default = no file)
c  CONTROL.IDENT.OFF .............................. (default)
c  CONTROL.IDENT(19)=<ident(19)>
c  FELT.STANDARD ................................... (default)
c  FELT.ARCHIVE
c  FELT.CYCLIC_ARCHIVE
c  ARCHIVE_DATE.TIME.ALL ........................... (default)
c  ARCHIVE_FIRST.DATE.TIME= <year,month,day,hour>
c  ARCHIVE_LAST.DATE.TIME=  <year,month,day,hour>
c  ARCHIVE_STEP.DATE.TIME.ALL ...................... (default)
c  ARCHIVE_STEP.DATE.TIME=  <year,month,day,hour>
c  INDEX.WITHOUT.DATA.ON ........................... (default)
c  INDEX.WITHOUT.DATA.OFF
c  NOT.COPY.PARAM=<param(s)>
c  NOT.COPY.LEVEL=<level(s)>
c  NOT.COPY.VERTICAL.COORD=<vertical_coordinate(s)>
c  SMOOTH.SHAP2.ALL.PARAM
c  SMOOTH.SHAP2.PARAM=<param(s)>
c  SMOOTH.SHAP4.ALL.PARAM
c  SMOOTH.SHAP4.PARAM=<param(s)>
c  SMOOTH.LOWBAND.ITERATIONS=<no_of_iterations_for_following_parameters>
c  SMOOTH.LOWBAND.ALL.PARAM
c  SMOOTH.LOWBAND.PARAM=<param(s)>
c  NOT.SMOOTH.PARAM=<param(s)>
c  AFTER.SMOOTH.CHANGE.PARAM.SCALE=<param,iscale,param,iscale,...>
c  RESCALE.ON ........................... (default for format.pack...)
c  RESCALE.OFF .......................... (default for other formats)
c  DEFINE.LOOP= <identifier>,<value1>,<value2>,...
c  DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>
c  CONTINUE
c  END
c======================================================================
c
c
c      DNMI library subroutines:  rfturbo (fast version of rfelt)
c                                 rlunit
c                                 rcomnt
c                                 lenstr
c                                 getvar
c                                 keywrd
c                                 prhelp
c                                 rmfile
c                                 hrdiff
c                                 vtime
c                                 daytim
c                                 ginter (+xytoxy)
c                                 gintuv
c                                 swapfile
c                                 bswap2
c                                 bigendian
c
c*********************************************************************
c
c-------------------------------------------------------------------
c  DNMI/FoU   xx.xx.198x   Anstein Foss ... ibm
c  DNMI/FoU   18.11.1992   Anstein Foss ... unix
c  DNMI/FoU   03.12.1992   Anstein Foss
c  DNMI/FoU   07.12.1992   Anstein Foss
c  DNMI/FoU   23.03.1993   Anstein Foss
c  DNMI/FoU   27.07.1993   Anstein Foss ... large fields & file
c  DNMI/FoU   27.01.1994   Anstein Foss
c  DNMI/FoU   12.03.1994   Anstein Foss ... rfturbo & 'no_gap'
c  DNMI/FoU   13.05.1994   Anstein Foss
c  DNMI/FoU   15.11.1994   Anstein Foss ... geo.grid_out=
c  DNMI/FoU   09.01.1995   Anstein Foss ... interp.nearest
c  DNMI/FoU   10.02.1995   Anstein Foss ... smoothing options
c  DNMI/FoU   14.06.1995   Anstein Foss ... tot.accum.to.step option
c  DNMI/FoU   12.10.1995   Anstein Foss ... rm bug in packing (fpack1)
c  DNMI/FoU   11.09.1997   Anstein Foss ... packed.output>32767 words
c  DNMI/FoU   22.09.1997   Anstein Foss ... define.loop
c  DNMI/FoU   21.03.2003   Anstein Foss ... automatic byte swap (input)
c  DNMI/FoU   22.03.2003   Anstein Foss ... Linux update (packed files)
c  DNMI/FoU   23.03.2003   Anstein Foss ... CREATE.xxx.ENDIAN options
c  DNMI/FoU   04.06.2003   Anstein Foss ... 0-padding format.standard
c-------------------------------------------------------------------
c
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for copflt.f
c
c  maxij:  max field size
c
ccc   parameter (maxij=100000)
c
ccc   parameter (maxset=50,mstore=30000,maxgrd=24,mloops=16)
ccc   parameter (mchang=80,mchnga=20,mintpm=80,mxyvec=40,mcompu=16)
ccc   parameter (maddpa=20,maddpt=100,macspa=12)
ccc   parameter (mreadf=40)
c
ccc   parameter (mpkscl=40,mpkdir=16,mpknot=16)
ccc   parameter (maxncp=32,mpsmooth=100,mpasmpsc=32)
c
ccc   parameter (mdefloop=100,mloopdef=2000)
c
c  don't change the following ......................................
c
ccc   parameter (limit=20+maxij)
c
ccc   parameter (mwrec=1024,minrec=64)
ccc   parameter (mwrech=mwrec/2)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'copflt.inc'
c
c..note: array(maxdim+1) for nice overflow testing
c
      common/a/ntim(2,maxset),nlev(2,maxset),npar(2,maxset),
     *         itimef(5),igridi(3,mloops),
     *         ictime(mloops),info(mloops),numset(2,mloops),
     *         igridc(mloops),igrido(7,mloops),gridot(6,mloops),
     *         iaddpa(5,maddpa),icontrol(3),
     *         nchang,nchnga,interp,nintpm,nxyvec,ncompu,naddpa,naddpt,
     *         nacspa,
     *         idrec1(mwrec),idrec2(2,mwrech),idata(limit+mwrec,4),
     *         istore(2,mstore),ichang(3,mchang),ichnga(4,2,mchnga),
     *         iintpm(2,mintpm+1),ixvec(mxyvec+1),iyvec(mxyvec+1),
     *         icompu(2,mcompu),isigma(4),iaddpt(maddpt),
     *         iacspa(3,macspa),
     *         id19ctrl(2,mloops),
     *         comput(mcompu),
     *         fileot,filein(mloops),filnam
      integer*2 idrec1,idrec2,idata
      integer*2 istore,ichang,iintpm,ixvec,iyvec
      integer*2 icompu,isigma,iaddpt,iacspa,id19ctrl
      character*40 comput
      character*256 fileot,filein,filnam
c
      common/b/iarkiv,nodatind,nnocppar,nnocplev,nnocpvco,
     *         ktimea(3),itimea(5,3),itimefa(4,3,mloops),
     *         nocppar(maxncp+1),nocplev(maxncp+1),nocpvco(maxncp+1),
     *         ismooth,nsmooth,npsmooth,npasmpsc,
     *         ipsmooth(mpsmooth+1,3),ipasmpsc(2,mpasmpsc+1)
      integer iarkiv,nodatind,nnocppar,nnocplev,nnocpvco
      integer ktimea,itimea,itimefa
      integer nocppar,nocplev,nocpvco
      integer ismooth,nsmooth,npsmooth,npasmpsc,ipsmooth,ipasmpsc
c
      common/p/npkscl,npkdir,npknot,
     *         ipkscl(6,mpkscl),ipkdir(mpkdir+1),ipknot(mpknot+1),
     *         ihead(16),idpack(24),ipack(limit),ipackw(limit)
      integer*2 ipkscl,ipkdir,ipknot
      integer*2 ihead,idpack,ipack,ipackw
c
      integer*2 idfile(32)
c
      integer   idefloop(3,mdefloop+1)
      integer*2 iloopdef(mloopdef+1)
c
      character*64 fcontrol
      character*8  scontrol
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*256 cinput,cipart
      character*256 finput
      character*1   tchar
c
      logical swapfile,bigendian
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
c..file unit for 'copflt.input'
      iuinp=9
c
c..file unit for output felt file
      iunit=20
c
c..file unit for all input felt files (for field copy)
      iunitc=30
C
C..file unit for control file
      iuctrl=40
c
c-----------------------------------------------------------------------
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: copflt <copflt.input>'
        write(6,*) '      or: copflt <copflt.input> <arguments>'
        write(6,*) '      or: copflt <copflt.input> ?     (to get help)'
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
          call prhelp(iuinp,'*=>')
          close(iuinp)
          goto 981
        end if
      end if
c
c
      write(6,*) 'reading input file: ',finput(1:lenstr(finput,1))
c
      nlines = 0
c
      fileot = '*'
      iformt =-1
      nogap  =-1
      iswapb = 0
      nstore = 0
      nsets  = 0
      newinh = 0
      nchang = 0
      nchnga = 0
      interp = 0
      nintpm = 0
      nxyvec = 0
      nxvec  = 0
      nyvec  = 0
      ncompu = 0
      naddpa = 0
      naddpt = 0
      nacspa = 0
      npkscl = 0
      npkdir = 0
      npknot = 0
      irescale=-1
      ipkversion=0
      mpksize=0
c
c..default for ps(sigma), vert.coord.,parameter,level_1,level_2
c..(used for 'compute_sigma.....')
      isigma(1)=2
      isigma(2)=8
      isigma(3)=1000
      isigma(4)=0
c
      iprmin = -32767
      iprmax = +32767
c
      fcontrol='*'
      icontrol(1)=0
      icontrol(2)=0
      icontrol(3)=0
c
      lprod=1
      iprod1=99
      iprod2= 1
      iarkiv=-1
      ktimea(1)=-1
      ktimea(2)=-1
      ktimea(3)=-1
      nodatind=1
      iendian=-1
c
      nnocppar=0
      nnocplev=0
      nnocpvco=0
      ismooth=0
      nsmooth=0
      npsmooth=0
      iterlowb=0
      npasmpsc=0
c
      ndefloop=0
      nloopdef=0
c
      do 100 iloop=1,mloops
c
        call rcomnt(iuinp,'*>','*',nlines,ierror)
        if(ierror.ne.0) goto 210
c
c
        if(iloop.eq.1) then
          ictime(iloop)   = 1
          info(iloop)     = 1
          filein(iloop)   = '*'
          numset(1,iloop) = 0
          numset(2,iloop) = 0
          igridc(iloop) = 0
          do i=1,7
            igrido(i,iloop)=-9999
          end do
          do i=1,6
            gridot(i,iloop)=-9999.
          end do
          id19ctrl(1,iloop)=0
          id19ctrl(2,iloop)=0
        else
          ictime(iloop)   = ictime(iloop-1)
          info(iloop)     = info(iloop-1)
          filein(iloop)   = filein(iloop-1)
          numset(1,iloop) = 0
          numset(2,iloop) = 0
          igridi(1,iloop) = igridi(1,iloop-1)
          igridi(2,iloop) = igridi(2,iloop-1)
          igridi(3,iloop) = igridi(3,iloop-1)
          igridc(iloop)   = igridc(iloop-1)
          do i=1,7
            igrido(i,iloop)=igrido(i,iloop-1)
          end do
          do i=1,6
            gridot(i,iloop)=gridot(i,iloop-1)
          end do
          id19ctrl(1,iloop)=id19ctrl(1,iloop-1)
          id19ctrl(2,iloop)=id19ctrl(2,iloop-1)
        end if
c
        iend=0
        ngridi=0
        ngrido=0
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
c======================================================================
c  FILE=<output_FELT_file>
c  FORMAT.STANDARD .............................. (default)
c  FORMAT.PC .................................... (swap bytes)
c  FORMAT.SWAP .................................. (swap bytes)
c  FORMAT.NORD .................................. (old NORD format)
c  FORMAT.PACK .................................. (packed format)
c  FORMAT.PACK_PC ............................... (PC/DOS packed format)
c  FORMAT.PACK_SWAP ............................. (packed+byte_swap)
c  FORMAT.SEQUENTIAL ............................ (ident+ifelt+ident+)
c  CREATE.MACHINE.ENDIAN ........................ (default)
c  CREATE.BIG.ENDIAN ............................ (only STANDARD/SWAP)
c  CREATE.LITTLE.ENDIAN
c  DATA_GAPS.ON ................................. (default)
c  DATA_GAPS.OFF ................................ (only for STANDARD)
c  FILE_IN=<input_FELT_file>
c  GRID=<producer,grid,grid_type> ................. (input)
c  GRID.UNCHANGED_OUT ............................. (default)
c  GRID_OUT=<grid,ix1,ix2,iy1,iy2,istep,irot>
c  GRID.INTERP_OUT=<grid,nx,ny,xp,yp,an,fi>
c  GRID.MEAN_OUT=<grid,ix1,ix2,iy1,iy2,istep,irot>
c  GEO.GRID_OUT=<grid,nlong,nlat,glong1,glat1,delta_long,delta_lat>
c  PACKING.VERSION.2.ON ...................... (default for pack, pack_swap)
c  PACKING.VERSION.2.OFF ..................... (default for pack_pc)
c  MAX.OUTPUT.PACK.FIELD.SIZE= 32767 .............. (default=32767)
c  CHANGE_IDENT=<ident_no,input_value,output_value> ... ident_no 1-20 ?
c  CHANGE_IDENT_A=<v.coord,param,lvl1,lvl2, v.coord,param,lvl1,lvl2>
c  INTERP.BILINEAR ................................ (default)
c  INTERP.BESSEL
c  INTERP.NEAREST.GRID.POINT
c  INTERP.BILINEAR_PARAM=<parmeter_number(s)> ..... (not as standard)
c  INTERP.BESSEL_PARAM=<parmeter_number(s)>   ..... (not as standard)
c  INTERP.NEAREST_PARAM=<parmeter_number(s)>  ..... (not as standard)
c  X.VECTOR_PARAM=<parmeter_number(s)> ..... u(x); wind,current etc.
c  Y.VECTOR_PARAM=<parmeter_number(s)> ..... v(y); wind,current etc.
c  COMPUTE_T(K)->T(C)=<param_in,param_out>
c  COMPUTE_T(C)->T(K)=<param_in,param_out>
c  COMPUTE_PRESSURE.TH->T(K)=<param_in,param_out>
c  COMPUTE_PRESSURE.TH->T(C)=<param_in,param_out>
c  SIGMA.PS=<vert.coord.,parameter,level_1,level_2>
c  COMPUTE_SIGMA.TH->T(K)=<param_in,param_out>
c  COMPUTE_SIGMA.TH->T(C)=<param_in,param_out>
c  ADD_PARAM/REL.HOUR=<param, nparam,param(s)_in, nhours,rel.hour(s)>
c  TOT.ACCUM.TO.STEP=<param, param_in, hours.step>
c  PACK_RESCALE=<v.coord,param,lvl1_min,lvl1_max,iscale,idiv>
c  PACK_DIRECTION_PARAM=<param(s)>
c  PACK_NOT_PARAM=<param(s)>
c  INFO.ON  ....................................... (default)
c  INFO.OFF
c  CHECK_TIME.ON  ................................. (default)
c  CHECK_TIME.OFF
c  PROG_LIMIT=<min_prog_hour,max_prog_hour> ....... (default = no limit)
c  CONTROL.FILE=<output_control_file> ............. (default = no file)
c  CONTROL.IDENT.OFF .............................. (default)
c  CONTROL.IDENT(19)=<ident(19)>
c  FELT.STANDARD ................................... (default)
c  FELT.ARCHIVE
c  FELT.CYCLIC_ARCHIVE
c  ARCHIVE_DATE.TIME.ALL ........................... (default)
c  ARCHIVE_FIRST.DATE.TIME= <year,month,day,hour>
c  ARCHIVE_LAST.DATE.TIME=  <year,month,day,hour>
c  ARCHIVE_STEP.DATE.TIME.ALL ...................... (default)
c  ARCHIVE_STEP.DATE.TIME=  <year,month,day,hour>
c  INDEX.WITHOUT.DATA.ON ........................... (default)
c  INDEX.WITHOUT.DATA.OFF
c  NOT.COPY.PARAM=<param(s)>
c  NOT.COPY.LEVEL=<level(s)>
c  NOT.COPY.VERTICAL.COORD=<vertical_coordinate(s)>
c  SMOOTH.SHAP2.ALL.PARAM
c  SMOOTH.SHAP2.PARAM=<param(s)>
c  SMOOTH.SHAP4.ALL.PARAM
c  SMOOTH.SHAP4.PARAM=<param(s)>
c  SMOOTH.LOWBAND.ITERATIONS=<no_of_iterations_for_following_parameters>
c  SMOOTH.LOWBAND.ALL.PARAM
c  SMOOTH.LOWBAND.PARAM=<param(s)>
c  NOT.SMOOTH.PARAM=<param(s)>
c  AFTER.SMOOTH.CHANGE.PARAM.SCALE=<param,iscale,param,iscale,...>
c  RESCALE.ON ........................... (default for format.pack...)
c  RESCALE.OFF .......................... (default for other formats)
c  DEFINE.LOOP= <identifier>,<value1>,<value2>,...
c  DEFINE.LOOP.FROM.TO.STEP= <identifier>,<first>,<last>,<step>
c  CONTINUE
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
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=0
              iswapb=0
            elseif(cinput(k1:k2).eq.'format.pc') then
c..format.pc
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=0
              iswapb=2
            elseif(cinput(k1:k2).eq.'format.swap') then
c..format.swap
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=0
              iswapb=1
            elseif(cinput(k1:k2).eq.'format.nord') then
c..format.nord
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=1
              iswapb=0
            elseif(cinput(k1:k2).eq.'format.pack') then
c..format.pack
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=2
              iswapb=0
            elseif(cinput(k1:k2).eq.'format.pack_pc') then
c..format.pack_pc
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=3
              iswapb=2
            elseif(cinput(k1:k2).eq.'format.pack_swap') then
c..format.pack_swap
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=2
              iswapb=1
            elseif(cinput(k1:k2).eq.'format.sequential') then
c..format.sequential
              if(iloop.ne.1) goto 215
              if(iformt.ne.-1) goto 214
              iformt=4
              iswapb=0
            elseif(cinput(k1:k2).eq.'create.machine.endian') then
c..create.machine.endian
              if(iloop.ne.1) goto 215
	      if(iendian.ne.-1) goto 214
	      iendian=0
            elseif(cinput(k1:k2).eq.'create.big.endian') then
c..create.big.endian
              if(iloop.ne.1) goto 215
	      if(iendian.ne.-1) goto 214
	      iendian=1
            elseif(cinput(k1:k2).eq.'create.little.endian') then
c..create.little.endian
              if(iloop.ne.1) goto 215
	      if(iendian.ne.-1) goto 214
	      iendian=2
            elseif(cinput(k1:k2).eq.'data_gaps.on') then
c..data_gaps.on
              if(iloop.ne.1) goto 215
              if(nogap.ne.-1) goto 214
              nogap=0
            elseif(cinput(k1:k2).eq.'data_gaps.off') then
c..data_gaps.off
              if(iloop.ne.1) goto 215
              if(nogap.ne.-1) goto 214
              nogap=1
            elseif(cinput(k1:k2).eq.'file_in') then
c..file_in=<input_felt_file>
              if(kv1.lt.1) goto 213
              filein(iloop)=cinput(kv1:kv2)
            elseif(cinput(k1:k2).eq.'grid') then
c..grid=<producer,grid,grid_type>
              if(kv1.lt.1) goto 213
              if(ngridi.ne.0) goto 214
              ngridi=1
              read(cipart,*,err=213) (igridi(i,iloop),i=1,3)
            elseif(cinput(k1:k2).eq.'grid.unchanged_out') then
c..grid.unchanged_out
              if(ngrido.ne.0) goto 214
              ngrido=1
              igridc(iloop)=0
            elseif(cinput(k1:k2).eq.'grid_out') then
c..grid_out=<grid,ix1,ix2,iy1,iy2,istep,irot>
              if(kv1.lt.1) goto 213
              if(ngrido.ne.0) goto 214
              ngrido=1
              read(cipart,*,err=213) (igrido(i,iloop),i=1,7)
              if(igrido(6,iloop).lt.1) igrido(6,iloop)=1
              if(igrido(7,iloop).lt.-180 .or.
     *           igrido(7,iloop).gt.+180 .or.
     *           mod(igrido(7,iloop),90).ne.0) goto 213
              igridc(iloop)=1
            elseif(cinput(k1:k2).eq.'grid.interp_out') then
c..grid.interp_out=<grid,nx,ny,xp,yp,an,fi
              if(kv1.lt.1) goto 213
              if(ngrido.ne.0) goto 214
              ngrido=1
              read(cipart,*,err=213) (igrido(i,iloop),i=1,3),
     *                               (gridot(i,iloop),i=1,4)
	      gridot(5,iloop)=60.
	      gridot(6,iloop)=0.
              if(igrido(2,iloop)*igrido(3,iloop).gt.maxij) goto 238
              if(igrido(2,iloop)*igrido(3,iloop).lt.1) goto 213
              if(gridot(3,iloop).le.0.) goto 213
              igridc(iloop)=2
            elseif(cinput(k1:k2).eq.'grid.mean_out') then
c..grid.mean_out=<grid,ix1,ix2,iy1,iy2,istep,irot>
              if(kv1.lt.1) goto 213
              if(ngrido.ne.0) goto 214
              ngrido=1
              read(cipart,*,err=213) (igrido(i,iloop),i=1,7)
              if(igrido(6,iloop).lt.3 .or.
     *           mod(igrido(6,iloop),2).ne.1) goto 213
              if(igrido(7,iloop).lt.-180 .or.
     *           igrido(7,iloop).gt.+180 .or.
     *           mod(igrido(7,iloop),90).ne.0) goto 213
              igridc(iloop)=3
            elseif(cinput(k1:k2).eq.'geo.grid_out') then
c..geo.grid_out=<grid,nlong,nlat,glong1,glat1,delta_long,delta_lat>
              if(kv1.lt.1) goto 213
              if(ngrido.ne.0) goto 214
              ngrido=1
              read(cipart,*,err=213) (igrido(i,iloop),i=1,3),
     *                               (gridot(i,iloop),i=1,4)
	      gridot(5,iloop)=0.
	      gridot(6,iloop)=0.
              if(igrido(2,iloop)*igrido(3,iloop).gt.maxij) goto 238
              if(igrido(2,iloop)*igrido(3,iloop).lt.1) goto 213
              if(gridot(1,iloop).lt.-180. .or.
     *           gridot(1,iloop).gt.+180. .or.
     *           gridot(2,iloop).lt.-90.  .or.
     *           gridot(2,iloop).gt.+90.) goto 213
              if(gridot(3,iloop).eq.0. .and. gridot(4,iloop).ne.0.)
     *                                                         goto 213
              if(gridot(3,iloop).ne.0. .and. gridot(4,iloop).eq.0.)
     *                                                         goto 213
              igridc(iloop)=4
            elseif(cinput(k1:k2).eq.'packing.version.2.on') then
c..packing.version.2.on ...................... (default for pack, pack_swap)
              if(iloop.ne.1) goto 215
              if(ipkvers.ne.0) goto 214
	      ipkversion=2
            elseif(cinput(k1:k2).eq.'packing.version.2.off') then
c..packing.version.2.off ..................... (default for pack_pc)
              if(iloop.ne.1) goto 215
              if(ipkvers.ne.0) goto 214
	      ipkversion=1
            elseif(cinput(k1:k2).eq.'max.output.pack.field.size') then
c..max.output.pack.field.size= 32767 ............ (default as first field)
              if(kv1.lt.1) goto 213
              if(iloop.ne.1) goto 215
              if(mpksize.ne.0) goto 214
              read(cipart,*,err=213) mpksize
            elseif(cinput(k1:k2).eq.'change_ident') then
c..change_ident=<ident_no,input_value,output_value> ... ident_no 1-20 ?
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              nchang=nchang+1
              if(nchang.gt.mchang) goto 233
              read(cipart,*,err=213) (ichang(i,nchang),i=1,3)
              if(ichang(1,nchang).lt.1 .or.
     *           ichang(1,nchang).gt.20) goto 213
              if(ichang(1,nchang).eq.1  .or.
     *           ichang(1,nchang).eq.10 .or.
     *           ichang(1,nchang).eq.11) goto 239
            elseif(cinput(k1:k2).eq.'change_ident_a') then
c..change_ident_a=<v.coord,param,lvl1,lvl2, v.coord,param,lvl1,lvl2>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              nchnga=nchnga+1
              if(nchnga.gt.mchnga) goto 233
              read(cipart,*,err=213) ((ichnga(i,j,nchnga),i=1,4),j=1,2)
            elseif(cinput(k1:k2).eq.'interp.bilinear') then
c..interp.bilinear ................................ (default)
              if(iloop.ne.1) goto 215
              if(interp.ne.0) goto 214
              interp=1
            elseif(cinput(k1:k2).eq.'interp.bessel') then
c..interp.bessel
              if(iloop.ne.1) goto 215
              if(interp.ne.0) goto 214
              interp=2
            elseif(cinput(k1:k2).eq.'interp.nearest.grid.point') then
c..interp.nearest.grid.point
              if(iloop.ne.1) goto 215
              if(interp.ne.0) goto 214
              interp=3
            elseif(cinput(k1:k2).eq.'interp.bilinear_param') then
c..interp.bilinear_param=<parmeter_number(s)> ..... (not as standard)
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nintpm+1
              i2=i1-1
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mintpm) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (iintpm(1,i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              do i=i1,i2
                iintpm(2,i)=1
              end do
              nintpm=i2
            elseif(cinput(k1:k2).eq.'interp.bilinear_param') then
c..interp.bessel_param=<parmeter_number(s)> ..... (not as standard)
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nintpm+1
              i2=i1-1
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mintpm) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (iintpm(1,i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              do i=i1,i2
                iintpm(2,i)=2
              end do
              nintpm=i2
            elseif(cinput(k1:k2).eq.'interp.nearest_param') then
c..interp.nearest_param=<parmeter_number(s)> ..... (not as standard)
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nintpm+1
              i2=i1-1
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mintpm) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (iintpm(1,i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              do i=i1,i2
                iintpm(2,i)=3
              end do
              nintpm=i2
            elseif(cinput(k1:k2).eq.'x.vector_param') then
c..x.vector_param=<parmeter_number(s)> ..... u(x); wind,current etc.
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nxvec+1
              i2=nxvec
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mxyvec) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ixvec(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              nxvec=i2
            elseif(cinput(k1:k2).eq.'y.vector_param') then
c..y.vector_param=<parmeter_number(s)> ..... v(y); wind,current etc.
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nyvec+1
              i2=nyvec
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mxyvec) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (iyvec(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              nyvec=i2
c-----------------------------------------------------------------------
            elseif(k2.gt.k1+7 .and. cinput(k1:k1+7).eq.'compute_') then
c..compute_t(k)->t(c)=<param_in,param_out>
c..compute_t(c)->t(k)=<param_in,param_out>
c..compute_pressure.th->t(k)=<param_in,param_out>
c..compute_pressure.th->t(c)=<param_in,param_out>
c..compute_sigma.th->t(k)=<param_in,param_out>
c..compute_sigma.th->t(c)=<param_in,param_out>
c......warning: no test here of what follows 'compute_'
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              ncompu=ncompu+1
              if(ncompu.gt.mcompu) goto 233
              read(cipart,*,err=213) (icompu(i,ncompu),i=1,2)
              comput(ncompu)=cinput(k1+8:k2)
            elseif(cinput(k1:k2).eq.'sigma.ps') then
c..sigma.ps=<vert.coord.,parameter,level_1,level_2>
c..(for 'compute_sigma....')
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) (isigma(i),i=1,4)
c-----------------------------------------------------------------------
            elseif(cinput(k1:k2).eq.'add_param/rel.hour') then
c..add_param/rel.hour=<param, nparam,param(s)_in, nhours,rel.hour(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              naddpa=naddpa+1
              if(naddpa.gt.maddpa) goto 233
              read(cipart,*,err=213) ipara,npara
              i1=naddpt+1
              i2=naddpt+npara
              naddpt=i2
              if(naddpt.gt.maddpt) goto 233
              read(cipart,*,err=213) ipara,npara,(iaddpt(i),i=i1,i2),
     *                                     nrelh
              i3=naddpt+1
              i4=naddpt+nrelh
              naddpt=i4
              if(naddpt.gt.maddpt .or. npara*nrelh.gt.mreadf) goto 233
              read(cipart,*,err=213) ipara,npara,(iaddpt(i),i=i1,i2),
     *                                     nrelh,(iaddpt(i),i=i3,i4)
              iaddpa(1,naddpa)=ipara
              iaddpa(2,naddpa)=i1
              iaddpa(3,naddpa)=i2
              iaddpa(4,naddpa)=i3
              iaddpa(5,naddpa)=i4
            elseif(cinput(k1:k2).eq.'tot.accum.to.step') then
c  tot.accum.to.step=<param, param_in, hours.step>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              nacspa=nacspa+1
              if(nacspa.gt.macspa) goto 233
              read(cipart,*,err=213) (iacspa(i,nacspa),i=1,3)
            elseif(cinput(k1:k2).eq.'pack_rescale') then
c..pack_rescale=<v.coord,param,lvl1_min,lvl1_max,iscale,idiv>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              npkscl=npkscl+1
              if(npkscl.gt.mpkscl) goto 233
              read(cipart,*,err=213) (ipkscl(i,npkscl),i=1,6)
            elseif(cinput(k1:k2).eq.'pack_direction_param') then
c..pack_direction_param=<param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=npkdir+1
              i2=npkdir
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpkdir) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ipkdir(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              npkdir=i2
            elseif(cinput(k1:k2).eq.'pack_not_param') then
c..pack_not_param=<param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=npknot+1
              i2=npknot
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpknot) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ipknot(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              npknot=i2
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
            elseif(cinput(k1:k2).eq.'control.file') then
c..control.file=<output_control_file> ............. (default = no file)
              if(fcontrol(1:1).ne.'*') goto 214
              if(kv1.lt.1) goto 213
              fcontrol=cinput(kv1:kv2)
            elseif(cinput(k1:k2).eq.'control.ident.off') then
c..control.ident.off
              id19ctrl(1,iloop)=0
              id19ctrl(2,iloop)=0
            elseif(cinput(k1:k2).eq.'control.ident(19)') then
c..control.ident(19)=<ident(19)>
              if(kv1.lt.1) goto 213
              id19ctrl(1,iloop)=1
              read(cipart,*,err=213) id19ctrl(2,iloop)
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
            elseif(cinput(k1:k2).eq.'archive_date.time.all') then
c..archive_date.time.all
              if(ktimea(1).ne.-1 .and. ktimea(1).ne.0) goto 214
              if(ktimea(2).ne.-1 .and. ktimea(2).ne.0) goto 214
              ktimea(1)=0
              ktimea(2)=0
            elseif(cinput(k1:k2).eq.'archive_first.date.time') then
c..archive_first.date.time= <year,month,day,hour>
              if(ktimea(1).ne.-1) goto 214
              ktimea(1)=1
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) (itimea(i,1),i=1,4)
            elseif(cinput(k1:k2).eq.'archive_last.date.time') then
c..archive_last.date.time= <year,month,day,hour>
              if(ktimea(2).ne.-1) goto 214
              ktimea(2)=1
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) (itimea(i,2),i=1,4)
            elseif(cinput(k1:k2).eq.'archive_step.date.time.all') then
c..archive_step.date.time.all
              if(ktimea(3).ne.-1 .and. ktimea(3).ne.0) goto 214
              ktimea(3)=0
            elseif(cinput(k1:k2).eq.'archive_step.date.time') then
c..archive_step.date.time= <year,month,day,hour>
              if(ktimea(3).ne.-1) goto 214
              ktimea(3)=1
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) (itimea(i,3),i=1,4)
              i=0
              j=0
              do n=1,4
                if(itimea(n,3).ne.0) i=i+1
                if(itimea(n,3).lt.0) j=j+1
              end do
              if(i.ne.1 .or. j.ne.0) goto 213
            elseif(cinput(k1:k2).eq.'index.without.data.on') then
c..index.without.data.on
               nodatind=1
            elseif(cinput(k1:k2).eq.'index.without.data.off') then
c..index.without.data.off
               nodatind=0
            elseif(cinput(k1:k2).eq.'not.copy.param') then
c..not.copy.param= <param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nnocppar+1
              i2=nnocppar
              ios=0
              do while (ios.eq.0)
                if(i2.gt.maxncp) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (nocppar(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              nnocppar=i2
            elseif(cinput(k1:k2).eq.'not.copy.level') then
c..not.copy.level= <level(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nnocplev+1
              i2=nnocplev
              ios=0
              do while (ios.eq.0)
                if(i2.gt.maxncp) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (nocplev(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              nnocplev=i2
            elseif(cinput(k1:k2).eq.'not.copy.vertical.coord') then
c..not.copy.vertical.coord= <vertical_coordinates(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=nnocpvco+1
              i2=nnocpvco
              ios=0
              do while (ios.eq.0)
                if(i2.gt.maxncp) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (nocpvco(i),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              nnocpvco=i2
c-smoothing-------------------------------------------------------------
            elseif(cinput(k1:k2).eq.'smooth.shap2.all.param') then
c..smooth.shap2.all.param
              if(iloop.ne.1) goto 215
	      if(ismooth.ne.0 .and. ismooth.ne.1) goto 214
              ismooth=1
            elseif(cinput(k1:k2).eq.'smooth.shap2.param') then
c..smooth.shap2.param=<param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=npsmooth+1
              i2=npsmooth
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpsmooth) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ipsmooth(i,1),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
	      do i=i1,i2
		ipsmooth(i,2)=1
		ipsmooth(i,3)=0
	      end do
              npsmooth=i2
            elseif(cinput(k1:k2).eq.'smooth.shap4.all.param') then
c..smooth.shap4.all.param
              if(iloop.ne.1) goto 215
	      if(ismooth.ne.0 .and. ismooth.ne.2) goto 214
              ismooth=2
            elseif(cinput(k1:k2).eq.'smooth.shap4.param') then
c..smooth.shap4.param=<param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=npsmooth+1
              i2=npsmooth
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpsmooth) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ipsmooth(i,1),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
	      do i=i1,i2
		ipsmooth(i,2)=2
		ipsmooth(i,3)=0
	      end do
              npsmooth=i2
            elseif(cinput(k1:k2).eq.'smooth.lowband.iterations') then
c..smooth.lowband.iterations=<no_of_iterations_for_following_parameters>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              read(cipart,*,err=213) iterlowb
              if(iterlowb.lt.1) goto 213
            elseif(cinput(k1:k2).eq.'smooth.lowband.all.param') then
c..smooth.lowband.all.param
              if(iloop.ne.1) goto 215
	      if(ismooth.ne.0 .and. ismooth.ne.3) goto 214
              if(iterlowb.lt.1) goto 252
              ismooth=3
	      nsmooth=iterlowb
            elseif(cinput(k1:k2).eq.'smooth.lowband.param') then
c..smooth.lowband.param=<param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              if(iterlowb.lt.1) goto 252
              i1=npsmooth+1
              i2=npsmooth
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpsmooth) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ipsmooth(i,1),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
	      do i=i1,i2
		ipsmooth(i,2)=3
		ipsmooth(i,3)=iterlowb
	      end do
              npsmooth=i2
            elseif(cinput(k1:k2).eq.'not.smooth.param') then
c..not.smooth.param=<param(s)>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=npsmooth+1
              i2=npsmooth
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpsmooth) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios) (ipsmooth(i,1),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
	      do i=i1,i2
		ipsmooth(i,2)=0
		ipsmooth(i,3)=0
	      end do
              npsmooth=i2
            elseif(cinput(k1:k2).eq.
     +		   'after.smooth.change.param.scale') then
c  after.smooth.change.param.scale=<param,iscale,param,iscale,...>
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              i1=npasmpsc+1
              i2=npasmpsc
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mpasmpsc) goto 233
                i2=i2+1
                read(cipart,*,iostat=ios)
     +			    ((ipasmpsc(j,i),j=1,2),i=i1,i2)
              end do
              i2=i2-1
              if(i2.lt.i1) goto 213
              npasmpsc=i2
c-smoothing-------------------------------------------------------------
            elseif(cinput(k1:k2).eq.'rescale.on') then
c  rescale.on ........................... (default for format.pack...)
              if(iloop.ne.1) goto 215
              if(irescale.ne.-1) goto 214
	      irescale=1
            elseif(cinput(k1:k2).eq.'rescale.off') then
c  rescale.off .......................... (default for other formats)
              if(iloop.ne.1) goto 215
              if(irescale.ne.-1) goto 214
	      irescale=0
            elseif(cinput(k1:k2).eq.'define.loop.from.to.step') then
c..define.loop.from.to.step= <identifier>,<first>,<last>,<step>
              if(iloop.ne.1) goto 215
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
                if(ndefloop.gt.mdefloop) goto 233
		idefloop(1,ndefloop)=id
		idefloop(2,ndefloop)=0
		idefloop(3,ndefloop)=nloopdef+1
	      end if
              j=nloopdef
              do i=i1,i2,i3
                nloopdef=nloopdef+1
                if(nloopdef.gt.mloopdef) goto 233
                iloopdef(nloopdef)=i
              end do
              idefloop(2,ndefloop)=idefloop(2,ndefloop)+nloopdef-j
            elseif(cinput(k1:k2).eq.'define.loop') then
c..define.loop= <identifier>,<value1>,<value2>,...
              if(iloop.ne.1) goto 215
              if(kv1.lt.1) goto 213
              read(cipart,*,end=213) id
              if(id.lt.-32767 .or. id.gt.32767) goto 213
              i=0
              if(ndefloop.gt.0) then
                if(idefloop(1,ndefloop).eq.id) i=ndefloop
              end if
              if(i.eq.0) then
                ndefloop=ndefloop+1
                if(ndefloop.gt.mdefloop) goto 233
                idefloop(1,ndefloop)=id
                idefloop(2,ndefloop)=0
                idefloop(3,ndefloop)=nloopdef+1
              end if
              i1=nloopdef+1
              i2=i1-1
              ios=0
              do while (ios.eq.0)
                if(i2.gt.mloopdef) goto 233
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
        if(iloop.eq.1 .and. ngridi.eq.0) goto 219
c
c..grid interpolation only for polarstereographic map projection
        if(igridc(iloop).eq.2 .and. igridi(3,iloop).ne.1) goto 237
c..geo.grid_out requires geographic input grid
        if(igridc(iloop).eq.4 .and. igridi(3,iloop).ne.2) goto 251
c
        if(iloop.eq.1) then
          if(nxvec.ne.nyvec) goto 234
          nxyvec=nxvec
          do i=1,nxyvec
            if(ixvec(i).eq.iyvec(i)) goto 235
          end do
        end if
c
c..producer no. and sequence on felt files
        if(igridi(1,iloop).lt.1 .or. igridi(1,iloop).gt.99) goto 222
        if(igridi(1,iloop).lt.lprod) goto 223
        lprod=igridi(1,iloop)
c
c..only one producer allowed on archive felt files
        if(iprod1.gt.igridi(1,iloop)) iprod1=igridi(1,iloop)
        if(iprod2.lt.igridi(1,iloop)) iprod2=igridi(1,iloop)
        if(iarkiv.gt.0 .and. iprod1.ne.iprod2) goto 224
c
c..file=
        if(fileot(1:1).eq.'*') goto 227
c
c..file_in=
        if(filein(iloop)(1:1).eq.'*') goto 220
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
            if(nstore.gt.mstore) goto 253
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
            if(nstore.gt.mstore) goto 253
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
          nlinex=nlines
          nlines=nlines+1
          read(iuinp,*,err=211) ((istore(i,j),i=1,2),j=n1,nstore)
c
c..check defined loops (value=identifier)
	  if(ndefloop.gt.0) then
	    call defloop(npar(1,n),nstore,mstore,istore,
     +			 ndefloop,mdefloop,idefloop,
     +			 nloopdef,mloopdef,iloopdef)
            if(nstore.gt.mstore) goto 253
	  end if
c
          if(ntim(1,n).eq.0 .or. nlev(1,n).eq.0
     +		            .or. npar(1,n).eq.0) then
            ntim(1,n)=0
            nlev(1,n)=0
            npar(1,n)=0
            nstore=lstore
          elseif(nxyvec.gt.0) then
c..for vector rotation:
c  always the 'x' parameter before the 'y' parameter
            np1=n1
            np2=nstore
            do np=np1,np2
              nx=0
              ny=0
              do i=1,nxyvec
                if(ixvec(i).eq.istore(2,np) .and. nx.eq.0) nx=i
                if(iyvec(i).eq.istore(2,np) .and. ny.eq.0) ny=i
              end do
              if(nx.gt.0) then
                if(ny.ne.0 .or. np.eq.np2) goto 236
                if(istore(2,np+1).ne.iyvec(nx)) goto 236
              end if
              if(ny.gt.0) then
                if(nx.ne.0 .or. np.eq.np1) goto 236
                if(istore(2,np-1).ne.ixvec(ny)) goto 236
              end if
            end do
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
        newinh=newinh+nih
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
  215 write(6,*) 'option only allowed in first ''specification',
     *           ' section'' '
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
  219 write(6,*) 'no input grid specified.'
      goto 240
  220 write(6,*) 'no input file (file_in=)'
      goto 240
  222 write(6,*) 'illegal producer no. found, (legal range is 1 - 99)'
      goto 240
  223 write(6,*) 'not ok producer sequence, not increasing'
      goto 240
  224 write(6,*) 'archive or cyclic_archive files: only one producer.'
      goto 240
  227 write(6,*) 'no output file specified.'
      goto 240
  232 iprhlp=1
      goto 240
c
  233 write(6,*) 'too many specifications of the type below:'
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  234 write(6,*) 'not same number of x.vector_param and',
     *           ' y.vector_param'
      goto 240
  235 write(6,*) 'same parameter no. found as x.vector_param and',
     *           ' y.vector_param'
      goto 240
  236 write(6,*) 'not ok parameter sequence, see x.vector_param and',
     *           ' y.vector_param above'
      nlines=nlinex
      goto 240
  237 write(6,*) 'grid interpolation only allowed for',
     *           ' polarstereographic grids.'
      goto 240
  238 write(6,*) 'too big output grid:'
      write(6,*) cinput(1:lenstr(cinput,1))
      write(6,*) 'max. size (nx*ny): ',maxij
      goto 240
  239 write(6,*) 'illegal to change producer no. and grid dimensions:'
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  251 write(6,*) 'geographic output grid requires a geographic',
     *           ' input grid.'
  252 write(6,*) 'no. of iterations for smooth.lowband not specified'
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 240
  253 write(6,*) 'too many time/level/parameter specifications'
      write(6,*) '         when expanding defined loops'
      write(6,*) 'max. no. of specifications (mstore): ',mstore
      goto 240
c
  240 write(6,*) 'error at line no. ',nlines,'   (or below)'
      if(iprhlp.eq.1) then
        write(6,*) 'help from ''copflt.input'':'
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
      if((iformt.eq.0 .or. iformt.eq.2) .and.
     +   iendian.gt.0 .and. iswapb.eq.0) then
        if(iendian.eq.1 .and. .not.bigendian()) iswapb=1
        if(iendian.eq.2 .and.      bigendian()) iswapb=1
      end if
c
      if(iformt.lt.0) then
        iformt=0
        iswapb=0
      end if
c
      if(iswapb.eq.2) then
        iswapb=0
        if(bigendian()) iswapb=1
      end if
c
      if(nogap.eq.-1) nogap=0
c
c..default interpolation type is bilinear (if needed)
      if(interp.eq.0) interp=1
c
c..interpolation etc. only if polarstereographic grid
c..(only subgrid if other grid_types, and then updating identification
c.. properly only if geographic grid (grid type 2)).
      do iloop=1,nloops
        if(igridi(3,iloop).ne.1) then
          if(igridc(iloop).eq.0) then
            continue
          elseif(igridc(iloop).eq.1   .and.
     *           igrido(7,iloop).eq.0) then
            continue
	  elseif(igridc(iloop).eq.4 .and. igridi(3,iloop).eq.2) then
	    continue
          else
            write(6,*) 'Grid rotation and interpolation only allowed'
            write(6,*) 'for polarstereographic grids.'
            goto 981
          end if
        end if
      end do
c
c..not specified options, use default
      if(iarkiv.eq.-1) iarkiv=0
      if(ktimea(1).eq.-1) ktimea(1)=0
      if(ktimea(2).eq.-1) ktimea(2)=0
      if(ktimea(3).eq.-1) ktimea(3)=0
c
      if(iarkiv.ne.0) nodatind=1
c
c..default rescaling
      if(irescale.eq.-1) then
	irescale=0
	if(iformt.eq.2 .or. iformt.eq.3) irescale=1
      end if
c
c..the max for old programs (and PC unpack!)
      if(mpksize.lt.32767) mpksize=32767
c
c..default packing version
      if(ipkversion.eq.0 .and. iformt.eq.2) ipkversion=2
      if(ipkversion.eq.0) ipkversion=1
c
      write(6,*) 'input o.k.'
c
c-----------------------------------------------------------------------
c
c..open output felt-file
c
      write(6,*) 'output file: ',fileot(1:lenstr(fileot,1))
c
      call rmfile(fileot,0,ierror)
c
      iupgap=0
c
      if(iformt.eq.0 .or. iformt.eq.1) then
c
        lrecl=2048
c-nord--------------------------------------------------------
        if(iformt.eq.1) lrecl=512
c-nord--------------------------------------------------------
        open(iunit,file=fileot,
     *             form='unformatted',access='direct',
     *             recl=lrecl/lrunit,
     *             status='unknown',iostat=ios,err=900)
c
      elseif(iformt.eq.2 .or. iformt.eq.3) then
c
c SGI: last record in a direct access file, written incompletely,
c      is padded to N*2048 bytes.
c      => using "system" (not available on Linux)
c Linux: last record is not padded => OK
c
#ifdef linux 
        open(iunit,file=fileot,
     +             form='unformatted',access='direct',
     +             recl=2048/lrunit,
     +             status='unknown',iostat=ios,err=900)
#else
        open(iunit,file=fileot,
     +             form='system',access='sequential',
     +             status='unknown',iostat=ios,err=900)
#endif
c
      elseif(iformt.eq.4) then
c
        open(iunit,file=fileot,
     +             form='unformatted',access='sequential',
     +             status='unknown',iostat=ios,err=900)
c
      end if
c
      filnam=filein(1)
      write(6,*) 'date/time from: ',filnam(1:lenstr(filnam,1))
      open(iunitc,file=filnam,
     *            form='unformatted',access='direct',
     *            recl=2048/lrunit,
     *            status='old',iostat=ios,err=920)
      read(iunitc,rec=1,iostat=ios,err=930) idfile
      if(swapfile(-iunitc)) call bswap2(32,idfile)
      close(iunitc)
      itimef(1)=idfile(5)
      itimef(2)=idfile(6)/100
      itimef(3)=idfile(6)-(idfile(6)/100)*100
      itimef(4)=idfile(7)/100
      itimef(5)=0
      write(6,*) 'date/time:      ',(itimef(i),i=1,4)
c
      if(ipkversion.eq.2 .and. idfile(14).eq.1) iupgap=1
c
      if(iarkiv.eq.0) then
        continue
      elseif(idfile(1).eq.997 .or. idfile(2).eq.998) then
c..archive or cyclic_archive input file
c..get first and last date/time
        itimefa(1,1,1)=idfile(20)
        itimefa(2,1,1)=idfile(21)/100
        itimefa(3,1,1)=idfile(21)-(idfile(21)/100)*100
        itimefa(4,1,1)=idfile(22)/100
        itimefa(1,2,1)=idfile(23)
        itimefa(2,2,1)=idfile(24)/100
        itimefa(3,2,1)=idfile(24)-(idfile(24)/100)*100
        itimefa(4,2,1)=idfile(25)/100
        do i=1,4
          itimefa(i,3,1)=0
          if(idfile(29).eq.i) itimefa(i,3,1)=idfile(30)
        end do
      else
c..standard felt file, one date/time
        do n=1,2
          do i=1,4
            itimefa(i,n,1)=itimef(i)
          end do
        end do
        do i=1,4
          itimefa(i,3,1)=0
        end do
        itimefa(4,3,1)=1
      end if
c
      call fcopy(iunit,iunitc,nloops,newinh,iformt,iswapb,
     +		 nogap,irescale,ipkversion,mpksize,iupgap)
c
      close(iunit)
c
      if(iformt.eq.0 .or. iformt.eq.1) then
        write(6,*) 'finished.'
        write(6,*) 'record 1, word  1 - 11 :'
        write(6,fmt='(1x,11i6)') (idrec1(i),i=1,11)
        if(iarkiv.ne.0) then
          write(6,*) 'record 1, word 20 - 30 :'
          write(6,fmt='(1x,11i6)') (idrec1(i),i=20,30)
        end if
      end if
c
c..output to control file
      if(fcontrol(1:1).ne.'*') then
        write(6,*) 'Control (status) file:'
        write(6,*) fcontrol(1:lenstr(fcontrol,1))
        icontrol(2)=icontrol(1)-icontrol(2)
        if(icontrol(2).eq.0 .and. icontrol(3).eq.0) then
          scontrol='ok'
        else
          scontrol='not.ok'
        end if
        write(6,*) 'year,month,day,hour,no.requested,no.missing,',
     *                                 'no.bad,status:'
        write(6,fmt='(1x,i4.4,3(1x,i2.2),3(1x,i6),1x,a8)')
     *               (itimef(i),i=1,4),(icontrol(i),i=1,3),scontrol
        open(iuctrl,file=fcontrol,
     *              form='formatted',access='sequential',
     *              status='unknown',iostat=ios)
        if(ios.eq.0) then
          write(iuctrl,fmt='(1x,i4.4,3(1x,i2.2),3(1x,i6),1x,a8)',
     *                 iostat=ios)
     *               (itimef(i),i=1,4),(icontrol(i),i=1,3),scontrol
          close(iuctrl)
        end if
        if(ios.ne.0) write(6,*) 'Open/write ERROR.  iostat= ',ios
      end if
c
      goto 990
c
  900 write(6,*) 'open error.  iostat=',ios,'   output file:'
      write(6,*) fileot(1:lenstr(fileot,1))
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
      write(6,*) 'copflt ***** stop 1 *****'
      call exit(1)
c
  982 continue
ccc   stop 2
      write(6,*) 'copflt ***** stop 2 *****'
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
