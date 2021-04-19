ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}focal-builddep:${BASE_IMAGE_TAG}

VOLUME /src
VOLUME /build
WORKDIR /build

COPY . /src

RUN /src/configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info  \
	    --localstatedir=/var --sysconfdir=/etc  \
      CFLAGS=-g && make && make install

ENTRYPOINT [ "/bin/bash"]
