      program flt2flt
c
c******************************************************************
c
c flt2flt - felt to felt
c
c purpose:
c
c master program for interpolation between different grids;
c spherical, spherical rotated and polarstereographic.
c
c method:
c
c fields from the input felt file are interpolated and written
c to the output felt file. velocity components are turned in the
c output grid. the output file must have been created with nyfelt.
c unless command line options are used, all fields will be interpolated.
c the geometry of the input and output grids is taken from a
c separate input file; 'flt2flt.def'.
c
c if same input and output grid, this program will check the grid
c parameters and use the definitions in flt2flt.def for output.
c possibly extra geometry specifications wil be added
c
c -i5 option:
c requires the 'ocean.interp.field=' specified in flt2flt.def.
c if "ocean_file" given on command line the field will be read from
c this file, otherwise the field is read from the output file.
c (in "ocean.interp.field=" one may use -32767 as unspecified value,
c  i.e. the first field found matching other values).
c Direction fields (specified in flt2flt.def) is never interpolated,
c always using the nearest grid point. Other fields are interpolated
c where the 4 nearest input grid points exists, and otherwise just
c using the nearest existing grid point.
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
c grid.lam= <grid_no,nx,ny,long1,lat1,dxkm,dykm,xref,yref> .... grid type 6
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
c externals (dnmi library):
c
c mrfturbo - read from felt file (fast version of mrfelt)
c mwfturbo - write to felt file  (fast version of mwfelt)
c qfelt    - inquire felt file content
c extenf   - extension for hemispheric input fields
c grd2grd  - interpolation routine
c grv2grv  - turn velocity components
c gridpar  - set grid parameters in field identification
c
c history:
c
c  DNMI/FoU  xx.11.1994  J.E. Haugen
c  DNMI/FoU  02.02.1995  Anstein Foss ... arguments,qfelt,'turbo'
c  DNMI/FoU  14.02.1995  Anstein Foss ... no size limit in grd2grd,grv2grv
c                                         min/max check,direction_param,++
c  DNMI/FoU  10.06.1995  Anstein Foss
c  DNMI/FoU  29.09.1995  Anstein Foss ... -c option
c  DNMI/FoU  23.02.1996  Anstein Foss ... match with level_2=-32767
c  DNMI/FoU  29.05.1996  Anstein Foss ... new qfelt(iexist=2)
c  DNMI/FoU  11.06.1996  Anstein Foss ... mercator, new grd2grd,grv2grv
c  DNMI/FoU  13.09.1996  Anstein Foss ... mercator completed (?)
c  DNMI/FoU  21.10.1996  Anstein Foss ... smoothing ("param.smooth=")
c  DNMI/FoU  14.07.1997  J.E. Haugen .... merge grids (-m option)
c  DNMI/FoU  31.07.1997  Anstein Foss ... merge width on command line ++
c  DNMI/FoU  21.09.1998  Anstein Foss ... -i5 option (ocean interp., WAM)
c  DNMI/FoU  09.10.1998  Anstein Foss ... -y option
c  DNMI/FoU  25.02.2000  Anstein Foss ... polar.south corrected +++
c  DNMI/FoU  21.11.2000  Anstein Foss ... EC -x1,-x2 opt changed, + -x3,-x4
c  DNMI/FoU  12.03.2003  Anstein Foss ... -y2,-y3 options
c  DNMI/FoU  12.05.2004  Anstein Foss ... bug fix (turn.180.direction...)
c  DNMI/FoU  03.02.2005  Anstein Foss ... -xN not changing datatype 4
c  DNMI/FoU  10.06.2005  Anstein Foss ... float() -> real()
c met.no/FoU 02.12.2008  Ole Vignes ..... lambert (tangent) grids
c met.no/FoU 17.02.2009  Trygve Aspelien. enabled ensemble member selection
c met.no/FoU 19.02.2009  Ole Vignes ..... added -T option
c met.no/FoU 09.11.2009  Ole Vignes ..... -c and -T now works together
c******************************************************************
c
      implicit none
c
c declarations
c
      include 'flt2flt.inc'
c
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c include file for flt2flt.f
c
ccc   integer maxhor,maxpar,mgeom
ccc  +       ,maxgrd,maxvec,maxtdr,maxdir,maxsgn,maxlim,maxscl,maxsmo
c
ccc   parameter (maxhor=75000)
ccc   parameter (maxpar=10000)
ccc   parameter (mgeom =100)
c
ccc   parameter (maxgrd= 50)
ccc   parameter (maxvec= 30)
ccc   parameter (maxtdr= 10)
ccc   parameter (maxdir= 20)
ccc   parameter (maxsgn= 20)
ccc   parameter (maxlim=100)
ccc   parameter (maxscl= 20)
ccc   parameter (maxsmo=100)
ccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc
c
c declarations
c
      integer mdata
c
      parameter (mdata=20+maxhor)
c
      real fa(maxhor),va(maxhor),
     +     fr(maxhor),vr(maxhor),
     +     fx(maxhor),vx(maxhor),alpha(maxhor),
     +     rinter(16*maxhor),vturn(4*maxhor,1),
     +     ga(6),gr(6),g(6),gainp(6)
      integer   jinter(maxhor*4)
      integer   ifounda(maxpar),ifoundr(maxpar)
      integer*2 in(16),idata(mdata),ident(20),
     +          ina(16,maxpar),inr(16,maxpar),idata2(mdata),i2dum
      equivalence (idata( 1),ident(1))
      integer*2 identr(20),igeomr(mgeom),idfile(32)
      logical lveloc,lexten,swapfile
c
      integer numgrd,numvec,numtdr,numdir,numsgn,numlim,numscl,numsmo
      integer igspec(4,maxgrd+1),iparvec(2,maxvec+1),itd2rh(3,maxtdr+1),
     +        ipardir(2,maxdir+1),iparsgn(maxsgn+1),
     +        iparlim(2,maxlim+1),iparscl(2,maxscl+1),
     +        iparsmo(2,maxsmo+1),iocean(7)
      real    gspec(6,maxgrd+1),parlim(2,maxlim+1)
c
      integer istop,nfounda,nfoundr,
     +        ioerr,ios,lindef,k1,k2,kv1,kv2,i0,i1,i2,
     +        ipack,icall1,icall2,inter,iundef,itime,ndimri,
     +        luarg,lures,ludef,luocn,
     +        nxa,nya,nxr,nyr,nxainp,nyainp,
     +        ipara,iparr,npara,nparr,i,ifields,nx,ny,ipar1,
     +        intopt,nparlim,npardir,
     +        iuparam,ivparam,iscale,ntd2rh,iextrapol,
     +        icdate,jpara,imerge,nwidth,icall3,kmerge,iread5,
     +        nxwrap,nxext1,nxext2,nyext1,nyext2,imember,
     +        iyeara,imontha,idaya,ihoura,iyearr,imonthr,idayr,ihourr,
     +        iybase,ihelapa,ihelapr,ihdiff
      real    udef,zpir18,zdeg18,dd,ddturn,ddmin,xwrap
c
c..cmdarg................................................
      integer       nopt
      parameter    (nopt=17)
      character*1   copt(nopt)
      integer       iopt(nopt)
      integer       iopts(2,nopt)
      integer       margs
      parameter    (margs=4)
      integer       nargs
      character*256 cargs(margs)
      integer       mispec
      parameter    (mispec=300)
      integer       ispec(mispec)
      integer       mrspec
      parameter    (mrspec=1)
      real          rspec(mrspec)
      integer       mcspec
      parameter    (mcspec=1)
      character*1   cspec(mcspec)
      integer       ierror
      integer       nerror
c..cmdarg................................................
c
      integer inpos(nopt)
      integer iproda,iprodr,igridr,idatyp,ihour,ivcoor,iparam,ilevel,
     +        igrida,iexist,icdtyp,iundefx,iend,nispec,n,n1,n2,j,
     +              jgrida,jgridr,igood,igtypa,igtypr,lgeomr,igtyp
c
      character*256 filarg,filres,fildef,filocn
      character*1   tchar
c
      integer    maxkey
      parameter (maxkey=5)
      integer   mkey,nkey,ikey,kwhere(5,maxkey)
      character*256 cinput,cdata
c
      integer lenstr
c
      integer ifa(maxhor)
      equivalence (fa(1),ifa(1))
c                                                                        
c..cmdarg...............................................................
      data copt/'n','g','d','t','v','p','l','r','i','e','x','u','c','m',
     +          'y','E','T'/
      data iopt/ 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 1 , 0 , 1 , 0 , 0 , 1 ,
     +           1 , 1 , 0/
c................1...2...3...4...5...6...7...8...9..10..11..12..13..14..
c..cmdarg.......15......................................................
c
      data inpos/1,  2,  9, 10, 11, 12, 13,  0,  0 , 0 , 0 , 0 , 0 , 0 ,
     +           0,  0,  0/
c
      data udef /1.e+35/
      data ipack /2/
c
      istop=255
c
c..cmdarg................................................
c..get command line arguments
      call cmdarg(nopt,copt,iopt,iopts,margs,nargs,cargs,
     +            mispec,ispec,mrspec,rspec,mcspec,cspec,
     +                                     ierror,nerror)
c..cmdarg................................................
c
      if(ierror.eq.0 .and. nargs.ne.3 .and. nargs.ne.4) ierror=-1
c
      if (ierror.eq.0 .and. iopts(1,9).gt.0) then
c..-iN option
        if(iopts(1,9).ne.1) ierror=-1
        if(ispec(iopts(2,9)).lt.1 .or.
     +     ispec(iopts(2,9)).gt.5) ierror=-1
      end if
      if (ierror.eq.0 .and. iopts(1,11).gt.0) then
c..-xN option
        if(iopts(1,11).ne.1) ierror=-1
        if(ispec(iopts(2,11)).lt.1 .or.
     +     ispec(iopts(2,11)).gt.4) ierror=-1
      end if
      if (ierror.eq.0 .and. iopts(1,15).gt.0) then
c..-yN option
        if(iopts(1,15).ne.1) ierror=-1
        if(ispec(iopts(2,15)).lt.1 .or.
     +     ispec(iopts(2,15)).gt.3) ierror=-1
      end if
