      program nctofelt
c
c************ MAN_BEGIN ************************************************
c
c  PURPOSE:
c     Convert from netcdf format to FELT file format
c
c  USAGE:
c
c     nctofelt -d fdef name_of_netcdffile name_of_feltfile <conversion_file
c
c        where fdef is the name of a flt2flt grid definition file.
c
c  DESCRIPTION:
c
c  Selected fields from the netcdf file are transferred to a FELT
c  file, which will be created.
c
c  The conversion_file contains a description of which variables in
c  the netcdf file are to be transferred to the FELT file, and how
c  corresponding fields in the two files are identified.
c
c  Line one:
c
c  The first line in the file contains the following 10 whole numbers:
c
c  - The number of two-dimensional FELT fields to be produced.
c    This number is also equal to the number of lines comprising
c    the rest of the file.
c  - The type of the FELT file (999,998 or 997)
c  - Producer number. This utility only allows one producer in the 
c    FELT file
c  - Three numbers giving the base date/time for the data 
c    (corresponding to word 5-7, record 1 in a standard FELT file;
c    corresponding to the first date/time for an archive FELT file).
c    (Year, month*100+day, hour*100+minute).
c  - Number of time steps in the FELT file (FELT record 1 word 26)
c    (for standard, type=999 FELT files, this can only be 1).
c  - Number of index slots in "innholdsfortegnelse" per time step
c    (FELT record 1 word 27)
c  - Unit for time step resolution (1=year, 2=month, 3=day, 4=hour,
c    5=minute) (FELT record 1 word 29)
c  - Length of time step (using unit as above) (FELT record 1 word 30)
c
c  Following lines:
c
c  Then follows lines of the following type:
c
c     FELT(date,grid,dtyp,fcl,lev1,lev2,vco,par) = varname(arg1,arg2,...)<LC>
c
c  <LC> is an optional linear conversion clause (see below).
c
c  The 'date' field is optional. So, an alternative form is:
c
c     FELT(grid,dtyp,fcl,lev1,lev2,vco,par) = varname(arg1,arg2,...)<LC>
c
c  Each line describes one two-dimensional field in the FELT file,
c  and the variable in the netcdf file from where the data are to be
c  transferred.
c
c  The words date,grid,dtyp,fcl,lev1,lev2,vco,par,varname,arg1,arg2,...
c  should be substituted by the following values:
c
c     date       - (Optional). Date/time on the form: YYYY-MM-DD:HHMM
c                  If not given, then the date/time given in line one
c                  (for a standard, type=999 FELT file) is used. Othervise,
c                  the date/time for the previous field is used.
c                  See also: 'NOTE about date/time' below.
c
c     grid       - Grid area
c                  This is the grid number described in felt.txt.
c                  This value may also take the form "gn:x1-x2:y1-y2"
c                  (apostrophes not included), where gn = grid number,
c                  x1 = lower x index, x2 = upper x index and y1, y2 =
c                  the lower and upper y indices. The lower x,y indices
c                  represent the indices in the resulting FELT array
c                  corresponding to the first value copied from the
c                  netcdf variable. x1,y1 defaults to 1,1. The upper
c                  x,y indices correspond to the last value copied from
c                  the netcdf variable. When x1-x2, y1-y2 is
c                  explisitly given as two integer ranges,
c                  only the rectangular part of the FELT
c                  array given by taking (x1,y1) as the lower left
c                  and (x2,y2) as the upper right corner,
c                  is filled with values. The other parts of the FELT
c                  array is filled with the missing value code.
c
c     dtyp       - Data type (1,2,3 or 4 - described in felt.txt)
c
c     fcl        - Forecast length (hours)
c
c     lev1       - Level 1 vertical co-ordinate value
c
c     lev2       - Level 2 vertical co-ordinate value
c
c     vco        - Type of vertical co-ordinate used
c                  (pressure, height, eta levels, sigma levels etc.
c                  Described in felt.txt)
c
c     par        - Parameter code. (Described in felt.txt)
c
c     variable   - Variable name identifying an array in the netcdf file
c
c     arg1,...   - Arguments to define which part of the netcdf array
c                  to use. 
c
c                  These arguments may either have values
c                  found for the corresponding dimension in the
c                  netcdf file, or they have the special values #1 or #2.
c                  Exactly one of the arguments must have the value
c                  #1. Another argument may have the value #2.
c                  If only the #1 argument is found, the two-dimensional
c                  field is defined by varying the array index
c                  corresponding the #1 argument from 1 up to the
c                  size of the dimension. If both the #1 and #2
c                  arguments are given, then values are copied from
c                  the netcdf array by first setting the #2 index
c                  to 1 while traversing all values of the #1 index
c                  from 1 and up to the dimension size, then setting
c                  the #2 index to 2 etc.
c
c                  If the argument is neither #1 or #2, it either
c                  corresponds to an index value in the netcdf array,
c                  or to a value found in another netcdf variable
c                  having the same name as the dimension corresponding
c                  to the argument. If such dimension variable exists,
c                  this alternative is the only possibility. Care
c                  should be taken to make unambiguous arguments
c                  corresponding to values in a dimension variable.
c                  If this variable is of type float or double, the
c                  argument should always include a decimal point.
c
c  The optional linear conversion clause, <LC>, is used if the values
c  in the netcdf file are to be transformed by a linear function before
c  they are stored in the FELT file. This clause must correspond to one
c  of the following forms:
c
c     * r1
c     * (-r1)
c     * r1 + r2
c     * r1 - r2
c     * (-r1) + r2
c     * (-r1) - r2
c     + r2
c     - r2
c
c  where r1 and r2 are non-negative numbers (with or without a decimal
c  point). Scientific notation is not allowed. Spaces as shown in the
c  forms above are not neccessary.
c
c  Example:
c
c                  Netcdf file:
c                     dimensions: lat = 3, lon = 5, pressure = 2;
c                     variables: lat(lat), lon(lon), 
c                                temperature(lat,lon,pressure);
c                     data: lat =  50, 60, 70; lon = 0, 5, 10, 15, 20;
c                           pressure = 1000.0, 850.0
c
c                  Conversion file:
c                     ... (line one)
c                     FELT(111,1,0,850,0,1,4) = temperature(#1,#2,850.0) +273
c
c  NOTE about date/time:
c
c        The number of lines corresponding to one date/time value must
c        not exceed the available number of index slots in the
c        "innholdsfortegnelse" for the given date/time value. This 
c        number is given on line one (described above).
c
c        Also, lines must be sorted in incresing data/time order, and
c        each date/time must correspond to one of the date/times
c        defined in line one ('base date/time' + n * 'time step',
c        n = 0,1,2,...).
c
c  EXTERNAL DEPENDENCIES:
c
c  External functions/subroutines:
c
c     From libmi.a:
c
c        lenstr
c        cmdarg
c        gridpar
c        vtime
c        mwfelt
c
c     From libnetcdf.a:
c
c        nf_get_varm_real
c        nf_get_varm_int
c        nf_get_varm_int2
c        nf_get_varm_int1
c        nf_open
c        nf_close
c        nf_strerror
c
c************ MAN_END **************************************************
c
      implicit none
      include "nf_ncf.inc"
      character*200 inpline
      integer i1,linenum,lparcount,rparcount,varix,feltix
      character*50 feltarg(20),vararg(20),word,wordinit
      character*50 feltid
      character*51 lcr1,lcr2
      real r1lc,r2lc,toreal
      logical linconv
      character*128 varname
      character*1 ch1,tab
      character*100 errtext
      integer inplen,wordlen,numf,lcoper
      integer toint,toint2,ncf_init,lenstr,new_ncvar,transfield
      integer ncf_close,chareport
c
c  Initialize: command line intepretation, open files etc.
c
      if (debug) write (*,*) 'Starting ...'
      if (ncf_init().ne.1) then
         stop
      endif
      if (debug) write (*,*) 'ncf_init OK'
c
      tab = char(11)
      wordinit = '                                                  '
      linenum = 1
      if (debug) write (*,*) '--- Reading from standard input:'
      do numf=1,numfields
         read (5,'(A200)',end=20) inpline
         inplen = lenstr(inpline,1)
         if (debug) write (*,*) '    New line: ',inplen,inpline
         word = wordinit
         wordlen = 0
         errtext = ''
         lparcount = 0
         rparcount = 0
         lcoper = 0
         lcr1 = ' '
         lcr2 = ' '
         feltix = 1
         varix = 1
         i1 = 1
         do while (i1 .le. inplen)
            ch1 = inpline(i1:i1)
            if (lparcount.le.2.and.rparcount.lt.2) then
               if (ch1.eq.'(') then
                  if (lparcount.eq.0) then
                     feltid = word
                  else
                     if (lparcount.ge.2) then
                        errtext = 'Illegal left parantesis'
                     else if (lparcount.ne.rparcount) then
                        errtext = 'Missing right parantesis'
                     else if (lparcount.eq.1) then
                        varname = word
                     end if
                  end if
                  lparcount = lparcount+1
                  word = wordinit
                  wordlen = 0
               else if (ch1.eq.')') then
                  rparcount = rparcount+1
                  if (rparcount.ne.lparcount) then
                     errtext = 'Too many right parantheses'
                  end if
                  if (rparcount.eq.1) then
                     feltarg(feltix) = word
                     feltix = feltix+1
                  else if (rparcount.eq.2) then
                     vararg(varix) = word
                     varix = varix+1
                  end if
                  word = wordinit
                  wordlen = 0
               else if (ch1.eq.',') then
                  if (lparcount.eq.1.and.rparcount.eq.0) then
                     feltarg(feltix) = word
                     feltix = feltix+1
                  else if (lparcount.eq.1.and.rparcount.eq.1) then
                     errtext = 'Illegal comma'
                  else if (lparcount.eq.2.and.rparcount.eq.1) then
                     vararg(varix) = word
                     varix = varix+1
                  else
                     errtext = 'Illegal comma'
                  end if
                  word = wordinit
                  wordlen = 0
               else if (ch1.eq.'=') then
                  if (lparcount.ne.1.or.rparcount.ne.1) then
                     wordlen = wordlen + 1
                     word(wordlen:wordlen) = ch1
                  end if
               else if (ch1.ne.' '.and.ch1.ne.tab) then
                  wordlen = wordlen + 1
                  word(wordlen:wordlen) = ch1
               end if
            else if (lparcount.ge.2.and.rparcount.ge.2) then
               if ((ch1.eq.'+'.or.ch1.eq.'-').and.
     +                          lparcount.eq.rparcount) then
                  if (lcoper.eq.1) then
                     if (wordlen.eq.0) then
                        errtext = 'Illegal sign on factor'
                     end if
                     lcr1 = word
                  end if
                  if (lcoper.ge.2) then
                     if (ch1.eq.'-') then
                        if (lcoper.eq.2) then
                           lcoper = 3
                        else
                           lcoper = 2
                        end if
                     end if
                  else
                     if (ch1.eq.'+') then
                         lcoper = 2
                     else
                        lcoper = 3
                     end if
                  end if
                  word = wordinit
                  wordlen = 0
               else if (ch1.eq.'*') then
                  if (lcoper.ne.0) then
                     errtext = 'Illegal extra operator (*, + or -)'
                  end if
                  lcoper = 1
                  word = wordinit
                  wordlen = 0
               else if (ch1.eq.'(') then
                  lparcount = lparcount + 1
               else if (ch1.eq.')') then
                  rparcount = rparcount + 1
               else if (ch1.ne.' '.and.ch1.ne.tab) then
                  if (ch1.eq.'e'.or.ch1.eq.'E') then
                     wordlen = wordlen + 1
                     word(wordlen:wordlen) = ch1
                     i1 = i1 + 1
                     if (i1.gt.inplen) then
                        errtext = 'Unrecognized e or E'
                     else
                        ch1 = inpline(i1:i1)
                        if (ch1.ne.'+'.and.ch1.ne.'-') then
                           errtext = 'Malformed scientific notation'
                        endif
                     endif
                  end if
                  wordlen = wordlen + 1
                  word(wordlen:wordlen) = ch1
               end if
            else
               errtext = 'Paranteses mismatch'
            end if
            if (lenstr(errtext,1).gt.1) then
               goto 10
            end if
            i1 = i1 + 1
         end do
         if (lcoper.eq.1) then
            lcr1 = word
         else if (lcoper.eq.2) then
            lcr2 = word
         else if (lcoper.eq.3) then
            if (word(1:1).eq.'-') then
               word(1:1) = '+'
               lcr2 = word
            else if (word(1:1).eq.'+') then
               word(1:1) = '-'
               lcr2 = word
            else
               lcr2 = '-' // word
            end if
         end if
         if (feltid(1:4).ne.'FELT') then
            errtext = 'Illegal name of first field. Should be FELT'
         end if
         if (lcr1(1:1).ne.' '.or.lcr2(1:1).ne.' ') then
            r1lc = 1.0
            if (lcr1(1:1).ne.' ') r1lc = toreal(lcr1,50)
            r2lc = 0.0
            if (lcr2(1:1).ne.' ') r2lc = toreal(lcr2,50)
            linconv = .true.
         else
            linconv = .false.
         end if
   10    continue
         if (lenstr(errtext,1).gt.1) then
            write (*,*) ' '
            write (*,*) 'SYNTAX ERROR AT LINE ',linenum,':'
            write (*,*) inpline
            write (*,*) errtext
            write (*,*) ' '
            goto 20
         end if
         feltix = feltix-1
         varix = varix-1
         if (debug) then
            write (*,*) feltid
            write (*,*) 'feltix=',feltix
            write (*,*) (feltarg(i1),i1=1,feltix)
            write (*,*) varname
            write (*,*) 'varix=',varix
            write (*,*) (vararg(i1),i1=1,varix)
            write (*,*) 'LC: ',linconv,r1lc,r2lc
            write (*,*) '--- Calling new_ncvar'
         end if
         if (new_ncvar(varname).lt.0) then
            stop
         end if
c         if (debug) then
c            if (chareport().lt.0) then
c               stop
c            end if
c         end if
         if (debug) write (*,*) '--- Calling transfield'
         if (transfield(numf,vararg,varix,feltarg,feltix,
     +                            linconv,r1lc,r2lc).lt.0) then
            stop
         end if
         linenum = linenum+1
      end do
   20 continue
      if (debug) write (*,*) '--- Calling ncf_close'
      if (ncf_close().lt.0) then
         stop
      end if
      end
c***********************************************************************
      integer function
     +   transfield(numf,vararg,ixvarcount,feltarg,ixfeltcount,
     +              linconv,r1lc,r2lc)
c--------------------------------------------------------------------
c
c  Purpose: Transfer one two-dimensional field from the netcdf file
c           to the FELT file.
c
c  numf        - Field counter. Starts at 1 for the first invocation
c                of transfield, and increases by 1 for each subsequent
c                invocation.
c  vararg      - Character array containing the arguments for the netcdf 
c                variable
c  ixvarcount  - Number of arguments for the netcdf variable
c  feltarg     - Arguments for the FELT description (char array)
c  ixfeltcount - Number of FELT arguments
c  linconv     - =.true. if linear conversion are to take place
c  r1lc        - New value = r1lc*(old value) + r2lc
c  r2lc          (if linconv .eq. .true.). 
c
c--------------------------------------------------------------------
      implicit none
      include "nf_ncf.inc"
      include "netcdf.inc"
      integer numf,ixvarcount,ixfeltcount
      character*(*) vararg(ixvarcount),feltarg(ixfeltcount)
      logical linconv
      real r1lc,r2lc
      character*50 varg
      integer year,month,day,hour,minute
      integer lfield,lncfield
      integer grid,ierror,ipack
      integer datatype,fclength,level1,level2,vertcoord,param
      integer*2 innh(16,64)
      integer toint,lenstr,chamatch,getvm_real,getvm_int,getvm_short
      integer getvm_byte,toint2,prepfelt,adjustdate
      integer startixs(ixvarcount),counts(ixvarcount),
     +        strides(ixvarcount),imaps(ixvarcount)
      integer hash1,hash2,ix,i1,i2,j1,j2,j3,j4,j5,ios,ixinnh,igrid
      integer isub1,isub2,jsub1,jsub2
      logical subfield
      character*100 errmsg
      character*80 chaerrmsg
c
c  ------------------------------------------------------
c  Loop to investigate arguments for the netcdf variable:
c  ------------------------------------------------------
c
      hash1 = 0
      hash2 = 0
c
c  These two variables are used to mark the position of the #1 and #2
c  arguments.
c
      do ix = 1,ixvarcount
         if (debug) then
            write (*,*) '  transfield: investigating var index no.',ix
         end if
         strides(ix) = 1
c
c  Take arguments from the netcdf variable in opposite order so it
c  will correspond to the internal argument order, and not the
c  external (CDL style) order:
c
         varg = vararg(ixvarcount+1-ix)
         if (varg(1:1).eq.'#') then
            startixs(ix) = 1
            counts(ix) = ncdimsize(ix)
            if (varg(2:2).eq.'1') then
               hash1 = ix
            else
               hash2 = ix
            end if
         else if (varg(1:2).eq.'-#') then
            startixs(ix) = 1
            counts(ix) = ncdimsize(ix)
c
c  Sample from the netcdf variable in opposite direction:
c
            if (varg(3:3).eq.'1') then
               hash1 = -ix
            else
               hash2 = -ix
            end if
         else
c  
c  Search for match in the charr system
c  
c     ncchid(ix) - Array identification number
c     varg - Character value to search for
c     j2         - Size of varg
c     j3         - Index of matched entry (returned)
c     j5         - =0 if content of varg is a non-negative integer
c                  (in this case it will be possible to use it as an
c                  index in the netcdf variable directly)
c     j1         - Match result (returned):
c                  <0: Error
c                  1: One exact match
c                  2: One partial match (all of varg matched,
c                     but extra characters in array).
c                  3: Two or more exact matches
c                  4: No match or several partial matches
c  
            j2 = lenstr(varg,1)
            j5 = 0
            do i1=1,j2
               j4 = ichar(varg(i1:i1))
               if (j4.lt.48.or.j4.gt.57) j5 = 1
            end do
            if (ncchid(ix).lt.0) then
               if (debug) write (*,*) 
     +         '  transfield: Direct index: ',varg
               if (j5.ne.0) then
                  errmsg = 'Direct index not integer'
                  goto 10
               end if
               j3 = toint(varg)
            else
               if (debug) then
                  write (*,*) 
     +            '  transfield: Searching for character match: ',
     +            varg
                  write (*,*) '              Length=',j2
                  write (*,*) '              Array no.=',ncchid(ix)
               end if
               j1 = chamatch(ncchid(ix),varg,j2,j3)
               if (debug) then
                  write (*,*) '              Result: j1=',j1,' j3=',j3
               end if
               if (j1.lt.0) then
                  errmsg = 'chamatch: ' // chaerrmsg(j1)
                  goto 10
               else if (j1.gt.2) then
c
c  A single match was not found. 
c
                  errmsg = 'Netcdf variable index not found, ' //
     +                     'ambiguous or out of range:' // varg
c23456789012345678901234567890123456789012345678901234567890123456789012
                  goto 10
               end if
            end if
            if (j3.lt.1.or.j3.gt.ncdimsize(ix)) then
               errmsg = 'Index out of range'
               goto 10
            else
               startixs(ix) = j3
               counts(ix) = 1
            end if
         end if
         imaps(ix) = 1
      end do
c
c  End argument loop
c
      if (hash1.eq.0) then
         errmsg = 'No #1 index in netcdf value'
         goto 10
      end if
      if (hash2.ne.0) then
         imaps(iabs(hash2)) = ncdimsize(iabs(hash1))
      end if
      lncfield = ncdimsize(iabs(hash1))
      if (hash2.ne.0) then
         lncfield = lncfield*ncdimsize(iabs(hash2))
      endif
      if (debug) then
         write (*,*) '  transfield: Arguments to nf_get_varm_...:'
         write (*,*) '     startixs:',(startixs(i1),i1=1,ixvarcount)
         write (*,*) '     counts:  ',(counts(i1),i1=1,ixvarcount)
         write (*,*) '     strides: ',(strides(i1),i1=1,ixvarcount)
         write (*,*) '     imaps:   ',(imaps(i1),i1=1,ixvarcount)
      endif
c
c  -------------------------------------------------------------
c  Read from the netcdf variable into one of the arrays used for
c  writing to the FELT file:
c  -------------------------------------------------------------
c
      if (nctype.eq.NF_FLOAT.or.nctype.eq.NF_DOUBLE) then
c
c  Read to the 'field' array and substitute all missing values
c  with the FELT missing value code:
c
         if (debug) write (*,*) 
     +      '  transfield: Reading field (getvm_real)'
         if (getvm_real(ncid,ncvarid,startixs,counts,strides,imaps,
     +                   lncfield).lt.0) then
            errmsg = 'Error return from getvm_real'
            goto 10
         end if
         ipack = 2
      else if (nctype.eq.NF_INT) then
c
c  Read to integer buffer array locally declared in 'getvm_int'.
c  Transfer to the 'field' array while substituting all missing values
c  with the FELT missing value code:
c
         if (debug) write (*,*) 
     +      '  transfield: Reading field (getvm_int)'
         if (getvm_int(ncid,ncvarid,startixs,counts,strides,imaps,
     +                   lncfield).lt.0) then
            errmsg = 'Error return from getvm_int'
            goto 10
         end if
         ipack = 2
      else if (nctype.eq.NF_SHORT) then
c
c  Read to the 'idata' array and substitute all missing values
c  with the FELT missing value code for idata (short):
c
         if (debug) write (*,*) 
     +      '  transfield: Reading field (getvm_short)'
         if (getvm_short(ncid,ncvarid,startixs,counts,strides,imaps,
     +                   lncfield).lt.0) then
            errmsg = 'Error return from getvm_short'
            goto 10
         end if
         ipack = 0
      else if (nctype.eq.NF_BYTE) then
c
c  Read to byte (integer*1) buffer array locally declared in 'getvm_byte'.
c  Copy to the 'idata' array while substituting all missing values
c  with the FELT missing value code for idata (short):
c
         if (debug) write (*,*) 
     +      '  transfield: Reading field (getvm_byte)'
         if (getvm_byte(ncid,ncvarid,startixs,counts,strides,imaps,
     +                   lncfield).lt.0) then
            errmsg = 'Error return from getvm_byte'
            goto 10
         end if
         ipack = 0
      end if
c
c  -----------------------------------------------------
c  Decode information from the FELT input specification:
c  -----------------------------------------------------
c
      if (debug) write (*,*) 
     +      '  transfield: Decode FELT input specification'
      year = -1
      if (ixfeltcount.lt.7.or.ixfeltcount.gt.8) then
         errmsg = 'FELT: Illegal number of comma-separated '//
     +                'id-fields. Should be 7 or 8'
         goto 10
      else if (ixfeltcount.eq.7) then
         igrid = 1
         datatype = toint(feltarg(2))
         fclength = toint(feltarg(3))
         level1 = toint(feltarg(4))
         level2 = toint(feltarg(5))
         vertcoord = toint(feltarg(6))
         param = toint(feltarg(7))
      else
         if (feltarg(1)(5:5).ne.'-'.or.feltarg(1)(8:8).ne.'-'.or.
     +            feltarg(1)(11:11).ne.':') then
            errmsg = 'FELT: Illegal date/time format.'//
     +                   ' Should be: YYYY-MM-DD:HHMM'
            goto 10
         else
            year = toint2(feltarg(1)(1:4),4)
            month = toint2(feltarg(1)(6:7),2)
            day = toint2(feltarg(1)(9:10),2)
            hour = toint2(feltarg(1)(12:13),2)
            minute = toint2(feltarg(1)(14:15),2)
            if (debug) write (*,*) year,month,day,hour,minute
            if (year.lt.1000.or.month.lt.1.or.month.gt.12.or.
     +               day.lt.1.or.day.gt.31.or.hour.lt.0.or.
     +            hour.gt.23.or.minute.lt.0.or.minute.gt.59) then
               errmsg = 'FELT:  Illegal date/time format.'//
     +                      ' Should be: YYYY-MM-DD:HHMM'
               goto 10
            end if
            igrid = 2
            datatype = toint(feltarg(3))
            fclength = toint(feltarg(4))
            level1 = toint(feltarg(5))
            level2 = toint(feltarg(6))
            vertcoord = toint(feltarg(7))
            param = toint(feltarg(8))
         end if
      end if
      j5 = lenstr(feltarg(igrid),1)
      j1 = 0
      j2 = 0
      j3 = 0
      j4 = 0
      do i1=1,j5
         if (feltarg(igrid)(i1:i1).eq.':') then
            if (j1.eq.0) then
               j1=i1
            else
               j3=i1
            end if
         end if
         if (feltarg(igrid)(i1:i1).eq.'-') then
            if (j2.eq.0) then
               j2=i1
            else
               j4=i1
            end if
         end if
      end do
      if (j1.gt.0) then
         subfield = .true.
         isub1 = toint2(feltarg(igrid)(j1+1:j2-1),j2-j1-1)
         isub2 = toint2(feltarg(igrid)(j2+1:j3-1),j3-j2-1)
         jsub1 = toint2(feltarg(igrid)(j3+1:j4-1),j4-j3-1)
         jsub2 = toint2(feltarg(igrid)(j4+1:j5),j5-j4)
         grid = toint2(feltarg(igrid),j1-1)
      else
         subfield = .false.
         grid = toint(feltarg(igrid))
      end if
c
c  -----------------------------------------------------
c  Prepare FELT identification for mwfelt (idata(1-20)):
c  -----------------------------------------------------
c
      if (debug) write (*,*) 
     +      '  transfield: Prepare FELT for mwfelt'
      if (grid.eq.idataid(2)) then
c
c  Same grid as previous field:
c
         lfield = idataid(10)
         lfield = lfield*idataid(11)
         do i1=1,20
            idata(i1) = idataid(i1)
            idata(20+lfield+i1) = idataid(20+i1)
         end do
      else
c
c  Get grid parameters from grid definition file:
c
         j1 = maxidata
         idata(2) = grid
         if (prepfelt(j1,idata,lfield).lt.0) then
            errmsg = 'Error return from prepfelt'
            goto 10
         endif
c
c  Save grid parameters for later use:
c
         do i1=1,20
            idataid(i1) = idata(i1)
            idataid(20+i1) = idata(20+lfield+i1)
         end do
         if (debug) then
            write (*,*) '   idata saved in idataid.'
            write (*,*) '      idataid(2)= ',idataid(2)
         endif
      endif
      idata(1)= producer
      idata(2)= grid
      if (debug) write (*,*) '   idata: ',(idata(i1),i1=1,20)
c
c  Save date/time from previous field:
c
      if (numf.eq.1) then
         prevdate(1) = datetime(1)
         prevdate(2) = datetime(2)*100 + datetime(3)
         prevdate(3) = datetime(4)*100 + datetime(5)
      end if
c
      if (felttype.eq.999) then
c
c  Date/time is taken from line one in the specification file:
c
         idata(12)= datetime(1)
         idata(13)= 100*datetime(2)+datetime(3)
         idata(14)= 100*datetime(4)+datetime(5)
      else
         if (year.ne.-1) then
            idata(12)= year
            idata(13)= month*100 + day
            idata(14)= hour*100 + minute
            if (idata(12).ne.prevdate(1).or.
     +          idata(13).ne.prevdate(2).or.
     +          idata(14).ne.prevdate(3)) then
c
c  Adjust ixdate:
c
               if (adjustdate(year,month,day,hour,minute,prevdate)
     +                         .lt. 0) then
                  errmsg = 'Error return from adjustdate'
                  goto 10
               endif
            endif
         else if (numf.eq.1) then
            errmsg = 'Date/time for the initial field must be given'
            goto 10
         else
c
c        (Othervise, date/time is kept from previous field)
c
            idata(12) = prevdate(1)
            idata(13) = prevdate(2)
            idata(14) = prevdate(3)
         end if
      end if
      idata(3) = datatype
      idata(4) = fclength
      idata(5) = vertcoord
      idata(6) = param
      idata(7) = level1
      idata(8) = level2
      idata(20)=-32767
c
c  Create the correct 16-word index in 'innholdsfortegnelse':
c
      if (felttype.eq.999) then
         ixinnh = numf
      else
         if (idata(12).eq.prevdate(1).and.
     +       idata(13).eq.prevdate(2).and.
     +       idata(14).eq.prevdate(3)) then
            ixslot = ixslot+1
            if (ixslot.gt.slotcount) then
               errmsg = 'Number of slots per date/time in FELT' //
     +                  ' innholdsfortegnelse exeeded'
               goto 10
            endif
         else
            ixslot = 1
         end if
         ixinnh = slotcount*ixdate + ixslot
      endif
      j1 = (ixinnh-1)/64
      j2 = ixinnh - j1*64
      j3 = j1 + 3
      if (debug) write (*,*) 
     +      '  transfield: ixdate,ixslot,ixinnh=',
     +      ixdate,ixslot,ixinnh
      if (debug) write (*,*) 
     +      '  transfield: Update "innholdsfortegnelse (read)"'
      read(11,rec=j3,iostat=ios,err=30) innh
      innh( 1,j2)=idata(1)
      innh( 2,j2)=idata(2)
      innh( 3,j2)=idata(12)
      innh( 4,j2)=idata(13)
      innh( 5,j2)=idata(14)
      innh( 6,j2)=-32767
      innh( 7,j2)=-32767
      innh( 8,j2)=-32767
      innh( 9,j2)=idata(3)
      innh(10,j2)=idata(4)
      innh(11,j2)=idata(5)
      innh(12,j2)=idata(6)
      innh(13,j2)=idata(7)
      innh(14,j2)=idata(8)
      innh(15,j2)=idata(9)
      innh(16,j2)=-32767
      if (debug) write (*,*) 
     +      '              (write)'
      write(11,rec=j3,iostat=ios,err=30) innh
      if (hash1 .lt. 0 .or. hash2 .lt. 0) then
         if (.not.subfield) then
            subfield = .true.
            isub1 = 1
            isub2 = idata(10)
            jsub1 = 1
            jsub2 = idata(11)
         end if
         if (hash1 .lt. 0) then
            j1 = isub1
            isub1 = isub2
            isub2 = j1
         end if
         if (hash2 .lt. 0) then
            j1 = jsub1
            jsub1 = jsub2
            jsub2 = j1
         end if
      end if
c
c  If only a rectangular subfield of the FELT field are to be transferred,
c  reorganize idata (ipack=0) or field (ipack >= 1) accordingly:
c
      if (subfield) then
         call subftrans(ipack,isub1,isub2,jsub1,jsub2)
      end if
      if (linconv) then
c
c  Perform a linear transformation of each field value
c
         if (ipack.eq.0) then
            do i1=1,lfield
               if (idata(20+i1).eq.imisv) then
                  field(i1) = fmisv
               else
                  field(i1) = r1lc*idata(20+i1) + r2lc
               end if
            end do
            ipack = 2
         else
            do i1=1,lfield
               if (field(i1).ne.fmisv) then
                  field(i1) = r1lc*field(i1) + r2lc
               end if
            end do
         end if
      end if
c
c  -------------------
c  Write to FELT file:
c  -------------------
c
      if (ipack.eq.0) then
c
c        No scaling:
c
         idata(20) = 0
      end if
      if (debug) write (*,*) '  transfield: idata(1-8):',
     +   (idata(i1),i1=1,8)
      if (debug) write (*,*) 
     +      '  transfield: Write to FELT file (mwfelt)'
      if (ipack.eq.0) then
         idata(20) = 0
      end if
      call mwfelt(2,nmfelt,11,ipack,lfield,field,1.0,
     +            40+lfield,idata,ierror)
      if (ierror.ne.0) then
         write (errmsg,'(A,I3)') 'mwfelt(2,...) error: ',ierror
         goto 10
      end if
      prevdate(1) = idata(12)
      prevdate(2) = idata(13)
      prevdate(3) = idata(14)
      if (debug) write (*,*) 'prevdate, saved from previous field:',
     +                (prevdate(i1),i1=1,3)
      transfield = 1
      return
   10 write (*,*) ' '
      write (*,*) 'Error return from transfield:'
      write (*,*) errmsg
      write (*,*) ' '
      transfield = -1
      return
   30 write (*,*) ' '
      write (*,*) 'Error return from transfield:'
      write (*,*) 'Read or write error while updating'//
     +            '"innholdsfortegnelse"'
      write (*,*) 'iostat= ',ios
      call pioerr(6,ios)
      write (*,*) ' '
      transfield = -1
      return
      end
c***********************************************************************
      subroutine subftrans(ipack,isub1,isub2,jsub1,jsub2)
      integer nx,ny,iy,ix,ipack,isub1,isub2,jsub1,jsub2,is,ito
      integer is1,is2,iswap,isb1,isb2,jsb1,jsb2,icount,jcount
      integer jct,ict
      real fswap
      include "nf_ncf.inc"
      if (debug) then
         write (*,*) '   subftrans: ipack,isub1,isub2,jsub1,jsub2=',
     +            ipack,isub1,isub2,jsub1,jsub2
      end if
      nx = idata(10)
      ny = idata(11)
      if (debug) write (*,*) '              nx,ny=',nx,ny
      icount = iabs(isub2-isub1)+1
      jcount = iabs(jsub2-jsub1)+1
      isb1=isub1
      isb2=isub2
      jsb1=jsub1
      jsb2=jsub2
      if (isub1>isub2) then
         isb1=nx+1-isub1
         isb2=nx+1-isub2
      endif
      if (jsub1>jsub2) then
         jsb1=ny+1-jsub1
         jsb2=ny+1-jsub2
      endif
      if (isub1>isub2) then
c
c   FELT(date,grid:isub1-isub2:....
c   where isub1 > isub2: Swap along the x dimension
c
         jct = jcount
         do while (jct.ge.1)
            is1 = icount*jct
            is2 = icount*(jct-1) + 1
            do while (is1.gt.is2)
               if (ipack.eq.0) then
                  iswap = idata(20+is1)
                  idata(20+is1) = idata(20+is2)
                  idata(20+is2) = iswap
               else
                  fswap = field(is1)
                  field(is1) = field(is2)
                  field(is2) = fswap
               end if
               is1 = is1 - 1
               is2 = is2 + 1
            enddo
            jct = jct-1
         enddo
      endif
      if (jsub1>jsub2) then
c
c   FELT(date,grid:isub1-isub2:jsub1-jsub2,...
c   where jsub1 > jsub2: Swap along the y dimension
c
         ict = icount
         do while (ict.ge.1)
            js1 = (jcount-1)*icount + ict
            js2 = ict
            do while (js1.gt.js2)
               if (ipack.eq.0) then
                  iswap = idata(20+js1)
                  idata(20+js1) = idata(20+js2)
                  idata(20+js2) = iswap
               else
                  fswap = field(js1)
                  field(js1) = field(js2)
                  field(js2) = fswap
               end if
               js1 = js1 - icount
               js2 = js2 + icount
            enddo
            ict = ict-1
         enddo
      endif
      is = icount*jcount
      iy = ny
      do while (iy.ge.1)
         if (iy.ge.jsb1.and.iy.le.jsb2) then
            ix = nx
            do while (ix.gt.isb2)
               ito = nx*(iy-1) + ix
               if (ipack.eq.0) then
                  idata(20+ito) = imisv
               else
                  field(ito) = fmisv
               end if
               ix = ix-1
            end do
            do while (ix.ge.isb1)
               ito = nx*(iy-1) + ix
               if (ipack.eq.0) then
                  idata(20+ito) = idata(20+is)
               else
                  field(ito) = field(is)
               end if
               ix = ix-1
               is = is-1
            end do
            do while (ix.ge.1)
               ito = nx*(iy-1) + ix
               if (ipack.eq.0) then
                  idata(20+ito) = imisv
               else
                  field(ito) = fmisv
               end if
               ix = ix-1
            end do
         else
            ix = nx
            do while (ix.ge.1)
               ito = nx*(iy-1) + ix
               if (ipack.eq.0) then
                  idata(20+ito) = imisv
               else
                  field(ito) = fmisv
               end if
               ix = ix-1
            end do
         end if
         iy = iy-1
      end do
      return
      end
c***********************************************************************
      integer function adjustdate (year,month,day,hour,minute,
     +                            prevtime)
c
c  Purpose: Adjust ixdate (from common /ncf/)
c
c  Check if the given date/time represents a whole number of time
c  steps from previous date/time (array prevtime). If so, adjust 
c  ixdate: the number of time steps between base time and the given time.
c
      implicit none
      integer year,month,day,hour,minute,prevtime(3)
      integer jtime(5)
      integer jstep,i1,ierror
      include "nf_ncf.inc"
c
      jtime(1) = prevtime(1)
      jtime(2) = prevtime(2) / 100
      jtime(3) = mod(prevtime(2),100)
      jtime(4) = prevtime(3) / 100
      if (debug) write (*,*) '  adjustdate:'
      if (debug) write (*,*) '    year,month,day,hour,minute: ',
     +            year,month,day,hour,minute
      if (timeunit.eq.3) then
         jstep = steplength*24
      else if (timeunit.eq.4) then
         jstep = steplength
      endif
      if (debug) write (*,*) '    jtime:',(jtime(i1),i1=1,5)
      if (debug) write (*,*) '    jstep:',jstep
      if (debug) write (*,*) '    Start loop'
      do while (.true.)
         if (timeunit.eq.1) then
            jtime(1) = jtime(1)+steplength
         else if (timeunit.eq.2) then
            jtime(2) = jtime(2)+steplength
            do while (jtime(2).gt.12)
               jtime(1) = jtime(1)+1
               jtime(2) = jtime(2)-12
            end do
         else if (timeunit.eq.3.or.timeunit.eq.4) then
            jtime(5) = jstep
            call vtime (jtime,ierror)
            if (ierror.ne.0) then
               write (*,*) '        vtime error:',ierror
               adjustdate = -1
               return
            end if
         end if
         ixdate = ixdate+1
         if (jtime(1).lt.year) goto 20
         if (jtime(1).gt.year) goto 10
         if (jtime(2).lt.month) goto 20
         if (jtime(2).gt.month) goto 10
         if (jtime(3).lt.day) goto 20
         if (jtime(3).gt.day) goto 10
         if (jtime(4).lt.hour) goto 20
         if (jtime(4).gt.hour) goto 10
         adjustdate = 1
         if (debug) write (*,*) '       OK return'
         return
   10    adjustdate = -1
         write (*,*) '       Error return, jtime:',(jtime(i1),i1=1,5)
         return
   20    continue
         if (debug) write (*,*) '       jtime:',(jtime(i1),i1=1,5)
      end do
      return
      end
c***********************************************************************
      integer function getvm_real (id,varid,startixs,counts,strides,
     +                              imaps,lfield)
c
c     Read real-valued array from netcdf variable.
c     Also convert missing values to
c     FELT missing value code (real).
c
      implicit none
      integer id,varid,startixs(*),counts(*),strides(*),imaps(*),lfield
      include "netcdf.inc"
      include "nf_ncf.inc"
      logical isnaninf
      integer i1, i2, jstat, nfstat
      real r1,r2,r3
C
C     Read from array section of NetCDF variable (real)
C     varid:    Variable ID
C     startixs:  Array of start indices (integer)
C     counts:  Array of value counts for each index (integer)
C     strides: Array defining strides (all 1)
C     imaps: Array defining jumps (in array element count) in
C            the destination array between data values corresponding
C            to consecutive index values in the netcdf array.
C     field:  Array to receive data (real)
C
      nfstat = nf_get_varm_real(id,varid,startixs,counts,
     +                          strides,imaps,field)
      if (nfstat.ne.NF_NOERR) then
         write (*,*) ' '
         write (*,*) 'nf_get_varm_real: '
         write (*,*) nf_strerror(nfstat)
         getvm_real = -1
         return
      end if
      do i1=1,lfield
         if (isnaninf(field(i1))) then
            field(i1) = fmisv
         else
            r3 = 0.1*field(i1)
            if (i1.le.150.and.debug) then
               write (*,*) i1,field(i1)
            end if
            do i2=1,misvalcount
               r1 = 0.1*misv_real(i2) - abs(0.0000001*misv_real(i2))
               r2 = 0.1*misv_real(i2) + abs(0.0000001*misv_real(i2))
               if (r3.ge.r1.and.r3.le.r2) field(i1) = fmisv
            enddo
         end if
      end do
      getvm_real = 1
      return
      end
c***********************************************************************
      integer function getvm_int (id,varid,startixs,counts,strides,
     +                              imaps,lfield)
c
c     Read integer-valued array from netcdf variable and convert to
c     real. Also convert missing values (integer code in netcdf) to
c     FELT missing value code (real).
c
      implicit none
      integer id,varid,startixs(*),counts(*),strides(*),imaps(*),lfield
      include "netcdf.inc"
      include "nf_ncf.inc"
      integer ibuff(lfield)
      integer i1, i2, jstat
c
      jstat = nf_get_varm_int(id,varid,startixs,counts,
     +                        strides,imaps,ibuff)
      if (jstat.ne.NF_NOERR) then
         write (*,*) ' '
         write (*,*) 'Error return from getvm_int:'
         write (*,*) nf_strerror(jstat)
         getvm_int = -1
         return
      end if
      do i1=1,lfield
         field(i1) = ibuff(i1)
         do i2=1,misvalcount
            if (ibuff(i1).eq.misv_int(i2)) then
               field(i1) = fmisv
            endif
         enddo
      end do
      getvm_int = 1
      return
      end
c***********************************************************************
      integer function getvm_short (id,varid,startixs,counts,strides,
     +                              imaps,lfield)
c
c     Read short-valued array from netcdf variable and store in idata.
c     Also convert missing values (short-valued code in netcdf) to
c     FELT missing value code (short).
c
      implicit none
      integer id,varid,startixs(*),counts(*),strides(*),imaps(*),lfield
      include "netcdf.inc"
      include "nf_ncf.inc"
      integer i1, i2, jstat
c
      jstat = nf_get_varm_int2(id,varid,startixs,counts,
     +                        strides,imaps,idata(21))
      if (jstat.ne.NF_NOERR) then
         write (*,*) ' '
         write (*,*) 'Error return from getvm_short:'
         write (*,*) nf_strerror(jstat)
         getvm_short = -1
         return
      end if
c      write (*,*) 'Set missing values: '
      do i1=21,lfield+20
         do i2=1,misvalcount
c            write (*,*) idata(i1), misv_short(i2), imisv
            if (idata(i1).eq.misv_short(i2)) then
               idata(i1) = imisv
            end if
         enddo
      end do
      getvm_short = 1
      return
      end
c***********************************************************************
      integer function getvm_byte (id,varid,startixs,counts,strides,
     +                              imaps,lfield)
c
c     Read byte-valued array from netcdf variable and store in idata.
c     Also convert missing values (byte-valued code in netcdf) to
c     FELT missing value code (short).
c
      implicit none
      integer id,varid,startixs(*),counts(*),strides(*),imaps(*),lfield
      include "netcdf.inc"
      include "nf_ncf.inc"
      integer i1, i2, jstat
      integer*1 ibuf(lfield)
c
      jstat = nf_get_varm_int1(id,varid,startixs,counts,
     +                        strides,imaps,ibuf)
      if (jstat.ne.NF_NOERR) then
         write (*,*) ' '
         write (*,*) 'Error return from getvm_byte:'
         write (*,*) nf_strerror(jstat)
         getvm_byte = -1
         return
      end if
      do i1=1,lfield
         idata(i1+20) = ibuf(i1)
         do i2=1,misvalcount
            if (ibuf(i1).eq.misv_byte(i2)) then
               idata(i1+20) = imisv
            end if
         end do
      end do
      getvm_byte = 1
      return
      end
c***********************************************************************
      integer function toint(chval)
      implicit none
      character*(*) chval
      integer i1,j1,j2,jsign,jfirst
      toint = 0
      jsign = 1
      jfirst = 1
      if (chval(1:1).eq.'-') then
         jsign = -1
         jfirst = 2
      end if
      do i1 = jfirst,9
         j1 = ichar(chval(i1:i1))-48
         if (j1.lt.0.or.j1.gt.9) goto 10
         toint = 10*toint+j1
      end do
   10 continue
      toint = jsign*toint
      return
      end
c***********************************************************************
      integer function toint2(chval,nch)
      implicit none
      character*(*) chval
      integer nch
      integer i1,j1,j2,jsign,jfirst
      jsign = 1
      jfirst = 1
      toint2 = 0
      if (chval(1:1).eq.'-') then
         jsign = -1
         jfirst = 2
      end if
      do i1 = jfirst,nch
         j1 = ichar(chval(i1:i1))-48
         if (j1.lt.0.or.j1.gt.9) goto 10
         toint2 = 10*toint2+j1
      end do
   10 continue
      toint2 = jsign*toint2
      return
      end
c***********************************************************************
      real function toreal(chval,nch)
      implicit none
      character*(*) chval
      integer nch
      integer i1,j1,j2,jsign,jstart,i2,i2start,jexp,jsign2
      real rfactor
c
      if (chval(1:1).eq.'-') then
         jsign = -1
         jstart = 2
      else if (chval(1:1).eq.'+') then
         jsign = 1
         jstart = 2
      else
         jsign = 1
         jstart = 1
      end if
      rfactor = 1.0
      toreal = 0.0
      do i1 = jstart,nch
         if (chval(i1:i1).eq.'e'.or.chval(i1:i1).eq.'E') then
            i2start=i1+1
            jsign2 = 1
            if (chval(i1+1:i1+1).eq.'+') then
               i2start=i1+2
            else if (chval(i1+1:i1+1).eq.'-') then
               i2start=i1+2
               jsign2 = -1
            end if
            jexp = 0
            do i2 = i2start,nch
               j1 = ichar(chval(i2:i2))-48
               if (j1.lt.0.or.j1.gt.9) goto 5
               jexp = 10*jexp+j1
            end do
    5       toreal = toreal*(10.0**(jsign2*jexp))
            goto 10
         end if
         if (chval(i1:i1).eq.'.') then
            rfactor = 0.1
         else
            j1 = ichar(chval(i1:i1))-48
            if (j1.lt.0.or.j1.gt.9) goto 10
            if (rfactor.lt.0.9) then
               toreal = toreal + rfactor*j1
               rfactor = rfactor*0.1
            else
               toreal = 10*toreal+j1
            end if
         end if
      end do
   10 continue
      toreal = toreal*jsign
      return
      end
c***********************************************************************
      integer function ncf_init()
      implicit none
      include "nf_ncf.inc"
      integer date1,date2,date3,ioptx(1)
      integer*2 ispecx(3)
      integer j1,i1
c      
c  Declarations for cmdarg 
c      
      character*1   copt(1)
      character*300  cargs(2),cspec(3)
      character*300 fltd1
      character*300 fltd2
      integer       ispec(2),iopt(1),iopts(2,1),nargs,ierror,nerror
      real          rspec(2)
      integer fltdef
C
C  NetCDF declarations
C
      include "netcdf.inc"
      integer nfstat
c
c  Decode command line arguments (cmdarg)
c      
      copt(1) = 'd'
      iopt(1) = 4
      call cmdarg(1,copt,iopt,iopts,2,nargs,cargs,
     +            0,ispec,0,rspec,3,cspec,
     +            ierror,nerror)
      if (ierror.ne.0.or.nargs.ne.2.or.iopts(1,1).le.0) then
         write (*,*) ' '
         write (*,*) 'nctofelt: Wrong command invocation'
         write (*,*) 
     +'          Should be:'
         write (*,*) ' '
         write (*,*) 
     +'              nctofelt -d fdef ncfile feltfile <conversionfile'
         write (*,*) ' '
         write (*,*) 
     +'          where fdef is the flt2flt.def grid definition file'
         write (*,*) ' '
         ncf_init = -1
         return
      end if
      nmnc = cargs(1)
      nmfelt = cargs(2)
      fltd1 = cspec(iopts(2,1))
      fltd2 = cspec(iopts(2,1))
c
c  Read line one of standard input (specification file):
c
      read (5,*) numfields,felttype,producer,date1,date2,date3,
     +           stepcount,slotcount,timeunit,steplength
c
c  Read the grid definition file (flt2flt.def):
c
      if (fltdef(10,fltd1,fltd2).lt.0) then
         write (*,*) 'nctofelt: Error while reading ',fltd1
         write (*,*) '          or ',fltd2
         ncf_init = -1
         return
      endif
c
c  Create FELT file
c
      ioptx(1) = 0
      datetime(1) = date1
      datetime(2) = date2/100
      datetime(3) = mod(date2,100)
      datetime(4) = date3/100
      datetime(5) = mod(date3,100)
      datetime(6) = stepcount
      datetime(7) = timeunit
      datetime(8) = steplength
      ispecx(1) = producer
      ispecx(2) = -32767
      ispecx(3) = slotcount
      if (debug) write (*,*) '  Creating FELT file.'
      j1=8
      if (felttype.eq.999) j1=5
      if (debug) write (*,*) '     felttype: ',felttype
      if (debug) write (*,*) '           j1: ',j1
      if (debug) write (*,*) '     datetime: ',(datetime(i1),i1=1,8)
      if (debug) write (*,*) '     ispecx: ',(ispecx(i1),i1=1,3)
      call crefelt(nmfelt,11,felttype,j1,datetime,0,3,ispecx,
     +             1,ioptx,ierror)
      if (debug) write (*,*) '  crefelt finished'
      if (ierror.ne.0) then
         write (*,*) 'Error return from crefelt, ierror=',ierror
         ncf_init = -1
         return
      endif
c
c  Initialize write to felt file loop (mwfelt)
c  Use unit=11
c
      call mwfelt(1,nmfelt,11,2,1,field,1.0,
     *            szfeltrec1,feltrec1,ierror)
      if (ierror.ne.0) then
         write (*,*) 'nctofelt: Error while initializing FELT file'
         ncf_init = -1
         return
      endif
c
c  Initialize idataid, ixdate and ixslot (from /ncf/ common)
c
      idataid(2) = -1
      ixdate = 0
      ixslot = 0
C      
C  Open NetCDF file for reading
C      
      if (debug) write (*,*) '  trying to open netcdf file: ', nmnc
      nfstat = nf_open(nmnc,0,ncid)
      if (nfstat.ne.NF_NOERR) then
         write (*,*) nf_strerror(nfstat)
         ncf_init = -1
         return
      endif
      if (debug) write (*,*) '  OK, netcdf file opened'
C      
      ncf_init = 1
      return
      end
c***********************************************************************
      integer function ncf_close()
      implicit none
      include "netcdf.inc"
      include "nf_ncf.inc"
      integer nfstat,ierror,ios
      integer*2 idrec1(1024)
C
C     Close NetCDF file
C
      nfstat = nf_close(ncid)
      if (nfstat.ne.NF_NOERR) then
         write (*,*) nf_strerror(nfstat)
         ncf_close = -1
         return
      endif
c
c  Close felt file (mwfelt)
c
      call mwfelt(3,nmfelt,11,2,1,field,1.0,
     *            1,feltrec1,ierror)
      if (ierror.ne.0) then
         write (*,*) 'nctofelt: Error while closing FELT file'
         ncf_close = -1
         return
      endif
      ncf_close = 1
      return
      end
c***********************************************************************
      integer function new_ncvar(varname)
      implicit none
      character*128 varname
      include "nf_ncf.inc"
      include "netcdf.inc"
c
c  Declarations for the charr system
c
      integer chainit,chaints,chafloats,chachars
      character*60 chaerrmsg
      character*80 errmsg
c
      integer j1,j2,j3,j4,j5,i1,i2,nfstat,idcha
      real r1
      integer dimcount,dimtype,dimsdim(100),dimzdim,jres
      integer ncdimvid
      integer dimidata(100000)
      real dimrdata(50000)
      character*50 dimcdata(4000)
      equivalence (dimidata,dimrdata,dimcdata)
      character*128 dimname
      character*128 dummy1
c
c  Nothing to do if varname .eq. ncvarname. The variable
c  is already the active netcdf variable.
c
      if (varname .eq. ncvarname) then
         new_ncvar = 1
         return
      end if
c
c  (Re-)initialize the charr system
c
      if (debug) then
         write (*,*) '  new_ncvar: (Re-)initialize the charr system'
      end if
      jres = chainit()
      if (jres.lt.0) then
	 errmsg = 'Error in new_ncvar. chainit did not succeed:'
	 goto 10
      end if
c
c  Find variable ID from variable name
c  ncvarname: Name of variable (char)
c  ncvarid:  Variable ID (returned)
c
      if (debug) then
         write (*,*) '  new_ncvar: Find variable ID (nf_inq_varid)'
         write (*,*) '     ncid: ',ncid
         write (*,*) '     varname: ',varname
      end if
      nfstat = nf_inq_varid(ncid,varname,ncvarid)
      if (nfstat.ne.NF_NOERR) then
	 errmsg = 'Error: new_ncvar -> nf_inq_varid:'
	 goto 20
      endif
      ncvarname = varname
c
c  Inquire information about the netcdf-variable
c  ncvarid:   Variable ID
c  dummy1:  Name of variable (returned char)
c  nctype:  NetCDF type (returned)
c  (NF_BYTE, NF_CHAR, NF_SHORT, NF_INT, NF_FLOAT, NF_DOUBLE)
c  ncdimcount:   Number of dimensions (returned)
c  ncdims: Array containing dimension IDs (returned)
c  j2:   Number of attributes (returned)
c
      if (debug) then
         write (*,*) '  new_ncvar: Inquire netcdf-variable (nf_inq_var)'
      end if
      nfstat = nf_inq_var(ncid,ncvarid,dummy1,nctype,ncdimcount,ncdims,
     +                    j2)
      if (nfstat.ne.NF_NOERR) then
	 errmsg = 'Error: new_ncvar -> nf_inq_var:'
         goto 20
      endif
c
c  Invert the sequence of dimension IDs in ncdims (so it will
c  correspond to the sequence in a CDL file):
c
c      if (ncdimcount.ge.2) then
c         do i1 = 1,ncdimcount/2
c            j1 = ncdims(ncdimcount+1-i1)
c            ncdims(ncdimcount+1-i1) = ncdims(i1)
c	    ncdims(i1) = j1
c         end do
c      end if
c
      misv_byte(1) = nf_fill_byte
      misv_short(1) = nf_fill_short
      misv_int(1) = nf_fill_int
      misv_real(1) = nf_fill_real
      misvalcount = 1
      if (j2.gt.0) then
C
C     Find attribute type and length
C     ncvarid:    Variable ID
C     _FillValue:   Attribute name
C     j4:    NetCDF type (returned number)
C            (NF_BYTE, NF_CHAR, NF_SHORT, NF_INT, NF_FLOAT, NF_DOUBLE)
C     j5:    Number of values (returned number)
C
	 if (debug) then
            write (*,*) '  new_ncvar: Inquire attribute (nf_inq_att)'
c23456789012345678901234567890123456789012345678901234567890123456789012
	 end if
         nfstat = nf_inq_att(ncid,ncvarid,'_FillValue',j4,j5)
         if (nfstat.eq.NF_NOERR.and.j5.eq.1) then
            if (j4.eq.NF_SHORT.or.j4.eq.NF_INT.or.
     +          j4.eq.NF_BYTE) then
C
C     Read attribute of NetCDF variable (int)
C     ncvarid:    Variable ID
C     '_FillValue':   Name of attribute (char)
C     j3:    Integer fill value (returned)
C
               if (debug) write (*,*)
     +         '  new_ncvar: Read attribute (nf_get_att_int)'
               nfstat = nf_get_att_int(ncid,ncvarid,'_FillValue',j3)
               if (nfstat.eq.NF_NOERR) then
                  misv_byte(1) = j3
                  misv_short(1) = j3
                  misv_int(1) = j3
               endif
            else if (j4.eq.NF_FLOAT.or.j4.eq.NF_DOUBLE) then
               if (debug) write (*,*)
     +         '  new_ncvar: Read attribute (nf_get_att_real)'
               nfstat = nf_get_att_real(ncid,ncvarid,'_FillValue',r1)
               if (nfstat.eq.NF_NOERR) then
                  misv_real(1) = r1
               endif
            endif
         endif
C-----------------------------------------------------------
C
C     Find attribute type and length
C     ncvarid:    Variable ID
C     missing_value:   Attribute name
C     j4:    NetCDF type (returned number)
C            (NF_BYTE, NF_CHAR, NF_SHORT, NF_INT, NF_FLOAT, NF_DOUBLE)
C     j5:    Number of values (returned number)
C
	 if (debug) then
            write (*,*) '  new_ncvar: Inquire attribute (nf_inq_att)'
c23456789012345678901234567890123456789012345678901234567890123456789012
	 end if
         nfstat = nf_inq_att(ncid,ncvarid,'missing_value',j4,j5)
         if (nfstat.eq.NF_NOERR) then
            if (j5.ge.maxmisvals) then
               errmsg = 'Error: Too many missing value codes'
               goto 30
            endif
            if (j4.eq.NF_SHORT.or.j4.eq.NF_INT.or.
     +          j4.eq.NF_BYTE) then
C
C     Read attribute of NetCDF variable (int)
C     ncvarid:    Variable ID
C     'missing_value':   Name of attribute (char)
C     dimidata:    Integer buffer to receive the missing values (returned)
C
               if (debug) write (*,*)
     +         '  new_ncvar: Read attribute (nf_get_att_int)'
               nfstat = nf_get_att_int(ncid,ncvarid,'missing_value',
     +                  dimidata)
               if (nfstat.eq.NF_NOERR) then
                  do i2=1,j5
                     misv_byte(i2+1) = dimidata(i2)
                     misv_short(i2+1) = dimidata(i2)
                     misv_int(i2+1) = dimidata(i2)
                  enddo
                  misvalcount = j5+1
               endif
            else if (j4.eq.NF_FLOAT.or.j4.eq.NF_DOUBLE) then
               if (debug) write (*,*)
     +         '  new_ncvar: Read attribute (nf_get_att_real)'
               nfstat = nf_get_att_real(ncid,ncvarid,'missing_value',
     +                  dimrdata)
               if (nfstat.eq.NF_NOERR) then
                  do i2=1,j5
                     misv_real(i2+1) = dimrdata(i2)
                  enddo
                  misvalcount = j5+1
               endif
            endif
         endif
      endif
C-----------------------------------------------------------------------
      if (debug) then
         write (*,*) 'Number of missing value codes: ', misvalcount
         do i1=1,misvalcount
            write (*,*) misv_short(i1)
         enddo
      endif
      do i1 = 1,ncdimcount
         j1 = ncdims(i1)
c
c  Inquire information about the i1th dimension:
c  j1:  Dimension ID
c  dimname: Name of dimension (returned char)
c  ncdimsize(i1):  Dimension size (returned)
c  
         if (debug) write (*,*)
     +      '  new_ncvar: Inquire dimension (nf_inq_dim)',i1
         nfstat = nf_inq_dim(ncid,j1,dimname,ncdimsize(i1))
         if (nfstat.ne.NF_NOERR) then
	    errmsg = 'Error: new_ncvar -> nf_inq_dim:'
            goto 20
         endif
         if (debug) write (*,*)
     +      '             Dimension name: ',dimname
c  
c  Check if a variable exists with the same name as the dimension
c  dimname: Name of variable (char)
c  ncdimvid:  Variable ID corresponding to dimension (returned)
c  
         if (debug) write (*,*)
     +      '  new_ncvar: Check if dim variable exists (nf_inq_varid)'
         nfstat = nf_inq_varid(ncid,dimname,ncdimvid)
         if (nfstat.eq.NF_ENOTVAR) then
c
c  No such variable exists
c
	    ncchid(i1) = -1
         else if (nfstat.ne.NF_NOERR) then
	    errmsg = 'Error: new_ncvar -> nf_inq_varid:'
            goto 20
	 else
c  
c  Variable corresponding to dimension exists.
c  Copy the values of this variable to the charr system:
c
c  Inquire information about the dimension variable
c  ncdimvid:   Variable ID
c  dummy1:  Name of variable (returned char)
c  dimtype:   NetCDF type (returned)
c  (NF_BYTE, NF_CHAR, NF_SHORT, NF_INT, NF_FLOAT, NF_DOUBLE)
c  dimcount:   Number of dimensions (returned)
c  dimsdim: Array containing dimension IDs (returned)
c  j2:   Number of attributes (returned)
c  
            if (debug) write (*,*)
     +      '  new_ncvar: Inquire dim variable (nf_inq_var)'
            nfstat = nf_inq_var(ncid,ncdimvid,dummy1,dimtype,dimcount,
     +                       dimsdim,j2)
            if (nfstat.ne.NF_NOERR) then
	       errmsg = 'Error: new_ncvar -> nf_inq_var (2):'
               goto 20
            endif
            if (dimcount.ne.1) then
	       errmsg = 'Error: new_ncvar -> dimcount.ne.1'
	       goto 30
            else
               j1 = dimsdim(1)
c  
c  Get the number of values in the dimension variable:
c
c  j1:  Dimension ID
c  dummy1: Name of dimension (returned char)
c  dimzdim:  Dimension size (returned)
c  
               if (debug) write (*,*)
     +         '  new_ncvar: Get number of values in dim variable'
               nfstat = nf_inq_dim(ncid,j1,dummy1,dimzdim)
               if (nfstat.ne.NF_NOERR) then
	          errmsg = 'Error: new_ncvar -> nf_inq_dim (2)'
                  goto 20
               endif
               if (dimtype.eq.NF_BYTE.or.dimtype.eq.NF_SHORT.or.
     +                dimtype.eq.NF_INT) then
                  if (dimzdim.gt.100000) then
	             errmsg = 'Error: new_ncvar -> dimzdim.gt.100000'
	             goto 30
		  end if
c  
c  Reads from an entire NetCDF variable (int)
c  ncdimvid:    Variable ID
c  dimidata:  Array to receive data (integer)
c  
                  if (debug) write (*,*)
     +            '  new_ncvar: Read variable (nf_get_var_int)'
                  nfstat = nf_get_var_int(ncid,ncdimvid,dimidata)
                  if (nfstat.ne.NF_NOERR) then
	             errmsg = 'Error: new_ncvar -> nf_get_var_int:'
                     goto 20
                  endif
c  
c  Put integer array into the charr system
c  
                  if (debug) write (*,*)
     +            '  new_ncvar: Put into the charr system (chaints)'
                  jres = chaints(dimzdim,dimidata,idcha)
                  if (jres.lt.0) then
		     errmsg = 'Error: new_ncvar -> chaints:'
		     goto 10
                  end if
	          ncchid(i1) = idcha
                  if (debug) write (*,*)
     +            '             Array ref=',ncchid(i1)
               else if (dimtype.eq.NF_CHAR) then
                  if (dimzdim.gt.4000) then
	             errmsg = 'Error: new_ncvar -> dimzdim.gt.4000'
	             goto 30
		  end if
c  
c  Reads from an entire NetCDF variable (text)
c  ncdimvid:    Variable ID
c  dimcdata:    Character variable to receive data
c  
                  if (debug) write (*,*)
     +            '  new_ncvar: Read variable (nf_get_var_text)'
                  nfstat = nf_get_var_text(ncid,ncdimvid,dimcdata)
                  if (nfstat.ne.NF_NOERR) then
	             errmsg = 'Error: new_ncvar -> nf_get_var_text:'
                     goto 20
                  endif
c  
c  Put character array into the charr system
c  
                  if (debug) write (*,*)
     +            '  new_ncvar: Put into the charr system (chachars)'
                  jres = chachars(dimzdim,dimcdata,idcha)
                  if (jres.lt.0) then
		     errmsg = 'Error: new_ncvar -> chachars:'
		     goto 10
                  end if
	          ncchid(i1) = idcha
                  if (debug) write (*,*)
     +            '             Array ref=',ncchid(i1)
               else if (dimtype.eq.NF_FLOAT.or.
     +               dimtype.eq.NF_DOUBLE) then
                  if (dimzdim.gt.50000) then
	             errmsg = 'Error: new_ncvar -> dimzdim.gt.50000'
	             goto 30
		  end if
c  
c  Reads from an entire NetCDF variable (real)
c  ncdimvid:    Variable ID
c  dimrdata:  Array to receive data (real)
c  
                  if (debug) write (*,*)
     +            '  new_ncvar: Read variable (nf_get_var_real)'
                  nfstat = nf_get_var_real(ncid,ncdimvid,dimrdata)
                  if (nfstat.ne.NF_NOERR) then
	             errmsg = 'Error: new_ncvar -> nf_get_var_real:'
                     goto 20
                  endif
c  
c  Put real array into the charr system
c  
                  if (debug) write (*,*)
     +            '  new_ncvar: Put into the charr system (chafloats)'
                  jres = chafloats(dimzdim,dimrdata,idcha)
                  if (jres.lt.0) then
		     errmsg = 'Error: new_ncvar -> chafloats:'
		     goto 10
                  end if
	          ncchid(i1) = idcha
                  if (debug) write (*,*)
     +            '             Array ref=',ncchid(i1)
               end if
            end if
         end if
         if (debug) write (*,*)
     +      '  new_ncvar: Dimension done'
      end do
      if (debug) write (*,*)
     +      '  new_ncvar: Normal return'
      new_ncvar = 1
      return
c
c  Error return from the charr system:
c
   10 write (*,*) ' '
      write (*,*) errmsg
      write (*,*) chaerrmsg(jres)
      write (*,*) ' '
      new_ncvar = -1
      return
c
c  Error return from netcdf:
c
   20 write (*,*) ' '
      write (*,*) errmsg
      write (*,*) nf_strerror(nfstat)
      write (*,*) ' '
      new_ncvar = -1
      return
c
c  Other error return:
c
   30 write (*,*) ' '
      write (*,*) errmsg
      write (*,*) ' '
      new_ncvar = -1
      return
      end
c
c********************************************************************
c  CHARR - System for storing and searching in variable length
c          character arrays.
c
c  The interface routines are integer functions returning a negative
c  value upon error. An error message corresponding to a negative
c  return value ('ires') can be retrieved by the character valued
c  function chaerrmsg(iret).
c
c  Interface for building up the arrays:
c
c     ires = chainit()
c        Initialize the CHARR system and make it empty
c
c     ires = chaints(count,intarr,intref)
c        Store an integer array as a new character array in CHARR.
c        count = number of elements, intarr = integer array.
c        intref (returned) = identification value in the CHARR
c        system.
c      
c     ires = chafloats(count,floatarr,floatref)
c        Store a float array as a new character array in CHARR.
c        count = number of elements, floatarr = float array.
c        floatref (returned) = identification value in the CHARR
c        system.
c      
c     ires = chachars(count,chararr,charref)
c        Store a Fortran character array as a new character array
c        in CHARR.
c        count = number of elements, chararr = input character array.
c        charref (returned) = identification value in the CHARR
c        system.
c
c  Interface for searching in the arrays:
c
c     ires = chamatch(refnum,chval,size,matchix)
c        Search in CHARR array identified by 'refnum' for a value given
c        by character argument 'chval' of 'size' characters.
c        The search result is returned in the function value ('ires')
c        and in the integer argument 'matchix':
c
c        ires=1:   One exact match found. Array index returned in 
c                  'matchix'
c
c        ires=2:   One partial match found. All of the 'chval' argument 
c                  match, but the CHARR array element contains extra 
c                  characters. Array index returned in 'matchix'
c
c        ires=3:   Two or more exact matches found. Array index for
c                  the first match returned in 'matchix'
c
c        ires=4:   No matches or several partial matches.
c
c  Function chainit 
c
      integer function chainit()
c
      implicit none
      include "nf_charr.inc"
      freeptr = 1
      arrcount = 0
      chainit = 1
      return
      end
C
C  Function chaints 
C
      integer function chaints(count,intarr,intref)
      implicit none
      integer count,intarr(*),intref
      include "nf_charr.inc"
      character*20 cbuf
      integer i1,i2,j1
C
      chaints = 1
      if (arrcount.ge.maxarrs) then
         chaints = -1
      else
         arrcount = arrcount+1
	 intref = arrcount
         arrs(1,arrcount) = freeptr
         arrs(2,arrcount) = count
         do i1 = 1,count
            write (cbuf,'(I10)') intarr(i1)
            do i2 = 1,10
               if (cbuf(i2:i2).ne.' ') then
                  j1 = 11-i2
                  if (freeptr+j1.gt.maxchars) then
                     chaints = -2
                  else
                     chars(freeptr:freeptr) = char(j1)
                     chars(freeptr+1:freeptr+j1) = cbuf(i2:10)
                     freeptr = freeptr+j1+1
                  end if
                  goto 10
               end if
            end do
   10       continue
            if (chaints.lt.0) goto 20
         end do
      end if
   20 continue
      return
      end
C
C  Function chafloats 
C
      integer function chafloats(count,floatarr,floatref)
      implicit none
      integer count,floatref
      real floatarr(*)
      include "nf_charr.inc"
      character*20 cbuf
      integer i1,i2,j1
C
      chafloats = 1
      if (arrcount.ge.maxarrs) then
         chafloats = -1
      else
         arrcount = arrcount+1
	 floatref = arrcount
         arrs(1,arrcount) = freeptr
         arrs(2,arrcount) = count
         do i1 = 1,count
            write (cbuf,'(F18.6)') floatarr(i1)
            do i2 = 1,11
               if (cbuf(i2:i2).ne.' ') then
                  j1 = 19-i2
                  if (freeptr+j1.gt.maxchars) then
                     chafloats = -2
                  else
                     chars(freeptr:freeptr) = char(j1)
                     chars(freeptr+1:freeptr+j1) = cbuf(i2:18)
                     freeptr = freeptr+j1+1
                  end if
                  goto 10
               end if
            end do
   10       continue
            if (chafloats.lt.0) goto 20
         end do
      end if
   20 continue
      return
      end
C
C  Function chachars 
C
      integer function chachars(count,chararr,charref)
      implicit none
      integer count,charref
      character*(*) chararr(*)
      include "nf_charr.inc"
      integer len1,i1,i2,i3,j1
C
      chachars = 1
      len1 = len(chararr(1))
      if (arrcount.ge.maxarrs) then
         chachars = -1
      else
         arrcount = arrcount+1
	 charref = arrcount
         arrs(1,arrcount) = freeptr
         arrs(2,arrcount) = count
         do i1 = 1,count
            do i2 = 1,len1
               if (chararr(i1)(i2:i2).ne.' ') then
                  do i3 = len1,1,-1
                     if (chararr(i1)(i3:i3).ne.' ') then
                        j1 = i3-i2+1
                        if (freeptr+j1.gt.maxchars) then
                           chachars = -2
                        else
                           chars(freeptr:freeptr) = char(j1)
                           chars(freeptr+1:freeptr+j1) =
     +                                            chararr(i1)(i2:i3)
                           freeptr = freeptr+j1+1
                        end if
                        goto 10
                     end if
		  end do
               end if
            end do
   10       continue
            if (chachars.lt.0) goto 20
         end do
      end if
   20 continue
      return
c23456789012345678901234567890123456789012345678901234567890123456789012
      end
C
C  Function chamatch 
C
      integer function chamatch(refnum,chval,size,matchix)
      implicit none
      integer refnum,size,matchix
      character*(*) chval
      include "nf_charr.inc"
      integer j1,j2,j3,j4,i1,jexact,jmatch,jpart1,jmpart1
      chamatch = 4
      if (refnum.lt.1.or.refnum.gt.arrcount) then
         chamatch = -3
      else
	 j1 = 0
	 j2 = arrs(1,refnum)
	 jexact = 0
	 jmatch = 0
	 jpart1 = 0
	 jmpart1 = 0
         do i1 = 1,arrs(2,refnum)
	    j3 = ichar(chars(j2:j2))
	    j4 = min(j3,size)
	    if (chars(j2+1:j2+j4).eq.chval(1:j4)) then
	       if (j3.eq.size) then
	          jexact = jexact+1
		  if (jmatch.eq.0) jmatch = i1
	       else if (j3.gt.size) then
	          jpart1 = jpart1+1
	          jmpart1 = i1
	       end if
	    end if
	    j2 = j2+j3+1
	 end do
	 if (jexact.eq.1) then
	    chamatch = 1
	    matchix = jmatch
	 else if (jexact.eq.0.and.jpart1.eq.1) then
	    chamatch = 2
	    matchix = jmpart1
	 else if (jexact.gt.1) then
	    chamatch = 3
	    matchix = jmatch
	 end if
      end if
      return
      end
C
C  Function chareport 
C
      integer function chareport()
      implicit none
      include "nf_charr.inc"
      integer i1,i2,j2,j3
      do i2 = 1,arrcount
	 write (*,*) '    Array no. ',i2
	 j2 = arrs(1,i2)
         do i1 = 1,arrs(2,i2)
	    j3 = ichar(chars(j2:j2))
	    write (*,*) '      size=',j3,' value=',chars(j2+1:j2+j3)
	    j2 = j2+j3+1
	 end do
      end do
      chareport = 1
      return
      end
C
C  Function chaerrmsg 
C
      character*80 function chaerrmsg(errnum)
      implicit none
      integer errnum
      if (errnum.eq.-1) then
         chaerrmsg = 'Too many arrays'
      else if (errnum.eq.-2) then
         chaerrmsg = 'Character buffer exhausted'
      else if (errnum.eq.-3) then
         chaerrmsg = 'Array not found'
      else
         chaerrmsg = 'Error code not recogniced'
      end if
      return
      end
c
c***************************************************************************
      integer function fltdef (ludef,fildef1,fildef2)
c
c ludef   - Unit used for opening the fildef file
c fildef1 - Grid and parameter definitions file name
c fildef2 - Grid and parameter definitions file name - if fildef1 is not 
c           found
c
c******************************************************************
c
c Purpose:
c
c Read a grid definition file such as 'flt2flt.def' (used in flt2flt).
c
c-------------
c flt2flt.def:
c---------------------------------------------------------------------------
c * comment ....
c # comment ....
c grid.pol= <grid_no,nx,ny,xp,yp,an,fi> ....................... grid type 1
c grid.geo= <grid_no,nx,ny,long1,lat1,dlong,dlat> ............. grid type 2
c grid.sph= <grid_no,nx,ny,long1,lat1,dlong,dlat,xscen,yscen> . grid type 3
c grid.pol.ext= <grid_no,nx,ny,xp,yp,an,fi,fpol> .............. grid type 4
c grid.mer= <grid_no,nx,ny,long1,lat1,dxkm,dykm,yconlat> ...... grid type 5
c vector.param= <x_param,y_param, x_param,y_param,...> 
c t.and.td.to.rh.param= <param_t,param_td,param_rh, param_t,...>
c direction.true.north.param= <param,param,...>
c turn.180.direction.true.north.param= <param,param,...>
c switch.sign.param= <param,param,...>
c # smoothing: -4=shap4 -2=shap2 1,2,3,...=lowbandpass filter
c param.smooth= <param,n_smooth, param,n_smooth,...>
c param.min= <param,min_value, param,min_value,...>
c param.max= <param,max_value, param,max_value,...>
c param.min.max= <param,min_value,max_value, param,...>
c set.param.scale= <param,iscale, param,iscale,...>
c check.undef.in.first.field ................................. (default)
c check.undef.in.each.field
c ocean.interp.field= <datatype,forecast_hour,v_coord,param,level1,level2>
c end
c---------------------------------------------------------------------------
c 
c 2003-03-17 Adapded from flt2flt.f by Egil Stoeren
c
c******************************************************************
c
      implicit none
c
c declarations
c
      include 'nf_fdef.inc'
c
      integer istop,ios,lindef,k1,k2,kv1,kv2,i1,i2,iundef,ludef
      integer i,j,iend,ierror,n
c
c
      character*72  fildef
      character*72  fildef1
      character*72  fildef2
      character*1   tchar
c
      integer    maxkey
      parameter (maxkey=5)
      integer   mkey,nkey,ikey,kwhere(5,maxkey)
      character*128 cinput,cdata
c
      istop=255
c
c
c......................................................................
c read grid and parameter definitions file
c
c open file
      fildef = fildef1
      open(unit=ludef,file=fildef1,
     +     form='formatted',access='sequential',
     +     status='old',iostat=ios)
      if (ios.ne.0) then
         fildef = fildef2
         open(unit=ludef,file=fildef2,
     +        form='formatted',access='sequential',
     +        status='old',iostat=ios)
         if (ios.ne.0) then
	    write(*,*) 'Open ERROR: ',fildef2
	    istop=100
	    goto 9000
         end if
      end if
c
c..get machine dependant termination character for free format read
      call termchar(tchar)
c
      numgrd=0
      numvec=0
      numtdr=0
      numdir=0
      numsgn=0
      numlim=0
      numscl=0
      numsmo=0
      iundef=-1
      do i=1,7
	iocean(i)=0
      end do
c
      lindef=0
      iend=0
      ios=0
c
      do while (iend.eq.0 .and. ios.eq.0)
c
	 lindef=lindef+1
	 read(ludef,fmt='(a)',iostat=ios) cinput
c
	 if (ios.ne.0) then
	    continue
	 elseif (cinput(1:1).eq.'*' .or. cinput(1:1).eq.'#') then
	    continue
	 else	    
c
            mkey=maxkey
            call keywrd(1,cinput,'=',';',mkey,kwhere,nkey,ierror)
            if(ierror.ne.0) then
	      nkey=0
	      iend=-2
	    end if
c
	    ikey=0
c
            do while (ikey.lt.nkey .and. ios.eq.0)
c
	       ikey=ikey+1
                k1=kwhere(2,ikey)
                k2=kwhere(3,ikey)
               kv1=kwhere(4,ikey)
               kv2=kwhere(5,ikey)
c
	       if (kv1.gt.0) cdata=cinput(kv1:kv2)//tchar
c
c---------------------------------------------------------------------------
c * comment ....
c # comment ....
c grid.pol= <grid_no,nx,ny,xp,yp,an,fi> ....................... grid type 1
c grid.geo= <grid_no,nx,ny,long1,lat1,dlong,dlat> ............. grid type 2
c grid.sph= <grid_no,nx,ny,long1,lat1,dlong,dlat,xscen,yscen> . grid type 3
c grid.pol.ext= <grid_no,nx,ny,xp,yp,an,fi,fpol> .............. grid type 4
c grid.mer= <grid_no,nx,ny,long1,lat1,dxkm,dykm,yconlat> ...... grid type 5
c vector.param= <x_param,y_param, x_param,y_param,...> 
c t.and.td.to.rh.param= <param_t,param_td,param_rh, param_t,...>
c direction.true.north.param= <param,param,...>
c turn.180.direction.true.north.param= <param,param,...>
c switch.sign.param= <param,param,...>
c # smoothing: -4=shap4 -2=shap2 1,2,3,...=lowbandpass filter
c param.smooth= <param,n_smooth, param,n_smooth,...>
c param.min= <param,min_value, param,min_value,...>
c param.max= <param,max_value, param,max_value,...>
c param.min.max= <param,min_value,max_value, param,...>
c set.param.scale= <param,iscale, param,iscale,...>
c check.undef.in.first.field ................................. (default)
c check.undef.in.each.field
c ocean.interp.field= <datatype,forecast_hour,v_coord,param,level1,level2>
c end
c---------------------------------------------------------------------------
c
	       if (cinput(k1:k1).eq.'*' .or. cinput(k1:k1).eq.'#') then
		  continue
	       elseif (cinput(k1:k2).eq.'end' .and. kv1.eq.0) then
c end
		  iend=+1
	       elseif (cinput(k1:k2).eq.
     +			'check.undef.in.first.field') then
c check.undef.in.first.field
		  if(iundef.ne.-1) iend=-4
		  iundef=1
	       elseif (cinput(k1:k2).eq.
     +			'check.undef.in.each.field') then
c check.undef.in.each.field
		  if(iundef.ne.-1) iend=-4
		  iundef=2
	       elseif (kv1.eq.0) then
c missing specifications behind '='
		  iend=-2
	       elseif (cinput(k1:k2).eq.'grid.pol') then
c grid.pol
		  numgrd=numgrd+1
		  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +					    (gspec(i,numgrd),i=1,4)
		  igspec(4,numgrd)=1
		   gspec(5,numgrd)=60.
		   gspec(6,numgrd)=0.
		  if (numgrd.gt.maxgrd) iend=-3
	       elseif (cinput(k1:k2).eq.'grid.geo') then
c grid.geo
		  numgrd=numgrd+1
		  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +					    (gspec(i,numgrd),i=1,4)
		  igspec(4,numgrd)=2
		   gspec(5,numgrd)=0.
		   gspec(6,numgrd)=0.
		  if (numgrd.gt.maxgrd) iend=-3
	       elseif (cinput(k1:k2).eq.'grid.sph') then
c grid.sph
		  numgrd=numgrd+1
		  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +					    (gspec(i,numgrd),i=1,6)
		  igspec(4,numgrd)=3
		  if (numgrd.gt.maxgrd) iend=-3
	       elseif (cinput(k1:k2).eq.'grid.pol.ext') then
c grid.pol.ext
		  numgrd=numgrd+1
		  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +					    (gspec(i,numgrd),i=1,5)
		  igspec(4,numgrd)=4
		   gspec(6,numgrd)=0.
		  if (numgrd.gt.maxgrd) iend=-3
	       elseif (cinput(k1:k2).eq.'grid.mer') then
c grid.mer
		  numgrd=numgrd+1
		  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +					    (gspec(i,numgrd),i=1,5)
		  igspec(4,numgrd)=5
		   gspec(6,numgrd)=0.
		  if (numgrd.gt.maxgrd) iend=-3
	       elseif (cinput(k1:k2).eq.'vector.param') then
c vector.param
		  i1=numvec+1
		  i2=numvec
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxvec)
		     i2=i2+1
		     read(cdata,*,iostat=ios)
     +				((iparvec(j,i),j=1,2),i=i1,i2)
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numvec=i2
		  if (numvec.gt.maxvec) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq.'t.and.td.to.rh.param') then
c t.and.td.to.rh.param
		  i1=numtdr+1
		  i2=numtdr
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxtdr)
		     i2=i2+1
		     read(cdata,*,iostat=ios)
     +			        ((itd2rh(j,i),j=1,3),i=i1,i2)
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numtdr=i2
		  if (numtdr.gt.maxtdr) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq.
     +		       'direction.true.north.param') then
