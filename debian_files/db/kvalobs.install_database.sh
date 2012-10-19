#!/bin/sh

DATABASE=kvalobs
DB_USER=kvalobs


set -e

su postgres -c "psql kvalobs -c \"ALTER USER $DB_USER WITH SUPERUSER\""  >> /dev/null || true


su $DB_USER -c "psql $DATABASE -f /usr/share/kvalobs/db/kvalobs_roles.sql"  >> /dev/null || true

ON_ERROR_STOP=1

su $DB_USER -c "psql $DATABASE --single-transaction -f /usr/share/kvalobs/db/kvalobs_schema.sql" >> /dev/null
