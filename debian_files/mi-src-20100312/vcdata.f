      program vcdata
c
c        this program reads data from dnmi felt files and selects
c        data for one or more vertical crossections.
c        standard interpolation routines are used.
c        data is stored on one file.
c        the program vcross can display the data.
c
c        Input data with vertical coordinate
c           - sigma (NORLAM)
c           - sigma (NORLAM) with ps and pb, sigma=0-2
c           - pressure
c           - potential temperature (isentropic surfaces)
c           - Z (depth) levels from ocean models
c           - eta (hybrid) (HIRLAM,...)
c           - sigma height (MEMO, sigma=(Z-Zs)/(H-Zs), Z=Zs+sigma*(H-Zs))
c		works also for MC2
c           - sigma.MM5 (note: just reading pressure in each level),
c		works for Simra too
c	    - UM MetOffice Unified Model  (just reading pressure in each level)
c
c        added: geostrophic wind in sigma levels.
c               sigma levels defined by ps and pb (sigma=0-2).
c               potential vorticity in sigma levels.
c
c        added: sigmadot (parameter 11),
c               input in sigma_1 levels (2:ks),
c               output crossections in sigma_2 levels (1:ks)
c               (in eta (hybrid) levels, etadot are found in the
c                full levels as the other parameters)
c
c        added: misc. grids
c               - polarstereographic (Norlam)
c               - geographic
c               - spherical rotated (Hirlam)
c
c        added: misc. output formats
c
c        change: output file changed from real*4 type to
c                integer*2 type (to save space, transfer time
c                and support conversions to other machine types)
c
c        added: geostrophic wind in isentropic (pot.temp.) levels
c               (computed from input Montgomery potential,
c                M=cp*T+g*z, parameter 90).
c               possibly use surface pressure.
c
c        added: preferable interpolation with hermit splines again
c               available (as it was before non-polarstereographic
c               grids were handled), but it may only be used when
c               each part of a crossection start and end in a
c               gridpoint and the line is straight in the input grid
c               (i.e. often not possible when coordinates are given
c                as geographic latitude and longitude)
c
c        change: only hermit interpolation used (again).
c                possibly double hermit interpolation (in 2 directions)
c                or in combination with linear interpolation.
c
c        added: factors for rotation of vector components (wind etc.)
c               to E/W and N/S components during plotting.
c
c        added: straight lines in geographic grid,
c               best with constant latitude or longitude,
c	        but "works" in other cases too.
c
c	 added: horizontal vorticity components,
c		(reading z,u,v,w, not able to compute z yet...)
c
c        note:  misc. "horizontal" computations are done here instead
c               of in the plotting program. increases the file size a
c               little bit, but makes a faster and simpler plotting
c               program, probably.
c               these "constants" are also stored at each timestep
c               (for simpler read).
c
c
c VCDATA.INPUT:
c-----------------------------------------------------------------------
c Example.
c=======================================================================
c *** vcdata.input  ('vcdata.input')
c ***
c *=> Data for Vertical Crossections from LAM50S.
c *=>
c *=> Environment var:
c *=>  none
c *=> Command format:
c *=>  vcdata vcdata.input felt.dat +0,+48  1814   18  LAM50S vcross.dat
c *=>                      <input>  <prog> <grid> <ks> <text> <output>
c ***
c ***------------------------------------------------------------------
c **
c ** Options:
c ** --------
c ** FILE=<output_file>
c ** FORMAT.STANDARD ............................ (default)
c ** FORMAT.SWAP
c ** FORMAT.PC
c ** FORMAT.IBM
c ** GRID=<producer,grid>
c ** TEXT=<short_plot_text>
c ** DATA.SIGMA.KS=<no_of_sigma_levels>
c ** DATA.SIGMA.K=<level_no,level_no,...>
c ** DATA.SIGMA.0-2.KS=<no_of_sigma_levels>   ... sigma(0-2) with ps,pb
c ** DATA.SIGMA.0-2.K=<level_no,level_no,...> ... sigma(0-2) with ps,pb
c ** DATA.ETA.LEVELS=<no_of_eta_levels>
c ** DATA.ETA.LEVEL=<level_no,level_no,...>
c ** DATA.PRESSURE=<level,level,...> ............ (hPa)
c ** DATA.POT.TEMP=<level,level,...> ............ (1/10 Kelvin)
c ** DATA.SEA.DEPTH=<depth,depth,...> ........... (m)
c ** DATA.SIGMA.HEIGHT.ALL.LEVELS=<no_of_levels>
c ** DATA.SIGMA.HEIGHT.LEVEL=<level_no,level_no,...>
c ** DATA.SIGMA.MM5.ALL.LEVELS=<no_of_levels>
c ** DATA.SIGMA.MM5.LEVEL=<level_no,level_no,...>
c ** DATA.UM.ALL.LEVELS=<no_of_levels>
c ** DATA.UM.LEVEL=<level_no,level_no,...>
c ** PROG.LIMIT=<min_prog_hour,max_prog_hour> ... (default = no limit)
c ** FILE.IN=<input_felt_file> .................. for following f/c's
c ** FORECAST=<datatype,hour, datatype,hour,...>
c ** PARAMETER=<param_no,param_no,....>
c ** PARAM.MIN= <param,min_value, param,min_value,...> ... test after interpol.
c ** PARAM.MAX= <param,max_value, param,max_value,...>
c ** PARAM.MIN.MAX= <param,min_value,max_value, param,...>
c ** COMPUTE.VORTICITY                <<< stored as parameter -1
c ** COMPUTE.DIVERGENCE               <<< stored as parameter -2
c ** COMPUTE.GEOSTROPHIC.WIND         <<< stored as parameter -3 and -4
c ** COMPUTE.POTENTIAL.VORTICITY      <<< stored as parameter -5
c ** COMPUTE.HORIZONTAL.VORTICITY     <<< stored as parameter -6 and -7
c ** SURFACE.PARAMETER=<param_no,..>  <<< 8=ps  (sigma.0-2: 78=pb)
c ** SURFACE.PARAMETER=<param_no,..>  <<< 301=sealevel 351=bottom
c ** SURFACE.PARAMETER.LEVEL=<param_no,level, param_no,level, ..>
c ** STRAIGHT.LINES.POLARSTEREOGRAPHIC.MAP ............. (default)
c ** STRAIGHT.LINES.INPUT.GRID
c ** STRAIGHT.LINES.GEOGRAPHIC.GRID
c ** INTERPOLATION.BEST ................................ (default)
c ** INTERPOLATION.SIMPLE
c ** END
c **
c **-------------------------------------------------------------------
c ** Options    Remember '......' syntax.
c ** ($... = environment var. ; #n = coomand line arg. no. n, n>1)
c **-------------------------------------------------------------------
c *>
c 'FILE= #7'                      <<< output file
c 'FORMAT.STANDARD'
c 'GRID= 88, #4'                  <<< producer_no,grid_no .
c 'TEXT= #6'                      <<< short plot text
c 'DATA.SIGMA.KS= #5'
c 'PROG.LIMIT= #3'                <<< only used if one FILE.IN
c 'FILE.IN= #2'                   <<< file for following FORECAST's
c 'FORECAST= 3,0,  2,6,  2,12, 2,18, 2,24'
c 'FORECAST= 2,30, 2,36, 2,42, 2,48'
c 'PARAMETER= 2,3,18,9,13,11'   <<< 2=u 3=v 18=th 9=q 13=omega 11=s.dot
c 'PARAM.MIN= 9,0.0'
c 'COMPUTE.VORTICITY'               <<< stored as parameter -1
c 'COMPUTE.DIVERGENCE'              <<< stored as parameter -2
c 'COMPUTE.GEOSTROPHIC.WIND'        <<< stored as parameter -3 and -4
c 'COMPUTE.POTENTIAL.VORTICITY'     <<< stored as parameter -5
c 'SURFACE.PARAMETER= 8'            <<< 8=ps   (sigma.0-2: 78=pb)
c 'END'
c **------------------------------------------------------------------
c ** Vertical crossections.
c **
c **  ITYPE: endpoint type, 0=end  1=I,J  2=X,Y  3=LAT,LONG(decimal)
c **                               4=LAT,LONG(degrees*100+minutes)
c **  NPOS:  no. of positions (with straight lines between them)
c **  X,Y:   coordinate pair (of one of the allowed position types)
c ** 'Name': crossection name
c **
c ** ITYPE, NPOS, X1,Y1, X2,Y2, ....., 'Name'
c *>
c 1, 2,  1,97, 121, 1, '(1,97)-(121,1)'
c 1, 2,  1,49, 121,49, '(1,49)-(121,49)'
c 1, 2, 61,97,  61, 1, '(61,97)-(61,1)'
c 0, 2, 00,00,  00,00, '=============='  <-- end of crossection list
c *
c *------------------------------------------------------------------
c *  PARAMETERS:
c *     2 = U
c *     3 = V
c *    18 = POTENTIAL TEMPERATURE
c *     9 = SPECIFIC HUMIDITY
c *    13 = OMEGA
c *    11 = SIGMADOT (0 in k=1, input in sigma1 levels,
c *                   interpolated to sigma2 levels)
c *    .... OTHER PARAMETERS
c *    -1 = VORTICITY
c *    -2 = DIVERGENCE
c *    -3 = X COMPONENT OF GEOSTROPHIC WIND (UG)
c *    -4 = Y COMPONENT OF GEOSTROPHIC WIND (VG)
c *    -5 = POTENTIAL VORTICITY
c *
c *  SURFACE PARAMETERS:
c *     8 = PS for sigma and eta
c *    78 = PB for sigma 0. - 2. vertical coordinate system (ps,pb)
c *   301 = sea elevation
c *   351 = sea bottom
c *   101 = topography for sigma height levels
c *--------------------------------------------------------------------
c=======================================================================
c
c
c=======================================================================
c  FILE=<output_file>
c  FORMAT.STANDARD ............................ (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  GRID=<producer,grid>
c  TEXT=<short_plot_text>
c  DATA.SIGMA.KS=<no_of_sigma_levels>
c  DATA.SIGMA.K=<level_no,level_no,...>
c  DATA.SIGMA.0-2.KS=<no_of_sigma_levels>   ... sigma(0-2) with ps,pb
c  DATA.SIGMA.0-2.K=<level_no,level_no,...> ... sigma(0-2) with ps,pb
c  DATA.ETA.LEVELS=<no_of_eta_levels>
c  DATA.ETA.LEVEL=<level_no,level_no,...>
c  DATA.PRESSURE=<level,level,...> ............ (hPa)
c  DATA.POT.TEMP=<level,level,...> ............ (1/10 Kelvin)
c  DATA.SEA.DEPTH=<depth,depth,...> ........... (m)
c  DATA.SIGMA.HEIGHT.ALL.LEVELS=<no_of_levels>
c  DATA.SIGMA.HEIGHT.LEVEL=<level_no,level_no,...>
c  DATA.SIGMA.MM5.ALL.LEVELS=<no_of_levels>
c  DATA.SIGMA.MM5.LEVEL=<level_no,level_no,...>
c  DATA.UM.ALL.LEVELS=<no_of_levels>
c  DATA.UM.LEVEL=<level_no,level_no,...>
c  PROG.LIMIT=<min_prog_hour,max_prog_hour> ... (default = no limit)
c  FILE.IN=<input_felt_file> .................... for following f/c's
c  FORECAST=<datatype,hour, datatype,hour,...>
c  PARAMETER=<param_no,param_no,....>
c  PARAM.MIN= <param,min_value, param,min_value,...> ... test after interpol.
c  PARAM.MAX= <param,max_value, param,max_value,...>
c  PARAM.MIN.MAX= <param,min_value,max_value, param,...>
c  COMPUTE.VORTICITY                <<< stored as parameter -1
c  COMPUTE.DIVERGENCE               <<< stored as parameter -2
c  COMPUTE.GEOSTROPHIC.WIND         <<< stored as parameter -3 and -4
c  COMPUTE.POTENTIAL.VORTICITY      <<< stored as parameter -5
c  COMPUTE.HORIZONTAL.VORTICITY     <<< stored as parameter -6 and -7
c  SURFACE.PARAMETER=<param_no,..>  <<< 8=ps   (sigma.0-2: 78=pb)
c  SURFACE.PARAMETER=<param_no,..>  <<< 301=sealevel 351=bottom
c  SURFACE.PARAMETER.LEVEL=<param_no,level, param_no,level, ..>
c  STRAIGHT.LINES.POLARSTEREOGRAPHIC.MAP ............. (default)
c  STRAIGHT.LINES.INPUT.GRID
c  STRAIGHT.LINES.GEOGRAPHIC.GRID
c  INTERPOLATION.BEST ................................ (default)
c  INTERPOLATION.SIMPLE
c  END
c=======================================================================
c
c-----------------------------------------------------------------------
c      DNMI library subroutines:  rfelt
c                                 rlunit
c                                 rcomnt
c                                 getvar
c                                 keywrd
c                                 prhelp
c                                 bswap2
c                                 gridpar
c                                 mapfield
c                                 xyconvert
c                                 uvconvert
c                                 rmfile
c                                 pos2pos
c                                 int2pos
c                                 asr4i2
c                                 bput*
c
c-----------------------------------------------------------------------
c  DNMI/FoU  21.09.1987  Anstein Foss
c  DNMI/FoU  03.03.1992  Anstein Foss
c  DNMI/FoU  27.02.1994  Anstein Foss
c  DNMI/FoU  07.03.1994  Anstein Foss
c  DNMI/FoU  30.03.1994  Anstein Foss
c  DNMI/FoU  31.05.1994  Anstein Foss
c  DNMI/FoU  26.08.1994  Anstein Foss ... single level topography
c  DNMI/FoU  18.05.1995  Anstein Foss ... eta levels, misc. grids, bput*
c  DNMI/FoU  20.09.1995  Anstein Foss ... interp.bilinear, rm bug.undef
c  DNMI/FoU  19.12.1995  Anstein Foss
c  DNMI/FoU  04.01.1996  Anstein Foss ... geost. wind in pot.temp levels
c  DNMI/FoU  14.06.1996  Anstein Foss ... minor int2pos,pos2pos update
c  DNMI/FoU  02.09.1996  Anstein Foss ... better height in eta levels
c  DNMI/FoU  20.09.1996  Anstein Foss ... hermit interpolation (again)
c  DNMI/FoU  17.11.1996  Anstein Foss ... only hermit interpolation
c  DNMI/FoU  28.05.1997  Anstein Foss ... sigma height levels (MEMO)
c  DNMI/FoU  08.10.1997  Anstein Foss ... rotation to E/W and N/S
c  DNMI/FoU  01.12.1998  Anstein Foss ... straight.lines.geographic.grid
c  DNMI/FoU  10.12.1999  Anstein Foss ... sigma.MM5
c  DNMI/FoU  17.03.2001  Anstein Foss ... automatic byte swap (input)
c  DNMI/FoU  03.02.2003  Anstein Foss ... param.min/max/min.max=...
c  DNMI/FoU  18.06.2003  Anstein Foss ... sigma.MM5 bugfix+++
c  DNMI/FoU  10.06.2004  Anstein Foss ... horizontal.vorticity
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c  DNMI/FoU  26.10.2005  Anstein Foss ... surface.parameter.level option
c  DNMI/FoU  09.11.2005  Anstein Foss ... no exit until all timesteps tried
c  DNMI/FoU  15.12.2005  Anstein Foss ... name !! refpos=... name=...
c  DNMI/FoU  27.03.2006  Anstein Foss ... data.um
c-----------------------------------------------------------------------
c
      include 'vcdata.inc'
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..vcdata.inc ..... include file for vcdata.f
c
c  maxij   : max input field size
c  maxd    : max data length (multi  level parameters), one timestep
c  maxd1   : max data length (single level parameters), one timestep
c  maxlev  : max no. of levels
c  maxpar  : max no. of multi  level parameters
c  maxpr1  : max no. of single level parameters
c  maxcrs  : max no. of crossections
c  maxtim  : max no. of timesteps
c  maxinp  : max no. of input positions, all crossections
c  maxpos  : max no. of positions along all crossections
c  maxflt  : max no. of fields for misc. horizontal computations
c  mindat  : max length of read field buffer
c  mparlim : max no. of parameter with min/max limits (after interpolation) 
c
ccc   parameter (maxij=80000)
ccc   parameter (maxd=1000000,maxd1=100000)
ccc   parameter (maxlev=50,maxpar=40,maxpr1=15)
ccc   parameter (maxcrs=80,maxtim=60)
ccc   parameter (maxinp=500)
ccc   parameter (maxpos=9000)
ccc   parameter (maxflt=14)
ccc   parameter (mparlim=50)
c
ccc   parameter (mindat=20+maxij+50)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c  no. of 'compute.xxxxx' parameters
      parameter (mcomp=7)
c
c..parameters added by compute option or always
      parameter (nparx=mcomp,npar1x=8)
c
c..length of second file header
      parameter (linfout=32)
c
c..buffer and file record length
      parameter (lbuff=1024)
      integer*2 buff(lbuff)
c
c..dimension xxx(max+1) for nice overflow testing
c
      integer ilev(maxlev+1),ipar(maxpar+nparx+1)
      integer iparlev(maxpar+nparx+1)
      integer ipar1(maxpr1+npar1x+1),itim(3,maxtim+1)
      integer ipar1lev(maxpr1+npar1x+1),kpar1(maxpr1+npar1x+1)
      integer numpos(9,maxcrs),itime(5,maxtim)
      integer khermit(2,maxpos,2),ihermit(2,maxpos)
      integer lname(maxcrs),iwhere(2,maxcrs,maxtim)
      integer igsize(4),igsiz2(4),iout(maxcrs)
      integer iparlim(2,mparlim+1)
      real    posinp(3,maxinp),posmap(2,maxpos),grid(6),grid2(6)
      real    geopos(maxpos,2),gxypos(maxpos,2),pardat(maxpos,4)
      real    outpos(maxpos,npar1x)
      real    dhermit(2,maxpos)
      real    vrange(2,maxcrs)
      real    alevel(maxlev),blevel(maxlev)
      real    parlim(2,mparlim+1)
      character*256 cname(maxcrs)
c
c..arrays for all crossections:
c..       cdat(npos_total,npar,nlev)  cdat1(npos_total,npar1)
c
c..output for each crossection:
c..       cdat(npos,nlev,npar)  cdat1(npos,npar1)
c
      real    cdat(maxd),cdat1(maxd1)
c
      integer isdat(maxpar*maxlev),isdat1(maxpr1+npar1x)
     +       ,isgrid(6)
      real    sgrid(6)
c
      real      f(maxij,maxflt),fmap(maxij,3)
      integer*2 indat(mindat)
c
      integer   icomp(mcomp)
      integer   ilevot(maxlev)
      integer   info(12),idato(4),idfout(4),infout(32)
c
      integer*2 idfile(32)
c
      character*256 fileot,filein(maxtim),text
c
      parameter (maxkey=20)
      integer   kwhere(5,maxkey)
      character*256 cinput,cipart
      character*256 finput
      character*1   tchar
c
      logical   swapfile,swap
c
      data undef/+1.e+35/
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
c..file unit for 'sondat.input'
      iuinp=9
c
c..file unit for all input felt files
      iunitf=20
c
c..file unit for output data file
      iunito=30
c
c--------------------------------------------------------------------
c
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '   usage: vcdata <vcdata.input>'
        write(6,*) '      or: vcdata <vcdata.input> <arguments>'
        write(6,*) '      or: vcdata <vcdata.input> ?    (to get help)'
        write(6,*)
        stop 1
      end if
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     *           access='sequential',form='formatted',
     *           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error: ',finput(1:lenstr(finput,1))
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
      write(6,*) 'reading input file: ',finput(1:lenstr(finput,1))
c
      nlines = 0
c
      iformt =-1
      nrprod = 0
      nrgrid = 0
      ivcoor = 0
      levspec= 0
      nlev   = 0
      ilevpn = 0
      ntim   = 0
      nfilin = 0
      npar   = 0
      npar1  = 0
      inter  = 0
      nparlim= 0
c
      do i=1,mcomp
        icomp(i) = 0
      end do
c
      isline=-1
c
      fileot ='*'
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
	cinput=' '
        read(iuinp,*,iostat=ios,err=211,end=212) cinput
c
c..check if input as environment variables or command line arguments
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
	  cipart=' '
c
ccc         l=kwhere(1,ikey)
           k1=kwhere(2,ikey)
           k2=kwhere(3,ikey)
          kv1=kwhere(4,ikey)
          kv2=kwhere(5,ikey)
c
c======================================================================
c  FILE=<output_file>
c  FORMAT.STANDARD ............................ (default)
c  FORMAT.SWAP
c  FORMAT.PC
c  FORMAT.IBM
c  GRID=<producer,grid>
c  TEXT=<short_plot_text>
c  DATA.SIGMA.KS=<no_of_sigma_levels>
c  DATA.SIGMA.K=<level_no,level_no,...>
c  DATA.SIGMA.0-2.KS=<no_of_sigma_levels>   ... sigma(0-2) with ps,pb
c  DATA.SIGMA.0-2.K=<level_no,level_no,...> ... sigma(0-2) with ps,pb
c  DATA.ETA.LEVELS=<no_of_eta_levels>
c  DATA.ETA.LEVEL=<level_no,level_no,...>
c  DATA.PRESSURE=<level,level,...> ............ (hPa)
c  DATA.POT.TEMP=<level,level,...> ............ (1/10 Kelvin)
c  DATA.SEA.DEPTH=<depth,depth,...> ........... (m)
c  DATA.SIGMA.HEIGHT.ALL.LEVELS=<no_of_levels>
c  DATA.SIGMA.HEIGHT.LEVEL=<level_no,level_no,...>
c  DATA.SIGMA.MM5.ALL.LEVELS=<no_of_levels>
c  DATA.SIGMA.MM5.LEVEL=<level_no,level_no,...>
c  DATA.UM.ALL.LEVELS=<no_of_levels>
c  DATA.UM.LEVEL=<level_no,level_no,...>
c  PROG.LIMIT=<min_prog_hour,max_prog_hour> ... (default = no limit)
c  FILE.IN=<input_felt_file> .................... for following f/c's
c  FORECAST=<datatype,hour, datatype,hour,...>
c  PARAMETER=<param_no,param_no,....>
c  PARAM.MIN= <param,min_value, param,min_value,...> ... test after interpol.
c  PARAM.MAX= <param,max_value, param,max_value,...>
c  PARAM.MIN.MAX= <param,min_value,max_value, param,...>
c  COMPUTE.VORTICITY                <<< stored as parameter -1
c  COMPUTE.DIVERGENCE               <<< stored as parameter -2
c  COMPUTE.GEOSTROPHIC.WIND         <<< stored as parameter -3 and -4
c  COMPUTE.POTENTIAL.VORTICITY      <<< stored as parameter -5
c  COMPUTE.HORIZONTAL.VORTICITY     <<< stored as parameter -6 and -7
c  SURFACE.PARAMETER=<param_no,..>  <<< 8=ps   (sigma.0-2: 78=pb)
c  SURFACE.PARAMETER=<param_no,..>  <<< 301=sealevel 351=bottom
c  SURFACE.PARAMETER.LEVEL=<param_no,level, param_no,level, ..>
c  STRAIGHT.LINES.POLARSTEREOGRAPHIC.MAP ............. (default)
c  STRAIGHT.LINES.INPUT.GRID
c  STRAIGHT.LINES.GEOGRAPHIC.GRID
c  INTERPOLATION.BEST ................................ (default)
c  INTERPOLATION.SIMPLE
c  END
c======================================================================
c
          if(cinput(k1:k2).eq.'file') then
c..file=<output_file>
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
          elseif(cinput(k1:k2).eq.'grid') then
c..grid=<producer,grid>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nrprod,nrgrid
            if(nrprod.lt.1 .or. nrprod.gt.99) goto 213
          elseif(cinput(k1:k2).eq.'text') then
c..text=<short_plot_text>
            if(kv1.lt.1) goto 213
            text=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'data.sigma.ks') then
c..data.sigma.ks=<no_of_sigma_levels>
            if(levspec.ne.0) goto 214
            levspec=1
            ivcoor=2
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nlev
            if(nlev.lt.1) goto 213
            if(nlev.gt.maxlev) goto 233
            do i=1,nlev
              ilev(i)=i
            end do
            ilevpn=+1
          elseif(cinput(k1:k2).eq.'data.sigma.k') then
c..data.sigma.k=<level_no,level_no,...>
            if(levspec.ne.0 .and. levspec.ne.2) goto 214
            levspec=2
            ivcoor=2
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.sigma.0-2.ks') then
c..data.sigma.0-2.ks=<no_of_sigma_levels>
            if(levspec.ne.0) goto 214
            levspec=3
            ivcoor=22
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nlev
            if(nlev.lt.1) goto 213
            if(nlev.gt.maxlev) goto 233
            do i=1,nlev
              ilev(i)=i
            end do
            ilevpn=+1
          elseif(cinput(k1:k2).eq.'data.sigma.0-2.k') then
c..data.sigma.0-2.k=<level_no,level_no,...>
            if(levspec.ne.0 .and. levspec.ne.4) goto 214
            levspec=4
            ivcoor=22
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.eta.levels') then
c..data.eta.levels=<no_of_eta_levels>
            if(levspec.ne.0) goto 214
            levspec=5
            ivcoor=10
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nlev
            if(nlev.lt.1) goto 213
            if(nlev.gt.maxlev) goto 233
            do i=1,nlev
              ilev(i)=i
            end do
            ilevpn=+1
          elseif(cinput(k1:k2).eq.'data.eta.level') then
c..data.eta.level=<level_no,level_no,...>
            if(levspec.ne.0 .and. levspec.ne.6) goto 214
            levspec=6
            ivcoor=10
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.pressure') then
c..data.pressure=<level,level,...>
            if(levspec.ne.0 .and. levspec.ne.7) goto 214
            levspec=7
            ivcoor=1
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.pot.temp') then
c..data.pot.temp=<level,level,...>
            if(levspec.ne.0 .and. levspec.ne.8) goto 214
            levspec=8
            ivcoor=4
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.sea.depth') then
c..data.sea.depth=<depth_m,depth_m,...>
            if(levspec.ne.0 .and. levspec.ne.9) goto 214
            levspec=9
            ivcoor=5
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.sigma.height.all.levels') then
c..data.sigma.height.all.levels=<no_of_levels>
            if(levspec.ne.0) goto 214
            levspec=10
            ivcoor=11
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nlev
            if(nlev.lt.1) goto 213
            if(nlev.gt.maxlev) goto 233
            do i=1,nlev
              ilev(i)=i
            end do
            ilevpn=+1
          elseif(cinput(k1:k2).eq.'data.sigma.height.level') then
c..data.sigma.height.level=<level_no,level_no,...>
            if(levspec.ne.0 .and. levspec.ne.11) goto 214
            levspec=11
            ivcoor=11
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.sigma.mm5.all.levels') then
c..data.sigma.mm5.all.levels=<no_of_levels>
            if(levspec.ne.0) goto 214
            levspec=12
            ivcoor=12
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nlev
            if(nlev.lt.1) goto 213
            if(nlev.gt.maxlev) goto 233
            do i=1,nlev
              ilev(i)=i
            end do
            ilevpn=+1
          elseif(cinput(k1:k2).eq.'data.sigma.mm5.level') then
c..data.sigma.mm5.level=<level_no,level_no,...>
            if(levspec.ne.0 .and. levspec.ne.13) goto 214
            levspec=13
            ivcoor=12
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'data.um.all.levels') then
c..data.um.all.levels=<no_of_levels>
            if(levspec.ne.0) goto 214
            levspec=14
            ivcoor=-12
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) nlev
            if(nlev.lt.1) goto 213
            if(nlev.gt.maxlev) goto 233
            do i=1,nlev
              ilev(i)=i
            end do
            ilevpn=+1
          elseif(cinput(k1:k2).eq.'data.um.level') then
