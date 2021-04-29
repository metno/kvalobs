ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}focal-kvbuild:${BASE_IMAGE_TAG} AS kvbins



FROM ${REGISTRY}focal-kvcpp-runtime:${BASE_IMAGE_TAG}

ARG DEBIAN_FRONTEND='noninteractive'
ARG kvuser=kvalobs
ARG kvuserid=5010

RUN apt-get update && apt-get --yes install \
   libperl5.30 libkvutil-perl

#Create a runtime user for kvalobs
RUN addgroup --gid $kvuserid $kvuser && \
  adduser --uid $kvuserid  --gid $kvuserid --disabled-password --disabled-login --gecos '' $kvuser

RUN mkdir -p /etc/kvalobs && chown ${kvuser}:${kvuser}  /etc/kvalobs
RUN mkdir -p /var/log/kvalobs && chown ${kvuser}:${kvuser}  /var/log/kvalobs
RUN mkdir -p /var/lib/kvalobs/run && chown ${kvuser}:${kvuser} /var/lib/kvalobs/run

COPY --from=kvbins /usr/bin/kvQabased /usr/bin/

VOLUME /etc/kvalobs
VOLUME /var/log/kvalobs

USER ${kvuser}:${kvuser}

ENTRYPOINT  ["/usr/bin/kvQabased" ]
