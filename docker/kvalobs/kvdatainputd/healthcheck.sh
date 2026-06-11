#! /bin/bash

set -e

TRANSACTIONFILE="/var/log/kvalobs/kvDataInputd_transaction.log"

#Check if the file is updated in the last five minutes.
file=$(find ${TRANSACTIONFILE} -mmin -5 -exec ls -1 {} \; 2>/dev/null)

#echo "file: '${file}'"
if [ -n "${file}" ]; then
  exit 0
fi

echo "HEALTHCHECK: File '${TRANSACTIONFILE}' has not been updated in the last five minutes."

exit 1
