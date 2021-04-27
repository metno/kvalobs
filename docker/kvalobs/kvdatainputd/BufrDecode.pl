#!/usr/bin/perl -w

use strict;
use Getopt::Long;
use Geo::BUFR;

# --dbhost,dbuser,db,database not used here. But perhaps used by aexecd?
my $Usage = "Usage: $0 --dbhost host:port --dbuser read_only_dbuser --db database"
    . " --bufr bufrfile --kvdata kvdatafile --loglevel level --bufrtables path";

# Will be used if neither --bufrtables nor $ENV{BUFR_TABLES} is set
use constant DEFAULT_TABLE_PATH => '/usr/share/kvbufrd/bufrtables';

# Parse command line options
my %option = ();
GetOptions(
    \%option,
    'dbhost=s',
    'dbuser=s',
    'db=s',
    'database=s',
    'bufr=s',
    'kvdata=s',
    'loglevel=s',
    'bufrtables=s',
);

if (!$option{bufr} && !$option{kvdata}) {
    print $Usage, "\n";
    exit(1);
}

my $loglevel;
if (!$option{loglevel}) {
    $loglevel = 3; # info is default
} elsif (lc $option{loglevel} eq 'error') {
    $loglevel = 1;
} elsif (lc $option{loglevel} eq 'warn') {
    $loglevel = 2;
} elsif (lc $option{loglevel} eq 'info') {
    $loglevel = 3;
} elsif (lc $option{loglevel} eq 'debug') {
    $loglevel = 4;
} else {
    die "loglevel must be one of error/warning/info/debug, is '$loglevel'";
}

# Set BUFR table path
if ($option{bufrtables}) {
    # Command line option --bufrtables overrides all
    Geo::BUFR->set_tablepath($option{bufrtables});
} elsif ($ENV{BUFR_TABLES}) {
    # If no --bufrtables option, use the BUFR_TABLES environment variable
    Geo::BUFR->set_tablepath($ENV{BUFR_TABLES});
} else {
    # If all else fails, use the default BUFR table path for Kvalobs machines
    Geo::BUFR->set_tablepath(DEFAULT_TABLE_PATH);
}

# Initialize BUFR object
my $bufr = Geo::BUFR->new();

# Process BUFR file
$bufr->fopen($option{bufr});
decode($bufr);
$bufr->fclose();


## Decode all BUFR messages in bufr file, and write the (last) decoded
## message in kvalobs format to kvdata file
sub decode {
    my $bufr = shift;          # BUFR object

    my ($message_header, $current_message_number, $current_ahl);
    my ($bufr_edition, $minute, $obstime);
    my $opened; # Set to true (1) when kvdata file is opened first time
    my $OUT; # File handle for kvdata file
  READLOOP:
    while (not $bufr->eof()) {

        # Read next observation. If an error is encountered during
        # decoding, skip this observation while printing the error
        # message to STDERR, also displaying ahl of bulletin if found.
        my ($data, $descriptors);
        eval {
            ($data, $descriptors) = $bufr->next_observation();
        };
        if ($@) {
            print "$@\n" if $loglevel >= 2;
            # Try to extract message number and ahl of the bulletin
            # where the error occurred
            $current_message_number = $bufr->get_current_message_number();
            if (defined $current_message_number) {
                my $error_msg = "In message $current_message_number";
                $current_ahl = $bufr->get_current_ahl();
                $error_msg .= " contained in bulletin with ahl $current_ahl\n"
                    if $current_ahl;
                print $error_msg if $error_msg && $loglevel >= 2;
            }
            next READLOOP;
        }
        next READLOOP unless $data;
        my $current_subset_number = $bufr->get_current_subset_number();
        # If next_observation() did find a BUFR message, subset number
        # should have been set to at least 1 (even in a 0 subset message)
        last READLOOP if $current_subset_number == 0;
        if ($current_subset_number == 1) {
            $current_message_number = $bufr->get_current_message_number();
            $current_ahl = $bufr->get_current_ahl() || '';
            $message_header = sprintf "Message %d", $current_message_number;
            $message_header .= (defined $current_ahl)
                ? "  $current_ahl\n" : "\n";
            print $message_header if $loglevel >= 4;

            # Find observation termin
            $bufr_edition = $bufr->get_bufr_edition();
            if ($bufr_edition > 4) {
                print "BUFR edition is $bufr_edition in $message_header\n"
                    if $loglevel >= 2;
            }
            my $year = ($bufr_edition == 4) ? $bufr->get_year()
                : '20' . sprintf("%02d", $bufr->get_year_of_century);
            $minute = $bufr->get_minute;
            $obstime = sprintf("%4d%02d%02d%02d%02d", $year,
                                  $bufr->get_month, $bufr->get_day,
                                  $bufr->get_hour, $minute);
            print $obstime, "\n" if $obstime && $loglevel >= 4;
        }
        my $data_category = $bufr->get_data_category;

        # Section 1 values are extracted only for subset 1, so handle trouble here
        next READLOOP if $bufr_edition > 4;

        # Data category = 0: Surface data - land, 1: Surface data - Sea
        next READLOOP if $data_category > 1;

        # Only minute=00 is interesting for kvalobs. This way we
        # also get rid of all tidal observations from UK (ISRZ.. EGRR)
        next READLOOP if $minute != 0;

        my $msg = extract($data, $descriptors, $obstime);
        if ($msg) {
            print $msg if $loglevel >= 3;
            # Write the kvalobs formatted message to kvdata file
            if (!$opened) {
                open($OUT, '>', $option{kvdata})
                    or die "Couldn't open $option{kvdata} for writing: $!";
                $opened = 1;
            }
            print $OUT $msg
                or die "Couldn't print message to $option{kvdata}: $!";
        }
    }
}

