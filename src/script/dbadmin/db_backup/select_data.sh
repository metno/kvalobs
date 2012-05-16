#! /bin/bash
#  Kvalobs - Free Quality Control Software for Meteorological Observations 
#
#  Copyright (C) 2007 met.no
#
#  Contact information:
#  Norwegian Meteorological Institute
#  Box 43 Blindern
#  0313 OSLO
#  NORWAY
#  email: kvalobs-dev@met.no
#
#  This file is part of KVALOBS
#
#  KVALOBS is free software; you can redistribute it and/or
#  modify it under the terms of the GNU General Public License as 
#  published by the Free Software Foundation; either version 2 
#  of the License, or (at your option) any later version.
#  
#  KVALOBS is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#  General Public License for more details.
#  
#  You should have received a copy of the GNU General Public License along 
#  with KVALOBS; if not, write to the Free Software Foundation Inc., 
#  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

set -a

startyear=$1
endyear=1500
DELETE="none"

# minimum ett argument
if [ -n "$1" ]; then
   echo "startyear=" $1
   startyear=$1
   endyear=$1
else
   echo "To few arguments"
   exit 0
fi

# to argumenter
if [ -n "$2" ]; then
   #if [ $2 = "delete" ]; then
   #   DELETE="delete"
   #   endyear=$1
   #else
      endyear=$2
   #fi
fi
 
# tre argumenter
#if [ -n "$3" ]; then
#   if [ $3 = "delete" ]; then
#      DELETE="delete"
#   fi
#fi

#echo "endyear=" $endyear


#startyear=2007
#endyear=2007
start_date="$startyear-01-01 00:00:00"
end_date="$endyear-12-31 23:59:59"

sql="save.sql"
echo "$start_date - $end_date "

#endname="$(date -d "$start_date" +'%Y%m').dat"
endname="$startyear-$endyear.dat"
fdata="data-$endname"
ftextdata="text_data-$endname"
fmodeldata="model_data-$endname"


#Opprett sqlfilen for aa ta data ut av databasen
echo "CREATE TEMP TABLE data_temp AS SELECT * FROM data WHERE obstime BETWEEN '$start_date' AND '$end_date';" > $sql 
echo "CREATE TEMP TABLE text_data_temp AS SELECT * FROM text_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $sql
echo "CREATE TEMP TABLE model_data_temp AS SELECT * FROM model_data WHERE obstime BETWEEN '$start_date' AND '$end_date';" >> $sql
echo "\\copy data_temp TO $fdata WITH DELIMITER AS '|'"  >> $sql
echo "\\copy text_data_temp TO $ftextdata WITH DELIMITER AS '|'" >> $sql
echo "\\copy model_data_temp TO $fmodeldata WITH DELIMITER AS '|'" >> $sql

psql -U kvalobs kvalobs -f $sql

TIME=`date +'%Y-%m-%d_%H'`

mv $fdata $fdata.$TIME
gzip -9 $fdata.$TIME

mv $ftextdata $ftextdata.$TIME
gzip -9 $ftextdata.$TIME

mv $fmodeldata $fmodeldata.$TIME
gzip -9 $fmodeldata.$TIME