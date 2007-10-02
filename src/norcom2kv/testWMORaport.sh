#! /bin/sh

FILELIST="data0000 data0600 data0300 data0900"

for FILE in $FILELIST ; do
  ./testWMORaport synop/$FILE > $FILE.pre ;
  echo "diff -c -b -B regres/$FILE.reg $FILE.pre";
  diff -c -b -B regres/$FILE.reg $FILE.pre
done
