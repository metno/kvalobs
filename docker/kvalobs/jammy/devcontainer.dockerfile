# ARG REGISTRY="registry.met/.no/met/obsklim/bakkeobservasjoner/data-og-kvalitet/kvalobs/kvbuild/staging/jammy"
ARG REGISTRY="jammy"
FROM ${REGISTRY}/kvbuilddep:latest
ARG DEBIAN_FRONTEND='noninteractive'
ARG USER=vscode

RUN apt update && apt install --yes  software-properties-common  gpg \
   less nano git  git-lfs locales sshpass build-essential  \
   libgmock-dev language-pack-nb-base 

#RUN apt-get update && apt-get install -y libgmock-dev language-pack-nb-base

RUN locale-gen "nb_NO.UTF-8" "en_US.UTF-8"

RUN useradd -ms /bin/bash ${USER}

ENV USER=${USER}
# For qabase to be able to run properly
# Add intern repos
# COPY docker/internrepo-4E8A0C14.asc /tmp/
# RUN apt-key add /tmp/internrepo-4E8A0C14.asc && rm /tmp/internrepo-4E8A0C14.asc && \
#   add-apt-repository 'deb [arch=amd64] http://internrepo.met.no/focal focal main contrib'

RUN apt-get update && apt-get --yes install \
   libkvutil-perl

ENTRYPOINT ["/bin/bash"]