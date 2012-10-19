#!/usr/bin/perl -w
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

use strict;

my $program_path=$ARGV[0];
#print "program_path=$program_path\n";
chdir $program_path;


my $max="";
my $maxold="";
while( <kvalobs.*.gz> ){
   if( $max ne "" ){
	if( -M $_ < -M $max ){
            $maxold=$max;
	    $max= $_;
	}
    }else{
     $max= $_;
    }
}

if ( $maxold ) {
  my $maxsz=(stat("$max"))[7];
  my $maxoldsz=(stat("$maxold"))[7];

  if(  ( $maxoldsz * 1.3 <  $maxsz  ) or ( $maxoldsz * 0.7 >  $maxsz )  ){
      print "WARNING for stor endring i bacup av systemet";
  }

# print "$max :: $maxsz :: $maxold :: $maxoldsz\n";
}
