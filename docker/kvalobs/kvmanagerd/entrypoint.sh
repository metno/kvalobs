#!/usr/bin/env bash

#: ${CONF:=application}

#When a process is killed by a signal, it has exit code 128 + signum. (LINUX)
#Killed by SIGTERM (15) => 128 + 15 = 143.

set -e

export PGPASSFILE=/etc/kvalobs/.pgpass

kvManagerd_PID=
got_exit_signal=false
running=true
trap _term SIGTERM SIGINT

_term() {
    echo "TERMINATING"
    running=false
    got_exit_signal=true    
}

#kill_pid pid [signal=SIGTERM [timeout=60]
kill_pid() {
    local pid sig timeout tuntil
    pid=$1
    sig=SIGTERM
    timeout=60
    
    shift
    [ $# -gt 0 ] && sig=$1; shift 
    [ $# -gt 0 ] && timeout=$1; shift 

    tuntil=$(($(date +'%s')+$timeout))
        
    if ! kill -$sig $pid &>/dev/null; then
      wait $pid
      return $?
    fi

    while [ $(date +'%s') -lt $tuntil ]; do
      if ! kill -0 $pid &>/dev/null; then
        wait $pid
        return $?
      fi
      sleep 1
    done

    kill -9 $pid &>/dev/null
    wait $pid &>/dev/null
    return $?
}

echo "ENTRYPOINT: NARGS: $# ARGS: '$@'"

echo "getent: $(getent passwd kvalobs)"
echo "id: $(id -u)"


if [ "$#" -eq 0 ]; then
  echo "ENTRYPOINT starting kvManagerdd"
  
  echo "Starting kvManagerd."
  /usr/bin/kvManagerd 2>&1 &
  kvManagerd_PID=$!
  echo "kvManagerd pid: $kvManagerd_PID"
  wait -n
  ec=$?

  if [ $got_exit_signal = true ]; then
    echo "Got exit signal: $got_exit_signal"
    kill_pid $kvManagerd_PID
    ec=$?
    echo "Killed kvManagerd on signal. Exit code: $ec"
  else
    kill -0 $kvDataInputd_PID &>/dev/null || echo "kvManagerd: died  ec: $ec"
  fi

  #It may be dead, but we call kill_pid for kvManagerd anyway. It will be killed
  #cleanly if it is not dead and we get the exit code if it is dead.

  kill_pid $kvManagerd_PID  
  echo "kvManagerd exit code: $?"

  #return the exitcode for the process that died in the first place.
  exit $ec  
elif [ "$1" = "bash" ]; then
    echo "ENTRYPOINT starting bash!"
    /bin/bash
else
    echo "ENTRYPOINT sleep forever!"
    while running="true"; do 
        sleep 1; 
    done
fi
