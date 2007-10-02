#! /bin/sh

KVDIR=$KVALOBS

export KVDIR 

PIDFILE=kv2kl.pid
CONF=kv2kl.conf


if [ "$NAME" ]; then
	PIDFILE=kv2kl-$NAME.pid
fi

$KVDIR/bin/kv2kl "$@"

rm -f $KVDIR/var/run/$PIDFILE