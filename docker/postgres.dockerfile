FROM postgres:13

RUN apt-get update && \
  apt-get -y install bzip2 less mg wget && \
  rm -rf /var/lib/apt/lists/*
RUN mkdir -p /var/lib/kvalobs && chown postgres.postgres /var/lib/kvalobs
RUN mkdir -p /usr/share/kvalobs && chown postgres.postgres /usr/share/kvalobs
COPY src/kvalobs_database/kvalobs_roles.sql    /docker-entrypoint-initdb.d/00-kvalobs_roles.sql
COPY docker/additional_roles.sql               /docker-entrypoint-initdb.d/05-additional_roles.sql 
COPY src/kvalobs_database/kvalobs_createdb.sql /docker-entrypoint-initdb.d/10-kvalobs_createdb.sql
COPY src/kvalobs_database/kvalobs_schema.sql   /docker-entrypoint-initdb.d/15-kvalobs_schema_sql
COPY docker/install_metadata.sh                /docker-entrypoint-initdb.d/80-install_metadata.sh
COPY src/kvalobs_database/upgrade/5.0.0.sql    /usr/share/kvalobs/5.0.0.sql
COPY docker/update_schema.sh                   /usr/share/kvalobs/
COPY docker/kvget-metadata.sh /usr/bin
RUN /usr/share/kvalobs/update_schema.sh 

