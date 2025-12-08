# ignored, but kept for easier use with kvbuild.sh
ARG REGISTRY

FROM ubuntu:noble
ARG kafka_VERSION=2.3.0-1build2

#Create a base image with all dependecies to build kvalobs.
#This includes metlibs-putools

#Keys for internrepo.met.no, postgresql and confluent (librdkafka) repos
COPY docker/pg-ACCC4CF8.asc docker/internrepo-4E8A0C14.asc docker/confluent_repo_key.asc /tmp/
RUN apt update && apt install -y gnupg2 software-properties-common apt-utils

RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/noble noble main contrib'

RUN apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt noble-pgdg main'


# RUN apt-key add /tmp/confluent_repo_key.asc && rm /tmp/confluent_repo_key.asc && \
#   add-apt-repository 'deb [arch=amd64] https://packages.confluent.io/clients/deb noble  main'

#Development tools and programming languages
RUN apt update && apt install -y \
  debhelper autotools-dev autoconf-archive debconf devscripts fakeroot \
  build-essential less nano automake libtool gfortran bison flex sqlite3 \
  omniidl python3 cmake google-mock libgmock-dev libgtest-dev g++-14 libstdc++-14-dev

#Dependencies for kvalobs
RUN apt update && apt install -y \
  libboost-date-time-dev libboost-filesystem-dev libboost-thread-dev \
  libboost-regex-dev libboost-program-options-dev libboost-system-dev \
  libboost-timer-dev libboost-dev \
  libc6-dev libcurl4-gnutls-dev libglibmm-2.4-dev \
  libmicrohttpd-dev libomniorb4-dev libomnithread4-dev libpq-dev libsasl2-dev\
  libssl-dev libxml++2.6-dev libxml2-dev libzstd-dev zlib1g-dev \
  librdkafka-dev=${kafka_VERSION}  metlibs-putools-dev\
  libperl-dev libdbd-pg-perl  liblog4cpp5-dev libcppunit-dev \
  libgnutls28-dev 

#librdkafka1=${kafka_VERSION} librdkafka++1=${kafka_VERSION} 

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