c..data.um.level=<level_no,level_no,...>
            if(levspec.ne.0 .and. levspec.ne.13) goto 214
            levspec=15
            ivcoor=-12
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nlev+1
            i2=nlev
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxlev) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ilev(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            nlev=i2
            do i=2,nlev
              if(ilevpn.eq.0 .and. ilev(i).lt.ilev(i-1)) ilevpn=-1
              if(ilevpn.eq.0) ilevpn=+1
              if(ilevpn.eq.+1 .and. ilev(i).le.ilev(i-1)) goto 213
              if(ilevpn.eq.-1 .and. ilev(i).ge.ilev(i-1)) goto 213
            end do
          elseif(cinput(k1:k2).eq.'prog.limit') then
c..prog.limit=<min_prog_hour,max_prog_hour>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)
            read(cipart,*,err=213) iprmin,iprmax
          elseif(cinput(k1:k2).eq.'file.in') then
c..file.in=<input_felt_file> .................... for following f/c
            if(nfilin.eq.maxtim) goto 233
            if(kv1.lt.1) goto 213
            nfilin=nfilin+1
            filein(nfilin)=cinput(kv1:kv2)
          elseif(cinput(k1:k2).eq.'forecast') then
c..forecast=<datatype,hour, datatype,hour,...>
            if(kv1.lt.1) goto 213
            if(nfilin.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=ntim+1
            i2=ntim
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxtim) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (itim(2,i),itim(3,i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            do i=i1,i2
              itim(1,i)=nfilin
            end do
            ntim=i2
          elseif(cinput(k1:k2).eq.'parameter') then
c..parameter=<param_no,param_no,....>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=npar+1
            i2=npar
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxpar) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ipar(i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            npar=i2
          elseif(cinput(k1:k2).eq.'param.min.max') then
c..param.min.max= <param,min_value,max_value, param,...>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nparlim+1
            i2=nparlim
            ios=0
            do while (ios.eq.0)
              if(i2.gt.mparlim) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (iparlim(1,i),parlim(1,i),
     +					 parlim(2,i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
	    do i=i1,i2
	      iparlim(2,i)=3
	    end do
            nparlim=i2
          elseif(cinput(k1:k2).eq.'param.min') then
c..param.min= <param,min_value, param,min_value,...> ... test after interpol.
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nparlim+1
            i2=nparlim
            ios=0
            do while (ios.eq.0)
              if(i2.gt.mparlim) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (iparlim(1,i),	
     +					 parlim(1,i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
	    do i=i1,i2
	      iparlim(2,i)=1
	       parlim(2,i)=+999999.
	    end do
            nparlim=i2
          elseif(cinput(k1:k2).eq.'param.max') then
c..param.max= <param,max_value, param,max_value,...>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=nparlim+1
            i2=nparlim
            ios=0
            do while (ios.eq.0)
              if(i2.gt.mparlim) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (iparlim(1,i),
     +					 parlim(2,i),i=i1,i2)
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
	    do i=i1,i2
	      iparlim(2,i)=2
	       parlim(1,i)=-999999.
	    end do
            nparlim=i2
          elseif(cinput(k1:k2).eq.'compute.vorticity') then
c..compute.vorticity
            icomp(1)=1
          elseif(cinput(k1:k2).eq.'compute.divergence') then
c..compute.divergence
            icomp(2)=1
          elseif(cinput(k1:k2).eq.'compute.geostrophic.wind') then
c..compute.geostrophic.wind
            icomp(3)=1
            icomp(4)=1
          elseif(cinput(k1:k2).eq.'compute.potential.vorticity') then
c..compute.potential.vorticity
            icomp(5)=1
          elseif(cinput(k1:k2).eq.'compute.horizontal.vorticity') then
c..compute.horizontal.vorticity
            icomp(6)=1
            icomp(7)=1
          elseif(cinput(k1:k2).eq.'surface.parameter') then
c..surface.parameter=<param_no,param_no,....>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=npar1+1
            i2=npar1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxpr1) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ipar1(i),i=i1,i2)
	      kpar1(i2)=0
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            npar1=i2
          elseif(cinput(k1:k2).eq.'surface.parameter.level') then
c..surface.parameter.level=<param_no,level, param_no,level,....>
            if(kv1.lt.1) goto 213
            cipart=cinput(kv1:kv2)//tchar
            i1=npar1+1
            i2=npar1
            ios=0
            do while (ios.eq.0)
              if(i2.gt.maxpr1) goto 233
              i2=i2+1
              read(cipart,*,iostat=ios) (ipar1(i),ipar1lev(i),i=i1,i2)
	      kpar1(i2)=1
            end do
            i2=i2-1
            if(i2.lt.i1) goto 213
            npar1=i2
          elseif(cinput(k1:k2).eq.
     +           'straight.lines.polarstereographic.map') then
c..straight.lines.polarstereographic.map
            if(isline.ne.-1) goto 214
            isline=1
          elseif(cinput(k1:k2).eq.'straight.lines.input.grid') then
c..straight.lines.input.grid
            if(isline.ne.-1) goto 214
            isline=0
          elseif(cinput(k1:k2).eq.
     +			'straight.lines.geographic.grid') then
c..straight.lines.geographic.grid
            if(isline.ne.-1) goto 214
            isline=2
c----------------------------------------------------------------
          elseif(cinput(k1:k2).eq.
     +           'preferable.interpolation.hermit.on') then
c..preferable.interpolation.hermit.on
            write(6,*) 'WARNING. OBSOLETE OPTION: ',cinput(k1:k2)
          elseif(cinput(k1:k2).eq.
     +           'preferable.interpolation.hermit.off') then
c..preferable.interpolation.hermit.off
            write(6,*) 'WARNING. OBSOLETE OPTION: ',cinput(k1:k2)
          elseif(cinput(k1:k2).eq.'interp.bessel') then
c..interp.bessel
            write(6,*) 'WARNING. OBSOLETE OPTION: ',cinput(k1:k2)
ccc         write(6,*) '        USING NEW OPTION: interpolation.best'
ccc         if(inter.ne.0) goto 214
ccc         inter=1
          elseif(cinput(k1:k2).eq.'interp.bilinear') then
c..interp.bilinear
            write(6,*) 'WARNING. OBSOLETE OPTION: ',cinput(k1:k2)
ccc         write(6,*) '        USING NEW OPTION: interpolation.simple'
ccc         if(inter.ne.0) goto 214
ccc         inter=2
c----------------------------------------------------------------
          elseif(cinput(k1:k2).eq.'interpolation.best') then
c..interpolation.best
            if(inter.ne.0) goto 214
            inter=1
          elseif(cinput(k1:k2).eq.'interpolation.simple') then
c..interpolation.simple
            if(inter.ne.0) goto 214
            inter=2
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
      if(nrprod.lt.1 .or. nrprod.gt.99) goto 219
c..data.......=
      if(ivcoor.eq.0) goto 220
c..file=
      if(fileot(1:1).eq.'*') goto 227
c..file.in= and forecast=
      if(nfilin.lt.1 .or. ntim.lt.1) goto 228
c
c..default output file format
      if(iformt.lt.0) iformt=0
c
c..default straight.lines.polarstereographic.map
      if(isline.eq.-1) isline=1
c
c..default interpolation type ('best')
      if(inter.eq.0) inter=1
c
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 210
c
      ityp=1
      ncross=0
      np=0
c
      do while (ityp.gt.0 .and. ncross.lt.maxcrs)
        n=ncross+1
        nlines=nlines+1
        read(iuinp,*,iostat=ios,err=211,end=212)
     *      ityp,npos,((posinp(i,j),i=1,2),j=np+1,np+npos),cname(n)
        if(ityp.gt.0) then
          if(npos.lt.2) then
            write(6,*) 'at least 2 positions needed to get a line'
            goto 240
          elseif(np+npos.gt.maxinp) then
            write(6,*) 'too many positions specified.   max:',maxinp
            goto 240
          elseif(ityp.gt.4) then
            write(6,*) 'illegal position type:',ityp
            goto 240
          end if
          if(ityp.eq.4) then
            do j=np+1,np+npos
              ilat=nint(posinp(1,j))
              ilon=nint(posinp(2,j))
              ilatd=ilat/100
              ilond=ilon/100
              ilatm=ilat-ilatd*100
              ilonm=ilon-ilond*100
              if(ilatd.ge. -90 .and. ilatd.le. +90 .and.
     +           ilond.ge.-360 .and. ilond.le.+360 .and.
     +           ilatm.ge. -59 .and. ilatm.le. +59 .and.
     +           ilonm.ge. -59 .and. ilonm.le. +59) then
                posinp(1,j)=real(ilatd)+real(ilatm)/60.
                posinp(2,j)=real(ilond)+real(ilonm)/60.
              else
                write(6,*) 'BAD LAT/LONG POSITION:'
                write(6,*) '    ',ilat,ilon,' ',
     +				cname(n)(1:lenstr(cname(n),1))
                goto 240
              end if
            end do
            ityp=2
          elseif(ityp.eq.3) then
            ityp=2
          else
            ityp=1
          end if
          numpos(1,n)=ityp
          numpos(2,n)=npos
          numpos(3,n)=np+1
          numpos(4,n)=np+npos
          numpos(5,n)=0
          numpos(6,n)=0
          numpos(7,n)=0
          numpos(8,n)=0
          numpos(9,n)=0
          ncross=n
          np=np+npos
        end if
      end do
c
      if(ityp.gt.0) then
        read(iuinp,*,iostat=ios,err=211,end=212) ityp
        if(ityp.gt.0) then
          write(6,*) 'too many crossections.   max:',maxcrs
          goto 240
        end if
      end if
c
      nposinp=np
c
      do i=1,nposinp
	posinp(3,i)=0.
      end do
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
  228 write(6,*) 'no input file or forecasts specified.'
      goto 240
  232 iprhlp=1
      goto 240
  233 write(6,*) 'too many specifications of the type below:'
      write(6,*) cinput
      goto 240
c
  240 write(6,*) 'error at line no. ',nlines,'   (or below)'
      if(iprhlp.eq.1) then
        write(6,*) 'help from ''vcdata.input'':'
        call prhelp(iuinp,'*=>')
      end if
  249 close(iuinp)
      stop 1
c
  250 close(iuinp)
c
      write(6,*) 'input o.k.'
c
c..input  level sequence: as specified in input (unless sigma/eta)
c..output level sequence: bottom to top (as plot)
      if((ivcoor.eq.2  .or. ivcoor.eq.22 .or.
     +    ivcoor.eq.10 .or. ivcoor.eq.12) .and. ilevpn.eq.-1) then
c..sigma/eta:
c..required sequence for geostrophic wind and potential vorticity
        do n=1,nlev/2
          i=ilev(n)
          ilev(n)=ilev(nlev+1-n)
          ilev(nlev+1-n)=i
        end do
        ilevpn=+1
      end if
      lo=-1
c..isentropic surfaces(4) and sigma heights(11) are stored in
c..the needed order, bottom to top
      if(ivcoor.eq.4 .or. ivcoor.eq.11) lo=+1

      if(ivcoor.eq.-12) then
c..UM, levels in order bottom to top
        if (ilevpn.eq.-1) then
          do n=1,nlev/2
            i=ilev(n)
            ilev(n)=ilev(nlev+1-n)
            ilev(nlev+1-n)=i
          end do
          ilevpn=+1
	end if
	ivcoor=12
	lo=+1
      end if
c
      if(ilevpn*lo.lt.0) then
        do n=1,nlev
          ilevot(n)=nlev+1-n
        end do
      else
        do n=1,nlev
          ilevot(n)=n
        end do
      end if
c
c..begerenser evt. tidsserien (bare hvis prognoser fra en file)
c
      if(nfilin.eq.1) then
        nt=0
        do n=1,ntim
          if(itim(3,n).ge.iprmin .and. itim(3,n).le.iprmax) then
            nt=nt+1
            if(nt.ne.n) then
              itim(1,nt)=itim(1,n)
              itim(2,nt)=itim(2,n)
              itim(3,nt)=itim(3,n)
            end if
          end if
        end do
        if(nt.lt.1) then
          write(6,*) ' ingen tidspunkt etter prognose-begrensning:'
          write(6,*) ' prog.limit = ',iprmin,iprmax
          stop 117
        end if
        ntim=nt
      end if
c
c..compute.xxxxx
      if(ivcoor.eq.2 .or. ivcoor.eq.22 .or. ivcoor.eq.10) then
ccc     icomp(6)=0
ccc     icomp(7)=0
        continue
      elseif(ivcoor.eq.1) then
        icomp(5)=0
ccc     icomp(6)=0
ccc     icomp(7)=0
      elseif(ivcoor.eq.4) then
        icomp(5)=0
        icomp(6)=0
        icomp(7)=0
      else
        icomp(3)=0
        icomp(4)=0
        icomp(5)=0
ccc     icomp(6)=0
ccc     icomp(7)=0
      end if
c
      do i=1,mcomp
        if(icomp(i).eq.1) then
          npar=npar+1
          ipar(npar)=-i
        end if
      end do
c
c..no. of identifiers for each level (alevel,blevel,...)
      nlvlid=1
      if(ivcoor.eq.2 .or. ivcoor.eq.22 .or. ivcoor.eq.10) nlvlid=2
c..sigma height: need alevel(1) to store htop and sigma in blevel
      if(ivcoor.eq.11) nlvlid=2
c..sigma.MM5,Sinra,UM,...: Using pressure in alle levels
      if(ivcoor.eq.12) nlvlid=0
c
      nlev1=1
      ilev1=1000
      if(ivcoor.eq.5) ilev1=0
c
      do i=1,npar1
	if (kpar1(i).eq.0) ipar1lev(i)=ilev1
      end do
c
c..x,y,s,coriolis,lat,long stored as 'one level' parameter in output file
c
      nnpar1=npar1+npar1x
c..x
      ipar1(npar1+1)=-1001
c..y
      ipar1(npar1+2)=-1002
c..s (length along crossection) ... sometimes rather low accuracy
ccc   ipar1(npar1+3)=-1003
c..ds (length increments along crossection, as computed now)
      ipar1(npar1+3)=-1007
c..coriolis parameter
      ipar1(npar1+4)=-1004
c..latitude
      ipar1(npar1+5)=-1005
c..longitude
      ipar1(npar1+6)=-1006
c..factors for vector rotation to E/W and N/S components
      ipar1(npar1+7)=-1008
      ipar1(npar1+8)=-1009
c
c..for sigma/eta
      nps=0
ccc   npb=0
c..for sigma heigt
      npzs=0
c..for sea depths
      npsurf=0
      npbott=0
      do n=1,npar1
        if(ipar1(n).eq.  8) nps=n
ccc     if(ipar1(n).eq. 78) npb=n
        if(ipar1(n).eq.101) npzs=n
        if(ipar1(n).eq.301) npsurf=n
        if(ipar1(n).eq.351) npbott=n
      end do
c
      npp=0
      do n=1,npar
        if(ipar(n).eq. 8) npp=n
      end do
c
c..output formats:
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
      if(iformt.eq.1) then
c..format.swap
        konvd=1
      elseif(iformt.eq.2) then
c..format.pc
        konvc=1
        konvd=1
      elseif(iformt.eq.3) then
c..format.ibm
        konvc=2
      endif
c
c..remove existing output file (unless it is a link)
      call rmfile(fileot,0,irmerr)
c
      iopeno=0
c
      do n=1,ntim
        do i=1,ncross
          iwhere(1,i,n)=0
          iwhere(2,i,n)=0
        end do
        do i=1,5
          itime(i,n)=0
        end do
      end do
c
      info(2)=nrprod
      info(3)=nrgrid
      info(9)=ncross
      info(10)=0
      info(12)=maxpos
c
c..min and max pressure for all timesteps and each crossection
      do n=1,ncross
        vrange(1,n)=+1.e+35
        vrange(2,n)=-1.e+35
      end do
c
c..define som tables
      call tabdef
c
      lcdat =maxd
      lcdat1=maxd1
c
      ifirst=0
      ntotal=0
      ifilin=0
      nt=0
c
      do 100 it=1,ntim
c
      if(itim(1,it).ne.ifilin) then
        if(ifilin.gt.0) close(iunitf)
        ifilin=itim(1,it)
c..open input felt file
        open(iunitf,file=filein(ifilin),
     *              access='direct',form='unformatted',
     *              recl=2048/lrunit,
     *              status='old',iostat=ios,err=930)
        swap= swapfile(-iunitf)
        irec=1
        read(iunitf,rec=irec,iostat=ios,err=930) idfile
	if (swap) call bswap2(32,idfile)
        idato(1)=idfile(5)
        idato(2)=idfile(6)/100
        idato(3)=idfile(6)-(idfile(6)/100)*100
        idato(4)=idfile(7)
        write(6,*) filein(ifilin)(1:lenstr(filein(ifilin),1))
        write(6,1001) idato(3),idato(2),idato(1),idato(4)
 1001   format(' time:  ',i2.2,':',i2.2,':',i4.4,1x,i4.4,' utc')
      end if
c
      info(1)=iunitf
      info(4)=itim(2,it)
      info(5)=itim(3,it)
c
      write(6,*) '   data.type,forecast.hour:',info(4),info(5)
c
      info(6)=ivcoor
      info(7)=nlev
      info(8)=npar
      info(11)=maxd
c
c..read data (in all levels)
      call rcdata(ifirst,info,npar,nlev,ipar,iparlev,ilev,ilevot,
     +            numpos,posinp,posmap,
     +            inter,isline,alevel,blevel,
     +            lcdat,cdat,itime(1,nt+1),indat,f,fmap,
     +            ntotal,geopos,gxypos,pardat,
     +            nhermit,khermit,ihermit,dhermit,
     +            igtype,grid,igsize,igtyp2,grid2,igsiz2,
     +            nx,ny,nparlim,iparlim,parlim,ierr)
c
      if(ierr.ne.0) write(6,*) 'rcdata(1).  ierr= ',ierr
      if(ierr.ne.0) goto 100
c
c..pressure  (ivcoor=1):  level = pressure (hPa)
c..pot.temp  (ivcoor=4):  level = deg. Kelvin *10
c..sea depth (ivcoor=5):  level = depth (m)
      if(ivcoor.eq.1 .or. ivcoor.eq.4 .or. ivcoor.eq.5) then
        scale=1.
        if(ivcoor.eq.4) scale=0.1
        if(ivcoor.eq.5) scale=-1.
        do nl=1,nlev
          nlo=ilevot(nl)
          alevel(nlo)=ilev(nl)*scale
          blevel(nlo)=0.
        end do
      end if
c
      if(npar1.gt.0) then
c
        info(6)=ivcoor
        if(ivcoor.eq.10) info(6)=2
        if(ivcoor.eq. 4) info(6)=2
        info(7)=nlev1
        info(8)=npar1
        info(11)=maxd1
c
c..read one level (surface) data
        call rcdata(ifirst,info,npar1,1,ipar1,ipar1lev,ilev1,1,
     +              numpos,posinp,posmap,
     +              inter,isline,alevl1,blevl1,
     +              lcdat1,cdat1,itime(1,nt+1),indat,f,fmap,
     +              ntotal,geopos,gxypos,pardat,
     +              nhermit,khermit,ihermit,dhermit,
     +              igtype,grid,igsize,igtyp2,grid2,igsiz2,
     +              nx,ny,nparlim,iparlim,parlim,ierr)
c
        if(ierr.ne.0) write(6,*) 'rcdata(2).  ierr= ',ierr
        if(ierr.ne.0) goto 100
c
      else
c
        alevl1=0.
        blevl1=0.
c
      end if
c
      nt=nt+1
c
      if((ivcoor.eq.2 .or. ivcoor.eq.22 .or. ivcoor.eq.10)
     +                                      .and. nps.gt.0) then
        if(ivcoor.eq.2 .or. ivcoor.eq.22) then
c..sigma: pressure range (ptop and psurface, not p(k=1) and p(k=ks))
          ptop=alevl1
        else
          ptop=alevel(nlev)
        end if
        do n=1,ncross
          vrange(1,n)=min(vrange(1,n),ptop)
          iadr1=ntotal*(nps-1)+numpos(6,n)
          iadr2=ntotal*(nps-1)+numpos(7,n)
          do i=iadr1,iadr2
            vrange(2,n)=max(vrange(2,n),cdat1(i))
          end do
        end do
      elseif(ivcoor.eq.1) then
c..pressure levels
        do n=1,ncross
          vrange(1,n)=min(vrange(1,n),alevel(nlev))
          vrange(2,n)=max(vrange(2,n),alevel(1))
        end do
      elseif((ivcoor.eq.4 .or. ivcoor.eq.12) .and. npp.gt.0) then
c..potential temperature levels(4) (isentropic surfaces)
c..sigma.MM5(12)
c..find min and max pressure
        ud=0.9*undef
        do n=1,ncross
          npos=numpos(5,n)
          do nl=1,nlev
            iadr1=ntotal*npar*(nl-1)+ntotal*(npp-1)+numpos(6,n)
            iadr2=ntotal*npar*(nl-1)+ntotal*(npp-1)+numpos(7,n)
            do i=iadr1,iadr2
              if(cdat(i).lt.ud) then
                vrange(1,n)=min(vrange(1,n),cdat(i))
                vrange(2,n)=max(vrange(2,n),cdat(i))
              end if
            end do
          end do
        end do
        if(nps.gt.0) then
          do n=1,ncross
            iadr1=ntotal*(nps-1)+numpos(6,n)
            iadr2=ntotal*(nps-1)+numpos(7,n)
            do i=iadr1,iadr2
              vrange(2,n)=max(vrange(2,n),cdat1(i))
            end do
          end do
        end if
      elseif(ivcoor.eq.5) then
c..sea depth
        ud=0.9*undef
        if(npbott.gt.0) then
          do n=1,ncross
            iadr1=ntotal*(npbott-1)+numpos(6,n)
            iadr2=ntotal*(npbott-1)+numpos(7,n)
            do i=iadr1,iadr2
              if(cdat1(i).lt.ud) then
c..change seadepth from positive to negative values
                cdat1(i)=-cdat1(i)
                vrange(1,n)=min(vrange(1,n),cdat1(i))
              end if
            end do
          end do
        else
          do n=1,ncross
            vrange(1,n)=min(vrange(1,n),alevel(1))
          end do
        end if
        if(npsurf.gt.0) then
          do n=1,ncross
            iadr1=ntotal*(npsurf-1)+numpos(6,n)
            iadr2=ntotal*(npsurf-1)+numpos(7,n)
            do i=iadr1,iadr2
              if(cdat1(i).lt.ud) vrange(2,n)=max(vrange(2,n),cdat1(i))
            end do
          end do
        else
          do n=1,ncross
            vrange(2,n)=max(vrange(2,n),alevel(nlev))
          end do
        end if
      elseif(ivcoor.eq.11) then
c..sigma height: height range
        htop=alevl1
        do n=1,ncross
          iadr1=ntotal*(npzs-1)+numpos(6,n)
          iadr2=ntotal*(npzs-1)+numpos(7,n)
          do i=iadr1,iadr2
            vrange(1,n)=min(vrange(1,n),cdat1(i))
          end do
          vrange(2,n)=max(vrange(2,n),htop)
        end do
	alevel(1)=htop
	do nl=2,nlev
	  alevel(nl)=-1.
	end do
      else
        do n=1,ncross
          vrange(1,n)=0.0
          vrange(2,n)=0.0
        end do
      end if
c
c..output to file
c
      if(iopeno.eq.0) then
c
c..reference and name positions
c..( 'name !! refpos=2.5 name=2.5,Position_Name name=1,2.5,Route_Name ...' )
c..positions recomputed from input position no. to crossection position no.
c
	call poscomp(ncross,numpos,posinp,cname)
c
c..character strings must have n*2 characters length
c..(stored in integer*2 words)
c
        ltext=lenstr(text,1)
        ltext=(ltext+1)/2*2
c
        mlname=0
        llname=0
        do n=1,ncross
          lname(n)= lenstr(cname(n),1)
          lname(n)=(lname(n)+1)/2*2
          if(mlname.lt.lname(n)) mlname=lname(n)
          llname=llname+lname(n)
        end do
c
c..open output file
        open(iunito,file=fileot,
     *              access='direct',form='unformatted',
     *              recl=(lbuff*2)/lrunit,
     *              status='unknown',iostat=ios,err=910)
c
c..one or more records to be filled with information later
        npmap=numpos(9,ncross)
        lhead= 4+linfout+ltext/2+6*4+1+ncross*2
     +        +npar+nnpar1+ncross+ncross+llname+5*ntim
     +        +ncross+1+2*npmap+4*ncross*ntim
        nreco=(lhead+lbuff-1)/lbuff
c
c.."dummy" first record(s)
        ireco=0
        do nr=1,nreco
          ibuff=1
          buff(1)=0
          call bputnd(iunito,ireco,buff,lbuff,ibuff,konvd,ierror)
          if(ierror.ne.0) goto 920
        end do
c
        iopeno=1
c
      end if
c
      iundef=1
c
      if(nlvlid.ge.1)
     +   call asr4i2(1,nlev,alevel(1),0,iundef,undef,isalvl,nud)
      if(nlvlid.ge.2)
     +   call asr4i2(1,nlev,blevel(1),0,iundef,undef,isblvl,nud)
c
      do i=1,ntotal
c..x
        outpos(i,1)=gxypos(i,1)
c..y
        outpos(i,2)=gxypos(i,2)
c..ds
        outpos(i,3)=pardat(i,1)
c..coriolis parameter
        outpos(i,4)=pardat(i,2)
c..latitude
        outpos(i,5)=geopos(i,2)
c..longitude
        outpos(i,6)=geopos(i,1)
c..factors for vector rotation to E/W and N/S components
        outpos(i,7)=pardat(i,3)
        outpos(i,8)=pardat(i,4)
      end do
c
      do 150 icross=1,ncross
c
      ireco1=ireco
      ibuff1=ibuff
      npos=numpos(5,icross)
c
      nudef=0
      ns=0
      do np=1,npar
c-----------------------------------------------------------------------
c       write(99,*) '====> Crs,Par,Tim: ',icross,ipar(np),itim(3,it)
c-----------------------------------------------------------------------
        do nl=1,nlev
          ns=ns+1
          iadr=ntotal*npar*(nl-1)+ntotal*(np-1)+numpos(6,icross)
c-----------------------------------------------------------------------
c         write(99,*) 'Crs,Tim,Par,Lvl:     ',
c    +                  icross,itim(3,it),ipar(np),nl
c         write(99,*) (cdat(i),i=iadr,iadr+npos-1)
c-----------------------------------------------------------------------
          call asr4i2(1,npos,cdat(iadr),0,iundef,undef,isdat(ns),nud)
c-----------------------------------------------------------------------
c         write(99,*) '         Isc: ',isdat(ns)
cc        write(99,*) 'Crs,Tim,Par,Lvl,Isc: ',
cc   +                  icross,itim(3,it),ipar(np),nl,isdat(ns)
cc        write(99,*) (cdat(i),i=iadr,iadr+npos-1)
c-----------------------------------------------------------------------
          nudef=nudef+nud
        end do
      end do
c
      nudef1=0
      do np=1,npar1
        iadr=ntotal*(np-1)+numpos(6,icross)
c-----------------------------------------------------------------------
c       write(99,*) 'Crs,Tim,Par1:     ',
c    +                  icross,itim(3,it),ipar1(np)
c       write(99,*) (cdat1(i),i=iadr,iadr+npos-1)
c-----------------------------------------------------------------------
        call asr4i2(1,npos,cdat1(iadr),0,iundef,undef,isdat1(np),nud)
c-----------------------------------------------------------------------
c       write(99,*) '         Isc: ',isdat1(np)
cc      write(99,*) 'Crs,Tim,Par1,Isc: ',
cc   +                  icross,itim(3,it),ipar1(np),isdat1(np)
cc      write(99,*) (cdat1(i),i=iadr,iadr+npos-1)
c-----------------------------------------------------------------------
        nudef1=nudef1+nud
      end do
      np=npar1
      iadr=numpos(6,icross)
      do n=1,npar1x
        np=np+1
c-----------------------------------------------------------------------
c       write(99,*) 'Crs,Tim,Par1:     ',
c    +                  icross,itim(3,it),ipar1(np)
c       write(99,*) (outpos(i,n),i=iadr,iadr+npos-1)
c-----------------------------------------------------------------------
        call asr4i2(1,npos,outpos(iadr,n),0,iundef,undef,isdat1(np),nud)
c-----------------------------------------------------------------------
c       write(99,*) '         Isc: ',isdat1(np)
c-----------------------------------------------------------------------
        nudef1=nudef1+nud
      end do
c
      if(nlvlid.ge.1) then
        call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +              isalvl,1,ierror)
        if(ierror.ne.0) goto 920
        call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +              alevel(1),nlev,ierror)
        if(ierror.ne.0) goto 920
      end if
      if(nlvlid.ge.2) then
        call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +              isblvl,1,ierror)
        if(ierror.ne.0) goto 920
        call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +              blevel(1),nlev,ierror)
        if(ierror.ne.0) goto 920
      end if
