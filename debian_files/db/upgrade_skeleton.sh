#!/bin/sh

DATABASE=kvalobs
DB_USER=kvalobs

ON_ERROR_STOP=1

su $DB_USER -c "psql $DATABASE --single-transaction -f __UPGRADE_FILE__" || exit 1
