ARG REGISTRY=noble
ARG BASE_IMAGE_TAG=latest

FROM ${REGISTRY}/kvbuilddep:${BASE_IMAGE_TAG}
ARG user=kvalobs
ARG userid=5010
ARG groupid=5010


RUN useradd -ms /bin/bash --uid ${userid}  --user-group ${user}

VOLUME /workspace/kvalobs

RUN mkdir -p /workspace/kvalobs && chown ${user}:${user} /workspace/kvalobs

WORKDIR /workspaces/kvalobs

USER ${user}:${groupid}
ENTRYPOINT [ "/bin/bash"]