c direction.true.north.param
		  i1=numdir+1
		  i2=numdir
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxdir)
		     i2=i2+1
		     read(cdata,*,iostat=ios) (ipardir(1,i),i=i1,i2)
		     ipardir(2,i2)=0
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numdir=i2
		  if (numdir.gt.maxdir) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq.
     +		       'turn.180.direction.true.north.param') then
c turn.180.direction.true.north.param
		  i1=numdir+1
		  i2=numdir
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxdir)
		     i2=i2+1
		     read(cdata,*,iostat=ios) (ipardir(1,i),i=i1,i2)
		     ipardir(2,i2)=180
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numdir=i2
		  if (numdir.gt.maxdir) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'switch.sign.param') then
c switch.sign.param
		  i1=numsgn+1
		  i2=numsgn
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxsgn)
		     i2=i2+1
		     read(cdata,*,iostat=ios) (iparsgn(i),i=i1,i2)
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numsgn=i2
		  if (numsgn.gt.maxsgn) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'param.smooth') then
c param.smooth
		  i1=numsmo+1
		  i2=numsmo
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxsmo)
		     i2=i2+1
		     read(cdata,*,iostat=ios)
     +				((iparsmo(j,i),j=1,2),i=i1,i2)
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numsmo=i2
		  if (numsmo.gt.maxsmo) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'param.min') then