c
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            nudef,1,ierror)
      if(ierror.ne.0) goto 920
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            nudef1,1,ierror)
      if(ierror.ne.0) goto 920
c
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            isdat(1),npar*nlev,ierror)
      if(ierror.ne.0) goto 920
c
      do np=1,npar
        do nl=1,nlev
          iadr=ntotal*npar*(nl-1)+ntotal*(np-1)+numpos(6,icross)
          call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +                cdat(iadr),npos,ierror)
          if(ierror.ne.0) goto 920
        end do
      end do
c
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            isdat1(1),nnpar1,ierror)
      if(ierror.ne.0) goto 920
c
      do np=1,npar1
        iadr=ntotal*(np-1)+numpos(6,icross)
        call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +              cdat1(iadr),npos,ierror)
        if(ierror.ne.0) goto 920
      end do
      iadr=numpos(6,icross)
      do n=1,npar1x
        call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +              outpos(iadr,n),npos,ierror)
        if(ierror.ne.0) goto 920
      end do
c
      iwhere(1,icross,nt)=ireco1+1
      iwhere(2,icross,nt)=ibuff1+1
c
  150 continue
c
  100 continue
c
      ntime=nt
c
      close(iunitf)
c
      if(ntime.lt.1) then
        write(6,*) '**** no data ****'
        stop 117
      end if