c
      if(ierror.ne.0) then
        write(*,*)
        write(*,*) ' usage:  flt2flt file_in file_out flt2flt.def',
     +                   ' [ocean_file] [options]'
        write(*,*)
        write(*,*) '   file_in     : input  felt file'
        write(*,*) '   file_out    : output felt file'
        write(*,*) '   flt2flt.def : definition file (grid etc.)'
        write(*,*) '   ocean_file  : used with -i5'
        write(*,*)
        write(*,*) '   options:'
        write(*,*) '     -n <producer_no>    : producer (1-99)'
        write(*,*) '     -r <input_grid_no>  : input grid no.'
        write(*,*) '     -g <output_grid_no> : output grid no.'
        write(*,*) '     -d <data_type>      : data type'
        write(*,*) '     -t <forecast_hour>  : forecast time'
        write(*,*) '     -v <v.coord._no>    : vertical coordinate'
        write(*,*) '     -p <parameter_no>   : parameter no.'
        write(*,*) '     -E <ens. memb.>     : ensemble member (lev 2)'
        write(*,*) '     -l <level>          : level (no.)'
        write(*,*) '     -i <interpolation>  : interpolation type:'
        write(*,*) '                             1 = bilinear'
        write(*,*) '                             2 = biquadratic'
        write(*,*) '                             3 = bicubic (default)'
        write(*,*) '                             4 = nearest gridpoint'
        write(*,*) '                             5 = bilinear or',
     +                                             ' nearest gridpoint'
        write(*,*) '                                 (ocean field',
     +                                             ' interpolation)'
        write(*,*) '     -y1                 : flat extrapolation',
     +                                             ' to undef.'
        write(*,*) '                           pos. after interpolation'
        write(*,*) '     -y2                 : gradient extrapolation'
        write(*,*) '     -y3                 : Laplace equation',
     +                                         ' extrapolation'
        write(*,*) '     -e                  : also when output exists'
c001122 write(*,*) '     -x1                 : datatypes in=1,2 out=3,3'
c001122 write(*,*) '     -x2                 : datatypes in=1,2 out=3,2'
        write(*,*) '     -x1                 : datatypes in=X,2 out=3,3'
        write(*,*) '     -x2                 : datatypes in=X,2 out=3,2'
        write(*,*) '     -x3                 : datatypes in=X,2 out=1,3'
        write(*,*) '     -x4                 : datatypes in=X,2 out=1,2'
        write(*,*) '                           (X=one found, 1,2,3,...',
     +                                         ' NOT 4)'
        write(*,*) '     -u                  : test undef in each field'
        write(*,*) '     -c                  : disable date/time and'
        write(*,*) '                           producer check'
        write(*,*) '     -m <merge_width>    : merge with fields in'
        write(*,*) '                           output file (=> -e)'
        write(*,*) '     -T                  : adjust forecast times on'
     +                                        ,' output file'
        write(*,*) '   (e.g. -t24 -v 1 -p1,2,3 -l 500,1000)'
        write(*,*)
        stop
      end if
c
      iproda=-32767
      igridr=-32767
      idatyp=-32767
      ihour =-32767
      ivcoor=-32767
      iparam=-32767
      ilevel=-32767
      igrida=-32767
      imember=-32767
      inter =3
      iexist=2
      icdtyp=0
      iundefx=-1
      icdate=1
      imerge=0
      nwidth=0
      iextrapol=0
c
      if (iopts(1, 1).eq.1) iproda=ispec(iopts(2,1))
      if (iopts(1, 2).eq.1) igridr=ispec(iopts(2,2))
      if (iopts(1, 3).eq.1) idatyp=ispec(iopts(2,3))
      if (iopts(1, 4).eq.1) ihour =ispec(iopts(2,4))
      if (iopts(1, 5).eq.1) ivcoor=ispec(iopts(2,5))
      if (iopts(1, 6).eq.1) iparam=ispec(iopts(2,6))
      if (iopts(1, 7).eq.1) ilevel=ispec(iopts(2,7))
      if (iopts(1, 8).eq.1) igrida=ispec(iopts(2,8))
      if (iopts(1, 9).gt.0) inter =ispec(iopts(2,9))
      if (iopts(1,10).gt.0) iexist=0
      if (iopts(1,11).gt.0) icdtyp=ispec(iopts(2,11))
      if (iopts(1,12).gt.0) iundefx=2
      if (iopts(1,13).gt.0) icdate=0
      if (iopts(1,14).gt.0) then
         imerge=1
         nwidth=ispec(iopts(2,14))
         nwidth=max(nwidth,1)
         iexist=0
      end if
      if (iopts(1,15).gt.0) then
         iextrapol=ispec(iopts(2,15))
         iextrapol=min(max(iextrapol,0),3)
      end if
      if (iopts(1,16).eq.1) imember=ispec(iopts(2,16))
c
      iprodr=iproda
      if(icdate.eq.0) iprodr=-32767
      if (iopts(1,17).gt.0) icdate=2
c
      nispec=0
      do n=1,nopt
        if(iopt(n).eq.1 .and. iopts(1,n).gt.0)
     +           nispec=max(nispec,iopts(2,n)+iopts(1,n)-1)
      end do
c
      filarg=cargs(1)
      filres=cargs(2)
      fildef=cargs(3)
      if(nargs.eq.4) then
        filocn=cargs(4)
      else
        filocn='*'
      end if
c
      write(*,*) 'flt2flt:'
      write(*,*) 'input file:'
      write(*,*) filarg(1:lenstr(filarg,1))
      write(*,*) 'output file:'
      write(*,*) filres(1:lenstr(filres,1))
c
c filarg - input  felt file name
c filres - output felt file name
c fildef - grid and parameter definitions file name
c luarg  - input  felt file unit
c lures  - output felt file unit
c ludef  - grid and parameter definitions file unit
c inter  - interpolation type
c
      ludef=10
      luarg=20
      lures=30
      luocn=40
c
c......................................................................
c read grid and parameter definitions file
c
c open file
      open(unit=ludef,file=fildef,
     +     form='formatted',access='sequential',
     +     status='old',iostat=ios)
      if (ios.ne.0) then
         write(*,*) 'Open ERROR: ',fildef(1:lenstr(fildef,1))
         istop=100
         goto 9000
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
c grid.lam= <grid_no,nx,ny,long1,lat1,dxkm,dykm,xref,yref> .... grid type 6
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
     +                        'check.undef.in.first.field') then
c check.undef.in.first.field
                  if(iundef.ne.-1) iend=-4
                  iundef=1
               elseif (cinput(k1:k2).eq.
     +                        'check.undef.in.each.field') then
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
     +                                     (gspec(i,numgrd),i=1,4)
                  igspec(4,numgrd)=1
                   gspec(5,numgrd)=60.
                   gspec(6,numgrd)=0.
                  if (numgrd.gt.maxgrd) iend=-3
               elseif (cinput(k1:k2).eq.'grid.geo') then
c grid.geo
                  numgrd=numgrd+1
                  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +                                     (gspec(i,numgrd),i=1,4)
                  igspec(4,numgrd)=2
                   gspec(5,numgrd)=0.
                   gspec(6,numgrd)=0.
                  if (numgrd.gt.maxgrd) iend=-3
               elseif (cinput(k1:k2).eq.'grid.sph') then
c grid.sph
                  numgrd=numgrd+1
                  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +                                     (gspec(i,numgrd),i=1,6)
                  igspec(4,numgrd)=3
                  if (numgrd.gt.maxgrd) iend=-3
               elseif (cinput(k1:k2).eq.'grid.pol.ext') then
c grid.pol.ext
                  numgrd=numgrd+1
                  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +                                     (gspec(i,numgrd),i=1,5)
                  igspec(4,numgrd)=4
                   gspec(6,numgrd)=0.
                  if (numgrd.gt.maxgrd) iend=-3
               elseif (cinput(k1:k2).eq.'grid.mer') then
c grid.mer
                  numgrd=numgrd+1
                  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +                                     (gspec(i,numgrd),i=1,5)
                  igspec(4,numgrd)=5
                   gspec(6,numgrd)=0.
                  if (numgrd.gt.maxgrd) iend=-3
               elseif (cinput(k1:k2).eq.'grid.lam') then
c grid.lam
                  numgrd=numgrd+1
                  read(cdata,*,iostat=ios) (igspec(i,numgrd),i=1,3),
     +                                            (g(i),i=1,6)
                  igspec(4,numgrd)=6
                  nxa=igspec(2,numgrd)
                  nya=igspec(3,numgrd)
                  call lamc2ws(g(1),g(2),nxa,nya,g(3),g(4),g(5),g(6),
     +                         gspec(1,numgrd),gspec(2,numgrd),i)
                  if (i.ne.0) iend=-2
                  do i=3,6
                     gspec(i,numgrd) = g(i)
                  enddo
                  if (numgrd.gt.maxgrd) iend=-3
               elseif (cinput(k1:k2).eq.'vector.param') then
c vector.param
                  i1=numvec+1
                  i2=numvec
                  ios=0
                  do while (ios.eq.0 .and. i2.le.maxvec)
                     i2=i2+1
                     read(cdata,*,iostat=ios)
     +                                ((iparvec(j,i),j=1,2),i=i1,i2)
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
     +                                ((itd2rh(j,i),j=1,3),i=i1,i2)
                  end do
                  if(ios.ne.0) i2=i2-1
                  if(i2.lt.i1) iend=-2
                  numtdr=i2
                  if (numtdr.gt.maxtdr) iend=-3
                  ios=0
               elseif (cinput(k1:k2).eq.
     +                       'direction.true.north.param') then
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
     +                       'turn.180.direction.true.north.param') then
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
     +                                ((iparsmo(j,i),j=1,2),i=i1,i2)
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
     +                                          parlim(1,i),i=i1,i2)
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
     +                                          parlim(2,i),i=i1,i2)
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
     +                                          parlim(2,i),i=i1,i2)
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
     +                                ((iparscl(j,i),j=1,2),i=i1,i2)
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
         write(*,*) 'ERROR at line no. ',lindef,' in ',
     +                                fildef(1:lenstr(fildef,1))
         write(*,*) 'Bad input:'
         write(*,*) cinput(1:lenstr(cinput,1))
         ierror=1
      elseif (iend.eq.-3) then
         write(*,*) 'ERROR at line no. ',lindef,' in ',
     +                                fildef(1:lenstr(fildef,1))
         write(*,*) 'Too many specifications:'
         write(*,*) cinput(1:lenstr(cinput,1))
         ierror=1
      elseif (iend.eq.-4) then
         write(*,*) 'ERROR at line no. ',lindef,' in ',
     +                                fildef(1:lenstr(fildef,1))
         write(*,*) 'Option already set:'
         write(*,*) cinput(1:lenstr(cinput,1))
         ierror=1
      elseif (ios.ne.0) then
         write(*,*) 'ERROR at line no. ',lindef,' in ',
     +                                fildef(1:lenstr(fildef,1))
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
     +                  gspec(4,n).lt.-327. .or. 
     +                  gspec(4,n).gt.+327. .or. 
     +                  gspec(5,n).lt. -90. .or. 
     +                  gspec(5,n).gt. +90.) then
                    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
                    ierror=1
               end if
            elseif (igspec(4,n).eq.2 .or. igspec(4,n).eq.3) then
