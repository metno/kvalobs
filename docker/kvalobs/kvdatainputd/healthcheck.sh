#! /bin/bash

if [ -f /var/log/kvalobs/kvDataInputd_gdb ]; then
  # We are running a debug session just report ok
  exit 0
fi

set -e

TRANSACTIONFILE="/var/log/kvalobs/kvDataInputd_transaction.log"

#Check if the file is updated in the last fifteen minutes.
file=$(find ${TRANSACTIONFILE} -mmin -15 -exec ls -1 {} \; 2>/dev/null)

#echo "file: '${file}'"
if [ -n "${file}" ]; then
  exit 0
fi

echo "HEALTHCHECK: File '${TRANSACTIONFILE}' has not been updated in the last fifteen minutes."

exit 1
