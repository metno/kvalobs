#!/bin/bash
cat $1 | grep -v "rows)" | sed '/^$/d' | sed '/--/d' | sed '/stationid/d' | sed 's/^.\{103\}//' | sed 's/|.*:/ /' | sed '/-32767/d'  > $1.XY.dat
Rscript plotXY.R $1.XY.dat
