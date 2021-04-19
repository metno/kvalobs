FROM focal-kvbuild:latest AS kvbins

ENTRYPOINT [ "/bin/bash" ]


FROM registry.met.no/obs/kvalobs/kvbuild/staging/focal-kvcpp-runtime:latest 
ARG DEBIAN_FRONTEND='noninteractive'
ARG kvuser=kvalobs
ARG kvuserid=5010

#Add bufrdecoder
RUN apt-get -y install libgeo-bufr-perl 
COPY docker/kvalobs/kvdatainputd/BufrDecode.pl /usr/local/bin

#Create a runtime user for kvalobs
RUN addgroup --gid $kvuserid $kvuser && \
  adduser --uid $kvuserid  --gid $kvuserid --disabled-password --disabled-login --gecos '' $kvuser

RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs

COPY --from=kvbins /usr/bin/kvDataInputd /usr/bin/
COPY --from=kvbins /usr/bin/aexecd* /usr/bin/
COPY docker/kvalobs/kvdatainputd/aexecd.conf /etc/kvalobs/
COPY docker/kvalobs/kvdatainputd/entrypoint.sh  /usr/bin/

RUN chmod +x /usr/bin/entrypoint.sh
VOLUME /etc/kvalobs
VOLUME /var/log/kvalobs


EXPOSE 8090

USER ${kvuser}:${kvuser}

#ENTRYPOINT [ "/bin/bash" ]
#ENTRYPOINT ["/bin/bash", "-c", "while true; do sleep 10000; done" ]
ENTRYPOINT  ["tini", "--", "/usr/bin/entrypoint.sh" ]
