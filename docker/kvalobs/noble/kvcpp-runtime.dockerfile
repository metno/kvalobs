ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvbuild:${BASE_IMAGE_TAG} AS kvbins
ENTRYPOINT [ "/bin/bash" ]

FROM ubuntu:noble
ARG DEBIAN_FRONTEND='noninteractive'
ARG kafka_VERSION=2.3.0-1build2

RUN apt update && apt install -y gpg software-properties-common apt-utils

#Keys for internrepo.met.no, postgresql and confluent (librdkafka) repos
COPY docker/pg-ACCC4CF8.asc docker/internrepo-4E8A0C14.asc docker/confluent_repo_key.asc /tmp/
RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
  add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/noble noble main contrib'

RUN apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt noble-pgdg main'


# RUN apt-key add /tmp/confluent_repo_key.asc && rm /tmp/confluent_repo_key.asc && \
#   add-apt-repository 'deb [arch=amd64] https://packages.confluent.io/clients/deb noble  main'

# NB NB NB
# The dependencies must be in sync with the *-dev dependencies in 
# registry.met.no/obs/kvalobs/kvbuild/staging/focal-builddep:latest

RUN apt  update && apt install -y \
  libboost-date-time1.83.0 libboost-filesystem1.83.0 libboost-thread1.83.0 \
  libboost-regex1.83.0  libboost-program-options1.83.0 libboost-system1.83.0 \
  libboost-timer1.83.0\
  libc6 libcurl3t64-gnutls libglibmm-2.4-1t64 \
  libmicrohttpd12t64 libomniorb4-3t64 libomnithread4 libpq5 libsasl2-2 \
  libssl3t64 libstdc++6 libxml++2.6-2v5 libxml2 libzstd1 zlib1g \
  postgresql-client-17 iproute2 gosu \
  librdkafka1=${kafka_VERSION} librdkafka++1=${kafka_VERSION} libmetlibs-putools8



#COPY --from=kvbins /usr/local/lib/libmetlibs*.so.* /usr/local/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.so.* /usr/lib/
COPY --from=kvbins /usr/lib/kvalobs10/db/*.so*  /usr/lib/kvalobs10/db/
COPY --from=kvbins /usr/lib/kvalobs10/decode/*.so*  /usr/lib/kvalobs10/decode/
COPY --from=kvbins /usr/share/kvalobs/VERSION /usr/share/kvalobs/VERSION
COPY --from=kvbins /usr/lib/kvalobs10/lib/*.so*  /usr/lib/kvalobs10/lib/

RUN mkdir -p /var/lib/kvalobs/run

ENTRYPOINT [ "/bin/bash" ]

