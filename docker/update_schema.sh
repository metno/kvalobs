#! /bin/sh
set -ex

cat > /docker-entrypoint-initdb.d/15-kvalobs_schema.sql.1 << EOF
\set ON_ERROR_STOP 1
\connect kvalobs

EOF

cat /docker-entrypoint-initdb.d/15-kvalobs_schema.sql.1 /docker-entrypoint-initdb.d/15-kvalobs_schema_sql > /docker-entrypoint-initdb.d/15-kvalobs_schema.sql
rm -f /docker-entrypoint-initdb.d/15-kvalobs_schema.sql.1 /docker-entrypoint-initdb.d/15-kvalobs_schema_sql
