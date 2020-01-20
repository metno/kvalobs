#!/bin/sh
FLAG=""
          
MYACLOCAL=/disk1/local/share/aclocal

while test -n "$1"; do
   case $1 in
    --help) 
        echo "Usage: $0 [OPTION]

autogen.sh is utilized for the eclipse autotools plugin and in the
continous build bots.

Options:
--help             display this help and exit
--force            use --force in the local autoreconf (replaces files)
"; exit 0;;
    *)
        FLAG="$FLAG $1"
        shift
        continue;; 
        esac
done

echo "autoreconf -i $FLAG -I $MYACLOCAL"

autoreconf -i $FLAG -I $MYACLOCAL

if test -f "${PWD}/configure"; then
    echo "======================================"
    echo "You are ready to run './configure'"
    echo "======================================"
else
    echo "  Failed to generate ./configure script!"
fi
