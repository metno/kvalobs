#! /bin/sh

# --------------------------------------------------------------
# script to load model data into the KVALOBS - database
# Created by j.schulze@met.no 2002-12-19 
# update 2003-07-09 [ backup-system established ]
# update 2004-01-19 [ RR_12/24 etc added        ]
# update 2004-01-29 [ PP added                  ]
# update 2008-07-07 [ use hirlam12 (not 20)     ]
# update 2009-03-10 [ use hirlam8 (not 12)     ]
# -------------------------------------------------------------- 

RUN=$1

# ---------ENVIRONMENT: ----------------------------------------
  
#export PGPASSWORD=`grep dbpass ~/.kvpasswd | sed -e 's/ *dbpass *//'` 

# ----------GENERAL PARAMETERS: ---------------------------------

# PATH:
#LOCALBIN=/metno/local/bin
LOCALBIN="$(kvconfig --libdir)/kvalobs/bin"

#BIN=$HOME/bin
#JOB=$HOME/cronjob/lam2kv

BIN=$(kvconfig --bindir)
JOB=$(kvconfig --localstatedir)/lib/kvalobs/lam2kv

DATA=$JOB/data
LOAD=$JOB/upload
ETC="$(kvconfig --sysconfdir)/kvalobs/lam2kv"
#ETC=$JOB/etc

KVPARAM=$ETC/parmap.kvalobs

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
OUTSQL=$LOAD/hirlam8_$TODAY.$RUN


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



LAMIN=`/usr/bin/find /opdata/hirlam8/h8km$RUN.dat -cmin -1000`

#dev-vm101 hack.
#For some reason the metdat cant read files from an NFS mount.
#As a work around we copy the files to /myopdata

MYHOST=`hostname`

if [ "$MYHOST" = "dev-vm101" ]; then
    rm -f /myopdata/*
    if [ "$LAMINz" != "z" ]; then
		echo "COPY: $LAMIN to /myopdata"
		cp $LAMIN /myopdata
		myfile=`basename $LAMIN`
        LAMIN="/myopdata/$myfile" 
        echo "LAMIN: $LAMIN"
    fi
fi

if [ "$LAMIN" ]; then
    
    # METDAT file OK and newer than 10 hours    
    echo "$METDAT $MET $POS $LAMIN +6,+11  8 LAM8 $OUTDAT"
    echo "$METDAT $MET $POS $LAMIN +12,+17 8 LAM8 $BACKDAT"
    $METDAT $MET $POS $LAMIN +6,+11  8 LAM8 $OUTDAT 
    $METDAT $MET $POS $LAMIN +12,+17 8 LAM8 $BACKDAT
    

    
#-------Process RR12 with hirlam00/12 ------------------------------

    if [ $RUN = "00" ] || [ $RUN = "12" ]; then
    
	TMP_RR12=$DATA/RR12.tmp
	OUT_RR12=$DATA/RR12.dat
	POS_RR12=$ETC/posdat.RR12
	SQL_RR12=$LOAD/hirlam8_RR12_$TODAY.$RUN
	MET_RR12=$ETC/metdat.RR12


# Gridnumber is 8 for Hirlam 8, 12 for Hirlam 12, 2001 for Hirlam 20
	$METDAT $MET_RR12 $POS $LAMIN +06,+18  8 LAM8 $TMP_RR12 
	
	$POSDAT $POS_RR12 $TMP_RR12 $OUT_RR12

	
	if [ -f "$OUT_RR12" ]; then
	    $BIN/model2kv -p $KVPARAM $POSE $OUT_RR12  $SQL_RR12
	fi
    fi

#-------Process RR24 with hirlam00 ---------------------------------


    if [ $RUN = "00" ]; then
    
	TMP_RR24=$DATA/RR24.tmp
	OUT_RR24=$DATA/RR24.dat
	POS_RR24=$ETC/posdat.RR24
	SQL_RR24=$LOAD/hirlam8_RR24_$TODAY.$RUN
	MET_RR24=$ETC/metdat.RR24


	$METDAT $MET_RR24 $POS $LAMIN +06,+30 8 LAM8 $TMP_RR24 

	$POSDAT $POS_RR24 $TMP_RR24 $OUT_RR24
	

	if [ -f "$OUT_RR24" ]; then
	    $BIN/model2kv -p $KVPARAM $POSE $OUT_RR24  $SQL_RR24
	fi
    fi

#-------Process PP  with all runs ------------------------------
    

    TMP_PP=$DATA/PP.tmp
    OUT_PP=$DATA/PP.dat
    POS_PP=$ETC/posdat.PP
    SQL_PP=$LOAD/hirlam8_PP_$TODAY.$RUN
    MET_PP=$ETC/metdat.PP


    $METDAT $MET_PP $POS $LAMIN +03,+9 8 LAM8 $TMP_PP 
    $POSDAT $POS_PP $TMP_PP $OUT_PP

    if [ -f "$OUT_PP" ]; then
	$BIN/model2kv -p $KVPARAM $POSE $OUT_PP  $SQL_PP
    fi

else

# METDAT file not present or too old (yesterdays run)
# in this case! use the old file (if present)
# remove the old file ( no duplication!!!)

    rm $OUTDAT

    if [ -f "$BACKDAT" ]; then  
	/bin/mv $BACKDAT $OUTDAT
    fi
    
fi
    


# run our own pose to select the requested data. 
# The number of parameters is limited by $ETC/parameters.kvalobs
# if the file is empty, do nothing (in sms -> warn somebody...)

if [ -f "$OUTDAT" ]; then
    $BIN/model2kv -p $KVPARAM $POSE $OUTDAT $OUTSQL 
fi

# load the stuff into the database

cd $LOAD


for FILE in * ; do

   $PSQL -U kvalobs kvalobs < $FILE
    
    if [ $? -eq 0 ] ; then
        rm $FILE
    fi

done