c grid.geo
c grid.sph
               if(gspec(1,n).lt.-327. .or. 
     +                  gspec(1,n).gt.+327. .or. 
     +                  gspec(2,n).lt. -90. .or. 
     +                  gspec(2,n).gt. +90. .or. 
     +                  gspec(3,n).eq.   0. .or. 
     +                  gspec(4,n).eq.   0. .or. 
     +                  gspec(5,n).lt.-327. .or. 
     +                  gspec(5,n).gt.+327. .or. 
     +                  gspec(6,n).lt. -90. .or. 
     +                  gspec(6,n).gt. +90.) then
                    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
                    ierror=1
               end if
            elseif (igspec(4,n).eq.5) then
c grid.mer
               if(gspec(1,n).lt.-327. .or. 
     +                  gspec(1,n).gt.+327. .or. 
     +                  gspec(2,n).lt. -90. .or. 
     +                  gspec(2,n).gt. +90. .or. 
     +                  gspec(3,n).eq.   0. .or. 
     +                  gspec(4,n).eq.   0. .or. 
     +                  gspec(5,n).lt. -90. .or. 
     +                  gspec(5,n).gt. +90.) then
                    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
                    ierror=1
               end if
            elseif (igspec(4,n).eq.6) then
c grid.lam
               if(gspec(1,n).lt.-327. .or. 
     +                  gspec(1,n).gt.+327. .or. 
     +                  gspec(2,n).lt. -90. .or. 
     +                  gspec(2,n).gt. +90. .or. 
     +                  gspec(3,n).eq.   0. .or. 
     +                  gspec(4,n).eq.   0. .or. 
     +                  gspec(5,n).lt.-180. .or. 
     +                  gspec(5,n).gt.+180. .or.
     +                  gspec(6,n).lt. -90. .or. 
     +                  gspec(6,n).gt. +90.) then
                    write(*,*) 'Grid definition ERROR: ',igspec(1,n)
                    ierror=1
               end if
            else
               write(*,*) 'PROGRAM ERROR: GRID SPEC in ',
     +                                fildef(1:lenstr(fildef,1))
               ierror=1
            end if
         end do
         if (inter.eq.5 .and. iocean(7).eq.0) then
            write(*,*) 'ERROR: command line option -i5 but no'
            write(6,*) '       "ocean.interp.field" in ',
     +                                fildef(1:lenstr(fildef,1))
            ierror=1
         end if
      end if
c
      if (ierror.ne.0) then
         istop=101
         goto 9000
      end if
c
      if (iundef.lt.1 .or. iundef.gt.2) iundef=1
c..possibly command line argument
      if (iundefx.eq.2) iundef=2
c......................................................................
c
c open input/output felt files
      call mrfturbo(1,filarg,luarg,in(1),ipack,1,1.,1.,1,i2dum,ierror)
      if (ierror.ne.0) then
         istop=1
         goto 9000
      end if
      call mwfturbo(1,filres,lures,ipack,1,1.,1.,1,i2dum,ierror)
      if (ierror.ne.0) then
         istop=2
         goto 9000
      end if
c open output felt file for read operations if merging data
      if (imerge.eq.1) then
        call mrfturbo(1,filres,lures,in(1),ipack,1,1.,1.,1,i2dum,ierror)
         if (ierror.ne.0) then
            istop=2
            goto 9000
         end if
      end if
      if(icdate.eq.2) then
c get base date/time of input file
         read(luarg,rec=1,iostat=istop) (idfile(i),i=1,32)
         if (istop.ne.0) goto 8000
         if(swapfile(-luarg)) call bswap2(32,idfile)
         iyeara = idfile(5)
         imontha = idfile(6)/100
         idaya = idfile(6)-(idfile(6)/100)*100
         ihoura = idfile(7)/100
c get base date/time of output file
         read(lures,rec=1,iostat=istop) (idfile(i),i=1,32)
         if (istop.ne.0) goto 8000
         if(swapfile(-lures)) call bswap2(32,idfile)
         iyearr = idfile(5)
         imonthr = idfile(6)/100
         idayr = idfile(6)-(idfile(6)/100)*100
         ihourr = idfile(7)/100
c compute difference in hours
         iybase = min(iyeara,iyearr)
         call hrelap(iybase,iyeara,imontha,idaya,ihoura,ihelapa)
         call hrelap(iybase,iyearr,imonthr,idayr,ihourr,ihelapr)
         ihdiff = ihelapr - ihelapa
      endif
c
c inquire input file content (with existing data)
      do i=1,16
        ina(i,1)=-32767
      end do
      ina( 1,1)=iproda
      ina( 2,1)=igrida
      ina( 9,1)=idatyp
      ina(10,1)=ihour
      ina(11,1)=ivcoor
      ina(12,1)=iparam
      ina(13,1)=ilevel
      call qfelt(0,0,1,maxpar,ina,ifounda,nfounda,
     +           iend,ierror,ioerr)
      call qfelt(luarg,1,1,maxpar,ina,ifounda,nfounda,
     +           iend,ierror,ioerr)
      if(ierror.ne.0) then
        write(*,*) 'qfelt error (input).  ierror,ioerr: ',ierror,ioerr
      end if
      if(iend.ne.1) then
        write(*,*) 'maxpar too small (input).  increase maxpar.'
        goto 9000
      end if
      if(nfounda.lt.1) then
        write(*,*) 'no existing input fields found'
        istop=100
        goto 9000
      end if
c
c inquire output file content
      do i=1,16
        inr(i,1)=-32767
      end do
      inr( 1,1)=iprodr
      inr( 2,1)=igridr
      inr( 9,1)=idatyp
      inr(10,1)=ihour
      if(icdate.eq.2.and.ihour.ge.0) inr(10,1)=ihour-ihdiff
      inr(11,1)=ivcoor
      inr(12,1)=iparam
      inr(13,1)=ilevel
      call qfelt(0,0,iexist,maxpar,inr,ifoundr,nfoundr,
     +           iend,ierror,ioerr)
      call qfelt(lures,1,iexist,maxpar,inr,ifoundr,nfoundr,
     +           iend,ierror,ioerr)
      if(ierror.ne.0) then
        write(*,*) 'qfelt error (output).  ierror,ioerr: ',ierror,ioerr
      end if
      if(iend.ne.1) then
        write(*,*) 'maxpar too small (output).  increase maxpar.'
        goto 9000
      end if
      if(nfoundr.lt.1) then
        write(*,*) 'no output fields found'
        istop=0
        goto 9000
      end if
c
      if(iexist.eq.0) then
c mark all output fields as not existing
        do i=1,nfoundr
          ifoundr(i)=0
        end do
      end if
c
c mark fields not to be interpolated as existing
c (qfelt may have sorted out some already)
      do n=1,7
        if(iopts(1,n).gt.1) then
          n1=iopts(2,n)
          n2=iopts(2,n)+iopts(1,n)-1
          do i=1,nfoundr
            if(ifoundr(i).lt.1) then
              igood=0
              do j=n1,n2
                if(icdate.eq.2.and.n.eq.4) then
                   if(inr(10,i).eq.ispec(j)-ihdiff) igood=1
                else
                   if(inr(inpos(n),i).eq.ispec(j)) igood=1
                endif
              end do
              if(igood.eq.0) ifoundr(i)=1
            end if
          end do
        end if
      end do
c
c datatype change between input and output
      if(icdtyp.eq.1) then
        do i=1,nfoundr
          if(inr(9,i).eq.3) then
            if(inr(10,i).le.0) then
c001122              inr(9,i)=1
              inr(9,i)=-32767
            else
              inr(9,i)=2
            end if
          end if
        end do
      elseif(icdtyp.eq.2) then
        do i=1,nfoundr
c001122          if(inr(9,i).eq.3 .and. inr(10,i).le.0) inr(9,i)=1
          if(inr(9,i).eq.3 .and. inr(10,i).le.0) inr(9,i)=-32767
        end do
      elseif(icdtyp.eq.3) then
        do i=1,nfoundr
          if(inr(9,i).eq.1) then
            if(inr(10,i).le.0) then
              inr(9,i)=-32767
            else
              inr(9,i)=2
            end if
          end if
        end do
      elseif(icdtyp.eq.4) then
        do i=1,nfoundr
          if(inr(9,i).eq.1 .and. inr(10,i).le.0) inr(9,i)=-32767
        end do
      end if
c
c find all input grids if not specified
      n=8
      if(iopts(1,n).eq.0) then
        jgrida=-32767
        n1=nispec+1
        do i=1,nfounda
          if(ina(2,i).ne.jgrida) then
            jgrida=ina(2,i)
            igood=0
            do j=n1,nispec
              if(jgrida.eq.ispec(j)) igood=1
            end do
            if(igood.eq.0) then
              if(nispec.eq.mispec) then
                write(*,*) 'mispec too small.  increase mispec.'
                goto 9000
              end if
              nispec=nispec+1
              if(iopts(1,n).eq.0) iopts(2,n)=nispec
              iopts(1,n)=iopts(1,n)+1
              ispec(nispec)=jgrida
            end if
          end if
        end do
      end if
c
c find all output grids if not specified
      n=2
      if(iopts(1,n).eq.0) then
        jgridr=-32767
        n1=nispec+1
        do i=1,nfoundr
          if(inr(2,i).ne.jgridr .and. ifoundr(i).lt.1) then
            jgridr=inr(2,i)
            igood=0
            do j=n1,nispec
              if(jgridr.eq.ispec(j)) igood=1
            end do
            if(igood.eq.0) then
              if(nispec.eq.mispec) then
                write(*,*) 'mispec too small.  increase mispec.'
                goto 9000
              end if
              nispec=nispec+1
              if(iopts(1,n).eq.0) iopts(2,n)=nispec
              iopts(1,n)=iopts(1,n)+1
              ispec(nispec)=jgridr
            end if
          end if
        end do
      end if
c
      npara=nfounda
      nparr=nfoundr
c
      ndimri = 1
      if (inter.eq.1) ndimri =  4
      if (inter.eq.2) ndimri =  9
      if (inter.eq.3) ndimri = 16
      if (inter.eq.5) ndimri =  6
c
c output grid loop
c
      do jgridr=iopts(2,2),iopts(2,2)+iopts(1,2)-1
c
      igridr=ispec(jgridr)
c
c find grid definition for igridr
      n=0
      do i=1,numgrd
        if (igspec(1,i).eq.igridr) n=i
      end do
      if(n.eq.0) then
        write(*,*) 'Missing grid definition for output grid ',igridr
        istop=101
        goto 8000
      end if
      nxr=igspec(2,n)
      nyr=igspec(3,n)
      igtypr=igspec(4,n)
      do i=1,6
        gr(i)=gspec(i,n)
      end do
      if(nxr*nyr.gt.maxhor) then
        write(*,*) 'Too large output grid: ',igridr
        write(6,*) '   size,max: ',nxr*nyr,maxhor
        istop=101
        goto 3000
      end if
