#!/usr/bin/perl -w

use strict;

my $program_path=$0;
$program_path =~ s/\/[\w.]*$//;
print "program_path=$program_path\n";
chdir $program_path;

while( <*DataList.*> ){
  if( -M > 14 ){ 
      print "$_\n";unlink;
  } 
}