c param.min
		  i1=numlim+1
		  i2=numlim
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxlim)
		     i2=i2+1
		     read(cdata,*,iostat=ios) (iparlim(1,i),
     +					        parlim(1,i),i=i1,i2)
		     iparlim(2,i2)=1
		      parlim(2,i2)=0.
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numlim=i2
		  if (numlim.gt.maxlim) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'param.max') then
c param.max
		  i1=numlim+1
		  i2=numlim
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxlim)
		     i2=i2+1
		     read(cdata,*,iostat=ios) (iparlim(1,i),
     +					        parlim(2,i),i=i1,i2)
		     iparlim(2,i2)=2
		      parlim(1,i2)=0.
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numlim=i2
		  if (numlim.gt.maxlim) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'param.min.max') then
c param.min.max
		  i1=numlim+1
		  i2=numlim
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxlim)
		     i2=i2+1
		     read(cdata,*,iostat=ios) (iparlim(1,i),parlim(1,i),
     +					        parlim(2,i),i=i1,i2)
		     iparlim(2,i2)=3
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numlim=i2
		  if (numlim.gt.maxlim) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'set.param.scale') then
c set.param.scale
		  i1=numscl+1
		  i2=numscl
		  ios=0
		  do while (ios.eq.0 .and. i2.le.maxscl)
		     i2=i2+1
		     read(cdata,*,iostat=ios)
     +				((iparscl(j,i),j=1,2),i=i1,i2)
		  end do
		  if(ios.ne.0) i2=i2-1
		  if(i2.lt.i1) iend=-2
		  numscl=i2
		  if (numscl.gt.maxscl) iend=-3
		  ios=0
	       elseif (cinput(k1:k2).eq. 'ocean.interp.field') then
