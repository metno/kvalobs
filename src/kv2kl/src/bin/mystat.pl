#! /usr/bin/perl


@mtime=stat($ARGV[0]);

if( $#mtime==-1  ){
    exit 1;
}

@datetime=gmtime($mtime[9]);


$year=$datetime[5]+1900;
$month=$datetime[4]+1;
$day=$datetime[3];


printf("$mtime[9],$year-%02d-%02d %02d:%02d:%02d", $month, $day, 
       $datetime[2], $datetime[1], $datetime[0]);


