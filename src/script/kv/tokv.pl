#! /usr/bin/perl -w

use strict;

# ****** DESCRIPTION ******
#The intention of this script is to copy data in a given kvalobs format 
#to a directory that is for data to be sendt to kvalobs.
#
#The script consist of two parts. The first part is a configuration section
#and the second part do the job.




# ****** START OF CONFIGURATION ******


my $datapath = "/opdata/offshore/tonsbergkilen/kldata";

#To run as a cronjob in AutoObs.
#my $lastrun = "/metno/autoobs/log/tokv-lastrun";
#my $backuppath="";
#my $destpath = '/metno/autoobs/data2kv';
#my $log2file="/metno/autoobs/log/tokv.log";
#my $tmppath = '/metno/autoobs/tmp';
#my $copy="cp";

#To run under sms. Fix the paths to something suitable.
#my $lastrun = "/metno/autoobs/log/tokv-lastrun";
#my $backuppath="/opdata/offshore/tonsbergkilen/kldata/backup";
#my $destpath = 'autoobs@autoobs:data2kv';
#my $log2file="/metno/autoobs/log/tokv.log";
#my $tmppath = '/metno/autoobs/tmp';
#my $copy="scp";

#To run on my computer :-)
my $lastrun = "/disk1/kvalobs/var/kvalobs/log/tokv-lastrun";
my $backuppath="";
my $destpath = '/disk1/kvalobs/var/kvalobs/data2kv';
my $log2file="/disk1/kvalobs/var/kvalobs/log/tokv.log";
my $tmppath = '/disk1/kvalobs/tmp';
my $copy="cp";


# ****** END OF CONFIURATION *******

#Test if the destination directory exist and is writeable
#This test is only done if $copy is equal to cp or mv.
if( ($copy eq "cp" || $copy eq "mv") &&
    (! -d $destpath && ! -w $destpath) ) {
	print "\n\n *** destpath: <$destpath> is not writeable or does not exist!\n\n";
	exit(1);
}

my $lastruntime=1256274140; #Last file that allready is saved.
my $file;
my @statbuf;
my $filetime;
my $movedToBackup;
my $now = time();
my $logfile;
my $notNewerThan=$now+15;

sub timestamp {
	my @t=gmtime( time() );
	my $year=$t[5]+1900;
	my $month=$t[4]+1;
	my $day=$t[3]+1;
	
	#print $logfile "$year-$month-$day $t[2]:$t[1]:$t[0] - ";
	printf $logfile "%4d-%02d-%02d %02d:%02d:%02d - ", $year, $month, $day, $t[2], $t[1], $t[0];
}


