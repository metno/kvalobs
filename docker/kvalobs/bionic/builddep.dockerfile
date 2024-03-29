# ignored, but kept for easier use with kvbuild.sh
ARG REGISTRY

FROM ubuntu:bionic

#Create a base image with all dependecies to build kvalobs.
#This includes metlibs-putools

#Key for internrepo.met.no and the postgresql repo
COPY docker/pg-ACCC4CF8.asc docker/internrepo-4E8A0C14.asc /tmp/
RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

#Add intertn repos and postgres repo
#RUN apt-key adv --keyserver hkp://keyserver.ubuntu.com:80 --recv-keys 4e8a0c1418494cf45d1b8533799e9fe74bb0156c &&\
#  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/focal focal main contrib'

RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/bionic bionic main contrib'

RUN apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt bionic-pgdg main'

RUN apt-get update && apt-get -y install \
  debhelper autotools-dev autoconf-archive debconf devscripts fakeroot \
  less mg \
  automake libtool gfortran bison flex sqlite3 libsqlite3-dev libpq-dev python3\
  libxml2-dev libxml++2.6-dev libboost-dev libboost-thread-dev \
  libboost-regex-dev libboost-filesystem-dev libboost-program-options-dev libboost-system-dev libboost-timer-dev \
  libomniorb4-dev  omniidl libperl-dev libdbd-pg-perl libcurl4-gnutls-dev liblog4cpp5-dev libcppunit-dev \
  cmake google-mock  zlib1g-dev libssl-dev libsasl2-dev libzstd-dev \
  librdkafka-dev \
  libmicrohttpd-dev libgnutls28-dev \
  metlibs-putools-dev

#Build libhttpserver
# RUN mkdir -p /usr/src/ && cd /usr/src && \
#   git clone https://github.com/etr/libhttpserver.git && \
#   cd libhttpserver && git checkout 0.18.2 && ./bootstrap && \
#   mkdir -p build && cd build && \
#  	../configure --prefix=/usr/local && \
#   make && make check && make install 


# Build metlibs-puctools. Installs in /usr/local.
# RUN mkdir -p /usr/src && cd /usr/src && \
#   git clone https://github.com/metno/metlibs-puctools.git && \
#   cd metlibs-puctools && mkdir -p build && cd build && \
#   cmake .. && make && make install && \
#   rm -rf /usr/src/metlibs-puctools

# Build metlibs-putools. Installs in /usr/local.
# RUN mkdir -p /usr/src && cd /usr/src && \
#   git clone https://github.com/metno/metlibs-putools.git && \
#   cd metlibs-putools && mkdir -p build && cd build && \
#   cmake .. && make && make install && \
#   rm -rf /usr/src/metlibs-putools 
  
