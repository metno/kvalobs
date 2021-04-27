#! /bin/bash

set -e

TRANSACTIONFILE="/var/log/kvalobs/kvDataInputd_transaction.log"

#Check if the file is updated in the last two minutes.
file=$(find ${TRANSACTIONFILE} -mmin -2 -exec ls -1 {} \; 2>/dev/null)

#echo "file: '${file}'"
if [ -n "${file}" ]; then
  exit 0
fi

exit 1