#checkAndCopy( file )
#checkAndCopy add missing commas to the end of each dataset in the file
#if the format is "kldata". It does nothing if the format is different from "kldata".
#In each case the file/result is copied to the $destpath.
sub checkAndCopy {
	my $file = shift(@_);
	my $i=0;
	my $line;
	my @path = split /\// , $file ;
	my $fname = $path[$#path];
	my $tmpfile = $tmppath . "/" . $fname . ".tmp";
	my $fixed = 0;
	my @header;
	my $nHeaderElem;
	my $nDataElem;
	my $commas;
	
	if( ! open( FILE , "<$file" ) ) {
		timestamp; print $logfile "FAILED to open file <$file>!\n";
		return 0;
	}
	
	while( defined($line = <FILE>) ) {
		$line =~ s/\s+$//; #Removes whitespace from the end, including CR characters.	
		if( $i == 0 ) {
			if( $line =~ /^ *kldata/ ) {
				if( ! open( TMPFILE, ">$tmpfile" ) ) {
					timestamp; print $logfile "FAILED to create temporary file <$tmpfile>!\n";
					return 0;
				}
				print TMPFILE "$line\n";
				$i++;
				next;
			} else {
				$tmpfile = "";
				last;
			}
		} elsif( $i == 1 ) {
			print TMPFILE "$line\n";
			@header = split /,/, $line;
			$nHeaderElem = $#header + 1;
			$i++;
			next;
		} 
		
		$i++;
		
		if( $line !~ /^ *(\d+){10,} *,/ ) { #Simple sanity check. This is possibly a timestamp. 
			print TMPFILE "$line\n";
			next;
		}

		$nDataElem = ($line =~ tr/,//);
		
		if( $nDataElem < $nHeaderElem ) {
			my $ii=0;
			$commas = "";
			
			while( $ii < ($nHeaderElem-$nDataElem) ) {
				$fixed = 1;
				$commas .= ",";
				$ii++;
			}
			
			print TMPFILE "$line$commas\n"; 
		} else {
			print TMPFILE "$line\n";
		}	
	}
	
	if( $tmpfile ) {
		if( $fixed ) {
			timestamp; print $logfile "Added missing commas. $file -> $destpath - ";
		} else {
			timestamp; print $logfile "$file -> $destpath - ";
		}
		
		qx/$copy $tmpfile $destpath\/$fname/;
		
		if( $? == 0 ) {
			 print $logfile "OK\n";
		} else  {
			print $logfile "FAILED\n";
			return 0;
		}
		
		qx/unlink $tmpfile/; #remove the temporary file.
	} else {
		timestamp; print $logfile "$file -> $destpath - ";
		
		qx/$copy $file $destpath/;
		
		if( $? == 0 ) {
			 print $logfile "OK\n";
		} else  {
			print $logfile "FAILED\n";
			return 0;
		}
	}
	
	return 1;
}

if( length( $log2file ) == 0 ) {
	$logfile = \*STDOUT;
} else {

	#Truncate the logfile to the last 500 lines.
	if( -e $log2file ) {
		qx/tail -500 $log2file > $log2file.tmp/;
		
		if( $? == 0 ) {
			qx/mv $log2file.tmp $log2file/;
		}
	}

	if( ! open( LOGFILE, ">>$log2file" ) ) {
		print "FAILED to open/create logfile <$log2file>. Logging to stdout.\n";
		$logfile = \*STDOUT;
	} else {
		$logfile = \*LOGFILE;
	}
}

timestamp; print $logfile "*** Start ***\n";

if( ! -d $tmppath || ! -w $tmppath ) {
	timestamp; print $logfile "$tmppath do not exist, is not a directory or is not writable!\n";
	timestamp; print $logfile "*** Stop ***\n";
	exit(1);
}

if( ! -d $datapath || ! -r $datapath ) {
	timestamp; print $logfile "The source directory <$datapath> do not exist, is not a directory or is not readable!\n";
	timestamp; print $logfile "*** Stop ***\n";
	exit(1);
} 

if( -e $lastrun ) {
	@statbuf = stat( $lastrun );
	$lastruntime = $statbuf[9]; #modification time
}

#Update the lastrun file.
qx/touch $lastrun/;

if ( $? != 0 ) {
	timestamp; print $logfile "FAILED to update the timestamp on the file <$lastrun>!\n";
}


if( opendir DIR, $datapath ) {
	while( ($file=readdir DIR) ) {
		$file = $datapath . "/" . $file;
		
		#Skip if the file does not exist, not readable or end with .dat.
		if( ! -e $file || ! -r $file ||  ($file !~/.+\.dat$/) ) {
			next;
		}
			
		@statbuf = stat( $file );
		$filetime = $statbuf[9]; #modification time
		
		#Only copy the file to the destination path 'destpath' if it is newer than the lastrun file
		#and older than nnotNewerThan (pore mans check to ensure that the file is completly written 
		#to the disk).
		
		if( $filetime >= $lastruntime && $filetime < $notNewerThan ) {
			if( checkAndCopy( $file ) ) {		
				if( length( $backuppath ) > 0  && -d $backuppath && -w $backuppath ) {
					timestamp; print $logfile "Backup. move  $file to $backuppath  - ";
					qx/mv $file $backuppath/;
				
					if( $?==0 ) {
						print $logfile "OK\n";
					} else {
						print $logfile "FAILED\n";
					}
				}
			} else {
				timestamp; print $logfile "FORMAT check failed. ($file)!\n";
			}
		}
	} 
}


timestamp; print $logfile "*** Stop ***\n";
