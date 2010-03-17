      integer maxgrd,maxvec,maxtdr,maxdir,maxsgn,maxlim,maxscl,maxsmo
c
      parameter (maxgrd= 50)
      parameter (maxvec= 30)
      parameter (maxtdr= 10)
      parameter (maxdir= 20)
      parameter (maxsgn= 20)
      parameter (maxlim=100)
      parameter (maxscl= 20)
      parameter (maxsmo=100)
      integer numgrd,numvec,numtdr,numdir,numsgn,numlim,numscl,numsmo
      integer igspec(4,maxgrd+1),iparvec(2,maxvec+1),itd2rh(3,maxtdr+1),
     +        ipardir(2,maxdir+1),iparsgn(maxsgn+1),
     +	      iparlim(2,maxlim+1),iparscl(2,maxscl+1),
     +	      iparsmo(2,maxsmo+1),iocean(7)
      real    gspec(6,maxgrd+1),parlim(2,maxlim+1)
      common /fdef/ numgrd,numvec,numtdr,numdir,numsgn,numlim,numscl,
     +              numsmo,igspec,iparvec,itd2rh,ipardir,iparsgn,
     +              iparlim,iparscl,iparsmo,iocean,gspec,parlim
