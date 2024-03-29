FROM registry.met.no/obs/kvalobs/kvbuild/staging/bionic-builddep:latest
VOLUME /src
VOLUME /build
WORKDIR /build

COPY . /src

RUN /src/configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info  \
	    --localstatedir=/var --sysconfdir=/etc  \
      CFLAGS=-g && make && make install

ENTRYPOINT [ "/bin/bash"]