c ocean.interp.field
		 read(cdata,*,iostat=ios) (iocean(i),i=1,6)
		 if(iocean(7).ne.0) iend=-4
		 iocean(7)=1
	       else
		  iend=-2
	       end if
c
c...........end do while (ikey.lt.nkey .and. ios.eq.0)
	    end do
c
	    if(ios.ne.0) iend=-2
c
	 end if
c
c.....end do while (iend.eq.0 .and. ios.eq.0)
      end do
c
      close(ludef)
c
      ierror=0
      if (iend.eq.-2) then
	 write(*,*) 'ERROR at line no. ',lindef,' in ',fildef
	 write(*,*) 'Bad input:'
	 write(*,*) cinput
	 ierror=1
      elseif (iend.eq.-3) then
	 write(*,*) 'ERROR at line no. ',lindef,' in ',fildef
	 write(*,*) 'Too many specifications:'
	 write(*,*) cinput
	 ierror=1
      elseif (iend.eq.-4) then
	 write(*,*) 'ERROR at line no. ',lindef,' in ',fildef
	 write(*,*) 'Option already set:'
	 write(*,*) cinput
	 ierror=1
      elseif (ios.ne.0) then
	 write(*,*) 'ERROR at line no. ',lindef,' in ',fildef
	 ierror=1
      end if