c
c prepare part of output field identification
      do i=1,20
        idata(i)=0
      end do
      call gridpar(-1,mdata,idata,igtypr,nxr,nyr,gr,ierror)
      if(ierror.ne.0) then
        write(*,*) 'GRIDPAR ERROR (OUTPUT): ',ierror
        goto 8000
      end if
      lgeomr=0
      if(idata(9).gt.999) lgeomr=idata(9)-(idata(9)/1000)*1000
      if(lgeomr.gt.mgeom) then
        write(*,*) 'flt2flt: increase mgeom'
        write(*,*) 'lgeomr = ',lgeomr
        write(*,*) 'mgeom  = ',mgeom
        goto 8000
      end if
      do j=1,20
        identr(j)=idata(j)
      end do
      do i=1,lgeomr
        igeomr(i)=idata(20+nxr*nyr+i)
      end do
c
c input grid loop
c
      do jgrida=iopts(2,8),iopts(2,8)+iopts(1,8)-1
c
      igrida=ispec(jgrida)
c
c find grid definition for igrida
      n=0
      do i=1,numgrd
        if (igspec(1,i).eq.igrida) n=i
      end do
      if(n.eq.0) then
        write(*,*) 'Missing grid definition for input grid ',igrida
        istop=101
        goto 8000
      end if
      nxa=igspec(2,n)
      nya=igspec(3,n)
      igtypa=igspec(4,n)
      do i=1,6
        ga(i)=gspec(i,n)
      end do
      if(nxa*nya.gt.maxhor) then
        write(*,*) 'Too large input grid: ',igrida
        write(6,*) '   size,max: ',nxa*nya,maxhor
        istop=101
        goto 2000
      end if
c
      ipar1  =0
      icall1 =2
      icall2 =2
      icall3 =2
      lexten =.false.
      itime  =-32767
      ifields=0
      kmerge =0
      iread5 =0
c
c store input values for test
      nxainp=nxa
      nyainp=nya
      do i=1,6
        gainp(i)=ga(i)
      end do
c
c check if extension of input field for hemispheric data
      if ((igtypa.eq.2 .or. igtypa.eq.3) .and. igrida.ne.igridr) then
         xwrap = 360./ga(3)
         nxwrap = nint(xwrap)
         if (abs(xwrap-real(nxwrap)).gt.0.1
     +                          .or. nxa.lt.nxwrap) nxwrap=0
c..handle grids that starts max half a gridunit away from the poles
c..("box" average values etc.)
         nyext1=0
         if(abs(ga(2)-ga(4)*0.55).gt.90.) nyext1=3
         nyext2=0
         if(abs(ga(2)+ga(4)*(real(nya-1)+0.55)).gt.90.) nyext2=3
         nxext1=0
         nxext2=0
          if (nxwrap.gt.0 .or. nyext1.gt.0 .or. nyext2.gt.0) then
            lexten = .true.
            if (nxwrap.gt.0) then
                nxext1=3
                nxext2=3
                nxa = nxa+nxext1+nxext2
                ga(1) = ga(1) - real(nxext1)*ga(3)
            end if
            if (nyext1.gt.0) then
               nya = nya+nyext1
               ga(2) = ga(2) - real(nyext1)*ga(4)
            end if
            if (nyext2.gt.0) nya = nya+nyext2
            if (nxa*nya.gt.maxhor) then
               write(*,*) 'flt2flt: increase maxhor'
               write(*,*) 'nxa*nya = ',nxa*nya
               write(*,*) 'maxhor = ',maxhor
               goto 8000
            end if
          end if
      end if
c
c print definition of grids
      if (icall1.eq.2) then
         write(*,*) 'input grid, igrida = ',igrida
         write(*,*) 'input grid type, igtypa = ',igtypa
         write(*,'(1x,''nxa,nya = '',2i10)') nxainp,nyainp
         write(*,'(1x,''ga = '',6f10.2)') (gainp(i),i=1,6)
         write(*,*) 'output grid, igridr = ',igridr
         write(*,*) 'output grid type, igtypr = ',igtypr
         write(*,'(1x,''nxr,nyr = '',2i10)') nxr,nyr
         write(*,'(1x,''gr = '',6f10.2)') (gr(i),i=1,6)
      end if
c
c main loop over output fields
c
      do iparr=1,nparr
c
c check output grid
      if (inr(2,iparr).ne.igridr) goto 1000
c check if the field already has been written
      if (ifoundr(iparr).gt.0) goto 1000
c
c vector/velocity components; assume that u was asked before v
      n=0
      do i=1,numvec
        if (iparvec(2,i).eq.inr(12,iparr)) n=i
      end do
      if (n.gt.0 .and. igrida.ne.igridr) goto 1000
c
c t and td to rh (from temp. and dew point temp. to relative humidity)
      ntd2rh=0
      do n=1,numtdr
        if (itd2rh(3,i).eq.inr(12,iparr)) ntd2rh=n
      end do
      if (ntd2rh.gt.0) inr(12,iparr)=itd2rh(2,ntd2rh)
c
c check if the field is present in the input file
      jpara=0
      ipara=0
      if(icdate.eq.1) then
        do while (jpara.eq.0 .and. ipara.lt.npara)
          ipara=ipara+1
          if (ina( 2,ipara).eq.igrida        .and.
     +        ina( 1,ipara).eq.inr( 1,iparr) .and.
     +        ina( 3,ipara).eq.inr( 3,iparr) .and.
     +        ina( 4,ipara).eq.inr( 4,iparr) .and.
     +        ina( 5,ipara).eq.inr( 5,iparr) .and.
c001122+      ina( 9,ipara).eq.inr( 9,iparr) .and.
     +        (ina( 9,ipara).eq.inr( 9,iparr)
     +                           .or. inr( 9,iparr).eq.-32767) .and.
     +        ina(10,ipara).eq.inr(10,iparr) .and.
     +        ina(11,ipara).eq.inr(11,iparr) .and.
     +        ina(12,ipara).eq.inr(12,iparr) .and.
     +        ina(13,ipara).eq.inr(13,iparr)) then
              if (imember.ne.-32767) then
                if ((ina(14,ipara).eq.inr(14,iparr)
     +                     .and. inr(14,iparr).eq.imember)) then
                  jpara=ipara
                endif
              elseif((ina(14,ipara).eq.inr(14,iparr)
     +                           .or. inr(14,iparr).eq.-32767)) then
                jpara=ipara
            endif
          endif
        end do
      elseif(icdate.eq.0) then
        do while (jpara.eq.0 .and. ipara.lt.npara)
          ipara=ipara+1
          if (ina( 2,ipara).eq.igrida        .and.
c001122+      ina( 9,ipara).eq.inr( 9,iparr) .and.
     +        (ina( 9,ipara).eq.inr( 9,iparr)
     +                           .or. inr( 9,iparr).eq.-32767) .and.
     +        ina(10,ipara).eq.inr(10,iparr) .and.
     +        ina(11,ipara).eq.inr(11,iparr) .and.
     +        ina(12,ipara).eq.inr(12,iparr) .and.
     +        ina(13,ipara).eq.inr(13,iparr)) then
              if (imember.ne.-32767) then
                if ((ina(14,ipara).eq.inr(14,iparr)
     +                     .and. inr(14,iparr).eq.imember)) then
                  jpara=ipara
                endif
              elseif((ina(14,ipara).eq.inr(14,iparr)
     +                     .or. inr(14,iparr).eq.-32767)) then
                jpara=ipara
            endif
          endif
        end do
      elseif(icdate.eq.2) then
        do while (jpara.eq.0 .and. ipara.lt.npara)
          ipara=ipara+1
          if (ina( 2,ipara).eq.igrida        .and.
     +        ina( 1,ipara).eq.inr( 1,iparr) .and.
c     +        (ina(9,ipara).eq.inr( 9,iparr)
c     +                           .or.inr( 9,iparr).eq.-32767) .and.
     +        ina(10,ipara).eq.inr(10,iparr)+ihdiff .and.
     +        ina(11,ipara).eq.inr(11,iparr) .and.
     +        ina(12,ipara).eq.inr(12,iparr) .and.
     +        ina(13,ipara).eq.inr(13,iparr)) then
              if (imember.ne.-32767) then
                if ((ina(14,ipara).eq.inr(14,iparr)
     +                     .and. inr(14,iparr).eq.imember)) then
                  jpara=ipara
                endif
              elseif((ina(14,ipara).eq.inr(14,iparr)
     +                     .or. inr(14,iparr).eq.-32767)) then
                jpara=ipara
            endif
          endif
        end do
      end if
      if(jpara.eq.0) goto 1000
c
c for interpolation type 5 (-i5, ocean interpolation, WAM)
      if (inter.eq.5 .and. iread5.eq.0) then
         do i=1,16
            in(i)=-32767
         end do
         in( 1)=inr(1,iparr)
         in( 2)=inr(2,iparr)
         in( 9)=iocean(1)
         in(10)=iocean(2)
         in(11)=iocean(3)
         in(12)=iocean(4)
         in(13)=iocean(5)
         in(14)=iocean(6)
         if(filocn(1:1).ne.'*') then
c read ocean definition field
            call mrfturbo(0,filocn,luocn,in(1),ipack,maxhor,fr(1),1.,
     +                    mdata,idata(1),ierror)
         else
            call mrfturbo(2,filres,lures,in(1),ipack,maxhor,fr(1),1.,
     +                    mdata,idata(1),ierror)
         end if
         if (ierror.ne.0) then
            write(*,*) 'Ocean definition field not found'
            istop=99
            goto 9000
         end if
c just a simple test (not of all grid parameters)
         if (idata(10).ne.nxr .or. idata(11).ne.nyr) then
            write(*,*) 'Bad size of ocean definition field'
            istop=99
            goto 9000
         end if
         i0=nxr*nyr*3
         do i=1,nxr*nyr
            if (fr(i).ne.udef) then
               jinter(i0+i)=1
            else
               jinter(i0+i)=0
            end if
         end do
         iread5=1
      end if
c
c prepare for input
      do i=1,16
         in(i)=ina(i,jpara)
      end do
c
c print one line for each output time
      if (itime.ne.in(10)) then
         if (ifields.gt.0)
     +      write(*,'(1x,''date:'',3i5.4,'' utc '',sp,i4,ss,'' : '',i6,
     +                '' fields  (grid in:'',i5,''  out:'',i5,'')'')')
     +         in(3),in(4),in(5),itime,ifields,igrida,igridr
         itime=in(10)
         ifields=0
      end if
