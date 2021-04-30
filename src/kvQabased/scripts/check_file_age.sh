#!/bin/bash

set -euo pipefail

TRANSACTION_LOG=$1
MAX_AGE_SECONDS=$2

if [ ! -f "$TRANSACTION_LOG" ]; then
    echo "$TRANSACTION_LOG does not exist"
    exit 0
fi

now=$(date +%s)
modtime=$(stat -c '%Y' "$TRANSACTION_LOG")

if (( "$now" - "$modtime" > "$MAX_AGE_SECONDS" )); then
    echo "file is too old"
    exit 1
fi

echo ok