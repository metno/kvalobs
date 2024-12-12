ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging-noble"
#ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging"

FROM ${REGISTRY}/kvbuilddep:latest
ARG DEBIAN_FRONTEND='noninteractive'
ARG USER=vscode

RUN apt-get update && apt-get install --yes python-is-python3 software-properties-common python3-pip gpg \
   less nano git python3-argcomplete git-lfs locales sshpass build-essential python3-dev python3.12-venv \
   libgmock-dev language-pack-nb-base

RUN locale-gen "nb_NO.UTF-8" "en_US.UTF-8"
RUN activate-global-python-argcomplete

RUN useradd -ms /bin/bash ${USER}

ENV USER=${USER}


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

ENTRYPOINT [ "/bin/bash" ]
