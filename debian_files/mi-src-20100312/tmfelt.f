      program tmfelt
c
c        Input from DNMI standard FELT file(s).
c        Output to sequential model i/o file,
c        standard unix fortran 77 format, use 'assign -F f77' on Cray.
c
c        Field identification and field data written in to separate
c        records.
c
c
c   ==================
c     'Input' files:
c   ==================
c
c   Example 1:  Lam parameter fields
c=======================================================================
c *** tmfelt.param  ('tmfelt.input' file)
c ***
c *=> LAM parameter fields
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    tmfelt  tmfelt.param  par1814.dat  1814  param.dat
c *=>                          <input>     <grid>  <output>
c ***
c ***------------------------------------------------------------------
c **
c *>  output file  ($... = env.var. ; #n = command line argument)
c '#4'      <<<<< 'param.dat'
c **
c **  options:
c **    'stop.missing'      : stop if field is missing
c **    'stop.fields=<min_no_of_fields>' : stop if too few fields
c **    'test.field'        : print identification/min/max/mean
c **    'format.ascii'      : formatted output (integer format)
c **    'swap_bytes'        : from dec/vax to dnmi cray model
c **    'replace=<old,new>' : replace any of the integer values below,
c **                          'new' may be an environment var. or a
c **                          command line arg., (use a strange value,
c **                          like -1001, to identify the 'old' value)
c *>    'end'               : end of options
c 'replace=-1001,#3'
c 'stop.missing'
c 'end'
c **
c ** 'grid=<producer_no,grid_no> ..... (necessary before first loop)
c ** loop type: 'loop.time+level+param'
c **            'loop.time+param' ........ no level loop
c *>            'loop.end' ............... end
c 'grid=88,-1001'            <<<< see 'replace=-1001,...' above
c 'loop.time+param'
c **
c ** 'file': 'file.dat' -  file name
c **         '$.....'   -  environment var.
c **         '#n'       -  command line argument no. n
c **         '='        -  same file as previous timestep
c **
c ** no. of timesteps
c *> each timestep: 'file',data_type,forecast_length  ('=' : same file)
c 1
c '#2',4,0
c ** no. of parameters
c ** for each parameter when 'loop.time+param':
c *> vertical_coordinate,parameter_no,level_1,level_2
c 4
c 2,101,1000,0, 2,102,1000,0, 2,103,1000,0, 2,104,1000,0
c **
c ** 'grid=<producer_no,grid_no> ..... (necessary before first loop)
c ** loop type: 'loop.time+level+param'
c **            'loop.time+param' ........ no level loop
c *>            'loop.end' ............... end
c 'Loop.End'
c **----------------------------------------------------------------
c=======================================================================
c
c   Example 2:  LAM analysis fields
c=======================================================================
c *** tmfelt.analys ('tmfelt.input' file)
c ***
c *=> LAM analysis fields (11 levels; z,u,v,rh)
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    tmfelt  tmfelt.analys  felt.dat  1814  analys.dat
c *=>                           <input>  <grid>  <output>
c ***
c ***------------------------------------------------------------------
c **
c *>  output file  ($... = env.var. ; #n = command line argument)
c '#4'    >>>>> 'analys.dat'
c **
c **  options:
c **    'stop.missing'      : stop if field is missing
c **    'stop.fields=<min_no_of_fields>' : stop if too few fields
c **    'test.field'        : print identification/min/max/mean
c **    'format.ascii'      : formatted output (integer format)
c **    'swap_bytes'        : from dec/vax to dnmi cray model
c **    'replace=<old,new>' : replace any of the integer values below,
c **                          'new' may be an environment var. or a
c **                          command line arg., (use a strange value,
c **                          like -1001, to identify the 'old' value)
c *>    'end'               : end of options
c 'replace=-1001,#3'
c 'end'
c **
c ** 'grid=<producer_no,grid_no> ..... (necessary before first loop)
c ** loop type: 'loop.time+level+param'
c **            'loop.time+param' ........ no level loop
c *>            'loop.end' ............... end
c 'grid=88,-1001'            <<<< see 'replace=-1001,...' above
c 'loop.time+level+param'
c **
c ** 'file': 'file.dat' -  file name
c **         '$.....'   -  environment var.
c **         '#n'       -  command line argument no. n
c **         '='        -  same file as previous timestep
c **
c ** no. of timesteps
c *> each timestep: 'file',data_type,forecast_length  ('=' : same file)
c 1
c '#2',1,0
c ** no. of levels
c *> for each level: level_1,level_2  (usually: level_2 = 0)
c 11
c 100,0, 150,0, 200,0, 250,0, 300,0, 400,0, 500,0,
c        700,0, 850,0, 925,0, 1000,0
c ** no. of parameters
c ** for each parameter when 'loop.time+level+param':
c *> vertical_coordinate,parameter_no    (here: z,u,v,rh)
c 4
c 1,1, 1,2, 1,3, 1,10
c **
c ** 'grid=<producer_no,grid_no> ..... (necessary before first loop)
c ** loop type: 'loop.time+level+param'
c **            'loop.time+param' ........ no level loop
c *>            'loop.end' ............... end
c 'Loop.End'
c **----------------------------------------------------------------
c=======================================================================
c
c   Example 3:  LAM boundary fields
c=======================================================================
c *** tmfelt.bound  ('tmfelt.input' file)
c ***
c *=> LAM boundary fields : ECMWF forecast (11 levels, +6 - +48)
c *=>
c *=> Environment var:
c *=>    none
c *=> Command format:
c *=>    tmfelt  tmfelt.bound  flte12.dat  bound.dat
c *=>                          <input>     <output>
c ***
c ***------------------------------------------------------------------
c **
c *>  output file  ($... = env.var. ; #n = command line argument)
c '#3'   <<<<<< 'bound.dat'
c **
c **  options:
c **    'stop.missing'      : stop if field is missing
c **    'stop.fields=<min_no_of_fields>' : stop if too few fields
c **    'test.field'        : print identification/min/max/mean
c **    'format.ascii'      : formatted output (integer format)
c **    'swap_bytes'        : from dec/vax to dnmi cray model
c **    'replace=<old,new>' : replace any of the integer values below,
c **                          'new' may be an environment var. or a
c **                          command line arg., (use a strange value,
c **                          like -10001, to identify the 'old' value)
c *>    'end'               : end of options
c 'stop.fields=80'   .....  stop if less than 80 fields found
c 'end'
c **
c ** 'grid=<producer_no,grid_no> ..... (necessary before first loop)
c ** loop type: 'loop.time+level+param'
c **            'loop.time+param' ........ no level loop
c *>            'loop.end' ............... end
c 'grid=98,503'
c 'loop.time+level+param'
c **
c ** 'file': 'file.dat' -  file name
c **         '$.....'   -  environment var.
c **         '#n'       -  command line argument no. n
c **         '='        -  same file as previous timestep
c **
c ** no. of timesteps
c *> each timestep: 'file',data_type,forecast_length  ('=' : same file)
c 8
c '#2',3,6,  '=',3,12, '=',3,18, '=',3,24,
c            '=',3,30, '=',3,36, '=',3,42, '=',3,48
c ** no. of levels
c *> for each level: level_1,level_2  (usually: level_2 = 0)
c 11
c 100,0, 150,0, 200,0, 250,0, 300,0, 400,0, 500,0,
c        700,0, 850,0, 925,0, 1000,0
c ** no. of parameters
c ** for each parameter when 'loop.time+level+param':
c *> vertical_coordinate,parameter_no    (here: z,u,v,rh)
c 4
c 1,1, 1,2, 1,3, 1,10
c **
c ** 'grid=<producer_no,grid_no> ..... (necessary before first loop)
c ** loop type: 'loop.time+level+param'
c **            'loop.time+param' ........ no level loop
c *>            'loop.end' ............... end
c 'Loop.End'
c **----------------------------------------------------------------
c=======================================================================
c
c  note:  *    - comment line
c         *>   - last comment line before 'real' program input
c         *=>  - help
c-----------------------------------------------------------------------
c
c      DNMI library subroutines:  rfelt
c                                 rlunit
c                                 rcomnt
c                                 chcase
c                                 getvar
c                                 prhelp
c                                 rmfile
c
c
c     program return codes (stop n):
c        "stop 1" : error in input specifications
c        "stop 2" : open error output file.
c        "stop 3" : field missing or too few fields
c
c
c-----------------------------------------------------------------------
c  DNMI/FoU  1989 - 1992  Anstein Foss ... ibm
c  DNMI/FoU   28.10.1992  Anstein Foss ... unix
c  DNMI/FoU   03.12.1993  Anstein Foss
c  DNMI/FoU   10.06.1995  Anstein Foss ... extra geometry identification
c  DNMI/FoU   21.04.1999  Anstein Foss ... include file
c  DNMI/FoU   30.01.2001  Anstein Foss ... 
c  DNMI/FoU   19.06.2003  Anstein Foss ... automatic byteswap (input)
c  DNMI/FoU   12.08.2004  Anstein Foss ... longer filenames etc.
c-----------------------------------------------------------------------
c
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c..include file for tmfelt.f
c
c  maxsiz : max field size
c
ccc   parameter (maxsiz=1000000)
c
ccc   parameter (maxtim=400,maxlev=400,maxpar=400)
ccc   parameter (mloops=50)
ccc   parameter (mrepla=100)
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
      include 'tmfelt.inc'
c
      parameter (limit=20+maxsiz)
c
      common/a/itim(2,maxtim),ilev(2,maxlev),ipar(4,maxpar),
     +         ident(20),ifelt(maxsiz+3)
      integer*2 itim,ilev,ipar
      integer*2 ident,ifelt
c
      integer*2  igrid(2,mloops)
      integer    ltype(mloops)
      integer    numtim(2,mloops),numlev(2,mloops),numpar(2,mloops)
      integer    irepla(2,mrepla)
c
      integer*2 in(16),lident(20)
c
      dimension ierr(3),ih(6)
      double precision sum
c
      character*256 finput,cinput,cipart,fileot,filein(maxtim),filnam
c
      logical swapfile,swap
c
      data ih/6*0/
      data in/16*0/
c
c..get record length unit in bytes for recl= in open statements
c..(machine dependant)
      call rlunit(lrunit)
c
c..file unit for the 'tmfelt.input' file
      iuinp = 9
c..input felt file unit (all files)
      ifilin=10
c..output file unit
      ifilot=20
c
      filnam='*'
c
      missin=0
      ntim=0
      nlev=0
      npar=0
      nrepla=0
c
      mstop=0
      nstop=0
      itestf=0
      iswapb=0
      iformt=0
c
c-----------------------------------------------------------------------
      narg=iargc()
      if(narg.lt.1) then
        write(6,*)
        write(6,*) '  usage: tmfelt <tmfelt.input>'
        write(6,*) '     or: tmfelt <tmfelt.input> <arguments>'
        write(6,*) '     or: tmfelt <tmfelt.input> ?     (to get help)'
        write(6,*)
        goto 981
      end if
      call getarg(1,finput)
c
      open(iuinp,file=finput,
     +           access='sequential',form='formatted',
     +           status='old',iostat=ios)
      if(ios.ne.0) then
        write(6,*) 'open error:'
        write(6,*) finput(1:lenstr(finput,1))
        goto 981
      end if
c
      if(narg.eq.2) then
        call getarg(2,cinput)
        if(cinput.eq.'?') goto 92
      end if
c
c
      write(6,*) 'reading input file:'
      write(6,*) finput(1:lenstr(finput,1))
c
      nlines=0
c..comments
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 88
c..output file
      nlines=nlines+1
      read(iuinp,*,err=90) fileot
c..comments
      call rcomnt(iuinp,'*>','*',nlines,ierror)
      if(ierror.ne.0) goto 88
c
c..options:
c      'stop.missing'      : stop if field is missing
c      'stop.fields=<min_no_of_fields>' : stop if too few fields
c      'test.field'        : print identification/min/max/mean
c      'format.ascii'      : formatted output (integer format)
c      'swap_bytes'        : from dec/vax to dnmi cray model
c      'replace=<old,new>' : replace any of the integer values below,
c                            'new' may be an environment var. or a
c                            command line arg., (use a strange value,
c                            like -10001, to identify the 'old' value)
c      'end'               : end of options
c
c..always convert to lowercase letters
c
      iend=0
c
      do while (iend.eq.0)
c
        nlines=nlines+1
        read(iuinp,*,err=90) cinput
c..for 'replace=<old,new>' : may use environment var., command lie arg.
c                            or user question.
        call getvar(1,cinput,1,1,1,ierror)
        if(ierror.ne.0) goto 92
        call chcase(1,1,cinput)
c
        if(cinput.eq.'stop.missing') then
          mstop=1
        elseif(cinput(1:12).eq.'stop.fields=') then
          cipart=cinput(13:)
          read(cipart,*,iostat=ios,err=85) nstop
        elseif(cinput.eq.'test.field') then
          itestf=1
        elseif(cinput.eq.'format.ascii') then
          iformt=1
        elseif(cinput.eq.'swap_bytes') then
          iswapb=1
        elseif(cinput(1:8).eq.'replace=') then
          nrepla=nrepla+1
          if(nrepla.gt.mrepla) goto 89
          cipart=cinput(9:)
          read(cipart,*,iostat=ios,err=85) (irepla(i,nrepla),i=1,2)
        elseif(cinput.eq.'end') then
          iend=1
        else
          goto 85
        end if
c
      end do
c
c
      do 50 iloop=1,mloops
c
        if(iloop.eq.1) then
          igrid(1,iloop)=0
          igrid(2,iloop)=0
        else
          igrid(1,iloop)=igrid(1,iloop-1)
          igrid(2,iloop)=igrid(2,iloop-1)
        end if
c
        call rcomnt(iuinp,'*>','*',nlines,ierror)
        if(ierror.ne.0) goto 88
c
c..producer/grid and loop type:
c
c     'grid=<producer_no,grid_no> ..... (necessary before first loop)
c     loop type: 'loop.time+level+param'
c                'loop.time+param' ........ no level loop
c                'loop.end' ............... end
c
c   always convert to uppercase letters
c
        ltype(iloop)=0
c
        do while (ltype(iloop).eq.0)
c
          nlines=nlines+1
          read(iuinp,*,err=90) cinput
          call chcase(1,1,cinput)
c
          if(cinput(1:5).eq.'grid=') then
c..grid=<producer_no,grid_no>
            cipart=cinput(6:)
            read(cipart,*,iostat=ios,err=85) iprodn,igridn
            igrid(1,iloop)=iprodn
            igrid(2,iloop)=igridn
          elseif(cinput.eq.'loop.time+level+param') then
c..loop.time+level+param
            ltype(iloop)=1
          elseif(cinput.eq.'loop.time+param') then
c..loop.time+param
            ltype(iloop)=2
          elseif(cinput.eq.'loop.end') then
c..loop.end
            ltype(iloop)=-1
          else
            goto 85
          end if
c
        end do
c
        if(ltype(iloop).eq.-1) goto 80
c
        if(igrid(1,iloop).eq.0) then
          write(6,*) 'missing option: grid=<producer_no,grid_no>'
          goto 90
        end if
c
c..'file',data_type,forecast_length
        call rcomnt(iuinp,'*>','*',nlines,ierror)
        if(ierror.ne.0) goto 88
        read(iuinp,*,err=90) nt
        if(nt.lt.1) goto 86
        n1=ntim+1
        ntim=ntim+nt
        if(ntim.gt.maxtim) goto 87
        numtim(1,iloop)=n1
        numtim(2,iloop)=ntim
        read(iuinp,*,err=90) (filein(n),(itim(i,n),i=1,2),n=n1,ntim)
c
c..level
        if(ltype(iloop).eq.1) then
          call rcomnt(iuinp,'*>','*',nlines,ierror)
          if(ierror.ne.0) goto 88
          read(iuinp,*,err=90) nl
          if(nl.lt.1) goto 86
          n1=nlev+1
          nlev=nlev+nl
          if(nlev.gt.maxlev) goto 87
          numlev(1,iloop)=n1
          numlev(2,iloop)=nlev
          read(iuinp,*,err=90) ((ilev(i,n),i=1,2),n=n1,nlev)
        else
          numlev(1,iloop)=0
          numlev(2,iloop)=0
        end if
c
c..parameter
        call rcomnt(iuinp,'*>','*',nlines,ierror)
        if(ierror.ne.0) goto 88
        read(iuinp,*,err=90) np
        if(np.lt.1) goto 86
        n1=npar+1
        npar=npar+np
        if(npar.gt.maxpar) goto 87
        numpar(1,iloop)=n1
        numpar(2,iloop)=npar
        if(ltype(iloop).eq.1) then
          npid=2
          do n=n1,npar
            ipar(3,n)=-32767
            ipar(4,n)=-32767
          end do
        else
          npid=4
        end if
        read(iuinp,*,err=90) ((ipar(i,n),i=1,npid),n=n1,npar)
c
   50 continue
c
      write(6,*) 'too many ''loops''.'
      write(6,*) 'max. no. (mloops): ',mloops
      goto 90
c
   80 nloops=iloop-1
      if(nloops.lt.1) then
        write(6,*) 'no ''loops''.  nothing to do.'
        goto 90
      end if
c
c..check if input as environment variables, command line arguments
c                    or possibly as 'user questions'.
c
      call getvar(1,fileot,1,1,1,ierror)
      if(ierror.ne.0) goto 92
c
      if(filein(1)(1:1).eq.'=') then
        write(6,*) 'first input filname can''t be ''=''.'
        goto 92
      end if
      call getvar(ntim,filein,1,1,1,ierror)
      if(ierror.ne.0) goto 92
c
      goto 95
c
   85 write(6,*) 'error in input:'
      write(6,*) cinput(1:lenstr(cinput,1))
      goto 90
c
   86 write(6,*) 'no. of timesteps, levels or parameters less than 1.'
      goto 90
c
   87 write(6,*) 'max limit exceeded (total for all ''loops''):'
      write(6,*) 'no. of timesteps,  max (maxtim): ',ntim,maxtim
      write(6,*) 'no. of levels,     max (maxlev): ',nlev,maxlev
      write(6,*) 'no. of parameters, max (maxpar): ',npar,maxpar
      goto 90
c
   88 write(6,*) 'error reading comment lines.'
      goto 90
c
   89 write(6,*) 'too many replace= options. max. no. is ',mrepla
      goto 90
c
   90 write(6,*) 'error at line no. ',nlines,'   (or below)'
      write(6,*) 'file: ',finput(1:lenstr(finput,1))
      close(iuinp)
      goto 981
c
   92 continue
      write(6,*) 'help from ',finput(1:lenstr(finput,1)),' :'
      call prhelp(iuinp,'*=>')
      close(iuinp)
      goto 981
c
   95 close(iuinp)
      write(6,*) 'input o.k.'
c--------------------------------------------------------------------
c
c..check use of replace= options
c
      if(nrepla.gt.0) then
        do n=1,nloops
          do i=1,nrepla
            if(igrid(1,n).eq.irepla(1,i)) igrid(1,n)=irepla(2,i)
            if(igrid(2,n).eq.irepla(1,i)) igrid(2,n)=irepla(2,i)
          end do
        end do
        do n=1,ntim
          do i=1,nrepla
            if(itim(1,n).eq.irepla(1,i)) itim(1,n)=irepla(2,i)
            if(itim(2,n).eq.irepla(1,i)) itim(2,n)=irepla(2,i)
          end do
        end do
        do n=1,nlev
          do i=1,nrepla
            if(ilev(1,n).eq.irepla(1,i)) ilev(1,n)=irepla(2,i)
            if(ilev(2,n).eq.irepla(1,i)) ilev(2,n)=irepla(2,i)
          end do
        end do
        do n=1,npar
          do i=1,nrepla
            if(ipar(1,n).eq.irepla(1,i)) ipar(1,n)=irepla(2,i)
            if(ipar(2,n).eq.irepla(1,i)) ipar(2,n)=irepla(2,i)
            if(ipar(3,n).eq.irepla(1,i)) ipar(3,n)=irepla(2,i)
            if(ipar(4,n).eq.irepla(1,i)) ipar(4,n)=irepla(2,i)
          end do
        end do
      end if
c
c--------------------------------------------------------------------
c
c..open output file
c
      call rmfile(fileot,0,ierror)
c
      if(iformt.eq.0) then
c..binary output format
        open(ifilot,file=fileot,
     +              access='sequential',form='unformatted',
     +                         status='unknown',iostat=ios)
      else
c..ascii output format
        open(ifilot,file=fileot,
     +              access='sequential',form='formatted',
     +                       status='unknown',iostat=ios)
      end if
      if(ios.ne.0) then
        write(6,*) ' *** open error. output file.   iostat=',ios
        write(6,*) ' *** ',fileot(1:lenstr(fileot,1))
        goto 982
      end if
c
      nfelt=0
c
      filnam='*'
c
      lident( 4)=-9999
      lident(12)=-9999
      lident(13)=-9999
      lident(14)=-9999
c
      if(iformt.ne.0) iswapb=0
      ierrop=0
c
c
c..read and write fields................................
c
      do 200 iloop=1,nloops
c
      in( 1)=igrid(1,iloop)
      in( 2)=igrid(2,iloop)
c
      do 210 nt=numtim(1,iloop),numtim(2,iloop)
c
      in( 9)=itim(1,nt)
      in(10)=itim(2,nt)
c
      if(filein(nt)(1:1).ne.'=' .and. filein(nt).ne.filnam) then
        if(filnam.ne.'*') close(ifilin)
c..open input file
        filnam=filein(nt)
        ih(1)=0
        open(ifilin,file=filnam,
     +              access='direct',form='unformatted',
     +              recl=2048/lrunit,status='old',iostat=ios)
	swap=swapfile(-ifilin)
c..read part of record 1 and get date/time
        if(ios.eq.0) read(ifilin,rec=1,iostat=ios) ident
        if(ios.eq.0 .and. swap) call bswap2(20,ident)
        if(ios.ne.0) then
          write(6,*) 'open/read error.  input file.  iostat=',ios
          write(6,*) 'file: ',filnam(1:lenstr(filnam,1))
          if(mstop.eq.1) then
            missin=1
            goto 990
          end if
          ierrop=1
        else
          month=ident(6)/100
          iday =ident(6)-month*100
          write(6,*) 'file: ',filnam(1:lenstr(filnam,1))
          write(6,1010) iday,month,ident(5),ident(7)
 1010     format(' time: ',2i4,2i6,' utc')
          ierrop=0
        end if
      end if
c
      if(ierrop.ne.0) then
        write(6,*) '....skipping one timestep'
        goto 210
      end if
c
      do 220 nl=numlev(1,iloop),numlev(2,iloop)
c
      if(nl.gt.0) then
        in(13)=ilev(1,nl)
        in(14)=ilev(2,nl)
      end if
c
      do 230 np=numpar(1,iloop),numpar(2,iloop)
c
      in(11)=ipar(1,np)
      in(12)=ipar(2,np)
      if(nl.eq.0) then
        in(13)=ipar(3,np)
        in(14)=ipar(4,np)
      end if
c
      ierr(2)=0
      call rfelt(ifilin,ip,in,ident,limit,ierr,ih)
      if(ip.ne.1) then
        write(6,1012) ip,ierr,in(1),in(2),(in(i),i=9,14)
 1012   format(' field not found.  ip=',i4,'  ierr=',3i9,
     *       /,'       in(1,2,9-14): ',8i6)
        missin=1
        if(mstop.eq.1) goto 990
        missin=0
        goto 230
      end if
c
      nfelt=nfelt+1
c
      if(ident( 4).ne.lident( 4) .or.
     +   ident(12).ne.lident(12) .or.
     +   ident(13).ne.lident(13) .or.
     +   ident(14).ne.lident(14)) then
        month=ident(13)/100
        iday =ident(13)-month*100
        ihour=ident(14)/100
        write(6,1015) nfelt,iday,month,ident(12),ihour,ident(4)
 1015   format(' field no.',i5,'   time:',2i3,i5,i3,' utc ',sp,i5,ss)
        lident( 4)=ident( 4)
        lident(12)=ident(12)
        lident(13)=ident(13)
        lident(14)=ident(14)
      end if
c
      ii=ident(10)
      jj=ident(11)
      isize=ii*jj
c
      if(itestf.eq.1) then
        write(6,*) 'field no. ',nfelt
        write(6,1022) ident
 1022   format('  ident: ',11i6,/,'         ',9i6)
        nudef=0
        imin=+32767
        imax=-32767
        sum=0.
        do i=1,isize
          if(ifelt(i).eq.-32767) then
            nudef=nudef+1
          else
            sum=sum+ifelt(i)
            if(imin.gt.ifelt(i)) imin=ifelt(i)
            if(imax.lt.ifelt(i)) imax=ifelt(i)
          end if
	end do
        if(isize.gt.nudef) then
          sum=sum/(isize-nudef)
          write(6,1024) imin,imax,sum,nudef
 1024     format('  min,max,mean,nundef: ',2i7,f12.4,i7)
        else
          write(6,1025) nudef
 1025     format('               nundef: ',26x,i7)
        end if
      end if
c
c..possibly extra geometry identification after field data
      lgeom=0
      if(ident(9).gt.999) lgeom=ident(9)-(ident(9)/1000)*1000
c
      lfelt=isize+lgeom
c
      if(iswapb.eq.1) then
c..swap bytes..................ident and ifelt treated as one array
	call bswap2(20+lfelt,ident)
      end if
c
      if(iformt.eq.0) then
c..binary format (unformatted)
        ifelt(lfelt+1)=0
        ifelt(lfelt+2)=0
        ifelt(lfelt+3)=0
c..for cray (reading 64 bit words), no problem for other machines.
        lfelt=((lfelt+3)/4)*4
        write(ifilot,iostat=ios,err=920) (ident(i),i=1,20)
        write(ifilot,iostat=ios,err=920) (ifelt(i),i=1,lfelt)
      else
c..ascii format
        write(ifilot,1050,iostat=ios,err=920) (ident(i),i=1,20)
        write(ifilot,1050,iostat=ios,err=920) (ifelt(i),i=1,lfelt)
 1050   format(1x,10i7)
      end if
c
  230 continue
c
  220 continue
c
  210 continue
c
  200 continue
c
      goto 990
c
c
  920 write(6,*) ' ** write error ** output file.  iostat= ',ios
      write(6,*) ' ** ',fileot(1:lenstr(fileot,1))
      nfelt=nfelt-1
      goto 990
c
  981 continue
ccc   stop 1
      write(6,*) 'tmfelt ***** stop 1 *****'
      call exit(1)
c
  982 continue
ccc   stop 2
      write(6,*) 'tmfelt ***** stop 2 *****'
      call exit(2)
c
  983 continue
ccc   stop 3
      write(6,*) 'tmfelt ***** stop 3 *****'
      call exit(3)
c
  990 continue
      if(filnam.ne.'*') close(ifilin)
      close(ifilot)
c
      write(6,*) 'no. of fields: ',nfelt
      if(mstop.eq.1 .and. missin.ne.0)    goto 983
      if(nstop.gt.0 .and. nfelt.lt.nstop) goto 983
c
      end