c
      if(ierror.eq.0) then
c check the grid definitions
         do n = 1,numgrd
	    if (igspec(4,n).eq.1 .or. igspec(4,n).eq.4) then
c grid.pol
c grid.pol.ext
	       if(gspec(3,n).le.   0. .or. 
     +	          gspec(4,n).lt.-327. .or. 
     +	          gspec(4,n).gt.+327. .or. 
     +	          gspec(5,n).lt. -90. .or. 
     +	          gspec(5,n).gt. +90.) then
		    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
		    ierror=1
	       end if
	    elseif (igspec(4,n).eq.2 .or. igspec(4,n).eq.3) then
c grid.geo
c grid.sph
	       if(gspec(1,n).lt.-327. .or. 
     +	          gspec(1,n).gt.+327. .or. 
     +	          gspec(2,n).lt. -90. .or. 
     +	          gspec(2,n).gt. +90. .or. 
     +	          gspec(3,n).eq.   0. .or. 
     +	          gspec(4,n).eq.   0. .or. 
     +	          gspec(5,n).lt.-327. .or. 
     +	          gspec(5,n).gt.+327. .or. 
     +	          gspec(6,n).lt. -90. .or. 
     +	          gspec(6,n).gt. +90.) then
		    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
		    ierror=1
	       end if
	    elseif (igspec(4,n).eq.5) then
