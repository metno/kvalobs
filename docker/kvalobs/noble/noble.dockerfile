# ignored, but kept for easier use with kvbuild.sh
FROM ubuntu:noble

#Create a base image with all dependecies to build kvalobs.
#This includes metlibs-putools

#Keys for internrepo.met.no, postgresql and confluent (librdkafka) repos
COPY docker/pg-ACCC4CF8.asc docker/internrepo-4E8A0C14.asc /etc/apt/trusted.gpg.d/
RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

RUN add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/noble noble main contrib'

RUN add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt noble-pgdg main'
