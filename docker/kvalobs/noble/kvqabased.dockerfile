ARG REGISTRY
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}kvbuild:${BASE_IMAGE_TAG} AS kvbins
ENTRYPOINT [ "/bin/bash" ]


FROM ${REGISTRY}kvcpp-runtime:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'
ARG kvuser=kvalobs
ARG kvuserid=5010


#Remove this when libkvutil-perl is available for ubuntu noble
RUN apt update && apt install --yes \
  libdbd-pg-perl libdbi-perl libperl5.38t64 postgresql bzip2 wget libclass-singleton-perl \
  libdatetime-locale-perl libdatetime-perl libdatetime-set-perl libdatetime-timezone-perl \
  libmodule-implementation-perl libmodule-runtime-perl libparams-classify-perl perl \
  libparams-validate-perl libset-infinite-perl libtry-tiny-perl libdatetime-event-sunrise-perl
COPY libkvutil-perl_2.12.3-1_amd64.deb /tmp/
RUN dpkg -i --ignore-depends=libperl5.30 /tmp/libkvutil-perl_2.12.3-1_amd64.deb
RUN rm -f /tmp/libkvutil-perl_2.12.3-1_amd64.deb
#End remove when metno-bufrtables_1.2.8_all.deb libkvutil-perl_2.12.3-1_amd64.deb are avalable fof ubunto noble


#Use this when libkvutil-perl is available for ubuntu noble
#RUN apt-get update && apt-get --yes install \
#  libperl5.38t64 libkvutil-perl

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

COPY src/kvQabased/scripts/check_file_age.sh /usr/bin/check_file_age

HEALTHCHECK CMD [ "check_file_age", "/var/log/kvalobs/kvQabased_transaction.log" ]
ENTRYPOINT  ["/usr/bin/kvQabased" ]
