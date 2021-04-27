ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}focal-kvbuild:${BASE_IMAGE_TAG} AS kvbins

ENTRYPOINT [ "/bin/bash" ]

FROM ${REGISTRY}focal-kvcpp-runtime:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'
ARG kvuser=kvalobs
ARG kvuserid=5010

#Add bufrdecoder
#RUN apt-get update && apt-get -y install libgeo-bufr-perl 
COPY docker/kvalobs/kvdatainputd/BufrDecode.pl /usr/local/bin

#Create a runtime user for kvalobs
#RUN addgroup --gid $kvuserid $kvuser && \
#  adduser --uid $kvuserid  --gid $kvuserid --disabled-password --disabled-login --gecos '' $kvuser

RUN useradd -ms /bin/bash --uid ${kvuserid} --user-group  ${kvuser}
#RUN useradd -m -s /bin/bash --uid 5010 --user-group 5010 kvalobs
  

RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs

COPY --from=kvbins /usr/bin/kvDataInputd /usr/bin/
COPY --from=kvbins /usr/bin/aexecd* /usr/bin/
COPY --from=kvbins /usr/local/lib/libhttpserver.so* /usr/local/lib/
COPY docker/kvalobs/kvdatainputd/aexecd.conf /etc/kvalobs/
COPY docker/kvalobs/kvdatainputd/entrypoint.sh \
  docker/kvalobs/kvdatainputd/healthcheck.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/entrypoint.sh /usr/local/bin/healthcheck.sh
VOLUME /etc/kvalobs
VOLUME /var/log/kvalobs
VOLUME /var/lib/kvalobs


EXPOSE 8090

USER ${kvuser}:${kvuser}
HEALTHCHECK --interval=60s --timeout=30s --start-period=60s --retries=10 CMD [ "/usr/local/bin/healthcheck.sh" ]
#ENTRYPOINT [ "/bin/bash" ]
#ENTRYPOINT ["/bin/bash", "-c", "while true; do sleep 10000; done" ]
ENTRYPOINT  ["/usr/local/bin/entrypoint.sh" ]
