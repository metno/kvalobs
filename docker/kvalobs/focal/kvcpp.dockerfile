ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}focal-kvbuild:${BASE_IMAGE_TAG} AS kvbins

FROM ${REGISTRY}focal-builddep:${BASE_IMAGE_TAG} AS dev
ARG DEBIAN_FRONTEND='noninteractive'

COPY docker/pg-ACCC4CF8.asc /tmp

RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

#Add intertn repos and postgres repo
RUN apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt focal-pgdg main'

#Add bufrdecoder


COPY --from=kvbins /usr/lib/libkvalobs_*.so.* /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.so /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.a /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.la /usr/lib/
COPY --from=kvbins /usr/lib/kvalobs/db/*.so*  /usr/lib/kvalobs/db/
COPY --from=kvbins /usr/lib/kvalobs/decode/*.so*  /usr/lib/kvalobs/decode/
#COPY --from=kvbins /usr/lib/kvalobs/lib/*.so*  /usr/lib/kvalobs/lib/
COPY --from=kvbins /usr/include/kvalobs /usr/include/kvalobs
COPY --from=kvbins /usr/lib/pkgconfig/libkv*.pc  /usr/lib/pkgconfig/

ENTRYPOINT [ "/bin/bash" ]


FROM ubuntu:bionic AS runtime
ARG DEBIAN_FRONTEND='noninteractive'

COPY docker/pg-ACCC4CF8.asc /tmp
RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

#Add intertn repos and postgres repo 
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv 4e8a0c1418494cf45d1b8533799e9fe74bb0156c && \
  add-apt-repository 'deb http://internrepo.met.no/bionic bionic main contrib' && \
  apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt bionic-pgdg main'

# NB NB NB
# The dependencies must be in sync with the *-dev dependencies in 
# registry.met.no/obs/kvalobs/kvbuild/staging/bionic-builddep:latest
RUN apt-get update && apt-get -y install \
  libboost-date-time1.65.1 libboost-filesystem1.65.1 libboost-thread1.65.1 \
  libboost-regex1.65.1  libboost-program-options1.65.1 \
  libc6 libcurl3-gnutls libglibmm-2.4-1v5 \
  libmicrohttpd12 libomniorb4-2 libomnithread4 libpq5 libsasl2-2 \
  libssl1.1 libstdc++6 libxml++2.6-2v5 libxml2 libzstd1 zlib1g \
  postgresql-client-12 iproute2 gosu

COPY docker/tini_0.19.0-amd64.deb /tmp
RUN dpkg -i /tmp/tini_0.19.0-amd64.deb && rm -f tini_0.19.0-amd64.deb

COPY --from=kvbins /usr/local/lib/libmetlibs*.so.* /usr/local/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.so.* /usr/lib/
COPY --from=kvbins /usr/lib/kvalobs/db/*.so*  /usr/lib/kvalobs/db/
COPY --from=kvbins /usr/lib/kvalobs/decode/*.so*  /usr/lib/kvalobs/decode/
COPY --from=kvbins /usr/lib/kvalobs/lib/*.so*  /usr/lib/kvalobs/lib/

RUN mkdir -p /var/lib/kvalobs/run

ENTRYPOINT [ "/bin/bash" ]

