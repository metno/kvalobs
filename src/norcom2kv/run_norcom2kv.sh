#! /bin/sh

BIN=$HOME/projects/kvalobs/kvalobs/src/norcom2kv/norcom2kv

#SYNOPDIR=$HOME/projects/kvalobs/kvalobs/src/norcom2kv/synop
SYNOPDIR=$HOME/projects/kvalobs/kvalobs/src/norcom2kv/synop1
#SYNOPDIR=/opdata/norcom/norsyn

TRACELEVEL=DEBUG
LOGLEVEL=DEBUG
LOGDIR=$HOME/projects/kvalobs/kvalobs/src/norcom2kv/log
TMPDIR=$HOME/projects/kvalobs/kvalobs/src/norcom2kv/tmp

export SYNOPDIR TRACELEVEL LOGLEVEL LOGDIR TMPDIR

echo "$BIN --synopdir=$SYNOPDIR --tmpdir=$TMPDIR --logdir=$LOGDIR \
          --tracelevel=$TRACELEVEL  --loglevel=$LOGLEVEL $*"

exec $BIN --synopdir=$SYNOPDIR --tmpdir=$TMPDIR --logdir=$LOGDIR \
          --tracelevel=$TRACELEVEL  --loglevel=$LOGLEVEL $*


