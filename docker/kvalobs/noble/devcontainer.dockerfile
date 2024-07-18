ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging/noble"
#ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging"

FROM ${REGISTRY}/kvbuilddep:latest
ARG DEBIAN_FRONTEND='noninteractive'

RUN apt-get update && apt-get install -y libgmock-dev language-pack-nb-base

# For qabase to be able to run properly
#Add intern repos
# COPY docker/internrepo-4E8A0C14.asc /tmp/
# RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
#   add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/focal focal main contrib'


#Use this when libkvutil-perl is available for ubuntu noble
# RUN apt-get update && apt-get --yes install \
#    libperl5.38t64 libkvutil-perl 


#Remove this when li
RUN apt update && apt install --yes \
   libdbd-pg-perl libdbi-perl libperl5.38t64 postgresql bzip2 wget libclass-singleton-perl \
   libdatetime-locale-perl libdatetime-perl libdatetime-set-perl libdatetime-timezone-perl \
   libmodule-implementation-perl libmodule-runtime-perl libparams-classify-perl perl \
   libparams-validate-perl libset-infinite-perl libtry-tiny-perl libdatetime-event-sunrise-perl
COPY metno-bufrtables_1.2.8_all.deb libkvutil-perl_2.12.3-1_amd64.deb /tmp/
RUN dpkg -i /tmp/metno-bufrtables_1.2.8_all.deb 
RUN dpkg -i --ignore-depends=libperl5.30 /tmp/libkvutil-perl_2.12.3-1_amd64.deb
RUN rm -f /tmp/metno-bufrtables_1.2.8_all.deb /tmp/libkvutil-perl_2.12.3-1_amd64.deb
#End remove when metno-bufrtables_1.2.8_all.deb libkvutil-perl_2.12.3-1_amd64.deb are avalable fof ubunto noble

#Add vscode user
RUN locale-gen en_US.UTF-8 nb_NO.UTF-8
RUN useradd -ms /bin/bash vscode
