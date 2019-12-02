#! /bin/sh


dir="$(dirname $0)"

$dir/configure --prefix=/usr --mandir=/usr/share/man --infodir=/usr/share/info --localstatedir=/var --sysconfdir=/etc CFLAGS=-g