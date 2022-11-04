#syntax=docker/dockerfile:1.2

ARG REGISTRY
ARG BUILDDEP_TAG=latest

FROM ${REGISTRY}builddep:${BUILDDEP_TAG}

VOLUME /src
VOLUME /build
WORKDIR /build

COPY . /src
COPY GITREF /usr/share/kvalobs/VERSION

RUN --mount=type=cache,target=/build cd /src/ && autoreconf -if && cd /build && \
      /src/configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info  \
	    --localstatedir=/var --sysconfdir=/etc  \
      CFLAGS=-g && make && make install

ENTRYPOINT [ "/bin/bash"]