## Extract data values from section 4 and return them in kvalobs format
## - Should we extract day etc for comparing with obstime extracted from section 1?
## - Should we extract lat, lon to compare with position in station table?
## - Should we extract sensor height and possibly exclude obs with 'unstandard' heights?
sub extract {
    my ($data, $desc, $obstime) = @_;

    my $cloud_type_count;  # Will be increased by one for each 020012
                           # (cloud type) encountered (0 initially)
    my $num_cloud_layers;  # Number of individual cloud layers, set to
                           # value of 031001 (delayed descriptor) if
                           # this is met immediately after a 020012
                           # descriptor (-1 initially)
    my $bad_cloud_data;    # Set to true if something serious wrong is
                           # found in cloud data. No more cloud data
                           # will then be attempted decoded.
    my $cloud_layer;       # Numbers the individual cloud layers
    my $surface_data;      # Set to false if 007062 ('Depth below
                           # sea/water surface') is encountered with a
                           # value different from 0
    $num_cloud_layers = -1;
    $cloud_type_count = $bad_cloud_data = 0;
    $surface_data = 1;

    my ($hour_p, $minute_p, $hour);
    my ($II, $iii);
    my %par;
    my $idx = -1;
  ID:
    foreach my $id (@{$desc}) {
        $idx++;
        if (!defined $data->[$idx]) {
        # Missing value. Only a few descriptors require a reset of
        # corresponding parameter in this case
            if ($id eq '004024') { # Time period or displacement [hour]
                $hour_p = undef;
            } elsif  ($id eq '004025') { # Time period or displacement [minute]
                $minute_p = undef;
            # Delayed descriptor replication factor should never be missing
            } elsif  ($id eq '031001') {
                if ($idx > 1 && $desc->[$idx-1] eq '020012') {
                    print 'WARNING: delayed descriptor replication'
                        . ' factor after 020012 undefined!!!' . "\n" if $loglevel >=2;
                    $bad_cloud_data = 1;
                } else {
                    print 'WARNING: delayed descriptor replication'
                        . ' factor 31001 undefined!!!' . "\n" if $loglevel >=2;
                }
            # Some counting needed for clouds even for missing values
            } elsif  ($id eq '020012' && !$bad_cloud_data) { # Cloud type
                $cloud_type_count++;
                if ($cloud_type_count > 3) {
                    $cloud_layer = $cloud_type_count - 3;
                }
            }
            next ID;
        }

        # Continue the loop for non-missing value. For most variables
        # we choose to not set the parameter if set before, because if
        # a descriptor unexpectedly occurs more than once it is likely
        # that the first occurrence is the 'standard' use of the
        # descriptor, while the later occurrence(s) might for example
        # be due to data required by regional or national reporting
        # practices, added after a standard WMO template
        my $value = $data->[$idx];
        if ($id eq '004024') { # Time period or displacement [hour]
            $hour_p = $value;
        } elsif  ($id eq '004025') { # Time period or displacement [minute]
            $minute_p = $value;
        } elsif ($id eq '001001') { # WMO block number
            $II = $value;
        } elsif ($id eq '001002') { # WMO station number
            $iii = $value;
        } elsif ($id eq '004004') { # Hour
            if (!defined $hour ) {
                $hour = $value;
            }
        } elsif ($id eq '010004') { # Pressure
            if (!defined $par{PO}) {
                $par{PO} = $value/100; # hPa
            }
        } elsif ($id eq '010051') { # Pressure reduced to mean sea level
            if (!defined $par{PR}) {
                $par{PR} = $value/100; # hPa
            }
        } elsif ($id eq '010061') { # 3-hour pressure change
            if (!defined $par{PP}) {
                $par{PP} = abs($value/100); # hPa, absolute value
            }
        } elsif ($id eq '010063') { # Characteristic of pressure tendency
            if (!defined $par{AA}) {
                $par{AA} = $value;
            }
        # For 011001 WIND DIRECTION and 011002 WIND SPEED we ought to check
        # that these are preceded by 007032 HEIGHT OF SENSOR ABOVE LOCAL
        # GROUND set to approximately 10, 008021 TIME SIGNIFICANCE set to 2
        # (Time average) and 004025 TIME PERIOD OR DISPLACEMENT (minutes) set
        # to a negative value 10 or less. There is, however, only one 011001
        # and one 011002 in all WMO templates I have seen. And if more winds
        # are included, I expect that the one we want (10 meter, time
        # average over last 10 minutes) will be the first one to occur. And
        # the present implementation will include the first occurrence of
        # 011001 and 011002 with non missing value only.
        } elsif ($id eq '011012') { # Wind speed at 10 m
            if (!defined $par{FF}) {
                $par{FF} = $value;
            }
        } elsif ($id eq '011002') { # Wind speed
            if (!defined $par{FF}) {
                $par{FF} = $value;
            }
        } elsif ($id eq '011011') { # Wind direction at 10 m
            if (!defined $par{DD}) {
                $par{DD} = $value;
            }
        } elsif ($id eq '011001') { #  Wind direction
            if (!defined $par{DD}) {
                $par{DD} = $value;
            }
        } elsif ($id eq '011041') { # Maximum gust speed
            if (defined $minute_p && defined $hour) {
                if ($minute_p == -10) {
                    if (!defined $par{FG_010}) {
                        $par{FG_010} = $value;
                    }
                } elsif ($minute_p >= -70 && $minute_p <= -50) {
                    if (!defined $par{FG_1}) {
                        $par{FG_1} = $value;
                    }
                # For time periods > 1 hour, we choose to decode this as FG for
                # termins 0,6,12,18 if time period is 6 hours, and for termins
                # 3,9,15,21 if time period is 3 hours. FG is defined as max wind
                # gust since last synoptic termin (0,6,12,18) and it is unlikely to
                # be reported for other termins than 0,3,6,9...
                } elsif ($minute_p == -360 && $hour%6 == 0) {
                    if (!defined $par{FG}) {
                        $par{FG} = $value;
                    } elsif ($minute_p == -180 && $hour%3 == 0) {
                        if (!defined $par{FG}) {
                            $par{FG} = $value;
                        }
                    }
                # Skip other periods (don't decode FG_X)
                }
            }
        } elsif ($id eq '011043') { # Maximum gust direction
            # DG is treated the same way as FG
            if (defined $minute_p && defined $hour) {
                if ($minute_p == -10) {
                    if (!defined $par{DG_010}) {
                        $par{DG_010} = $value;
                    }
                } elsif ($minute_p >= -70 && $minute_p <= -50) {
                    if (!defined $par{DG_1}) {
                        $par{DG_1} = $value;
                    }
                } elsif ($minute_p == -360 && $hour%6 == 0) {
                    if (!defined $par{DG}) {
                        $par{DG} = $value;
                    } elsif ($minute_p == -180 && $hour%3 == 0) {
                        if (!defined $par{DG}) {
                            $par{DG} = $value;
                        }
                    }
                # Skip other periods (don't decode DG_X)
                }
            }
        } elsif ($id eq '011042') { # Maximum wind speed
            if (defined $minute_p && defined $hour) {
                if ($minute_p == -10) {
                    # FX is "Vindhastighet, maks. 10 minutt glidende middel siden
                    # forrige hovedobservasjon, m/s". Assume this is reported for
                    # termins 0,3,6,9... only
                    if (($minute_p == -360 && $hour%6 == 0)
                            || ($minute_p == -180 && $hour%3 == 0 && $hour%6 != 0)) {
                        if (!defined $par{FX_1}) {
                            $par{FX_1} = $value;
                        }
                    }
                } elsif ($minute_p >= -70 && $minute_p <= -50) {
                    if (!defined $par{FG_1}) {
                        $par{FG_1} = $value;
                    }
                }
                # Skip other periods (don't decode FG_X)
            }
         } elsif ($id eq '012104') { #  Dry bulb temperature at 2m (data width 16 bits)
            if (!defined $par{TA}) {
                $par{TA} = sprintf("%.1f",$value - 273.15); # Celcius
            }
        } elsif ($id eq '012004') { # Dry bulb temperature at 2m (12 bits)
            if (!defined $par{TA}) {
                $par{TA} = sprintf("%.1f",$value - 273.15); # Celcius
            }
        } elsif ($id eq '012101') { # Temperature/dry bulb temperature (16 bits)
            if (!defined $par{TA}) {
                $par{TA} = sprintf("%.1f",$value - 273.15); # Celcius
            }
        } elsif ($id eq '012001') { # Temperature/dry bulb temperature (12 bits)
            if (!defined $par{TA}) {
                $par{TA} = sprintf("%.1f",$value - 273.15); # Celcius
            }
        } elsif ($id eq '012006') { # Dew-point temperature at 2m (16 bits)
            if (!defined $par{TD}) {
                $par{TD} = sprintf("%.1f",$value - 273.15); # Celcius
            }
        } elsif ($id eq '012103') { # Dew-point temperature (16 bits)
            if (!defined $par{TD}) {
                $par{TD} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012113') { # Ground minimum temperature at 2m (data width 16 bits)
            if (!defined $par{TGN_12}) {
                $par{TGN_12} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012013') { # Ground minimum temperature at 2m (12 bits)
            if (!defined $par{TGN_12}) {
                $par{TGN_12} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012114') { # Maximum temperature at 2m, past 12 hours (16 bits)
            if (!defined $par{TAX_12}) {
                $par{TAX_12} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012014') { # Maximum temperature at 2m, past 12 hours (12 bits)
            if (!defined $par{TAX_12}) {
                $par{TAX_12} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012111') { # Maximum temperature at height and over period specified
             if ($idx >= 2 && $desc->[$idx-1] eq '004024'
                     && defined $data->[$idx-1] && $data->[$idx-1] == 0
                     && $desc->[$idx-2] eq '004024'&& defined $data->[$idx-2] ) {
                 if ($data->[$idx-2] == -12 && !defined $par{TAX_12}) {
                     $par{TAX_12} = sprintf("%.1f",$value - 273.15); # Celcius
                 } elsif ($data->[$idx-2] == -1 && !defined $par{TAX}) {
                     $par{TAX} = sprintf("%.1f",$value - 273.15); # Celcius
                 }
             }
            # Do we also need to consider 12021 'Maximum temperature at 2m'?
         } elsif ($id eq '012115') { # Minimum temperature at 2m, past 12 hours (16 bits)
            if (!defined $par{TAN_12}) {
                $par{TAN_12} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012015') { # Minimum temperature at 2m, past 12 hours (12 bits)
            if (!defined $par{TAN_12}) {
                $par{TAN_12} = sprintf("%.1f",$value - 273.15); # Celcius
            }
         } elsif ($id eq '012112') { # Minimum temperature at height and over period specified
             if ($idx >= 2 && $desc->[$idx-1] eq '004024'
                     && defined $data->[$idx-1] && $data->[$idx-1] == 0
                     && $desc->[$idx-2] eq '004024'&& defined $data->[$idx-2] ) {
                 if ($data->[$idx-2] == -12 && !defined $par{TAN_12}) {
                     $par{TAN_12} = sprintf("%.1f",$value - 273.15); # Celcius
                 } elsif ($data->[$idx-2] == -1 && !defined $par{TAN}) {
                     $par{TAN} = sprintf("%.1f",$value - 273.15); # Celcius
                 }
             }
             # Do we also need to consider 12022 'Minimum temperature at 2m'?
         } elsif ($id eq '013003') { # Relative humidity
            if (!defined $par{UU}) {
                $par{UU} = $value;
            }
        } elsif ($id eq '020001') { # Horizontal visibility
            if (!defined $par{VV}) {
                $par{VV} = $value;
            }
        } elsif ($id eq '020003') { # Present weather
            if (!defined $par{WW}) {
                my $WW = $value;
                if ($WW < 100) {
                    $par{WW} = $WW;
                } elsif ($WW < 200) { # 508-511 and w1w1 (in 333 9 group) ignored here
                    $par{WAWA} = $WW - 100;
                }
            }
        } elsif ($id eq '020004') { # Past weather (1)
            if (!defined $par{W1}) {
                my $W1 = $value;
                if ($W1 < 10) {
                    $par{W1} = $W1;
                } else {
                    $par{WA1} = $W1 - 10;
                }
            }
        } elsif ($id eq '020005') { # Past weather (2)
            if (!defined $par{W2}) {
                my $W2 = $value;
                if ($W2 < 10) {
                    $par{W2} = $W2;
                } else {
                    $par{WA2} = $W2 - 10;
                }
            }
        } elsif ($id eq '020010') { # Cloud cover (total)
            if (!defined $par{NN}) {
                $par{NN} = NNtoWMO_N($value);
            }
        } elsif ($id eq '031001') {
            # Delayed descriptor replication factor; this should be
            # number of cloud layers if previous descriptor is cloud
            # type, according to all WMO recommended templates
            if ($desc->[$idx-1] eq '020012') {
                if ($value <= 4) {
                    $num_cloud_layers = $value;
                } else {
                    $bad_cloud_data = 1;
                }
            }
        # Note that we don't decode 008002 Vertical significance
        # (surface observations), since we have no parameter
        # corresponding to 008002 in Kvalobs
        } elsif ($id eq '020011' && !$bad_cloud_data) { # Cloud amount
            if ($cloud_type_count == 0) { #  First occurrence
                if (!defined $par{NH}) {
                    $par{NH} = $value;
                }
            } elsif ($cloud_type_count < 3) {
                $bad_cloud_data = 1; # There should always be three
                                     # 020012 following first 008002
            } elsif ($num_cloud_layers > -1) {
                $cloud_layer = $cloud_type_count - 2;
                if ($cloud_layer <= $num_cloud_layers) {
                    if ($cloud_layer == 1) {
                        $par{NS1} = $value;
                     } elsif ($cloud_layer == 2) {
                        $par{NS2} = $value;
                     } elsif ($cloud_layer == 3) {
                        $par{NS3} = $value;
                    } elsif ($cloud_layer == 4) {
                        $par{NS4} = $value;
                    }
                }
            } else { # rdb-files always have 0 or 4 cloud layers
                     # (without a delayed descriptor replication factor)
                $cloud_layer = $cloud_type_count - 2;
                if ($cloud_layer < 5) {
                    if ($cloud_layer == 1) {
                        $par{NS1} = $value;
                     } elsif ($cloud_layer == 2) {
                        $par{NS2} = $value;
                     } elsif ($cloud_layer == 3) {
                        $par{NS3} = $value;
                    } elsif ($cloud_layer == 4) {
                        $par{NS4} = $value;
                    }
                }
            }
        } elsif ($id eq '020012' && !$bad_cloud_data) { # Cloud type
            $cloud_type_count++;
            if ($cloud_type_count > 3) {
                $cloud_layer = $cloud_type_count - 3;
                if ($num_cloud_layers > -1) {
                    if ($value < 10 # Accept one digit values only
                            && $cloud_layer <= $num_cloud_layers) {
                        if ($cloud_layer == 1) {
                            $par{CC1} = $value;
                        } elsif ($cloud_layer == 2) {
                            $par{CC2} = $value;
                        } elsif ($cloud_layer == 3) {
                            $par{CC3} = $value;
                        } elsif ($cloud_layer == 4) {
                            $par{CC4} = $value;
                        }
                    }
                } elsif ($cloud_layer < 5) { # rdb-files always have 0 or 4 cloud layers
                    if ($cloud_layer == 1) { # here 2 digit values should never occur,
                                             # so don't bother checking for this
                        $par{CC1} = $value;
                    } elsif ($cloud_layer == 2) {
                        $par{CC2} = $value;
                    } elsif ($cloud_layer == 3) {
                        $par{CC3} = $value;
                    } elsif ($cloud_layer == 4) {
                        $par{CC4} = $value;
                    }
                }
            } else {
                # Convert 020012 Cloud type in BUFR into one digit CL (0513), CM
                # (0515) and CH (0509) in TAC
                if ($cloud_type_count == 1 && !defined $par{CL}) {
                    my $CL = $value;
                    if ($CL >= 30 && $CL < 40 ) {
                        $par{CL} = $CL - 30;
                    } elsif ($CL != 62) {
                        print "CL in BUFR is $CL, not accepted, should be between 30 and 40\n"
                            if $loglevel >= 2;
                    }
                } elsif ($cloud_type_count == 2 && !defined $par{CM}) {
                    my $CM = $value;
                    if ($CM >= 20 && $CM < 30 ) {
                        $par{CM} = $CM - 20;
                    } elsif ($CM != 61) {
                        print "CM in BUFR is $CM, not accepted, should be between 20 and 30\n"
                            if $loglevel >= 2;
                    }
                } elsif ($cloud_type_count == 3 && !defined $par{CH}) {
                    my $CH = $value;
                    if ($CH >= 10 && $CH < 20 ) {
                        $par{CH} = $CH - 10;
                    } elsif ($CH != 60) {
                        print "CH in BUFR is $CH, not accepted, should be between 10 and 20\n"
                            if $loglevel >= 2;
                    }
                }
            }
        } elsif ($id eq '020013' && !$bad_cloud_data) { # Height of base of cloud
            if ($cloud_type_count == 0) { # First occurrence
                if (!defined $par{HL}) {
                    $par{HL} = $value;
                }
#     Note that 020013 in individual cloud layers comes
#     AFTER 020012 and therefore must be treated
#     differently than 008002 and 020011
            } elsif ($cloud_type_count < 4) {
               $bad_cloud_data = 1; # There should always be three 020012
                                    # following first 008002
           } elsif ($num_cloud_layers > -1) {
               $cloud_layer = $cloud_type_count - 3;
               if ($cloud_layer <= $num_cloud_layers) {
                    if ($cloud_layer == 1) {
                        $par{HS1} = $value;
                    } elsif ($cloud_layer == 2) {
                        $par{HS2} = $value;
                    } elsif ($cloud_layer == 3) {
                        $par{HS3} = $value;
                    } elsif ($cloud_layer == 4) {
                        $par{HS4} = $value;
                    }
                }
          } else { # rdb-files always have 0 or 4 cloud layers
               $cloud_layer = $cloud_type_count - 3;
               if ($cloud_layer == 1) {
                   $par{HS1} = $value;
               } elsif ($cloud_layer == 2) {
                   $par{HS2} = $value;
               } elsif ($cloud_layer == 3) {
                   $par{HS3} = $value;
               } elsif ($cloud_layer == 4) {
                   $par{HS4} = $value;
               }
           }
        # For all RR_x: treat trace and 0 at end of loop
        } elsif ($id eq '013023') { # Total precipitation past 24 hours
            if (!defined $par{RR_24}) {
                $par{RR_24} = $value;
            }
        } elsif ($id eq '013022') { # Total precipitation past 12 hours
            if (!defined $par{RR_12}) {
                $par{RR_12} = $value;
            }
        } elsif ($id eq '013021') { # Total precipitation past 6 hours
            if (!defined $par{RR_6}) {
                $par{RR_6} = $value;
            }
        } elsif ($id eq '013020') { # Total precipitation past 3 hours
            if (!defined $par{RR_3}) {
                $par{RR_3} = $value;
            }
        } elsif ($id eq '013019') { # Total precipitation past 1 hours
            if (!defined $par{RR_1}) {
                $par{RR_1} = $value;
            }
        } elsif ($id eq '013011') { # Total precipitation/total water equivalent
            if (defined $hour_p) {
                if ($hour_p == -24 && !defined $par{RR_24}) {
                    $par{RR_24} = $value;
                } elsif ($hour_p == -12 && !defined $par{RR_12}) {
                    $par{RR_12} = $value;
                } elsif ($hour_p == -6 && !defined $par{RR_6}) {
                    $par{RR_6} = $value;
                } elsif ($hour_p == -3 && !defined $par{RR_3}) {
                    $par{RR_3} = $value;
                } elsif ($hour_p == -1 && !defined $par{RR_1}) {
                    $par{RR_1} = $value;
                }
            }
        } elsif ($id eq '013013') { # Total snow depth
            # Don't check for SA missing, because SA might earlier
            # have been set to 0 if EE < 10, which probably means that
            # 20062 has been wrongly encoded, not 13013
            my $SA = $value * 100; # cm
            # SA has some special values in BUFR as well as in Kvalobs
            if ($SA == -1) { # Trace: less than 0.5 cm snow
                $par{SA} = 0;
            } elsif  ($SA == -2) { # Snow cover not continuos
                $par{SA} = -1;
            } elsif  ($SA == 0) { # Zero snow coded as -1 in Kvalobs. Stupid but true
                $par{SA} = -1;
            } else {
                $par{SA} = $SA; # cm
            }
        } elsif ($id eq '020062') { #  State of the ground (with or without snow)
            if (!defined $par{EE}) {
                $par{EE} = $value;
            }
            if ($value <= 10 && !defined $par{SA}) {
                $par{SA} = -1; # Special value in kvalobs
            }
        # Equating 013012 with SS_24 is dubious generally, but this is
        # how SS_24 is encoded in kvalobs for Norwegian avalanche
        # stations (and we have no better kvalobs parameter for depth
        # of fresh snow anyway)
        } elsif ($id eq '013012') { # Depth of fresh snow
            if (!defined $par{SS_24}) {
                $par{SS_24} = $value * 100; # cm
            }
        } elsif ($id eq '014031') { # Total sunshine
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{OT_1}) {
                    $par{OT_1} = $value;
                } elsif ($hour_p == -24 && !defined $par{OT_24}) {
                    $par{OT_24} = $value;
                }
            }
        } elsif ($id eq '013033') { # Evaporation/evapotranspiration
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{EV_1}) {
                    $par{EV_1} = $value;
                } elsif ($hour_p == -24 && !defined $par{EV_24}) {
                    $par{EV_24} = $value;
                }
            }
        } elsif ($id eq '014002') { # Long-wave radiation
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{QL}) {
                    if ($value >= 0) { # downward
                        $par{QLI} = sprintf("%.2f",$value/3600); # W/m2
                    } else {           # upward
                        $par{QLO} = sprintf("%.2f",$value/3600);
                    }
                } elsif ($hour_p == -24 && !defined $par{QL_24}) {
                    $par{QL_24} = sprintf("%.2f",$value/(24*3600));
                }
            }
        } elsif ($id eq '014004') { # Short-wave radiation
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{QK}) {
                    $par{QK} = sprintf("%.2f",$value/3600);
                } elsif ($hour_p == -24 && !defined $par{QK_24}) {
                    $par{QK_24} = sprintf("%.2f",$value/(24*3600));
                }
            }
        } elsif ($id eq '014016') { # Net radiation
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{QNET}) {
                    $par{QNET} = sprintf("%.2f",$value/3600);
                } elsif ($hour_p == -24 && !defined $par{QNET_24}) {
                    $par{QNET_24} = sprintf("%.2f",$value/(24*3600));
                }
            }
        } elsif ($id eq '014028') { # Global solar radiation
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{QSI}) {
                    $par{QSI} = sprintf("%.2f",$value/3600);
                } elsif ($hour_p == -24 && !defined $par{QSI_24}) {
                    $par{QSI_24} = sprintf("%.2f",$value/(24*3600));
                }
            }
        } elsif ($id eq '014029') { # Diffuse solar radiation
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{QD}) {
                    $par{QD} = sprintf("%.2f",$value/3600);
                } elsif ($hour_p == -24 && !defined $par{QD_24}) {
                    $par{QD_24} = sprintf("%.2f",$value/(24*3600));
                }
            }
        } elsif ($id eq '014030') { # Direct solar radiation
            if (defined $hour_p) {
                if ($hour_p == -1 && !defined $par{QS}) {
                    $par{QS} = sprintf("%.2f",$value/3600);
                } elsif ($hour_p == -24 && !defined $par{QS_24}) {
                    $par{QS_24} = sprintf("%.2f",$value/(24*3600));
                }
            }
        # The following parameters might be possible for platforms,
        # but don't really expect to see them for foreign stations
        # with wmonr
        } elsif ($id eq '022061') { # State of the sea
            if (!defined $par{SG}) {
                $par{SG} = $value;
            }
        } elsif ($id eq '022011') { # Period of waves (instrumentally measured)
            if (!defined $par{PWA}) {
                $par{PWA} = $value;
            }
        } elsif ($id eq '022012') { # Period of waves (visually measured)
            if (!defined $par{PW}) {
                $par{PW} = $value;
            }
        } elsif ($id eq '022021') { # Heigth of waves
            if (!defined $par{HWA}) {
                $par{HWA} = $value;
            }
        } elsif ($id eq '022022') { # Heigth of wind waves
            if (!defined $par{HW}) {
                $par{HW} = $value;
            }
        }
    }
    # Precipitation = -0.1 means "trace" (less than 0.05 kg/m2), is
    # represented as 0 in Kvalobs, while precipitation = 0 is
    # represented as -1
    foreach my $RR (qw(RR_24 RR_12 RR_6 RR_3 RR_1)) {
        if (defined $par{$RR} && $par{$RR} == 0) {
            $par{$RR} = -1;
        } elsif (defined $par{$RR} && $par{$RR} == -0.1) {
            $par{$RR} = 0;
        }
    }

    return unless $II && $iii; # Skip stations without WMO number

    # To be changed - decode Austrian, British, Dutch, Finish, German,
    # Hungarian, Irish, Spanish and Swedish stations only. For new countries
    # to be added: check
    # https://www.wmo.int/pages/prog/www/ois/volume-a/StationIDs_Global_1509.pdf
    return unless $II == 2
        || $II == 3
        || ($II == 6 && $iii >=200 && $iii < 400)
        || ($II == 8 && $iii <= 494)
        || $II == 10
	|| ($II == 11 && $iii < 400)
	|| ($II == 12 && $iii >=700 && $iii <= 998);

    my $wmonr = sprintf("%02d%03d", $II, $iii);
    my $header = "kldata/wmonr=$wmonr/type=7";
    my @params = sort keys %par;
    my $param_str = join(',', @params);
    return unless $param_str; # Skip stations without observations
    my @values;
    foreach my $param (@params) {
        push @values, $par{$param};
    }
    my $data_str = $obstime . ',' . join(',', @values);
    return "$header\n$param_str\n$data_str\n";
}

## Convert value of NN (in percent) into WMO code (table 2700)
sub NNtoWMO_N {
    my $NN = shift;

    if ($NN == 0) {
        return 0;
    } elsif ($NN <= 15) {   # 1/10 or less
        return 1;
    } elsif ($NN <= 30) {   # 2/10 - 3/10
        return 2;
    } elsif ($NN <= 45) {   # 4/10
        return 3;
    } elsif ($NN <= 55) {   # 5/10
        return 4;
    } elsif ($NN <= 65) {   # 6/10
        return 5;
    } elsif ($NN <= 80) {   # 7/10 - 8/10
        return 6;
    } elsif ($NN <= 99) {   # 9/10 or more, but not 10/10
        return 7;
    } elsif ($NN > 100) { # Sky obscured by fog or other meteorological phenomena
        return 9;  # NN will be 113 in WMO BUFR, 109 in surf files
    } else {
        return 8;
    }
}
