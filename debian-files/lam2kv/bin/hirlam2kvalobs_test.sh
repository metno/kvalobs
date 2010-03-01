#! /bin/sh

# --------------------------------------------------------------
# script to load model data into the KVALOBS - database
# Created by j.schulze@met.no 2002-12-19 
# update 2003-07-09 [ backup-system established ]
# update 2004-01-19 [ RR_12/24 etc added        ]
# update 2004-01-29 [ PP added                  ]
# update 2008-07-07 [ use hirlam12 (not 20)     ]
# -------------------------------------------------------------- 

RUN=$1

# ---------ENVIRONMENT: ----------------------------------------
  
export PGPASSWORD=`grep dbpass ~/.kvpasswd | sed -e 's/ *dbpass *//'` 

# ----------GENERAL PARAMETERS: ---------------------------------

# PATH:
LOCALBIN=/metno/local/bin

BIN=$HOME/bin
JOB=$HOME/cronjob/lam2kv

DATA=$JOB/data
LOAD=$JOB/testupload
ETC=$JOB/etc

# LOCAL PROGRAMS:

PSQL=psql

METDAT=$LOCALBIN/metdat
POSDAT=$LOCALBIN/posdat


TODAY=`/bin/date +%m%d`

# FILES:

POS=$ETC/stations.kvalobs
MET=$ETC/metdat.1h
POSE=$ETC/pose.kvalobs


# STANDARD 1h values:

OUTDAT=$DATA/last_tempo.dat
BACKDAT=$DATA/last_tempo.back
OUTSQL=$LOAD/hirlam12_$TODAY.$RUN


#----------------JOB:--------------------------------------------

# First part of the job...
# get the input files for metdat from the KVALOBS-base
# creates:
#         stations.kvalobs -- stationlist for metdat interpolation
#         parmap.kvalobs   -- parameter map for data-selection
# both files er semi static and will be kept until
# next write (in case of a database crash will the system work 
# anyway...

cd $ETC

$BIN/kv2metdat 

cd $JOB


# use the input files to generate a metdat file from fields
# old fashion way ... remember station.kvalobs includes
# the kvalobs station number as name ...

LAMIN=`/usr/bin/find /opdata/hirlam12/h12sf$RUN.dat -cmin -1000`

if [ "$LAMIN" ]; then
    
    # METDAT file OK and newer than 10 hours    
    

    $METDAT $MET $POS $LAMIN +6,+11  12 LAM12 $OUTDAT 
    $METDAT $MET $POS $LAMIN +12,+17 12 LAM12 $BACKDAT
    

    
#-------Process RR12 with hirlam00/12 ------------------------------

    if [ $RUN = "00" ] || [ $RUN = "12" ]; then
    
	TMP_RR12=$DATA/RR12.tmp
	OUT_RR12=$DATA/RR12.dat
	POS_RR12=$ETC/posdat.RR12
	SQL_RR12=$LOAD/hirlam12_RR12_$TODAY.$RUN
	MET_RR12=$ETC/metdat.RR12


	$METDAT $MET_RR12 $POS $LAMIN +06,+18  12 LAM12 $TMP_RR12 
	
	$POSDAT $POS_RR12 $TMP_RR12 $OUT_RR12

	
	if [ -f "$OUT_RR12" ]; then
	    $BIN/model2kv $POSE $OUT_RR12  $SQL_RR12
	fi
    fi

#-------Process RR24 with hirlam00 ---------------------------------


    if [ $RUN = "00" ]; then
    
	TMP_RR24=$DATA/RR24.tmp
	OUT_RR24=$DATA/RR24.dat
	POS_RR24=$ETC/posdat.RR24
	SQL_RR24=$LOAD/hirlam12_RR24_$TODAY.$RUN
	MET_RR24=$ETC/metdat.RR24


	$METDAT $MET_RR24 $POS $LAMIN +06,+30 12 LAM12 $TMP_RR24 

	$POSDAT $POS_RR24 $TMP_RR24 $OUT_RR24
	

	if [ -f "$OUT_RR24" ]; then
	    $BIN/model2kv $POSE $OUT_RR24  $SQL_RR24
	fi
    fi

#-------Process PP  with all runs ------------------------------
    

    TMP_PP=$DATA/PP.tmp
    OUT_PP=$DATA/PP.dat
    POS_PP=$ETC/posdat.PP
    SQL_PP=$LOAD/hirlam12_PP_$TODAY.$RUN
    MET_PP=$ETC/metdat.PP


    $METDAT $MET_PP $POS $LAMIN +03,+9 12 LAM12 $TMP_PP 
    $POSDAT $POS_PP $TMP_PP $OUT_PP

    if [ -f "$OUT_PP" ]; then
	$BIN/model2kv $POSE $OUT_PP  $SQL_PP
    fi

fi
    

# run our own pose to select the requested data. 
# The number of parameters is limited by $ETC/parameters.kvalobs
# if the file is empty, do nothing (in sms -> warn somebody...)

if [ -f "$OUTDAT" ]; then
    $BIN/model2kv $POSE $OUTDAT $OUTSQL 
fi
