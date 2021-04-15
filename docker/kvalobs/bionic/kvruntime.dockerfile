FROM bionic-kvbuild:latest AS kvbins

ENTRYPOINT [ "/bin/bash" ]

FROM ubuntu:bionic AS kv_bin_deps

COPY docker/pg-ACCC4CF8.asc /tmp

RUN apt-get update && apt-get install -y gnupg2 software-properties-common apt-utils

#Add intertn repos and postgres repo
RUN apt-key adv --keyserver keyserver.ubuntu.com --recv 4e8a0c1418494cf45d1b8533799e9fe74bb0156c && \
  add-apt-repository 'deb http://internrepo.met.no/bionic bionic main contrib' && \
  apt-key add /tmp/pg-ACCC4CF8.asc && rm /tmp/pg-ACCC4CF8.asc && \
  add-apt-repository 'deb http://apt.postgresql.org/pub/repos/apt bionic-pgdg main'

RUN apt-get update && apt-get -y install \
  libboost-date-time1.65.1 libboost-filesystem1.65.1 libboost-thread1.65.1 \
  libboost-regex1.65.1  libboost-program-options1.65.1 \
  libc6 libcurl3-gnutls libglibmm-2.4-1v5 \
  libmicrohttpd12 libomniorb4-2 libomnithread4 libpq5 libsasl2-2 \
  libssl1.1 libstdc++6 libxml++2.6-2v5 libxml2 libzstd1 zlib1g \
  postgresql-client-12 iproute2 gosu



COPY --from=kvbins /usr/local/lib/libmetlibs*.so.* /usr/local/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.so.* /usr/lib/
COPY --from=kvbins /usr/lib/kvalobs/db/*.so*  /usr/lib/kvalobs/db/
COPY --from=kvbins /usr/lib/kvalobs/decode/*.so*  /usr/lib/kvalobs/db/
COPY --from=kvbins /usr/lib/kvalobs/lib/*.so*  /usr/lib/kvalobs/lib/
COPY --from=kvbins /usr/include/kvalobs /usr/include
COPY --from=kvbins /usr/lib/pkgconfig/libkv*.pc  /usr/lib/pkgconfig/

ENTRYPOINT [ "/bin/bash" ]

