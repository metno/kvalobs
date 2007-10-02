#!/usr/bin/perl -w

use POSIX;
use strict;

#my $splitter="\\s+";
#my $splitter=";";

my $fromfile=$ARGV[0];
my $fromfile_doc=$ARGV[1];
my $line;

open(MYFILE,$fromfile_doc) or die "Can't open $fromfile: $!\n";
$line= <MYFILE>;
$line= <MYFILE>;
close(MYFILE);
my $version= $line;
print_header($version);

open(MYFILE,$fromfile) or die "Can't open $fromfile: $!\n";
while( defined($line=<MYFILE>) ){
   # $line =~ s/^\s*(.*)\s*$/$1/;#Her utf?res en trim
    chomp $line;
    $line .= "<br>\n";
    print $line;
}
close(MYFILE);         
print"<hr>\n";

open(MYFILE,$fromfile_doc) or die "Can't open $fromfile: $!\n";
while( defined($line=<MYFILE>) ){
   # $line =~ s/^\s*(.*)\s*$/$1/;#Her utf?res en trim
    chomp $line;
    $line .= "<br>\n";
    print $line;
}
close(MYFILE);
print_tail();


sub print_header{
    my $versjon = shift;
    print"<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0 Transitional//EN\"\n";
    print"               \"http://www.w3.org/TR/REC-html40/loose.dtd\"> \n";
    print"<html>\n";
    print"\n";
    print"    <!--#include file=\"header.shtml\" -->\n";
    print"\n";
    print"<h2>Kvalobsdatabasens tabeller</h2>\n";
    print"<h3>$versjon</h3>\n";
    print"<hr>\n";
}


sub print_tail{
print "<br>\n";
print "<br>\n";
print "samarbeid program database:<br>\n";
print "<img src=\"images/UML_samarbeid_program_database.gif\"></td> \n";
print "   <!--#include file=\"footer.shtml\" --> \n";
print "\n";
print "\n";
print "<!-- Created: Fri Aug 30 14:03:08 GMT 2002 -->\n";
print "<!-- hhmts start -->\n";
my ($sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst) = gmtime(time);
$mon += 1;
$year += 1900;
#print "$sec,$min,$hour,$mday,$mon,$year,$wday,$yday,$isdst\n";
print "Last modified: $mday/$mon $hour:$min:$sec GMT $year\n";
print "    </table>\n";
print "<!-- hhmts end -->\n";
print "</html>\n";
}












