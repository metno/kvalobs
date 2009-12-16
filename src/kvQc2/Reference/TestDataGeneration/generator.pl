#!/usr/bin/perl
#
# set missing values to -11111 at first and then update them all later
# NB we rely on the -32676 for counting good values ...


$user_name= getpwuid $>;

if($#ARGV < 4) {
       &shouldo;
       exit;
}


$DATE=$ARGV[0];
$N=$ARGV[1];
$STID=$ARGV[2];
$TID=$ARGV[3];
$PID=$ARGV[4];

# substr(controlinfo,7,1)=3 || 1 || 2 --- dont forget need for this ???

$M=$N-1;

$CHK="SELECT * FROM data WHERE obstime='$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767 AND (SELECT COUNT(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767)=$N;";

$CHK2="SELECT * FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767;";

print $CHK,"\n";
print "\n";
print $CHK2,"\n";
print "\n";

$UP1="UPDATE data SET original=((SELECT SUM(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767)+(SELECT COUNT(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original=-1)) WHERE obstime='$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767 AND (SELECT COUNT(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767)=$N;";

$UP2="UPDATE data SET controlinfo='9999999999992990' WHERE obstime='$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767 AND (SELECT COUNT(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767)=$N;"; 

print "\n";
print $UP1,"\n";
print "\n";
print $UP2,"\n";

for ($i=1; $i < $N; $i++){
          #print $i,"\n";
          $j=$i-1;
          print "UPDATE data SET controlinfo='9999999999992990' WHERE obstime BETWEEN date '$DATE' -interval '$i days' AND date '$DATE' -interval '$j days' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767 AND (SELECT COUNT(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767)=$N;\n";
print "\n";
          print "UPDATE data SET original=-11111 WHERE obstime BETWEEN date '$DATE' -interval '$i days' AND date '$DATE' -interval '$j days' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767 AND (SELECT COUNT(original) FROM data WHERE obstime BETWEEN date '$DATE' - interval '$M days' AND '$DATE' AND stationid=$STID AND typeid=$TID AND paramid=$PID AND original>-32767)=$N;\n";
print "\n";
}

print "\n";
print $CHK,"\n";
print "\n";
print $CHK2,"\n";

## ----------------------------------------------------

sub shouldo {
  print
(" 
  Hi $user_name, usage is:
          $0 \"YYYY-MM-DD 06:00:00\" NUMBER_MISSING_POINTS StationID TypeID ParamID 
")
}

## ----------------------------------------------------