c
      call bputnd(iunito,ireco,buff,lbuff,ibuff,konvd,ierror)
      if(ierror.ne.0) goto 920
c
c..max length of the crossections
      mpos=0
      do i=1,ncross
        npos=numpos(5,i)
        if(mpos.lt.npos) mpos=npos
      end do
c
c..information record(s)
c
      do n=1,linfout
        infout(n)=0
      end do
c
c..file identification
      idfout( 1)=121
      idfout( 2)=2
      idfout( 3)=lbuff*2
      idfout( 4)=linfout
c
      infout( 1)=ivcoor
      infout( 2)=mpos
      infout( 3)=ncross
      infout( 4)=nlev
      infout( 5)=npar
      infout( 6)=nlev1
      infout( 7)=nnpar1
      infout( 8)=ntim-ntime
      infout( 9)=ntime
      infout(10)=nlvlid
      infout(11)=nrprod
      infout(12)=nrgrid
      infout(13)=ltext
      infout(14)=mlname
      infout(15)=npmap
      infout(16)=igtype
      infout(17)=igsize(1)
      infout(18)=igsize(2)
      infout(19)=igsize(3)
      infout(20)=igsize(4)
      infout(21)=igtyp2
      infout(22)=igsiz2(1)
      infout(23)=igsiz2(2)
      infout(24)=igsiz2(3)
      infout(25)=igsiz2(4)