c
c read input field
      call mrfturbo(2,filarg,luarg,in(1),ipack,maxhor,fa(1),1.,
     +              mdata,idata(1),ierror)
      if (ierror.ne.0) goto 1000
      ifields=ifields+1
c
c check geometry for input field
      if (ipar1.eq.0) then
         call gridpar(+1,mdata,idata,igtyp,nx,ny,g,ierror)
         if(ierror.ne.0) then
            write(*,*) 'GRIDPAR ERROR (INPUT): ',ierror
            if (igrida.ne.igridr) goto 8000
            write(*,*) 'CONTINUING ... input grid = output grid'
         end if
         if (igtyp.ne.igtypa .or.
     +             nx.ne.nxainp .or. ny.ne.nyainp .or.
     +       abs(g(1)-gainp(1)).ge.0.01 .or.
     +             abs(g(2)-gainp(2)).ge.0.01 .or.
     +       abs(g(3)-gainp(3)).ge.0.01 .or.
     +             abs(g(4)-gainp(4)).ge.0.01 .or.
     +       abs(g(5)-gainp(5)).ge.0.01 .or.
     +             abs(g(6)-gainp(6)).ge.0.01) then
            write(*,*) 'flt2flt: input geometry from field data does'
            write(*,*) '   not match geometry in the definition file'
            write(*,'(1x,''nxa,nya = '',2i10)') nxainp,nyainp
            write(*,'(1x,''nx ,ny  = '',2i10)') nx,ny
            write(*,'(1x,''ga = '',6f10.2)') (gainp(i),i=1,6)
            write(*,'(1x,''g  = '',6f10.2)') (g(i),i=1,6)
            write(*,*) 'igtypa = ',igtypa
            write(*,*) 'igtyp  = ',igtyp
            if (igrida.ne.igridr .or.
     +                nx.ne.nxainp .or. ny.ne.nyainp) goto 8000
            write(*,*) 'CONTINUE ... input grid = output grid = ',
     +                                                        igrida
            write(*,*) 'Grid identification will be corrected !!!'
         end if
         ipar1=1
      end if
c
c t and td to rh (from temp. and dew point temp. to relative humidity)
      if (ntd2rh.gt.0) then
         in(12)=itd2rh(1,ntd2rh)
         call mrfturbo(2,filarg,luarg,in(1),ipack,maxhor,va(1),1.,
     +                 mdata,idata(1),ierror)
         if (ierror.ne.0) goto 1000
         call ttd2rh(nx*ny,va(1),fa(1),fa(1),udef)
         ident( 6)=itd2rh(3,ntd2rh)
         ident(20)=-1
      end if
c
      lveloc=.false.
c
      if(igrida.eq.igridr) then
        do j = 1,nxr*nyr
          fr(j) = fa(j)
        end do
        goto 500
      end if
c
c vector/velocity components; read also v if u was asked
      n=0
      do i=1,numvec
        if (iparvec(1,i).eq.inr(12,iparr)) n=i
      end do
      if (n.gt.0) then
         lveloc=.true.
         iuparam=iparvec(1,n)
         ivparam=iparvec(2,n)
         in(12)=ivparam
         call mrfturbo(2,filarg,luarg,in(1),ipack,maxhor,va(1),1.,
     +                 mdata,idata(1),ierror)
         if (ierror.ne.0) goto 1000
         ifields=ifields+1
      end if
c
c extend input field for hemispheric data
      if (lexten) then
          do i = 1,nxainp*nyainp
            fr(i) = fa(i)
          end do
          call extendf2(fr,fa,nxainp,nyainp,
     +                       nxwrap,nxext1,nxext2,nyext1,nyext2)
          if (lveloc) then
             do i = 1,nxainp*nyainp
               vr(i) = va(i)
             end do
             call extendf2(vr,va,nxainp,nyainp,
     +                          nxwrap,nxext1,nxext2,nyext1,nyext2)
         end if
      end if
c
c true north direction parameter (dd)
c interpolated as vector/velocity components
      npardir=0
      do i=1,numdir
         if (ipardir(1,i).eq.ident(6)) npardir=i
      end do
      if (npardir.gt.0) then
         if (inter.eq.4 .or. inter.eq.5) then
            npardir=-npardir
         else
            zpir18 = 2.0*asin(1.0)/180.
            do j = 1,nxa*nya
               if (fa(j).ne.udef) then
                  dd = fa(j)*zpir18
                  fa(j) = sin(dd)
                  va(j) = cos(dd)
               else 
                  va(j) = udef
               end if
            end do
         end if
      end if
c
c possibly check output min and max values
      nparlim=0
      intopt=0
      if (inter.eq.3 .and. .not.lveloc) then
         do i=1,numlim
            if (ident(6).eq.iparlim(1,i)) nparlim=i
         end do
c possibly enable check in interpolation routine
         if (nparlim.gt.0) then
            if ((iparlim(2,nparlim).eq.1 .or. iparlim(2,nparlim).eq.3)
     +                         .and. parlim(1,nparlim).eq.0.0) intopt=1
            if ((iparlim(2,nparlim).eq.2 .or. iparlim(2,nparlim).eq.3)
     +                         .and. parlim(2,nparlim).eq.0.0) intopt=1
         end if
      elseif (npardir.lt.0) then
c only 'nearest gridpoint' interpolation (no u,v rotation needed)
         npardir=-npardir
         if (ipardir(2,npardir).ne.0) then
            iscale=ident(20)
            ddmin = 10.**iscale
            ddturn = real(ipardir(2,npardir))
            do j = 1,nxa*nya
               if (fa(j).ne.udef) then
                  dd = fa(j) + ddturn
                  if (dd.le.  0.) dd=dd+360.
                  if (dd.gt.360.) dd=dd-360.
c..assure nonzero value when scaling to integer*2
                  dd = max(dd,ddmin)
                  fa(j) = dd
               end if
            end do
         end if
         npardir=0
         intopt=1
      end if
c
c interpolate field
      call grd2grd(icall1,igtypa,ga(1),igtypr,gr(1),
     +             fa(1),nxa,nya,fr(1),nxr,nyr,
     +             jinter(1),rinter(1),ndimri,
     +             udef,inter,intopt,iundef,ierror)
      if(ierror.ne.0) then
        write(*,*) 'Coordinate conversion error'
        write(*,*) '   Error from grd2grd/xyconvert: ',ierror
        goto 8000
      end if
      icall1=3
c
      if (lveloc) then
c interpolate v component
         call grd2grd(icall1,igtypa,ga(1),igtypr,gr(1),
     +                va(1),nxa,nya,vr(1),nxr,nyr,
     +                jinter(1),rinter(1),ndimri,
     +                udef,inter,intopt,iundef,ierror)
         if(ierror.ne.0) then
            write(*,*) 'Coordinate conversion error'
            write(*,*) '   Error from grd2grd/xyconvert: ',ierror
            goto 8000
         end if
c turn velocity components
         call grv2grv(icall2,igtypa,ga(1),igtypr,gr(1),
     +                fr(1),vr(1),nxr,nyr,vturn(1,1),udef,ierror)
         if(ierror.ne.0) then
            write(*,*) 'Velocity/vector conversion error'
            write(*,*) '   Error from grv2grv/uvconvert: ',ierror
            goto 8000
         end if
         icall2=3
      end if
c
c true north direction parameter (dd), and interpolation
      if (npardir.gt.0) then
c interpolate v component
         call grd2grd(icall1,igtypa,ga(1),igtypr,gr(1),
     +                va(1),nxa,nya,vr(1),nxr,nyr,
     +                jinter(1),rinter(1),ndimri,
     +                udef,inter,intopt,iundef,ierror)
         if(ierror.ne.0) then
            write(*,*) 'Coordinate conversion error'
            write(*,*) '   Error from grd2grd/xyconvert: ',ierror
            goto 8000
         end if
         zdeg18 = 180./(2.0*asin(1.0))
         iscale=ident(20)
         ddmin = 10.**iscale
         ddturn = real(ipardir(2,npardir))
         do j = 1,nxr*nyr
            if (fr(j).ne.udef) then
               dd = zdeg18*atan2(fr(j),vr(j)) + ddturn
               if (dd.le.  0.) dd=dd+360.
               if (dd.gt.360.) dd=dd-360.
c..assure nonzero value when scaling to integer*2
               dd = max(dd,ddmin)
               fr(j) = dd
            else
               fr(j) = udef
            end if
         end do
      end if
c
  500 continue
c
c prepare for output
      ident( 2)=igridr
      ident( 9)=identr(9)
      ident(10)=nxr
      ident(11)=nyr
      ident(15)=identr(15)
      ident(16)=identr(16)
      ident(17)=identr(17)
      ident(18)=identr(18)
      if(lgeomr.gt.0) then
        do i=1,lgeomr
          idata(20+nxr*nyr+i)=igeomr(i)
        end do
      end if

c prepare for output
      if (icdtyp.ne.0 .and. ident(3).ne.4) then
        if (icdtyp.eq.1) then
c001122          if(ident(3).eq.1 .and. ident(4).le.0) ident(3)=3
          if(ident(4).le.0) ident(3)=3
          if(ident(3).eq.2 .and. ident(4).gt.0) ident(3)=3
        elseif(icdtyp.eq.2) then
c001122          if(ident(3).eq.1 .and. ident(4).le.0) ident(3)=3
          if(ident(4).le.0) ident(3)=3
        elseif(icdtyp.eq.3) then
          if(ident(4).le.0) ident(3)=1
          if(ident(3).eq.2 .and. ident(4).gt.0) ident(3)=3
        elseif(icdtyp.eq.4) then
          if(ident(4).le.0) ident(3)=1
        end if
      end if
      if (lveloc) ident(6)=iuparam
c
c smoothing
      if (numsmo.gt.0) then
         n=0
         do i=1,numsmo
            if (iparsmo(1,i).eq.ident(6)) n=i
         end do
         if (n.gt.0) then
            if(iparsmo(2,n).gt.0)
     +               call smooth(iparsmo(2,n),nxr,nyr,fr(1),udef)
         end if
      end if
c
      if (nparlim.gt.0) then
         if (iparlim(2,nparlim).eq.1 .or. iparlim(2,nparlim).eq.3) then
c check min value
            do i=1,nxr*nyr
               if (fr(i).ne.udef) fr(i)=max(fr(i),parlim(1,nparlim))
            end do
         end if
         if (iparlim(2,nparlim).eq.2 .or. iparlim(2,nparlim).eq.3) then
c check max value
            do i=1,nxr*nyr
               if (fr(i).ne.udef) fr(i)=min(fr(i),parlim(2,nparlim))
            end do
         end if
      end if
