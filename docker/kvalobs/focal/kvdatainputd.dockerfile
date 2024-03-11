ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvbuild:${BASE_IMAGE_TAG} AS kvbins

ENTRYPOINT [ "/bin/bash" ]


FROM ${REGISTRY}kvcpp-runtime:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'
ARG kvuser=kvalobs
ARG kvuserid=5010
ENV PGPASSFILE=/etc/kvalobs/.pgpass

#Add bufrdecoder 
RUN apt-get update && apt-get -y install libgeo-bufr-perl  metno-bufrtables
COPY docker/kvalobs/kvdatainputd/BufrDecode.pl /usr/local/bin

#Add kvalobs user
RUN useradd -ms /bin/bash --uid ${kvuserid} --user-group  ${kvuser}
RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs

#Copy bins
COPY --from=kvbins /usr/bin/kvDataInputd /usr/bin/
COPY --from=kvbins /usr/bin/aexecd* /usr/bin/
COPY docker/kvalobs/kvdatainputd/aexecd.conf /etc/kvalobs/
COPY docker/kvalobs/kvdatainputd/entrypoint.sh \
  docker/kvalobs/kvdatainputd/healthcheck.sh \
  docker/kvalobs/kvdatainputd/kv_get_stinfosys_params.sh /usr/local/bin/
RUN chmod +x /usr/local/bin/entrypoint.sh /usr/local/bin/healthcheck.sh \
  /usr/local/bin/kv_get_stinfosys_params.sh

VOLUME /etc/kvalobs
VOLUME /var/log/kvalobs
VOLUME /var/lib/kvalobs

EXPOSE 8090

USER ${kvuser}:${kvuser}
HEALTHCHECK --interval=60s --timeout=30s --start-period=60s --retries=10 CMD [ "/usr/local/bin/healthcheck.sh" ]
ENTRYPOINT  ["/usr/local/bin/entrypoint.sh" ]