c
      ireco=0
      ibuff=0
c
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            idfout(1),4,ierror)
      if(ierror.ne.0) goto 920
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            infout(1),linfout,ierror)
      if(ierror.ne.0) goto 920
c
c..text
      call bputch(iunito,ireco,buff,lbuff,ibuff,konvd,konvc,
     +            text,1,ltext,ierror)
      if(ierror.ne.0) goto 920
c
c..input grid parameters
      do i=1,6
        sgrid(i)=grid(i)
        call asr4i2(1,1,sgrid(i),0,iundef,undef,isgrid(i),nud)
      end do
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            isgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
      call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            sgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
c
c..presentation grid parameters
      do i=1,6
        sgrid(i)=grid2(i)
        call asr4i2(1,1,sgrid(i),0,iundef,undef,isgrid(i),nud)
      end do
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            isgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
      call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            sgrid(1),6,ierror)
      if(ierror.ne.0) goto 920
c
c..vertical range
      call asr4i2(1,2*ncross,vrange(1,1),0,iundef,undef,isrange,nud)
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            isrange,1,ierror)
      if(ierror.ne.0) goto 920
      call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            vrange,2*ncross,ierror)
      if(ierror.ne.0) goto 920
c
c..parameter no. (multilevel and single level)
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            ipar(1),npar,ierror)
      if(ierror.ne.0) goto 920
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            ipar1(1),nnpar1,ierror)
      if(ierror.ne.0) goto 920