c grid.mer
	       if(gspec(1,n).lt.-327. .or. 
     +	          gspec(1,n).gt.+327. .or. 
     +	          gspec(2,n).lt. -90. .or. 
     +	          gspec(2,n).gt. +90. .or. 
     +	          gspec(3,n).eq.   0. .or. 
     +	          gspec(4,n).eq.   0. .or. 
     +	          gspec(5,n).lt. -90. .or. 
     +	          gspec(5,n).gt. +90.) then
		    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
		    ierror=1
	       end if
	    else
	       write(*,*) 'PROGRAM ERROR: GRID SPEC in ''flt2flt.def'''
	       ierror=1
	    end if
         end do
      end if
c
      if (ierror.ne.0) then
	 istop=101
	 goto 9000
      end if
      fltdef = 1
      return
 9000 fltdef = -1
      return
      end
c******************************************************************
      integer function prepfelt(ldata,idata,lfield)
c
c  Prepare grid identification in idata (idata(9,15-18) and
c  eventually idata(20+lfield+1...)
c  ldata (dimension of idata) should be at least 20+(nx*ny)+18,
c  where nx,ny represents the field dimensions (taken from /fdef/ 
c  common).
c
c  lfield is set to the field size (nx*ny).
c
c  grid area is taken from idata(2) and is used to identify the
c  grid parameters stored in /fdef/ common.
c
      implicit none
c
c declarations
c
      integer ldata,lfield
      integer*2 idata(ldata)
      include 'nf_fdef.inc'
      integer i1,igix,igtype,ierror,nx,ny
      real grid(6)
c
c      write (*,*) '    prepfelt: Search for grid area in igspec:'
      do i1=1,numgrd
c         write (*,*) '              igspec(1,)=',igspec(1,i1)
         if (idata(2).eq.igspec(1,i1)) then
	    igix = i1
	    goto 10
	 endif
      end do
      write (*,*) "Grid area ",idata(2),
     +            " not found in grid definition file"
      prepfelt = -1
      return
   10 continue
      igtype = igspec(4,igix)
      do i1=1,6
         grid(i1)=gspec(i1,igix)
      end do
      nx = igspec(2,igix)
      ny = igspec(3,igix)
      lfield = nx*ny
c      write (*,*) '              igtype,nx,ny=',igtype,nx,ny
      call gridpar(-1,ldata,idata,igtype,nx,ny,grid,ierror)
      prepfelt = 1
      if (ierror.ne.0) then
         write (*,*) "Error return from gridpar, ierror=",ierror
	 prepfelt = -1
      endif
      return
      end
c
c************************************************************************
      subroutine vtime(itime,ierror)
c
c        'itime' is updated to give "verifying" date,time
c                                               (with prog.time = 0)
c        input:  itime(5) - itime(1): year
c                           itime(2): month (1-12)
c                           itime(3): day (1-28/29/30/31)
c                           itime(4): time in hours (00-23)
c                           itime(5): time in hours of prognosis
c                                     (negative, zero or positive)
c        output: itime(5) -  as above, itime(5)=0
c                ierror   -  0 = o.k. input date/time
c                            1 = not o.k. input date/time
c                                ('itime' not changed)
c
c-----------------------------------------------------------------------
c  DNMI/FoU  xx.xx.1992  Anstein Foss
c-----------------------------------------------------------------------
c
      integer itime(5)
c
      integer mdays(12)
      data mdays/31,28,31,30,31,30,31,31,30,31,30,31/
c
      iy=itime(1)
      im=itime(2)
      id=itime(3)
      ih=itime(4)
c
c..test input time
      ierror=0
      if(im.lt.1 .or. im.gt.12) then
        ierror=1
      else
        md=mdays(im)
        if(im.eq.2) then
          if(iy/4*4.eq.iy) md=29
          if(iy/100*100.eq.iy .and. iy/400*400.ne.iy) md=28
        endif
        if(id.lt.1 .or. id.gt.md) ierror=2
      endif
      if(ih.lt.00 .or. ih.gt.23) ierror=3
c
      if(ierror.ne.0) return
c
      ih=ih+itime(5)
      if(ih.ge.0 .and. ih.le.23) goto 50
      if(ih.lt.0) goto 30
c
      nd=ih/24
      ih=ih-24*nd
      do 20 n=1,nd
        id=id+1
        if(id.gt.md) then
          im=im+1
          if(im.gt.12) then
            iy=iy+1
            im=1
            md=mdays(im)
          elseif(im.eq.2) then
            md=mdays(im)
            if(iy/4*4.eq.iy) md=29
            if(iy/100*100.eq.iy .and. iy/400*400.ne.iy) md=28
          else
            md=mdays(im)
          endif
          id=1
        endif
   20 continue
      goto 50
c
   30 nd=(-ih+23)/24
      ih=ih+24*nd
      do 40 n=1,nd
        id=id-1
        if(id.lt.1) then
          im=im-1
          if(im.lt.1) then
            iy=iy-1
            im=12
            md=mdays(im)
          elseif(im.eq.2) then
            md=mdays(im)
            if(iy/4*4.eq.iy) md=29
            if(iy/100*100.eq.iy .and. iy/400*400.ne.iy) md=28
          else
            md=mdays(im)
          endif
          id=md
        endif
   40 continue
c
   50 itime(1)=iy
      itime(2)=im
      itime(3)=id
      itime(4)=ih
      itime(5)=0
c
      return
      end
C
C********************************************************************
      logical function isnaninf(r1)
C
C   0 0000
C   1 0001
C   2 0010
C   3 0011
C   4 0100
C   5 0101
C   6 0110
C   7 0111
C   8 1000
C   9 1001
C   A 1010
C   B 1011
C   C 1100
C   D 1101
C   E 1110
C   F 1111
C   
C   Single precision:
C
C   30          20           10           0
C   1098 7654 3210 9876 5432 1098 7654 3210
C   
C   0111 1111 1111 1111 1111 1111 1111 1111  7FFFFFFF 2147483647
C   0111 1111 1000 0000 0000 0000 0000 0000  7F800000 2139095040
C   1111 1111 1111 1111 1111 1111 1111 1111  FFFFFFFF         -1
C   1111 1111 1000 0000 0000 0000 0000 0000  FF800000   -8388608
C
      real r1,r2
      integer i2
      equivalence (r2,i2)
      r2 = r1
      isnaninf = .false.
      if (i2.ge.-8388608.and.i2.le.-1) isnaninf = .true.
      if (i2.ge.2139095040.and.i2.le.2147483647) isnaninf = .true.
      return
      end
C
C      logical function isdblnaninf(r1)
C
C   Double precision:
C
C     60           50          40           30          20           10           0
C   3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210 9876 5432 1098 7654 3210
C
C   0111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111
C   7FFFFFFF 2147483647                     FFFFFFFF         -1
C   7FFFFFFF FFFFFFFF 9223372036854775807
C
C   0111 1111 1111 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
C   7FF00000 2146435072                     00000000          0
C   7FF00000 00000000 9218868437227405312
C
C   1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111 1111
C   FFFFFFFF         -1                     FFFFFFFF         -1
C   FFFFFFFF FFFFFFFF -1
C
C   1111 1111 1111 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000 0000
C   FFF00000   -1048576                     00000000          0
C   FFF00000 00000000 -4503599627370496
C
C      double precision r1,r2
C      integer*8 i2
C      equivalence (r2,i2)
C      r2 = r1
C      isdblnaninf = .false.
C      if (i2.ge.-4503599627370496.and.i2.le.-1) isdblnaninf = .true.
C      if (i2.ge.9218868437227405312.and.i2.le.9223372036854775807)
C     +                                          isdblnaninf = .true.
C      return
C      end
