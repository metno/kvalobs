ARG REGISTRY="registry.met.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging"

FROM ${REGISTRY}/kvbuilddep:latest
ARG DEBIAN_FRONTEND='noninteractive'

RUN apt-get update && apt-get install -y libgmock-dev language-pack-nb-base

# For qabase to be able to run properly
#Add intern repos
# COPY docker/internrepo-4E8A0C14.asc /tmp/
# RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
#   add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/focal focal main contrib'

RUN apt-get update && apt-get --yes install \
   libperl5.30 libkvutil-perl 

#Add vscode user
RUN locale-gen en_US.UTF-8 nb_NO.UTF-8
RUN useradd -ms /bin/bash vscode
