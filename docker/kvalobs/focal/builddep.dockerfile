FROM ubuntu:focal

#Create a base image with all dependecies to build kvalobs.
#This includes metlibs-putools

#Key for the postgresql repo
COPY docker/pg-ACCC4CF8.asc /tmp
RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

#Add intertn repos and postgres repo
# RUN apt-key adv --keyserver keyserver.ubuntu.com --recv kvget && \
#   add-apt-repository 'deb http://internrepo.met.no/focal focal main contrib' && \
#   apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
#   add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt focal-pgdg main'

RUN apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt focal-pgdg main'

RUN apt-get update && apt-get -y install \
  debhelper autotools-dev autoconf-archive debconf devscripts fakeroot \
  less mg \
  automake libtool gfortran bison flex sqlite3 libsqlite3-dev libpq-dev python3\
  libxml2-dev libxml++2.6-dev libboost-dev libboost-thread-dev \
  libboost-regex-dev libboost-filesystem-dev libboost-program-options-dev libboost-system-dev libboost-timer-dev \
  libomniorb4-dev  omniidl libperl-dev libdbd-pg-perl libcurl4-gnutls-dev liblog4cpp5-dev libcppunit-dev \
  cmake google-mock  zlib1g-dev libssl-dev libsasl2-dev libzstd-dev \
  libmicrohttpd-dev libgnutls28-dev

# metlibs-putools-dev

# Build metlibs-puctools. Installs in /usr/local.
RUN mkdir -p /usr/src && cd /usr/src && \
  git clone https://github.com/metno/metlibs-puctools.git && \
  cd metlibs-puctools && mkdir -p build && cd build && \
  cmake .. && make && make install && \
  rm -rf /usr/src/metlibs-puctools

# Build metlibs-putools. Installs in /usr/local.
RUN mkdir -p /usr/src && cd /usr/src && \
  git clone https://github.com/metno/metlibs-putools.git && \
  cd metlibs-putools && mkdir -p build && cd build && \
  cmake .. && make && make install && \
  rm -rf /usr/src/metlibs-putools 
  