c
c change sign
      n=0
      do i=1,numsgn
        if (iparsgn(i).eq.ident(6)) n=i
      end do
      if (n.gt.0) then
         do i=1,nxr*nyr
            if (fr(i).ne.udef) fr(i) = -fr(i)
         end do
      end if
c
c automatic scaling of output field
      ident(20)=-32767
c
c change output scaling
      n=0
      do i=1,numscl
        if (iparscl(1,i).eq.ident(6)) n=i
      end do
      if (n.gt.0) ident(20)=iparscl(2,n)
c
      if(icdate.eq.0) then
c producer check disabled
        idata( 1)=inr(1,iparr)
c date/time check disabled (this is probably not necessary, but safe)
        idata(12)=-32767
        idata(13)=-32767
        idata(14)=-32767
      elseif(icdate.eq.2) then
c adjust forecast length (and possibly data type) on output field
         idata(4)=idata(4)-ihdiff
         if (idata(4).eq.0.and.idata(3).eq.2) idata(3)=3
         if (idata(4).gt.0) idata(3)=2
      end if
c
c............................................................................
c Merging of input and output grid:
c We have coded this option for the following application:
c The 2006 grid (Hirlam 0.5 degree) and 1997 grid (Hirlam 0.1 degree)
c are combined into a new grid 2000 (0.2 degree).
c The horizontal extent of the 2000 grid is smaller than the 2006 grid,
c but cover parts (or whole) of the 1997 grid. The combined grid contains
c values from the 1997 grid where this grid har well defined values and
c values from the 2006 grid elsewhere. The merging in done like this
c
c 1) nyfelt ......... felt2000        : create the felt file for the
c                                       combined grid.
c 2) flt2flt felt2006 felt2000 ...    : interpolate from 2006 to 2000 grid.
c 3) flt2flt felt1997 felt2000 ... -m : interpolate from 1997 to 2000 grid
c                                       and merge the interpolated field with
c                                       the field already present in felt2000.
c 
c NB. The different options in flt2flt is not taken into account when merging.
c     However, they should all work if step 2) and 3) is done in exactly the
c     same way (interpolation of the same fields and with the same options)
c     except for the -m options.
c............................................................................
c
      if (imerge.eq.1) then
c
c read the corresponding field from the output file and merge with the
c interpolated field before overwriting the output field.
c
c prepare for input
      do i=1,16
         in(i)=ina(i,jpara)
      end do
      in(2)=igridr
      if(icdate.eq.0) then
c date/time check disabled
        in(3)=-32767
        in(4)=-32767
        in(5)=-32767
      elseif(icdate.eq.2) then
        in(10)=in(10)-ihdiff
      end if
      in(12)=ident(6)
c 
c read field
      call mrfturbo(2,filres,lures,in(1),ipack,maxhor,fx(1),1.,
     +              mdata,idata2(1),ierror)
      if (ierror.ne.0) then
         write(*,*) 'ERROR could not find output file for read'
         goto 1000
      end if
c
c check input merge grid parameters
      if (kmerge.eq.0) then
         call gridpar(+1,mdata,idata2,igtyp,nx,ny,g,ierror)
         if(ierror.ne.0) then
            write(*,*) 'GRIDPAR ERROR (MERGE INPUT): ',ierror
            istop=102
            goto 8000
         end if
         if (igtyp.ne.igtypr .or.
     +             nx.ne.nxr .or. ny.ne.nyr .or.
     +       abs(g(1)-gr(1)).ge.0.01 .or.
     +             abs(g(2)-gr(2)).ge.0.01 .or.
     +       abs(g(3)-gr(3)).ge.0.01 .or.
     +             abs(g(4)-gr(4)).ge.0.01 .or.
     +       abs(g(5)-gr(5)).ge.0.01 .or.
     +             abs(g(6)-gr(6)).ge.0.01) then
            write(*,*) 'flt2flt: input merge geometry not equal'
            write(*,*) '   output geometry in the definition file'
            write(*,'(1x,''nx ,ny  = '',2i10)') nx,ny
            write(*,'(1x,''nxr,nyr = '',2i10)') nxr,nyr
            write(*,'(1x,''g  = '',6f10.2)') (g(i),i=1,6)
            write(*,'(1x,''gr = '',6f10.2)') (gr(i),i=1,6)
            write(*,*) 'igtyp  = ',igtyp
            write(*,*) 'igtypr = ',igtypr
            istop=102
            goto 8000
         end if
         kmerge=1
      end if
c
c merge fields
      call mergefs(icall3,fr(1),fx(1),nxa,nya,nxr,nyr,
     +             igtypa,ga(1),igtypr,gr(1),fa(1),va(1),
     +             alpha(1),nwidth,iundef,udef)
      icall3=3
c
c treat v-component if vector field
      if (lveloc) then
         in(12)=ivparam
         call mrfturbo(2,filres,lures,in(1),ipack,maxhor,vx(1),1.,
     +                 mdata,idata2(1),ierror)
         if (ierror.ne.0) then
            write(*,*) 'ERROR could not find output file for read'
            goto 1000
         end if
         call mergefs(icall3,vr(1),vx(1),nxa,nya,nxr,nyr,
     +                igtypa,ga(1),igtypr,gr(1),fa(1),va(1),
     +                alpha(1),nwidth,iundef,udef)
      end if
c
      end if
c
c..extrapolation to undefined positions
      if (iextrapol.gt.0)
     +   call extrapol(iextrapol,nxr,nyr,fr(1),fa(1),ifa(1),udef)
c 
c write output field
      call mwfturbo(2,filres,lures,ipack,nxr*nyr,fr(1),1.,
     +              mdata,idata(1),ierror)
      if (ierror.ne.0) then
         istop=3
         goto 8000
      end if
c
      if (lveloc) then
c
         ident(6)=ivparam
c change sign ?
         n=0
         do i=1,numsgn
           if (iparsgn(i).eq.ident(6)) n=i
         end do
         if (n.gt.0) then
            do i=1,nxr*nyr
               if (vr(i).ne.udef) vr(i) = -vr(i)
            end do
         end if
c automatic scaling of output field
         ident(20)=-32767
c change output scaling ?
         n=0
         do i=1,numscl
           if (iparscl(1,i).eq.ident(6)) n=i
         end do
         if (n.gt.0) ident(20)=iparscl(2,n)
c
c..extrapolation to undefined positions
         if (iextrapol.gt.0)
     +      call extrapol(iextrapol,nxr,nyr,vr(1),va(1),ifa(1),udef)
c
         call mwfturbo(2,filres,lures,ipack,nxr*nyr,vr(1),1.,
     +                 mdata,idata(1),ierror)
         if (ierror.ne.0) then
            istop=3
            goto 8000
         end if
c
      end if
c
c end loop over fields
 1000 continue
      end do
c
c print one line for for the last output time
      if (ifields.gt.0)
     +   write(*,'(1x,''date:'',3i5.4,'' utc '',sp,i4,ss,'' : '',i6,
     +             '' fields  (grid in:'',i5,''  out:'',i5,'')'')')
     +      in(3),in(4),in(5),itime,ifields,igrida,igridr
c
c end loop over input grids
 2000 continue
      end do
c
c end loop over output grids
 3000 continue
      end do
c
      istop=0
c
 8000 continue
c close input/output felt files (also updating output file)
      call mrfturbo(3,filarg,luarg,in(1),ipack,1,1.,1.,1,i2dum,ierror)
      if (ierror.ne.0 .and. istop.eq.0) istop=4
      call mwfturbo(3,filres,lures,ipack,1,1.,1.,1,i2dum,ierror)
      if (ierror.ne.0 .and. istop.eq.0) istop=5
c
 9000 continue
      if (istop.eq.1) write(*,*) 'open error input felt file'
      if (istop.eq.2) write(*,*) 'open error output felt file'
      if (istop.eq.3) write(*,*) 'write error output felt file'
      if (istop.eq.4) write(*,*) 'close error input felt file'
      if (istop.eq.5) write(*,*) 'close/update error output felt file'
c
      if (istop.lt.0 .or. istop.gt.255) istop=255
c
      if (istop.ne.0) write(*,*) 'flt2flt: ERROR EXIT ',istop
      if (istop.ne.0) call exit(istop)
c
      stop
      end
c
c***********************************************************************
c
      subroutine ttd2rh(n,t,td,rh,udef)
c
c from temperature and dew point temperature to relative humidity
c
      implicit none
c
      integer n
      real    t(n),td(n),rh(n),udef
c
      integer i,l
      real    ta,x,et,etd
      real    ewt(41)
c
c saturation pressure for t=-100,-95,-90,...+100 degrees celsius
      data ewt/.000034,.000089,.000220,.000517,.001155,.002472,
     *         .005080,.01005, .01921, .03553, .06356, .1111,
     *         .1891,  .3139,  .5088,  .8070,  1.2540, 1.9118,
     *         2.8627, 4.2148, 6.1078, 8.7192, 12.272, 17.044,
     *         23.373, 31.671, 42.430, 56.236, 73.777, 95.855,
     *         123.40, 157.46, 199.26, 250.16, 311.69, 385.56,
     *         473.67, 578.09, 701.13, 845.28, 1013.25/
c
      ta= -273.15 + 105.
c
      do i = 1,n
         if (t(i).ne.udef .and. td(i).ne.udef) then
            x=(t(i)+ta)*0.2
            x=max(x, 1.001)
            x=min(x,40.999)
            l=x
            et = ewt(l)+(ewt(l+1)-ewt(l))*(x-l)
            x=(td(i)+ta)*0.2
            x=max(x, 1.001)
            x=min(x,40.999)
            l=x
            etd = ewt(l)+(ewt(l+1)-ewt(l))*(x-l)
            etd = min(et,etd)
            rh(i) = 100.*etd/et
         else
            rh(i)=udef
         end if
      end do
c
      return
      end
c
c***********************************************************************
c
      subroutine smooth(nsmooth,nx,ny,z,undef)
c
c        smoothing (interface routine to fsmooth,fshap2,fshap4)
c
c        nsmooth>0  : call fsmooth(nsmooth,...)
c        nsmooth=-2 : call fshap2(...)
c        nsmooth=-4 : call fshap4(...)
c
      implicit none
c
      include 'flt2flt.inc'
c
c input/output
      integer nsmooth,nx,ny
      real    z(nx*ny),undef
c
c local
      integer nundef,i
      real    a(maxhor),w1(maxhor),w2(maxhor)
      complex zc(maxhor),ac(maxhor)
c
      if(nsmooth.eq.0) return
c
      nundef=0
      do i = 1,nx*ny
        if (z(i).eq.undef) nundef=1
      end do
