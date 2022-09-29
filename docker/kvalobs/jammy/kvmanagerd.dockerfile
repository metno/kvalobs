ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvbuild:${BASE_IMAGE_TAG} AS kvbins

FROM ${REGISTRY}kvcpp-runtime:${BASE_IMAGE_TAG}

ARG kvuser=kvalobs
ARG kvuserid=5010

#Create a runtime user for kvalobs
RUN addgroup --gid $kvuserid $kvuser && \
  adduser --uid $kvuserid  --gid $kvuserid --disabled-password --disabled-login --gecos '' $kvuser

RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs
RUN mkdir -p /var/lib/kvalobs/run && chown ${kvuser}:${kvuser} /var/lib/kvalobs/run

COPY --from=kvbins /usr/bin/kvManagerd /usr/bin/
COPY docker/kvalobs/kvmanagerd/entrypoint.sh  /usr/bin/

RUN chmod +x /usr/bin/entrypoint.sh
VOLUME /etc/kvalobs
VOLUME /var/log/kvalobs

USER ${kvuser}:${kvuser}

ENTRYPOINT  ["/usr/bin/kvManagerd" ]
CMD [ "--log-to-stdout" ]
