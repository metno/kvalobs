#! /bin/bash

usage="\
Usage: version [-h] [-d]

This script get the version of kvalobs from configute.ac.

If the -d option is given it will print the version and the date on
the form version-YYYYMMDD.

-h display this help and exit.
-d print version and date.
"

version=$(grep 'AC_INIT(' configure.ac | sed 's/AC_INIT( *\[kvalobs\] *, *\[\(.*\)\] *,.*/\1/')
today=""

while getopts "hd" opt; do
  case $opt in
    h)
      echo "$usage"
      exit 0
      ;;
    d)
      today="-$(date +'%Y%m%d')"
      ;;
    \?)
      echo "Invalid option: -$OPTARG" >&2
      echo "$usage" >&2
      exit 1
      ;;
  esac
done

echo "$version$today"
