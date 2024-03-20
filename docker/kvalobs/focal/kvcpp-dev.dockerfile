ARG REGISTRY
ARG BASE_IMAGE_TAG=latest
FROM ${REGISTRY}kvbuild:${BASE_IMAGE_TAG} AS kvbins
ENTRYPOINT [ "/bin/bash" ]

FROM ${REGISTRY}kvbuilddep:${BASE_IMAGE_TAG}
ARG DEBIAN_FRONTEND='noninteractive'

RUN apt-get install -y gpg software-properties-common apt-utils

COPY --from=kvbins /usr/lib/libkvalobs_*.so.* /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.so /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.a /usr/lib/
COPY --from=kvbins /usr/lib/libkvalobs_*.la /usr/lib/
COPY --from=kvbins /usr/lib/kvalobs10/db/*.so*  /usr/lib/kvalobs10/db/
COPY --from=kvbins /usr/lib/kvalobs10/decode/*.so*  /usr/lib/kvalobs10/decode/
COPY --from=kvbins /usr/lib/kvalobs10/lib/*.so*  /usr/lib/kvalobs10/lib/
COPY --from=kvbins /usr/include/kvalobs /usr/include/kvalobs
COPY --from=kvbins /usr/lib/pkgconfig/libkv*.pc  /usr/lib/pkgconfig/

ENTRYPOINT [ "/bin/bash" ]