c
c..no. of positions in each crossection
      do i=1,ncross
        iout(i)=numpos(5,i)
      end do
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            iout,ncross,ierror)
      if(ierror.ne.0) goto 920
c
c..crossection names
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            lname,ncross,ierror)
      if(ierror.ne.0) goto 920
      do i=1,ncross
        call bputch(iunito,ireco,buff,lbuff,ibuff,konvd,konvc,
     +              cname(i),1,lname(i),ierror)
        if(ierror.ne.0) goto 920
      end do
c
c..date/time
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            itime,5*ntime,ierror)
      if(ierror.ne.0) goto 920
c
c..no. of map positions in each crossection
      do i=1,ncross
        iout(i)=numpos(9,i)-numpos(8,i)+1
      end do
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            iout,ncross,ierror)
      if(ierror.ne.0) goto 920
c
c..map positions
      npmap=numpos(9,ncross)
      call asr4i2(1,2*npmap,posmap(1,1),0,iundef,undef,ispmap,nud)
      call bputi4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            ispmap,1,ierror)
      if(ierror.ne.0) goto 920
      do i=1,npmap
c..map x
        outpos(i,1)=posmap(1,i)
c..map y
        outpos(i,2)=posmap(2,i)
      end do
      call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            outpos(1,1),npmap,ierror)
      if(ierror.ne.0) goto 920
      call bputr4(iunito,ireco,buff,lbuff,ibuff,konvd,
     +            outpos(1,2),npmap,ierror)
      if(ierror.ne.0) goto 920
