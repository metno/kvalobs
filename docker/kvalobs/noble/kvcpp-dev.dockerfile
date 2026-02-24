ARG REGISTRY
ARG BASE_IMAGE_TAG=latest
FROM ${REGISTRY}kvbuild:${BASE_IMAGE_TAG} AS kvbins
ENTRYPOINT [ "/bin/bash" ]

FROM ${REGISTRY}kvbuilddep:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'

RUN apt install -y gpg software-properties-common apt-utils

COPY --from=kvbins /usr/lib/libkvalobs_*.so.* /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.so /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.a /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.la /usr/lib/
COPY --from=kvbins /usr/lib/kvalobs11/db/*.so*  /usr/lib/kvalobs11/db/
COPY --from=kvbins /usr/lib/kvalobs11/decode/*.so*  /usr/lib/kvalobs11/decode/
COPY --from=kvbins /usr/lib/kvalobs11/lib/*.so*  /usr/lib/kvalobs11/lib/
COPY --from=kvbins /usr/include/kvalobs /usr/include/kvalobs
COPY --from=kvbins /usr/lib/pkgconfig/libkv*.pc  /usr/lib/pkgconfig/

ENTRYPOINT [ "/bin/bash" ]

