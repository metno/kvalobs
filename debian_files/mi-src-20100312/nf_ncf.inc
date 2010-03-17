c  Common block /ncf/
c  ------------------
c
c  Netcdf:
c
c  ncnm          - Name of netcdf file
c  ncid          - File id for the netcdf file
c
c  Variables describing the current netcdf variable:
c
c  ncvarname     - (char) Name of the current netcdf variable
c  ncvarid       - Id number of the variable in the netcdf system
c  nctype        - Type of variable (one of: NF_BYTE, NF_CHAR, NF_SHORT,
c                  NF_INT, NF_FLOAT, NF_DOUBLE)
c  ncdimcount    - Number of dimensions
c  ncdims        - Array containing dimension IDs
c  ncdimsize     - Array containing dimension sizes
c  ncchid        - Array containing id numbers for the charr arrays
c                  containing the content of the netcdf arrays
c                  corresponding to the dimensions. = -1 if no
c                  such content array could be found.
c
c  Arrays containing different variants of missing values in the netcdf file:
c  Each of these arrays may contain up to maxmisvals numbers.
c
c  misv_byte, misv_short, misv_int and misv_real
c
c  misvalcount   - The actual number of missing value codes in each array.
c
c  FELT file:
c
c  nmfelt        - Name of FELT file
c  field         - FELT field
c  idata         - Packed FELT field (integers)
c  idataid       - Grid identification from idata (used to initialize idata
c                  when the grid definition is repeated from field to field)
c  fmisv         - Missing value (real) in FELT fields (+1.e+35)
c  imisv         - Missing value (integer) in FELT fields (-32767)
c  producer      - Producer number (1-99)
c  numfields     - Number of fields to be transferred
c  felttype      - Type of FELT file (999,998 or 997)
c  stepcount     - Number of time steps (termins) comprising all date/times in
c                  the FELT file
c  slotcount     - Number of index slots in FELT innholdsfortegnelse for each
c                  time step
c  timeunit      - Time unit used for steplength (1=year, 2=month, 3=day, 4=hour,
c                  5=minute)
c  steplength    - Length of a time step (in units given in timeunit)
c  datetime(8)   - Array containing date/time for the first field (year,month,
c                  day,hour,minute). datetime(6-8) is used locally in ncf_init
c  prevdate(3)   - Array containing the previous date (YYYY, MMDD, and HHMM)
c  ixdate        - The current time step.
c  ixslot        - The current slot (between 1 and slotcount). Initialized to 0
c
c  Debugging:
c  debug         - = .true. if debugging output is wanted.
c
      integer maxncdims,maxidata,maxfield,szfeltrec1,maxmisvals
      parameter (maxmisvals = 100)
      parameter (maxncdims = 100)
      parameter (maxidata = 15000000)
      parameter (maxfield = 15000000)
      parameter (szfeltrec1 = 32)
      integer imisv
      parameter (imisv = -32767)
      real fmisv
      parameter (fmisv = +1.e+35)
      logical debug
      parameter (debug = .false.)
      integer ncid,ncvarid,nctype,ncdimcount,ncdims,
     +       ncdimsize,ncchid
      integer*2 idata,idataid,feltrec1
      integer misv_byte,misv_short,misv_int,misvalcount
      integer producer,numfields,felttype,stepcount,slotcount,timeunit
      integer steplength,datetime,prevdate,ixdate,ixslot
      real misv_real
      real field
      character*128 ncvarname
      character*80 nmnc,nmfelt
      common /ncf/ nmnc,ncid,ncvarname,ncvarid,nctype,ncdimcount,
     +       ncdims(maxncdims),ncdimsize(maxncdims),ncchid(maxncdims),
     +       nmfelt,field(maxfield),idata(maxidata),idataid(40),
     +       feltrec1(szfeltrec1),misvalcount,
     +       misv_byte(maxmisvals),misv_short(maxmisvals),
     +       misv_int(maxmisvals),misv_real(maxmisvals),producer,
     +       numfields,felttype,stepcount,slotcount,timeunit,steplength,
     +       datetime(8),prevdate(3),ixdate,ixslot
c23456789012345678901234567890123456789012345678901234567890123456789012