c
c..pointer to data for each crossection and time
      do it=1,ntime
        call bputi4d(iunito,ireco,buff,lbuff,ibuff,konvd,
     +               iwhere(1,1,it),2*ncross,ierror)
        if(ierror.ne.0) goto 920
      end do
c
      call bputnd(iunito,ireco,buff,lbuff,ibuff,konvd,ierror)
      if(ierror.ne.0) goto 920
c
      close(iunito)
c
      goto 990
c
  910 write(6,*) 'open error. output file.  iostat:',ios
      write(6,*) fileot(1:lenstr(fileot,1))
ccc   stop 117
      call exit(3)
c
  920 write(6,*) 'write error. output file.'
      write(6,*) fileot(1:lenstr(fileot,1))
ccc   stop 117
      call exit(3)
c
  930 write(6,*) 'open/read error. input felt file.  iostat:',ios
      write(6,*) filein(ifilin)(1:lenstr(filein(ifilin),1))
ccc   stop 117
      call exit(3)
c
  990 continue
c
      end
c
c***********************************************************************
c
      subroutine poscomp(ncross,numpos,posinp,cname)
c
c..recompute reference, name and route positions
c..cname(n) = 'Name !! refpos=2.5 name=2.5,Position_Name name=1,2.5,Route_Name ...' )
c..positions recomputed from input position no. to crossection position no.
c
      implicit none
