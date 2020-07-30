#!/usr/bin/env bash

#: ${CONF:=application}

export PGPASSFILE=/etc/kvalobs/.pgpass
aexecd_PID=
kvDataInputd_PID=
running=true

echo "ENTRYPOINT: NARGS: $# ARGS: '$@'"


trap term_handler SIGTERM SIGINT

term_handler() {
    running=false
    if [ -z "$kvDataInputd_PID" ]; then
        echo "No PID for kvDataInputd"
        exit 0
    fi

    kill -SIGTERM $kvDataInputd_PID
    if [ -z "$aexecd_PID" ]; then
        echo "No PID for aexecd"
    else 
        kill -SIGTERM $aexecd_PID
    fi
}

/usr/bin/aexecd &>/dev/null & 
aexecd_PID=$!
echo "aexec pid: $aexecd_PID"

if [ "$#" -eq 0 ]; then
    echo "ENTRYPOINT starting kvDataInputd"
    /usr/bin/kvDataInputd 2>&1 &
    kvDataInputd_PID=$!
    wait "$kvDataInputd_PID"
    ec=$?
    echo "kvDataInputd exit code: $ec"

    #When a process is killed by a signal, it has exit code 128 + signum. (LINUX)
    #Killed by SIGTERM (15) => 128 + 15 = 143.
elif [ "$1" = "bash" ]; then
    echo "ENTRYPOINT starting bash!"
    /bin/bash
else
    echo "ENTRYPOINT sleep forever!"
    while running="true"; do 
        sleep 1; 
    done
fi