c
      if (nsmooth.gt.0) then
         call fsmooth(nsmooth,nx,ny,z(1),a(1),nundef,undef,w1(1),w2(1))
      elseif (nsmooth.eq.-2) then
         call fshap2(nx,ny,z(1),a(1),nundef,undef,w1(1),w2(1))
      elseif (nsmooth.eq.-4) then
         call fshap4(nx,ny,z(1),a(1),zc,ac,nundef,undef,w1(1),w2(1))
      else
         write(6,*) 'WARNING. Unknown smoothing: ',nsmooth
      end if
c
      return
      end
c
c***********************************************************************
c
      subroutine fsmooth(nsmooth,nx,ny,z,a,nundef,undef,w1,w2)
c
c        low-bandpass filter, removing short wavelengths
c        (not a 2. or 4. order shapiro filter)
c
c        G.J.Haltiner, Numerical Weather Prediction,
c                         Objective Analysis,
c                            Smoothing and filtering
c
c        input:   nsmooth  - no. of iterations
c                 z(nx,ny) - the field
c                 a(nx,ny) - a work matrix
c                w1(nx,ny) - a work matrix (nundef>0)
c                w2(nx,ny) - a work matrix (nundef>0)
c        output:  z(nx,ny) - the field
c
      integer nsmooth,nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
c
      nxm1=nx-1
      nym1=ny-1
c
      s=0.25
c
      if(nundef.le.0) then
c
        do n=1,nsmooth
c
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
c
        end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
          do i=2,nxm1
            w1(i,j)=1.
            if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
          end do
        end do
        do j=2,nym1
          do i=1,nx
            w2(i,j)=1.
            if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
          end do
        end do
c
        do n=1,nsmooth
c
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
c
        end do
c
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine fshap2(nx,ny,z,a,nundef,undef,w1,w2)
c
c        2. ordens shapiro-filter (glatting av felt)
c        udefinerte punkt gitt med verdi=undef (og nundef>0)
c
c        input:   z(nx,ny)
c                 a(nx,ny) ... arbeids-matrise
c                w1(nx,ny) ... arbeids-matrise (nundef>0)
c                w2(nx,ny) ... arbeids-matrise (nundef>0)
c        output:  z(nx,ny)
c
      integer nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
c
      nxm1=nx-1
      nym1=ny-1
c
      if(nundef.le.0) then
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
          s=-0.5
        end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
          do i=2,nxm1
            w1(i,j)=1.
            if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
          end do
        end do
        do j=2,nym1
          do i=1,nx
            w2(i,j)=1.
            if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
          end do
        end do
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
          s=-0.5
        end do
c
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine fshap4(nx,ny,z,a,zc,ac,nundef,undef,w1,w2)
c
c        4. ordens shapiro-filter (glatting av felt)
c        udefinerte punkt gitt med verdi=undef (og nundef>0)
c
c        input:   z(nx,ny)
c                 a(nx,ny) ... arbeids-matrise (real)
c                zc(nx,ny) ... arbeids-matrise (complex)
c                ac(nx,ny) ... arbeids-matrise (complex)
c                w1(nx,ny) ... arbeids-matrise (nundef>0)  (real)
c                w2(nx,ny) ... arbeids-matrise (nundef>0)  (real)
c        output:  z(nx,ny)
c
      integer nx,ny,nundef
      real    undef
      real    z(nx,ny),a(nx,ny),w1(nx,ny),w2(nx,ny)
      complex zc(nx,ny),ac(nx,ny)
c
      real    s
      complex sc
c
      nxm1=nx-1
      nym1=ny-1
c
      if(nundef.le.0) then
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
          s=-0.5
        end do
c
        do j=1,ny
          do i=1,nx
            zc(i,j)=cmplx(z(i,j))
          end do
        end do
c
        sc=cmplx(0.,0.5)
        do k=1,2
          sc=sc*0.5