c
c..input/output
      integer ncross
      integer numpos(9,ncross)
      real    posinp(3,*)
      character*(*) cname(ncross)
c
c..local
      integer nc,k,klast,kk,k1,k2,k3,kf,kl,ios,i,ip,kn,n
      integer lname,lbuild,kbuild,kb
      real    pos(2),rp,rp1,rp2
      character*12  cpos(2)
      character*256 cpart,cbuild,cread
      character*1   tchar
c
      integer lenstr
c
c..termination character for free format read (machine dependant)
      call termchar(tchar)
c
      lname= len(cname(1))
      lbuild=len(cbuild)
      if (lbuild.gt.lname) lbuild=lname
c
      do nc=1,ncross
c
	k=index(cname(nc),'!!')
c
	if (k.gt.0) then
c
	  cbuild=' '
	  kbuild=k+1
	  cbuild=cname(nc)(1:kbuild)
c
	  cpart=' '
	  cpart=cname(nc)
	  call chcase(1,1,cpart)
	  kk=lenstr(cpart,0)
	  klast=k+1
c
	  do while (klast.lt.kk)
c
	    ios=-999
	    k1=klast+1
	    do while (k1.lt.kk-3 .and. cpart(k1:k1).eq.' ')
	      k1=k1+1
	    end do
	    n=1
	    kn=0
	    k2=0
	    k3=k1+1
	    do while (k3.lt.kk .and. cpart(k3:k3).ne.' ')
	      if(cpart(k3:k3).eq.'=') then
	        k2=k3
	      elseif (cpart(k3:k3).eq.',') then
	        kn=k3
	        n=n+1
	      end if
	      k3=k3+1
	    end do
	    if (cpart(k3:k3).eq.' ') k3=k3-1
	    if (k2.gt.0) then
	      if (cpart(k1:k2-1).eq.'mark') then
	        n=n-1
	      elseif (cpart(k1:k2-1).ne.'refpos') then
	        n=0
	      end if
	      if (k1.lt.k2 .and. k2.lt.k3 .and. n.gt.0) then
	        if (n.gt.2) n=2
	        if (kn.eq.0) kn=k3+1
	        cread=cpart(k2+1:kn-1)//tchar
	        read(cread,*,iostat=ios) (pos(i),i=1,n)
	        i=0
	        do while (ios.eq.0 .and. i.lt.n)
	          i=i+1
		  ip= int(pos(i))
		  if (ip.lt.1) ip=1
		  if (ip.gt.numpos(2,nc)-1) ip=numpos(2,nc)-1
		  rp1=posinp(3,numpos(3,nc)-1+ip)
		  rp2=posinp(3,numpos(3,nc)-1+ip+1)
		  rp=rp1+(rp2-rp1)*(pos(i)-real(ip))
		  cpos(i)=' '
		  write(cpos(i),fmt='(f12.2)',iostat=ios) rp
		end do
		if (ios.eq.0) then
		  kb=kbuild+1
		  kbuild=kbuild+(k2-k1+1)
		  if (kbuild.lt.lbuild)
     +			cbuild(kb:kbuild)=cname(nc)(k1:k2)
		  do i=1,n
		    kf=1
		    do while (kf.lt.12 .and. cpos(i)(kf:kf).eq.' ')
		      kf=kf+1
		    end do
		    if (cpos(i)(10:12).eq.'.00') then
		      kl=9
		    elseif (cpos(i)(12:12).eq.'0') then
		      kl=11
		    else
		      kl=12
		    end if
		    kb=kbuild+1
		    kbuild=kbuild+(kl-kf+1)+1
		    if (kbuild.lt.lbuild)
     +			cbuild(kb:kbuild)=cpos(i)(kf:kl)//','
		  end do
		  if (kn.lt.k3) then
		    kb=kbuild+1
		    kbuild=kbuild+(k3-kn)+1
		    if (kbuild.lt.lbuild)
     +			cbuild(kb:kbuild)=cname(nc)(kn+1:k3)//' '
     		  else
     		    cbuild(kbuild:kbuild)=' '
     		  end if
 		end if
	      end if
	    end if
	    if (ios.ne.0) then
	      kb=kbuild+1
	      kbuild=kbuild+(k3-k1+1)+1
	      if (kbuild.lt.lbuild)
     +			cbuild(kb:kbuild)=cname(nc)(k1:k3)//' '
	    end if
	    klast=k3
	  end do
c
	  cname(nc)(1:kbuild)=cbuild(1:kbuild)
	  do k=kbuild+1,lname
	    cname(nc)(k:k)=' '
	  end do
c
	end if
c
      end do
c
      return
      end
