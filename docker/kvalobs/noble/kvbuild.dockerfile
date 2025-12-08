ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvbuilddep:${BASE_IMAGE_TAG}

VOLUME /src
VOLUME /build
WORKDIR /build

COPY . /src
COPY GITREF /usr/share/kvalobs/VERSION

RUN --mount=type=cache,target=/build cd /src/ && autoreconf -if && cd /build && \
      CC=gcc-14 CXX=g++-14 /src/configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info  \
      --localstatedir=/var --sysconfdir=/etc  \
      CFLAGS=-g && make && make install

ENTRYPOINT [ "/bin/bash"]