c
          do j=1,ny
            do i=2,nxm1
              ac(i,j)=zc(i,j)+sc*(zc(i-1,j)+zc(i+1,j)-2.*zc(i,j))
            end do
          end do
          do j=1,ny
            ac( 1,j)=zc( 1,j)
            ac(nx,j)=zc(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              zc(i,j)=ac(i,j)+sc*(ac(i,j-1)+ac(i,j+1)-2.*ac(i,j))
            end do
          end do
          do i=1,nx
            zc(i, 1)=ac(i, 1)
            zc(i,ny)=ac(i,ny)
          end do
          sc=cmplx(0.,-0.5)
        end do
c
        do j=1,ny
          do i=1,nx
            z(i,j)=real(zc(i,j))
          end do
        end do
c
      else
c
        udef=0.9*undef
        do j=1,ny
          do i=2,nxm1
            w1(i,j)=1.
            if(max(z(i-1,j),z(i,j),z(i+1,j)).gt.udef) w1(i,j)=0.
          end do
        end do
        do j=2,nym1
          do i=1,nx
            w2(i,j)=1.
            if(max(z(i,j-1),z(i,j),z(i,j+1)).gt.udef) w2(i,j)=0.
          end do
        end do
c
        s=0.5
        do k=1,2
          s=s*0.5
          do j=1,ny
            do i=2,nxm1
              a(i,j)=z(i,j)+s*(z(i-1,j)+z(i+1,j)-2.*z(i,j))*w1(i,j)
            end do
          end do
          do j=1,ny
            a( 1,j)=z( 1,j)
            a(nx,j)=z(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              z(i,j)=a(i,j)+s*(a(i,j-1)+a(i,j+1)-2.*a(i,j))*w2(i,j)
            end do
          end do
          do i=1,nx
            z(i, 1)=a(i, 1)
            z(i,ny)=a(i,ny)
          end do
          s=-0.5
        end do
c
        do j=1,ny
          do i=1,nx
            zc(i,j)=cmplx(z(i,j))
          end do
        end do
c
        sc=cmplx(0.,0.5)
        do k=1,2
          sc=sc*0.5
c
          do j=1,ny
            do i=2,nxm1
              if(w1(i,j).ne.0.)
     *          ac(i,j)=zc(i,j)+sc*(zc(i-1,j)+zc(i+1,j)-2.*zc(i,j))
            end do
          end do
          do j=1,ny
            ac( 1,j)=zc( 1,j)
            ac(nx,j)=zc(nx,j)
          end do
          do j=2,nym1
            do i=1,nx
              if(w2(i,j).ne.0.)
     *          zc(i,j)=ac(i,j)+sc*(ac(i,j-1)+ac(i,j+1)-2.*ac(i,j))
            end do
          end do
          do i=1,nx
            zc(i, 1)=ac(i, 1)
            zc(i,ny)=ac(i,ny)
          end do
          sc=cmplx(0.,-0.5)
        end do
c
        do j=1,ny
          do i=1,nx
            z(i,j)=real(zc(i,j))
          end do
        end do
c
      end if
c
      return
      end
c
c**********************************************************************
c
      subroutine mergefs(icall,fr,fx,nxa,nya,nxr,nyr,
     +                   igtypa,ga,igtypr,gr,x,y,
     +                   alpha,nwidth,iundef,udef)
c
c****************************************************************
c
c     mergefs - merge fields
c
c  purpose:
c
c     merge two fields fr and fx according to the formula
c
c     fr(i,j)=alpha(i,j)*fx(i,j)+(1.-alpha(i,j))*fr(i,j)
c
c     where alpha is a weight function defined as
c
c     alpha = 1 in all points outside the area of field fr.
c           = 0 in all points well inside the area of field fr.
c           = varying linearly from 1. to 0. in the boundary
c             zone of field fr.
c
c     prior to the call of this routine both fields fr and fx must
c     been interpolated to the same grid.
c
c     alpha is set to 1. in the nearest 2.5 griddistanced close to the
c     boundary of the fr-grid, because these points usually are not
c     given proper values from the interpolation in routine grd2grd.
c     
c     the width of the 'relaxation zone' inside the '2.5-zone' is an
c     input parameter (nwidth = number of griddistances).
c
c  input/output parameters:
c
c     icall  - input, switch for preparations/merging
c              = 1: preparations
c              = 2: preparations and merging
c              = 3: merging
c     fr     - input/output, values in the large grid
c     fx     - input field, values in the small grid
c     nxa    - input, x-dimension of the small grid
c     nya    - input, y-dimension of the small grid
c     nxr    - input, x-dimension of the large grid
c     nyr    - input, y-dimension of the large grid
c     igtypa - input, grid type for the small grid
c     ga     - input, grid parameters for the small grid
c     igtypr - input, grid type for the large grid
c     gr     - input, grid parameters for the large grid
c     x,y    - work array for calculation of positions
c     alpha  - input/output, the relaxation function
c     nwidth - input, width of the relaxation zone
c     iundef - switch for checking undefined values
c               = 0 - not check undefined values
c               = 1 - check undefined values when icall=2   (first field)
c               = 2 - check undefined values when icall=2,3 (each  field)
c     udef   - value in 'undefined' points
c
c  for description of g = ga and gr (for igtype = igtypa and igtypr),
c  see documentation in subroutine grd2grd.
c
c  we consider undefined values to be significant, like islands in the
c  ocean, rather than always preferring an exsiting value.
c
c  externals:
c
c     xyconvert - performs all coordinate conversions
c
c  history:
c
c     j.e. haugen/dnmi   jul -97.
c     a. foss/dnmi       jul -97 ... iundef option
c
c****************************************************************
c
      implicit none
c
      integer icall,nxa,nya,nxr,nyr,igtypa,igtypr,nwidth,iundef
      real fr(nxr,nyr),fx(nxr,nyr),ga(6),gr(6),
     +     x(nxr,nyr),y(nxr,nyr),alpha(nxr,nyr),udef
c
      integer i,j,ierror
      real xmin1,xmax1,xmin2,xmax2,
     +     ymin1,ymax1,ymin2,ymax2,
     +     dadr,dadrc,r,width
c
c icall.le.2: prepare alpha(i,j) to be used in subsequent calls.
c 
      if (icall.le.2) then
c
c compute the positions of small grid as a function of gridpoints
c in the large grid (=>positions will be .le.0 or .ge.nxa/nya
c in points outside the small grid).
c
      do j=1,nyr
      do i=1,nxr
         x(i,j)=real(i)
         y(i,j)=real(j)
      enddo
      enddo
c
      call xyconvert(nxr*nyr,x(1,1),y(1,1),
     +               igtypr,gr(1),igtypa,ga(1),
     +               ierror)
c
c calculate alpha in all points
c
      width=real(nwidth)
      xmin1=2.5
      xmax1=xmin1+width
      ymin1=2.5
      ymax1=ymin1+width
      xmax2=real(nxa)-1.5
      xmin2=xmax2-width
      ymax2=real(nya)-1.5
      ymin2=ymax2-width
      dadr=-1./width
      dadrc=dadr/sqrt(2.)
c
      do j=1,nyr
      do i=1,nxr
         if (x(i,j).le.xmin1 .or. x(i,j).ge.xmax2 .or.
     +       y(i,j).le.ymin1 .or. y(i,j).ge.ymax2) then
            alpha(i,j)=1.
         else
     +   if (x(i,j).ge.xmax1 .and. x(i,j).le.xmin2 .and.
     +       y(i,j).ge.ymax1 .and. y(i,j).le.ymin2) then
            alpha(i,j)=0.
         else
            if (x(i,j).lt.xmax1) then
               if (y(i,j).lt.ymax1) then
                  r=sqrt((x(i,j)-xmin1)**2.+(y(i,j)-ymin1)**2.)
                  alpha(i,j)=1.+r*dadrc
               else
     +         if (y(i,j).gt.ymin2) then
                  r=sqrt((x(i,j)-xmin1)**2.+(y(i,j)-ymax2)**2.)
                  alpha(i,j)=1.+r*dadrc
               else
                  r=x(i,j)-xmin1
                  alpha(i,j)=1.+r*dadr
               endif
            else
     +      if (x(i,j).gt.xmin2) then
               if (y(i,j).lt.ymax1) then
                  r=sqrt((x(i,j)-xmax2)**2.+(y(i,j)-ymin1)**2.)
                  alpha(i,j)=1.+r*dadrc
               else
     +         if (y(i,j).gt.ymin2) then
                  r=sqrt((x(i,j)-xmax2)**2.+(y(i,j)-ymax2)**2.)
                  alpha(i,j)=1.+r*dadrc
               else
                  r=xmax2-x(i,j)
                  alpha(i,j)=1.+r*dadr
               endif
            else
     +      if (y(i,j).lt.ymax1) then
               r=y(i,j)-ymin1
               alpha(i,j)=1.+r*dadr
            else
     +      if (y(i,j).gt.ymin2) then
               r=ymax2-y(i,j)
               alpha(i,j)=1.+r*dadr
            endif
         endif
      enddo
      enddo
c
      if (iundef.eq.1 .and. icall.eq.2) then
         do j=1,nyr
         do i=1,nxr
            if (alpha(i,j).gt.0. .and. alpha(i,j).lt.1.) then
               if (fr(i,j).eq.udef .or. fx(i,j).eq.udef) then
                  if (alpha(i,j).gt.0.5) then
                     alpha(i,j)=1.
                  else
                     alpha(i,j)=0.
                  endif
               endif
            endif
         enddo
         enddo
      endif
c
      endif
c
c icall.ge.2: merge fields
c
      if (icall.ge.2) then
c
      if (iundef.ne.2) then
c
         do j=1,nyr
         do i=1,nxr
            fr(i,j)=alpha(i,j)*fx(i,j)+(1.-alpha(i,j))*fr(i,j)
         enddo
         enddo
c
      else
c
         do j=1,nyr
         do i=1,nxr
            r=fr(i,j)
            fr(i,j)=alpha(i,j)*fx(i,j)+(1.-alpha(i,j))*fr(i,j)
            if (alpha(i,j).gt.0. .and. alpha(i,j).lt.1.) then
               if (r.eq.udef .or. fx(i,j).eq.udef) then
                  if (alpha(i,j).gt.0.5) then
                     fr(i,j)=fx(i,j)
                  else
                     fr(i,j)=r
                  endif
               endif
            endif
         enddo
         enddo
c
      endif
c
      endif
c
      return
      end
c
c**********************************************************************
c
      subroutine extrapol(iextrapol,nx,ny,field,fwork,indx,undef)
c
c        extrapolation to undefined positions after interpolation
c
c        iextrapol=1 : fill with mean of neighbours,
c                      repeated scans until no undefined values
c        iextrapol=2 : extrapolate by gradients (or mean),
c                      repeated scans until no undefined values
c        iextrapol=3 : advanced fill, as FILL in ecom3d
c
      implicit none
c
      integer iextrapol,nx,ny
      integer indx(nx,ny)
      real    field(nx,ny),fwork(nx,ny),undef
c
      integer nundef,i,j,i1,i2,j1,j2,ip,jp,nf,nfg,ipp,jpp
      real    fs,fsg
c
      if(iextrapol.eq.3) then
        call extrapol3(nx,ny,field,indx,undef,100)
        return
      end if
c
      nundef=1
c
      do while (nundef.gt.0 .and. nundef.lt.nx*ny)
c
        nundef=0
c
        do j=1,ny
          do i=1,nx
            fwork(i,j)=field(i,j)
          end do
        end do
c
        if(iextrapol.eq.1) then
c
          do j=1,ny
            do i=1,nx
              if(fwork(i,j).eq.undef) then
                i1=max(i-1,1)
                i2=min(i+1,nx)
                j1=max(j-1,1)
                j2=min(j+1,ny)
                fs=0.
                nf=0
                do jp=j1,j2
                  do ip=i1,i2
                    if(fwork(ip,jp).ne.undef) then
                      fs=fs+fwork(ip,jp)
                      nf=nf+1
                    end if
                  end do
                end do
                if(nf.gt.0) then
                  field(i,j)=fs/real(nf)
                else
                  nundef=nundef+1
                end if
              end if
            end do
          end do
c
        else
c
          do j=1,ny
            do i=1,nx
              if(fwork(i,j).eq.undef) then
                i1=max(i-1,1)
                i2=min(i+1,nx)
                j1=max(j-1,1)
                j2=min(j+1,ny)
                fs=0.
                nf=0
                fsg=0.
                nfg=0
                do jp=j1,j2
                  do ip=i1,i2
                    if(fwork(ip,jp).ne.undef) then
                      fs=fs+fwork(ip,jp)
                      nf=nf+1
                      ipp=2*ip-i
                      jpp=2*jp-j
                      if(ipp.ge.1 .and. ipp.le.nx .and.
     +                         jpp.ge.1 .and. jpp.le.ny) then
                        if(fwork(ipp,jpp).ne.undef) then
                          fsg=fsg+(2.0*fwork(ip,jp)-fwork(ipp,jpp))
                          nfg=nfg+1
                        end if
                      end if
                    end if
                  end do
                end do
                if(nfg.gt.0 .and. nfg.ge.nf/2) then
                  field(i,j)=fsg/real(nfg)
                elseif(nf.gt.0) then
                  field(i,j)=fs/real(nf)
                else
                  nundef=nundef+1
                end if
              end if
            end do
          end do
c
        end if
c
      end do
c
      if(nundef.gt.0)
     +  write(6,*) 'EXTRAPOLATION WARNING: all pos. undefined'
c
      return
      end
c
c**********************************************************************
c
      subroutine extrapol3(nx,ny,field,indx,undef,nloop)
c
c        extrapolation to undefined positions after interpolation:
c        DNMI Ecom3d FILL
c
      implicit none
c
      integer nx,ny,nloop
      integer indx(nx*ny)
      real    field(nx*ny),undef
c
      integer nundef,i,j,ij,n,n1a,n1b,n2a,n2b,n3a,n3b,n4a,n4b,n5a,n5b
      integer loop
      real    avg,cor,error
c
      nundef=0
      avg=0.0
      do ij=1,nx*ny
        if(field(ij).ne.undef) then
          avg=avg+field(ij)
        else
          nundef=nundef+1
        end if
      end do
c
      if(nundef.eq.0) return
c
      if(nundef.eq.nx*ny) then
        write(6,*) 'EXTRAPOLATION WARNING: all pos. undefined'
        return
      end if
c
      avg=avg/real(nx*ny-nundef)
c
      n=0
      n1a=n+1
      do j=2,ny-1
        do i=2,nx-1
          if(field((j-1)*nx+i).eq.undef) then
            n=n+1
            indx(n)=(j-1)*nx+i
          end if
        end do
      end do
      n1b=n
      n2a=n+1
      i=1
      do j=2,ny-1
        if(field((j-1)*nx+i).eq.undef) then
          n=n+1
          indx(n)=(j-1)*nx+i
        end if
      end do
      n2b=n
      n3a=n+1
      i=nx
      do j=2,ny-1
        if(field((j-1)*nx+i).eq.undef) then
          n=n+1
          indx(n)=(j-1)*nx+i
        end if
      end do
      n3b=n
      n4a=n+1
      j=1
      do i=1,nx
        if(field((j-1)*nx+i).eq.undef) then
          n=n+1
          indx(n)=(j-1)*nx+i
        end if
      end do
      n4b=n
      n5a=n+1
      j=ny
      do i=1,nx
        if(field((j-1)*nx+i).eq.undef) then
          n=n+1
          indx(n)=(j-1)*nx+i
        end if
      end do
      n5b=n
c
c..much faster than in the DNMI ecom3d ocean model,
c..where method was found to fill undefined values with
c..rather sensible values...
c
      cor=1.6
c
      do ij=1,nx*ny
        if(field(ij).eq.undef) field(ij)=avg
      end do
c
      do loop=1,nloop
c
        do n=n1a,n1b
          ij=indx(n)
          error= (field(ij-nx)+field(ij-1)
     +                 +field(ij+1)+field(ij+nx))*0.25-field(ij)
          field(ij)= field(ij) + error*cor
        end do
c
        do n=n2a,n2b
          ij=indx(n)
          field(ij)= field(ij+1)
        end do
c
        do n=n3a,n3b
          ij=indx(n)
          field(ij)= field(ij-1)
        end do
c
        do n=n4a,n4b
          ij=indx(n)
          field(ij)= field(ij+nx)
        end do
c
        do n=n5a,n5b
          ij=indx(n)
          field(ij)= field(ij-nx)
        end do
c
      end do
c
      return
      end
c
c**********************************************************************
c
      subroutine hrelap(iybase,iyear,imonth,iday,ihour,ihrtot)
c
c compute number of hours elapsed since iybase.01.01 00:00 (newyear)
c
      implicit none
      integer iybase,iyear,imonth,iday,ihour,ihrtot
      integer idmax(12),i
      data idmax /31,28,31,30,31,30,31,31,30,31,30,31/
      ihrtot = 0
      do i = iybase,iyear-1
         if ((mod(i,4).eq.0.and.mod(i,100).ne.0) .or.
     $        mod(i,400).eq.0 ) then
            ihrtot = ihrtot + 366*24
         else
            ihrtot = ihrtot + 365*24
         endif
      enddo
      do i = 1,imonth-1
         ihrtot = ihrtot + idmax(i)*24
      enddo
      ihrtot = ihrtot + (iday-1)*24 + ihour
      if (imonth.gt.2) then
         if ((mod(iyear,4).eq.0.and.mod(iyear,100).ne.0) .or.
     $        mod(iyear,400).eq.0 ) then
            ihrtot = ihrtot + 24
         endif
      endif
c
      end
