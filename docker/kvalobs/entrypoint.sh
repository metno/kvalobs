#! /bin/sh




doExit() {
  exit 0
}

trap doExit SIGTERM 


created="/etc/build_user_created"


if [ -f "$created" ]; then  
  su - `cat $created`
  while true; do 
    sleep 10000; 
  done
  exit 0
fi 

set -e

groupadd -r $user

useradd --no-log-init -r -g $groupid -G sudo


